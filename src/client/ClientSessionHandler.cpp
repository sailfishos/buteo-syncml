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

#include "ClientSessionHandler.h"

#include <QVariant>

#include "SyncAgentConfig.h"
#include "SyncAgentConfigProperties.h"
#include "SyncTarget.h"
#include "FinalPackage.h"
#include "AlertPackage.h"
#include "LocalMappingsPackage.h"
#include "StoragePlugin.h"
#include "ServerAlertedNotification.h"

#include "SyncMLLogging.h"

using namespace DataSync;

ClientSessionHandler::ClientSessionHandler( const SyncAgentConfig* aConfig, QObject* aParent )
 : SessionHandler(aConfig, ROLE_CLIENT, aParent),
   iConfig(aConfig)
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}


ClientSessionHandler::~ClientSessionHandler()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

void ClientSessionHandler::initiateSync()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !prepareSync() )
    {
        return;
    }

    QString sessionId = generateSessionID();
    setupSession( sessionId );

    setupSyncTargets();

    if( getStorages().count() != iConfig->getSourceDbs().count() )
    {
        qCCritical(lcSyncML) << "Could not create all targets, aborting sync";
        abortSync( DATABASE_FAILURE, "Could not create all sync targets" );
        return;
    }

    if( !getTransport().init() )
    {
        qCCritical(lcSyncML) << "Could not initialize transport";
        abortSync( CONNECTION_ERROR, "Could not initiate transport" );
        return;
    }

    composeClientInitializationPackage();

    // If doing sync without init phase, move to sending items state
    if( isSyncWithoutInitPhase() ) {
        setSyncState( SENDING_ITEMS );
    }
    else {
        setSyncState( LOCAL_INIT );
    }

    sendNextMessage();
    getTransport().receive();

}

void ClientSessionHandler::handleNotificationXML( QList<Fragment*>& aFragments )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !prepareSync() )
    {
        return;
    }

    processMessage( aFragments, true );
}

void ClientSessionHandler::handleNotificationPackage( const SANDS& aData )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !prepareSync() )
    {
        return;
    }

    // Set up session

    if( aData.iHeader.iSessionId == 0 ) {
        params().setSessionId( generateSessionID() );
    }
    else {
        params().setSessionId( QString::number( aData.iHeader.iSessionId ) );
    }

    if( !getConfig()->getLocalDeviceName().isEmpty() ) {
        params().setLocalDeviceName( getConfig()->getLocalDeviceName() );
    }
    else {
        params().setLocalDeviceName( getConfig()->getDeviceInfo().getDeviceID() );
    }

    params().setRemoteDeviceName( aData.iHeader.iServerIdentifier );

    QString verDTD;
    QString verProto;

    if( aData.iHeader.iVersion == SYNCML_1_1 ) {
        qCDebug(lcSyncML) << "Setting SyncML 1.1 protocol version";
        setProtocolVersion( SYNCML_1_1 );
        verDTD = SYNCML_DTD_VERSION_1_1;
        verProto = DS_VERPROTO_1_1;
    }
    else if( aData.iHeader.iVersion == SYNCML_1_2 ) {
        qCDebug(lcSyncML) << "Setting SyncML 1.2 protocol version";
        setProtocolVersion( SYNCML_1_2 );
        verDTD = SYNCML_DTD_VERSION_1_2;
        verProto = DS_VERPROTO_1_2;
    }

    if( getConfig()->extensionEnabled( SYNCWITHOUTINITPHASEEXTENSION ) ) {
        setSyncWithoutInitPhase( true );
    }

    HeaderParams headerParams;
    headerParams.verDTD = verDTD;
    headerParams.verProto = verProto;
    headerParams.sessionID = params().sessionId();
    headerParams.sourceDevice = params().localDeviceName();
    headerParams.targetDevice = params().remoteDeviceName();
    headerParams.meta.maxMsgSize = params().localMaxMsgSize();

    if( getConfig()->extensionEnabled( EMITAGSEXTENSION ) )
    {
        insertEMITagsToken( headerParams );
    }

    setLocalHeaderParams( headerParams );

    // Set up targets

    for( int i = 0; i< aData.iSyncInfo.count(); ++i ) {

        SyncMode syncMode( aData.iSyncInfo[i].iSyncType );

        syncMode.toClientInitiated();

        QString serverURI = aData.iSyncInfo[i].iServerURI;

        if( !serverURI.contains( SYNCML_URI_PREFIX ) ) {
            serverURI.prepend( SYNCML_URI_PREFIX );
        }

        QString mime = aData.iSyncInfo[i].iContentType;

        if( mime.isEmpty() )
        {
            mime = convertSANURItoMIME( serverURI );
        }

        if( mime.isEmpty() )
        {
            qCCritical(lcSyncML) << "Could not find MIME for server URI:" << serverURI;
            continue;
        }

        qCDebug(lcSyncML) << "Searching for storage with MIME type" << mime;
        StoragePlugin* source = createStorageByMIME( mime );

        if( !source ) {
            qCCritical(lcSyncML) << "Could not found matching storage for MIME:" << mime;
            continue;
        }

        SyncTarget* target = createSyncTarget( *source, syncMode );

        if( !target ) {
            qCCritical(lcSyncML) << "Could not a target for MIME:" << mime;
            continue;
        }

        target->setTargetDatabase( serverURI );

        // Use slow sync if this is the preferred mode or if we don't have a last anchor
        if( iConfig->getSyncMode().syncType() == TYPE_SLOW || target->getRemoteLastAnchor().isEmpty() ) {
            qCDebug(lcSyncML) << "Did not find last remote anchor or slow sync is forced, using slow sync";
            SyncMode* mode = target->getSyncMode();
            mode->toSlowSync();
            target->setSyncMode( *mode );
        }

        addSyncTarget( target );

    }

    if( getStorages().count() != aData.iSyncInfo.count() ) {
        qCCritical(lcSyncML) << "Could not create all targets, aborting";
        abortSync( DATABASE_FAILURE, "Could not create all sync targets");
        return;
    }

    composeClientInitializationPackage();

    // If doing sync without init phase, move to sending items state
    if( isSyncWithoutInitPhase() ) {
        setSyncState( SENDING_ITEMS );
    }
    else {
        setSyncState( LOCAL_INIT );
    }

    sendNextMessage();
    getTransport().receive();

}

void ClientSessionHandler::suspendSync()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// @todo: existing implementation was considered flawed, reimplement to be working
	// as soon as possible
	Q_ASSERT( 0 );

}

void ClientSessionHandler::resumeSync()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// @todo: existing implementation was considered flawed, reimplement to be working
	// as soon as possible
	Q_ASSERT( 0 );
}

void ClientSessionHandler::messageReceived( HeaderParams& aHeaderParams )
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	SyncState state = getSyncState();

	if( state == PREPARED ) {

		// Server is initiating a sync
		setupSession( aHeaderParams );

	}

	else if( state == LOCAL_INIT ) {

		// Initialization message from server

		// When we sent initialization message we might not have necessarily
		// known the remote device id, so update it based on the received header
        params().setRemoteDeviceName( aHeaderParams.sourceDevice );

		// Update remote device id also to header params
		HeaderParams headerParams = getLocalHeaderParams();
		headerParams.targetDevice = aHeaderParams.sourceDevice;
		setLocalHeaderParams( headerParams );
	}

}

ResponseStatusCode ClientSessionHandler::syncAlertReceived( const SyncMode& aSyncMode,
                                                            CommandParams& aAlertParams )
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	ResponseStatusCode status;

	SyncState syncState = getSyncState();

	if( syncState == PREPARED ) {

		// Server is initiating a sync

		status = setupTargetByServer( aSyncMode, aAlertParams );

	}
	else if( syncState == LOCAL_INIT || syncState == REMOTE_INIT ) {

		// Server is acknowledging a sync mode
		status = acknowledgeTarget( aSyncMode, aAlertParams );
		setSyncState( REMOTE_INIT );

	}
	else if( syncState == SENDING_ITEMS ) {

		// Allow Alert only if we're doing sync without init phase
		if( isSyncWithoutInitPhase() ) {
			status = acknowledgeTarget( aSyncMode, aAlertParams );
			setSyncState( REMOTE_INIT );
		}
		else {
			status = COMMAND_NOT_ALLOWED;
		}

	}
	else {

		// Don't allow sync related alerts in other states
		status = COMMAND_NOT_ALLOWED;
	}

	return status;

}

bool ClientSessionHandler::syncReceived()
{

	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	SyncState syncState = getSyncState();

	if( syncState == SENDING_ITEMS || syncState == RECEIVING_ITEMS ) {
		setSyncState( RECEIVING_ITEMS );
		return true;
	}
	else if( syncState == REMOTE_INIT ) {

		// Allow only if doing sync without init phase
		if( isSyncWithoutInitPhase() ) {
			setSyncState( RECEIVING_ITEMS );
			return true;
		}
		else {
			return false;
		}
	}
	else {
		// We don't allow sync in other states
		return false;
	}

}

bool ClientSessionHandler::mapReceived()
{

	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// Client should never receive Map
	return false;
}

void ClientSessionHandler::finalReceived()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	SyncState syncState = getSyncState();

	switch( syncState )
	{
        case PREPARED:
        {
            composeClientInitializationPackage();
            setSyncState( LOCAL_INIT );
            break;
        }
        case REMOTE_INIT:
        {

            composeClientModificationsPackage();
            setSyncState( SENDING_ITEMS );

            break;
        }
        case RECEIVING_ITEMS:
        {

            composeDataUpdateStatusPackage();
            setSyncState( SENDING_MAPPINGS );

            break;
        }
        case SENDING_ITEMS:
        case SENDING_MAPPINGS:
        {
            setSyncState( FINALIZING );
            break;
        }
        default:
        {
            QString errorMsg;
            SyncState state = getLastError(errorMsg);
            abortSync( state, errorMsg );
            break;
        }
	}
}

void ClientSessionHandler::messageParsed()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// if we have 101 for SyncHdr Status in SyncBody
	// send a result Alert
	if( isRemoteBusyStatusSet()) {
		composeResultAlert();
		resetRemoteBusyStatus();
	}

	SyncState syncState = getSyncState();

	switch( syncState )
	{
        case PREPARED:
        case LOCAL_INIT:
        case REMOTE_INIT:
        case SENDING_ITEMS:
        case RECEIVING_ITEMS:
        {
            sendNextMessage();
            getTransport().receive();

            break;
        }
        case SENDING_MAPPINGS:
        {
            // Check if we need to send the next message
            if( shouldSendDataUpdateStatus() )
            {
                sendNextMessage();
                getTransport().receive();
            }
            else
            {
                qCDebug(lcSyncML) << "Omitting update status package!";
                finishSync();
            }
            break;
        }
        case FINALIZING:
        {
            finishSync();
            break;
        }
        default:
        {
            break;
        }
	}

}

bool ClientSessionHandler::shouldSendDataUpdateStatus()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
    bool shouldSend = true;

    if( 0 < getConfig()->getAgentProperty( OMITDATAUPDATESTATUSPROP ).toInt() )
    {
        // Check if we have anything other than status for the SyncHdr, and the
        // Final package to send to the server

        const QList<StatusParams*>& responseStatuses = getResponseGenerator().getStatuses();

        if( ( 1 == responseStatuses.count() ) &&
                ( SYNCML_ELEMENT_SYNCHDR == responseStatuses.at(0)->cmd ) &&
                ( getResponseGenerator().getPackages().count() <= 1 )
          )
        {
            qCDebug(lcSyncML) << "There is only one status fragment in response and it's for the sync header,\
                    so we can omit the update status package";
            shouldSend = false;
        }
    }
    return shouldSend;
}

void ClientSessionHandler::resendPackage()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	SyncState syncState = getSyncState();

	switch( syncState )
	{
	case LOCAL_INIT:
	{
        getDevInfHandler().reset();
		composeClientInitializationPackage();
		break;
	}
	case SENDING_ITEMS:
	{
		composeClientModificationsPackage();
		break;
	}
	case SENDING_MAPPINGS:
	{
		composeDataUpdateStatusPackage();
		break;
	}
	default:
	{
		abortSync( INTERNAL_ERROR,  "Internal state machine error" );
		break;
	}
	}

}

ResponseStatusCode ClientSessionHandler::setupTargetByServer( const SyncMode& aSyncMode,
                                                              CommandParams& aAlertParams )
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( aAlertParams.items.isEmpty() )
    {
        qCWarning(lcSyncML) << "Received alert without any items! Cmd Id:" << aAlertParams.cmdId;
        return INCOMPLETE_COMMAND;
    }

    ItemParams& item = aAlertParams.items.first();
    const MetaParams& meta = item.meta;

    if( item.source.isEmpty() || meta.type.isEmpty() )
    {
        qCWarning(lcSyncML) << "Received alert that did not pass validation! Cmd Id:" << aAlertParams.cmdId;
		return INCOMPLETE_COMMAND;
	}

	SyncMode syncMode = aSyncMode;

	syncMode.toClientInitiated();

    qCDebug(lcSyncML) << "Searching for storage with MIME type" << meta.type;
    StoragePlugin* source = createStorageByMIME( meta.type );

	if( !source ) {
        qCDebug(lcSyncML) << "Could not found matching storage for MIME:" << meta.type;
		return NOT_FOUND;
	}

	SyncTarget* target = createSyncTarget( *source, syncMode );

	if( !target ) {
		return COMMAND_FAILED;
	}

	ResponseStatusCode status = SUCCESS;

    target->setTargetDatabase( item.source );

	if( target->getSyncMode()->syncType() == TYPE_FAST )
	{
	    //We are in fast sync mode, load mappings for target
	    target->loadUIDMappings();

    }
	else
    {
        //In slow mode, all mappings become invalid
        qCDebug(lcSyncML) << "In client mode slow sync, clear all mappings";
        target->clearUIDMappings();
    }

	addSyncTarget( target );

	// Set target database now that we know what it is so that we can write proper
	// targetref to returned status
    item.target = target->getSourceDatabase();

	return status;

}

ResponseStatusCode ClientSessionHandler::acknowledgeTarget( const SyncMode& aSyncMode,
                                                            CommandParams& aAlertParams )
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( aAlertParams.items.isEmpty() )
    {
        qCWarning(lcSyncML) << "Received alert without any items! Cmd Id:" << aAlertParams.cmdId;
        return INCOMPLETE_COMMAND;
    }

    const ItemParams& item = aAlertParams.items.first();
    const MetaParams& meta = item.meta;
    const AnchorParams& anchors = meta.anchor;

    if( item.target.isEmpty() || anchors.next.isEmpty() )
    {
        qCWarning(lcSyncML) << "Received alert that did not pass validation! Cmd Id:" << aAlertParams.cmdId;
		return INCOMPLETE_COMMAND;
	}

	SyncMode syncMode = aSyncMode;
    SyncTarget* target = getSyncTarget( item.target );

	if( !target ) {
		return NOT_FOUND;
	}

	ResponseStatusCode status = SUCCESS;


    target->setRemoteNextAnchor( anchors.next );

	// Analyze sync mode proposed by server. According to OMA DS 1.2 specification,
	// client SHOULD follow sync mode given by server even if it is different than the
	// sync mode sent by client.

	// Explicitly order sync target to revert in case of slow sync, it is not enough
	// to just set a new sync mode. Also we need to clear all mappings as they are now
	// invalid
	if( syncMode.syncType() != TYPE_FAST )
	{
	    qCDebug(lcSyncML) << "Server requested revertion to slow sync for database"
	                << target->getSourceDatabase() <<", complying and clearing mappings";
	    target->revertSyncMode();
        target->clearUIDMappings();
	}

	// Set mode proposed by server
	target->setSyncMode(syncMode);

	return status;

}

void ClientSessionHandler::setupSyncTargets()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

    QList<QString> sources;

    if (iConfig != NULL) {
        sources = iConfig->getSourceDbs();
    }

    foreach(const QString& sourceDb, sources) {

		StoragePlugin* plugin = createStorageByURI( sourceDb );
        SyncTarget* target = NULL;

		if (plugin != NULL) {
            target = createSyncTarget( *plugin, iConfig->getSyncMode() );
        }

        if (target != NULL) {
            QString targetDb = iConfig->getTarget( sourceDb );
            target->setTargetDatabase( targetDb );
            // Use slow sync if this is the preferred mode or if we don't have a last anchor
            if (iConfig->getSyncMode().syncType() == TYPE_SLOW || target->getRemoteLastAnchor().isEmpty()) {
                qCDebug(lcSyncML) << "Did not find last remote anchor, forcing slow sync";
                SyncMode* mode = target->getSyncMode();
                mode->toSlowSync();
                target->setSyncMode( *mode );
			}

            addSyncTarget( target );
        }

	}

}


void ClientSessionHandler::composeClientInitializationPackage()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// Compose initialization
	composeClientInitialization();

	// If we are to do sync without separate initialization phase, send local modifications too
	if( isSyncWithoutInitPhase() ) {
		discoverClientLocalChanges();
		composeLocalChanges();
	}

	// Close the package by appending Final
	getResponseGenerator().addPackage( new FinalPackage() );
}

void ClientSessionHandler::composeClientModificationsPackage()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	discoverClientLocalChanges();
	composeLocalChanges();

	// Close the package by appending Final
	getResponseGenerator().addPackage( new FinalPackage() );
}

void ClientSessionHandler::composeDataUpdateStatusPackage()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    // Mappings need to be sent in data update status package if we have not
    // yet sent them
    bool fastMapsSend = false;

    int configValue = getConfig()->getAgentProperty( FASTMAPSSENDPROP ).toInt();

    if( configValue > 0 )
    {
        fastMapsSend = true;
    }

    if( !fastMapsSend )
    {
        const QList<SyncTarget*>& targets = getSyncTargets();

        foreach( const SyncTarget* target, targets )
        {

            QList<UIDMapping> mappings = target->getUIDMappings();

            if (mappings.size() > 0 ) {
                LocalMappingsPackage* localMappingsPackage = new LocalMappingsPackage( target->getSourceDatabase(),
                                target->getTargetDatabase(),
                                mappings);

                connect( localMappingsPackage, SIGNAL( newMapWritten( int, int, const QString&, const QString& ) ),
                         this, SLOT( newMapReference( int, int, const QString&, const QString& ) ) );

                getResponseGenerator().addPackage( localMappingsPackage );
            }
        }
    }

    // Close the package by appending Final
    getResponseGenerator().addPackage( new FinalPackage() );

}


void ClientSessionHandler::composeClientInitialization()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// Authentication
    authentication().composeAuthentication( getResponseGenerator(), getDatabaseHandler(),
                                            params().localDeviceName(),
                                            params().remoteDeviceName() );

    // Device info exchange
    getDevInfHandler().composeLocalInitiatedDevInfExchange( getStorages(),
                                                            getProtocolVersion(),
                                                            ROLE_CLIENT,
                                                            getResponseGenerator() );

    // Sync init packages include alerts to inform server about the databases we wish to sync
    const QList<SyncTarget*>& targets = getSyncTargets();

    foreach( const SyncTarget* target, targets) {

        if (target != NULL) {
            AlertPackage* package = new AlertPackage(  target->getSyncMode()->toSyncMLCode(),
                                                       target->getSourceDatabase(),
                                                       target->getTargetDatabase(),
                                                       target->getLocalLastAnchor(),
                                                       target->getLocalNextAnchor() );
            getResponseGenerator().addPackage( package );
        }

	}

}

void ClientSessionHandler::composeResultAlert()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

    AlertPackage* package = new AlertPackage( RESULT_ALERT,
                                              params().localDeviceName(),
                                              params().remoteDeviceName() );

	getResponseGenerator().addPackage( package );

}

void ClientSessionHandler::discoverClientLocalChanges()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

	// @todo: check what we should do if discovering local changes fails. Probably
	//        internal abort as we can't reliable achieve synchronization?

    if (iConfig != NULL) {
        const QList<SyncTarget*> targets = getSyncTargets();

        foreach (SyncTarget* target, targets) {
            if( target != NULL && !target->discoverLocalChanges(ROLE_CLIENT) ) {
                qCWarning(lcSyncML) << "Error in discovering changes for target" << target->getSourceDatabase();
            }
        }
    }
}

QString ClientSessionHandler::convertSANURItoMIME( const QString& aServerURI )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    QString mime;
    if( getConfig()->extensionEnabled( SANMAPPINGSEXTENSION ) )
    {

        QStringList mappings = getConfig()->getExtensionData( SANMAPPINGSEXTENSION ).toStringList();

        for( int i = 0; i < mappings.count(); i+=2 )
        {
            if( aServerURI.contains( mappings[i], Qt::CaseInsensitive ) )
            {
                mime = mappings[i+1];
                qCDebug(lcSyncML) << "Found mapping for server URI" << aServerURI <<":" << mime;
                break;
            }
        }

    }

    return mime;
}
