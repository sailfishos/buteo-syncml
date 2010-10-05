/*
* This file is part of buteo-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, 
* this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may 
* be used to endorse or promote products derived from this software without 
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/
#include "ServerSessionHandler.h"

#include "SyncAgentConfig.h"
#include "StoragePlugin.h"
#include "SyncTarget.h"
#include "AlertPackage.h"
#include "FinalPackage.h"
#include "DevInfPackage.h"
#include "AuthHelper.h"
#include "ServerAlertedNotification.h"
#include "StorageProvider.h"

#include "LogMacros.h"


using namespace DataSync;

ServerSessionHandler::ServerSessionHandler( const SyncAgentConfig* aConfig, QObject* aParent )
 : SessionHandler( aConfig, ROLE_SERVER, aParent ),
   iConfig(aConfig)

{
    FUNCTION_CALL_TRACE;

}

ServerSessionHandler::~ServerSessionHandler()
{
    FUNCTION_CALL_TRACE;
}

void ServerSessionHandler::initiateSync()
{
    FUNCTION_CALL_TRACE;

    if( !prepareSync() )
    {
        return;
    }

    QString sessionId = generateSessionID();
    setupSession( sessionId );
    ProtocolVersion protocolVersion = getProtocolVersion();

    LOG_DEBUG("Using protocol version " << protocolVersion);

    QList<QString> sourceDbs = iConfig->getSourceDbs();
    QList< QPair<QString, QString> > storages;

    for( int i = 0; i < sourceDbs.count(); ++i )
    {
        const QString& sourceURI = sourceDbs[i];
        StorageContentFormatInfo info;
        if( iConfig->getStorageProvider()->getStorageContentFormatInfo( sourceURI,
                                                                        info ) )
        {
            QPair<QString, QString> storage;
            storage.first = sourceURI;
            storage.second = info.getPreferredTx().iType;
            storages.append( storage );
        }

    }

    if( storages.count() != sourceDbs.count() )
    {
        LOG_CRITICAL( "Could not find all targets, aborting sync" );
        abortSync( DATABASE_FAILURE, "Could not find all sync targets" );
        return;
    }

    if( !getTransport().init() )
    {
        LOG_CRITICAL( "Could not initialize transport" );
        abortSync( CONNECTION_ERROR, "Could not initiate transport" );
        return;
    }

    if( protocolVersion == SYNCML_1_1 ) {
        serverInitiatedSyncDS11( storages );
    }
    else if ( protocolVersion == SYNCML_1_2 ){
        serverInitiatedSyncDS12( storages );
    }
    else {
        abortSync( INTERNAL_ERROR, "Unknown protocol version" );
    }
}

void ServerSessionHandler::serveRequest( QList<Fragment*>& aFragments )
{
    FUNCTION_CALL_TRACE;

    if( !prepareSync() )
    {
        return;
    }

    processMessage( aFragments, true );
}

void ServerSessionHandler::suspendSync()
{
    FUNCTION_CALL_TRACE;

    // Sync cannot be suspended or resumed from server. Therefore this function
    // should never be called. Abort with internal error in those situations
    abortSync( INTERNAL_ERROR, "Not supported in server mode" );
}

void ServerSessionHandler::resumeSync()
{
    FUNCTION_CALL_TRACE;

    // Sync cannot be suspended or resumed from server. Therefore this function
    // should never be called. Abort with internal error in those situations
    abortSync( INTERNAL_ERROR, "Not supported in server mode" );
}

void ServerSessionHandler::messageReceived( HeaderParams& aHeaderParams )
{

    FUNCTION_CALL_TRACE;

    SyncState state = getSyncState();

    if( state == PREPARED ) {
        // Sync session initialization
        setupSession( aHeaderParams );

    }

}

ResponseStatusCode ServerSessionHandler::syncAlertReceived( const SyncMode& aSyncMode,
                                                            CommandParams& aAlertParams )
{

    FUNCTION_CALL_TRACE;

    ResponseStatusCode status;

    SyncState syncState = getSyncState();

    if( syncState == PREPARED || syncState == REMOTE_INIT ) {

        // Client is sending initial alerts for sync
        status = setupTargetByClient( aSyncMode, aAlertParams );
        setSyncState( REMOTE_INIT );

    }
    else if( syncState == LOCAL_INIT ) {

        // Client is acknowledging alert, to revert to slow sync
        status = acknowledgeTarget( aSyncMode, aAlertParams );

    }
    else {

        // Don't allow sync related alerts outside init phase
        status = COMMAND_NOT_ALLOWED;

    }

    return status;

}

bool ServerSessionHandler::syncReceived()
{

    FUNCTION_CALL_TRACE;

    SyncState syncState = getSyncState();

    if( syncState == REMOTE_INIT ) {

        // If we receive a sync when in remote init, it means that
        // client is doing sync without init phase. Check if reverts occurred,
        // in that case we can't do sync without init phase. Then, all
        // modifications sent by client must be ignored.

        bool reverts = false;

        const QList<SyncTarget*>& targets = getSyncTargets();

        for( int i = 0; i < targets.count(); ++i ) {
            if( targets[i]->reverted() ) {
                reverts = true;
                break;
            }
        }

        if( reverts ) {
            return false;
        }
        else {
            setSyncWithoutInitPhase(true);
            setSyncState( RECEIVING_ITEMS );
            return true;
        }
    }
    else if( syncState == LOCAL_INIT )
    {
        setSyncState( RECEIVING_ITEMS );
        return true;
    }
    else if( syncState == RECEIVING_ITEMS ) {
        return true;
    }
    else {
        // We don't allow sync in other states
        return false;
    }

}

bool ServerSessionHandler::mapReceived()
{

    FUNCTION_CALL_TRACE;

    SyncState syncState = getSyncState();

    // If we sent the last set of items and received mappings from client, send the final
    if( syncState == SENDING_ITEMS && getResponseGenerator().packageQueueEmpty() ) {
        setSyncState( RECEIVING_MAPPINGS );
        return true;
    }
    // If we receive mappings before we close our sending items package, don't send the final
    else if( syncState == SENDING_ITEMS ) {
        return true;
    }
    else if( syncState == RECEIVING_MAPPINGS ) {
        return true;
    }
    else {
        // We don't allow map in other states
        return false;
    }

}

void ServerSessionHandler::finalReceived()
{
    FUNCTION_CALL_TRACE;

    SyncState syncState = getSyncState();

    switch( syncState )
    {
        case REMOTE_INIT:
        {
            composeServerInitializationPackage();
            setSyncState( LOCAL_INIT );
            break;
        }
        case RECEIVING_ITEMS:
        {

            const QList<SyncTarget*>& targets = getSyncTargets();

            // Search for targets that need their modifications sent
            bool sendServerModifications = false;

            foreach( const SyncTarget* syncTarget, targets) {

                if( syncTarget->getSyncMode()->syncDirection() != DIRECTION_FROM_CLIENT ) {
                    sendServerModifications = true;
                    break;
                }

            }

            if( sendServerModifications ){

                composeServerModificationsPackage();
                setSyncState( SENDING_ITEMS );

            }
            // Don't need to send server modifications
            else {
                composeMapAcknowledgementPackage();
                setSyncState( FINALIZING );
            }

            break;
        }
        case SENDING_ITEMS:
        {
            // If we have no more items to send
            if( getResponseGenerator().packageQueueEmpty() ) {
                composeMapAcknowledgementPackage();
                setSyncState( FINALIZING );
            }
            break;
        }
        case RECEIVING_MAPPINGS:
        {
            setSyncState( FINALIZING );
            composeMapAcknowledgementPackage();
            break;
        }
        default:
        {
            // Try to find out what error occurred
            QString errorMsg;
            SyncState state = getLastError(errorMsg);
            abortSync( state, errorMsg );
            break;
        }

    }

}

void ServerSessionHandler::messageParsed()
{
    FUNCTION_CALL_TRACE;

    SyncState syncState = getSyncState();

    switch( syncState )
    {
        case PREPARED:
        case REMOTE_INIT:
        case LOCAL_INIT:
        case RECEIVING_ITEMS:
        case SENDING_ITEMS:
        case RECEIVING_MAPPINGS:
        {
            sendNextMessage();
            getTransport().receive();

            break;
        }
        case FINALIZING:
        {

            sendNextMessage();

            // Check if we've finished sending everything we need to send to client
            if( getResponseGenerator().packageQueueEmpty() )
            {
                finishSync();
            }

            break;
        }
        default:
        {
            break;
        }
    }

}

void ServerSessionHandler::resendPackage()
{
    FUNCTION_CALL_TRACE;

    SyncState syncState = getSyncState();

    switch( syncState )
    {
        case LOCAL_INIT:
        {
            composeServerInitializationPackage();
            break;
        }
        case SENDING_ITEMS:
        {
            composeServerModificationsPackage();
            break;
        }
        case FINALIZING:
        {
            composeMapAcknowledgementPackage();
            break;
        }
        default:
        {
            abortSync( INTERNAL_ERROR,  "Internal state machine error" );
            break;
        }

    }
}

ResponseStatusCode ServerSessionHandler::setupTargetByClient( const SyncMode& aSyncMode,
                                                              CommandParams& aAlertParams )
{
    FUNCTION_CALL_TRACE;

    if( aAlertParams.items.isEmpty() )
    {
        LOG_WARNING( "Received alert without any items! Cmd Id:" << aAlertParams.cmdId );
        return INCOMPLETE_COMMAND;
    }

    // Require remote device source database (which is our target database), and
    // either remote device target database (which is our source database) or mime type
    // of our source database. Also Next anchor
    const ItemParams& item = aAlertParams.items.first();
    const MetaParams& meta = item.meta;
    const AnchorParams& anchors = meta.anchor;
    if( item.source.isEmpty() ||
        anchors.next.isEmpty() ||
        ( item.target.isEmpty() && meta.type.isEmpty() ) )
    {
        LOG_WARNING( "Received alert that did not pass validation! Cmd Id:" << aAlertParams.cmdId );
        return INCOMPLETE_COMMAND;
    }

    StoragePlugin* source = 0;

    if( !item.target.isEmpty() ) {
        source = createStorageByURI( item.target );
    }
    else if( !meta.type.isEmpty() ){
        source = createStorageByMIME( meta.type );
    }

    if( !source ) {
        return NOT_FOUND;
    }

    SyncTarget* target = createSyncTarget( *source, aSyncMode );

    if( !target ) {
        return COMMAND_FAILED;
    }

    ResponseStatusCode status = SUCCESS;

    target->setSyncMode( aSyncMode );
    target->setRemoteNextAnchor( anchors.next );
    target->setTargetDatabase( item.source );

    if( anchorMismatch( aSyncMode, *target, anchors.last ) )
    {

        LOG_DEBUG("Anchor mismatch, refresh required");
        // Anchor mismatch, must revert to slow sync
        status = REFRESH_REQUIRED;

        // @todo: we should implement this better, so that it'd check synccaps of remote device
        //        in all cases, not just for from-client syncs

        bool clientInitiedRefresh = false;

        int datastoreIndex = -1;
        const QList<Datastore>& datastores = getDevInfHandler().getRemoteDeviceInfo().datastores();

        LOG_DEBUG("Count for remote device stores:::"  << datastores.count());

        for( int i = 0; i < datastores.count(); ++i )
        {
            if( datastores[i].getSourceURI() == target->getTargetDatabase() )
            {
                datastoreIndex = i;
                break;
            }
        }

        if( datastoreIndex != -1 )
        {
            if( datastores[datastoreIndex].syncCaps().contains( DataSync::SYNCTYPE_FROMCLIENTSLOW ) &&
                (clientInitiedRefresh = target->setRefreshFromClient()) )
            {
                LOG_DEBUG("Anchor mismatch, client refresh required, sending 203 as client supports refresh");
            }

            if( !clientInitiedRefresh ){
                LOG_DEBUG("Anchor mismatch, refresh required, sending 202");
                target->revertSyncMode();
            }
        }
        else
        {
            LOG_DEBUG("Anchor mismatch but have no device info available, reverting");
            target->revertSyncMode();
        }

    }


    if( target->getSyncMode()->syncType() == TYPE_FAST ) {

        // We are in fast sync mode, load mappings for target
        target->loadUIDMappings();
    }
    else {

        // In slow mode, all mappings become invalid
        target->clearUIDMappings();
    }

    addSyncTarget( target );

    return status;

}


ResponseStatusCode ServerSessionHandler::acknowledgeTarget( const SyncMode& /*aSyncMode*/,
                                                            CommandParams& aAlertParams )
{
    FUNCTION_CALL_TRACE;

    if( aAlertParams.items.isEmpty() )
    {
        LOG_WARNING( "Received alert without any items! Cmd Id:" << aAlertParams.cmdId );
        return INCOMPLETE_COMMAND;
    }

    const ItemParams& item = aAlertParams.items.first();

    if( item.target.isEmpty() )
    {
        LOG_WARNING( "Received alert that did not pass validation! Cmd Id:" << aAlertParams.cmdId );
        return INCOMPLETE_COMMAND;
    }

    SyncTarget* target = getSyncTarget( item.target );

    if( !target ) {
        return NOT_FOUND;
    }

    target->setRemoteNextAnchor( item.meta.anchor.next );

    target->revertSyncMode();

    // In slow mode, all mappings become invalid
    target->clearUIDMappings();

    return SUCCESS;


}

void ServerSessionHandler::composeSyncML11ServerAlertedSyncPackage( const QList< QPair<QString, QString> >& aStorages )
{
    FUNCTION_CALL_TRACE;

    qint32 alertCode = iConfig->getSyncMode().toSyncMLCode();

    for( int i = 0; i < aStorages.count(); ++i )
    {
        AlertPackage* package = new AlertPackage(aStorages[i].first,
                                                 aStorages[i].second,
                                                 alertCode );

        getResponseGenerator().addPackage(package);
    }

    // Close the package by appending Final
    getResponseGenerator().addPackage( new FinalPackage() );

}

void ServerSessionHandler::composeAndSendSyncML12ServerAlertedSyncPackage( const QList< QPair<QString, QString> >& aStorages )
{
    FUNCTION_CALL_TRACE;

    SANHandler handler;
    SANData data;

    data.iVersion = SYNCML_1_2;
    data.iUIMode = SANUIMODE_BACKGROUND;
    data.iInitiator = SANINITIATOR_SERVER;
    data.iSessionId = 0;
    data.iServerIdentifier = getConfig()->getLocalDeviceName();

    int syncType = getConfig()->getSyncMode().toSyncMLCode();
    for( int i = 0; i < aStorages.count(); ++i )
    {
        SANSyncInfo syncInfo;
        syncInfo.iSyncType = syncType;
        syncInfo.iServerURI = aStorages[i].first;
        syncInfo.iContentType = aStorages[i].second;
        data.iSyncInfo.append( syncInfo );
    }

    QByteArray message;
    QString user = getConfig()->getUsername();
    QString password = getConfig()->getPassword();
    if( !handler.generateSANMessage( data, user, password, message ) )
    {
        LOG_CRITICAL( "Could not generate SyncML 1.2 SAN package" );
        abortSync( INTERNAL_ERROR, "Error while generating 1.2 SAN package" );
        return;
    }
    if( !getTransport().sendSAN( message ) )
    {
        LOG_CRITICAL( "Could not send SyncML 1.2 SAN package" );
        abortSync( INTERNAL_ERROR, "Error while sending 1.2 SAN package" );
    }

}

void ServerSessionHandler::composeServerInitializationPackage()
{
    FUNCTION_CALL_TRACE;

    composeServerInitialization();

    // Close the package by appending Final
    getResponseGenerator().addPackage( new FinalPackage() );

}

void ServerSessionHandler::composeServerModificationsPackage()
{
    FUNCTION_CALL_TRACE;

    // If doing sync without init phase, also send initialization
    if( isSyncWithoutInitPhase() ) {
        composeServerInitialization();
    }

    composeLocalChanges();

    // Close the package by appending Final
    getResponseGenerator().addPackage( new FinalPackage() );
}

void ServerSessionHandler::composeMapAcknowledgementPackage()
{
    FUNCTION_CALL_TRACE;

    // Close the package by appending Final
    getResponseGenerator().addPackage( new FinalPackage() );

}

void ServerSessionHandler::composeServerInitialization()
{
    FUNCTION_CALL_TRACE;

    // Sync init packages include alerts to inform client about the databases we wish to sync
    const QList<SyncTarget*>& targets = getSyncTargets();

    foreach( const SyncTarget* target, targets) {

        AlertPackage* package = new AlertPackage( target->getSyncMode()->toSyncMLCode(),
                                                  target->getSourceDatabase(),
                                                  target->getTargetDatabase(),
                                                  target->getLocalLastAnchor(),
                                                  target->getLocalNextAnchor() );

        getResponseGenerator().addPackage( package );

    }

}

void ServerSessionHandler::serverInitiatedSyncDS11( const QList< QPair<QString, QString> >& aStorages )
{
    FUNCTION_CALL_TRACE;

    // Send initialization package to client
    composeSyncML11ServerAlertedSyncPackage( aStorages );
    setSyncState( PREPARED );
    sendNextMessage();
    getTransport().receive();

}

void ServerSessionHandler::serverInitiatedSyncDS12( const QList< QPair<QString, QString> >& aStorages )
{
    FUNCTION_CALL_TRACE;

    // Send initialization package to client
    composeAndSendSyncML12ServerAlertedSyncPackage( aStorages );
    setSyncState( PREPARED );
    getTransport().receive();

}
