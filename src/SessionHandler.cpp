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

#include "SessionHandler.h"

#include "ChangeLog.h"
#include "SyncAgentConfig.h"
#include "SyncAgentConfigProperties.h"
#include "CommandHandler.h"
#include "AlertPackage.h"
#include "SyncTarget.h"
#include "LocalChangesPackage.h"
#include "FinalPackage.h"
#include "StoragePlugin.h"
#include "ConflictResolver.h"
#include "AuthHelper.h"
#include "StorageProvider.h"

#include "LogMacros.h"

using namespace DataSync;

SessionHandler::SessionHandler( const SyncAgentConfig* aConfig,
                                const Role& aRole,
                                QObject* aParent ) :
    QObject( aParent ),
    iDatabaseHandler( aConfig->getDatabaseFilePath() ),
    iCommandHandler( aRole ),
    iDevInfHandler( aConfig->getDeviceInfo() ),
    iConfig(aConfig),
    iSyncState( NOT_PREPARED ),
    iSyncWithoutInitPhase( false ),
    iSyncFinished( false ),
    iSessionClosed(false) ,
    iProcessing( false ),
    iProtocolVersion( SYNCML_1_2 ),
    iRemoteReportedBusy(false),
    iRole( aRole )

{
    FUNCTION_CALL_TRACE;

    Q_ASSERT(iConfig != NULL);

    // Set session handler for the storage provider, so that it can ask
    // properties like protocol version from the session handler.
    StorageProvider *storageProvider = iConfig->getStorageProvider();
    if (storageProvider != 0)
    {
        storageProvider->setSessionHandler(this);
    }

}


SessionHandler::~SessionHandler()
{
    FUNCTION_CALL_TRACE;

    // Make sure that all allocated objects are released.
    releaseStoragesAndTargets();

}

bool SessionHandler::prepareSync()
{
    FUNCTION_CALL_TRACE;

    if( !iDatabaseHandler.isValid() ) {
        abortSync( INTERNAL_ERROR, "Could not open database file" );
        return false;
    }

    // Check for credentials if authentication is to be used. Don't
    // allow empty username, allow empty password
    if( iConfig->getAuthType() != AUTH_NONE &&
        iConfig->getUsername().isEmpty() )
    {
        LOG_CRITICAL( "Authentication requested to be used, but no credentials provided" );
        abortSync( AUTHENTICATION_FAILURE, "Authentication requested to be used, but no credentials provided" );
        return false;
    }

    authentication().setSessionParams( getConfig()->getAuthType(),
                                       getConfig()->getUsername(),
                                       getConfig()->getPassword(),
                                       getConfig()->getNonce(),
                                       false );


    int localMaxMsgSize = DEFAULT_MAX_MESSAGESIZE;

    int confValue = getConfig()->getAgentProperty( MAXMESSAGESIZEPROP ).toInt();

    if( confValue > 0 )
    {
        localMaxMsgSize = confValue;
    }

    params().setLocalMaxMsgSize( localMaxMsgSize );
    params().setRemoteMaxMsgSize( localMaxMsgSize );

    // Set up transport
    Transport& transport = getTransport();

    connect( &transport, SIGNAL(sendEvent(DataSync::TransportStatusEvent, QString )),
             this, SLOT(setTransportStatus(DataSync::TransportStatusEvent , QString )));
    connect( &transport, SIGNAL(readXMLData(QIODevice *, bool)) ,
             &iParser, SLOT(parseResponse(QIODevice *, bool)));
    connect( &transport, SIGNAL(readSANData(QIODevice *)) ,
             this, SLOT(SANPackageReceived(QIODevice *)));
    connect( this, SIGNAL(purgeAndResendBuffer()) ,
             &transport, SLOT(purgeAndResendBuffer()));

    setLocalNextAnchor( QString::number( QDateTime::currentDateTime().toTime_t() ) );
    connectSignals();
    iItemReferences.clear();
    setSyncState( PREPARED );

    return true;

}

void SessionHandler::abortSync( SyncState aSyncState, const QString& aDescription )
{
    FUNCTION_CALL_TRACE;

    if( !iSessionClosed )
    {
        LOG_DEBUG("Aborting sync with state" << aSyncState << ", Reason:" << aDescription);

        iSyncState = aSyncState;
        iSyncFinished = true;
        iSyncError = aDescription;

        // If we are processing a message, we must wait until the whole message has been processed
        // (and in server mode response has been sent). If we are not processing a message, we can
        // abort right away.
        if( !iProcessing ) {
            exitSync();
        }
    }
}

bool SessionHandler::syncFinished()
{
    return iSyncFinished;
}

void SessionHandler::setSyncState( SyncState aSyncState )
{
    FUNCTION_CALL_TRACE;

    if( iSyncState != aSyncState ) {
        iSyncState = aSyncState;
        LOG_DEBUG("Sync state changed to " << iSyncState);
        emit syncStateChanged( iSyncState );
    }
}

SyncState SessionHandler::getSyncState() const
{
    return iSyncState;
}

void SessionHandler::finishSync()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Finishing sync");

    iSyncState = SYNC_FINISHED;
    iSyncFinished = true;

}

void SessionHandler::setTransportStatus(DataSync::TransportStatusEvent aEvent , QString aErrorString ) {
    FUNCTION_CALL_TRACE;

    switch (aEvent) {

        case TRANSPORT_CONNECTION_OPENED:
        case TRANSPORT_CONNECTION_CLOSED:
        {
            break;
        }
        case TRANSPORT_DATA_INVALID_CONTENT_TYPE:
        case TRANSPORT_DATA_INVALID_CONTENT:
        {
            abortSync( INVALID_SYNCML_MESSAGE, aErrorString);
            break;
        }
        case TRANSPORT_CONNECTION_FAILED:
        case TRANSPORT_CONNECTION_TIMEOUT:
        case TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED:
        {
            abortSync( CONNECTION_ERROR , aErrorString );
            break;
        }
        case TRANSPORT_CONNECTION_ABORTED:
        {
            LOG_DEBUG( "Connection Error" );
            abortSync( CONNECTION_ERROR , aErrorString );
            break;
        }
        case TRANSPORT_SESSION_REJECTED:
        {
            // Remote aborted after receiving the first message from us, likely
            // candidate for unsupported sync type.
            if( cmdRespMap.isEmpty() ) {
                LOG_DEBUG( "Unsupported sync type" );
                abortSync( UNSUPPORTED_SYNC_TYPE , "Unsupported sync type" );
            }
            else
            {
                LOG_DEBUG( "Connection Error" );
                abortSync( CONNECTION_ERROR , aErrorString );
            }
            break;
        }
        default:
        {
            LOG_DEBUG( "Unknown transport status code: " << aEvent );
            abortSync( INTERNAL_ERROR, aErrorString );
            break;
        }
    }
}
void SessionHandler::handleParsingComplete( bool aLastMessageInPackage )
{
    FUNCTION_CALL_TRACE;

    QList<DataSync::Fragment*> fragments = iParser.takeFragments();

    processMessage( fragments, aLastMessageInPackage );
}

void SessionHandler::processMessage( QList<Fragment*>& aFragments, bool aLastMessageInPackage )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Beginning to process received message..." );
    iProcessing = true;

    while( !aFragments.isEmpty() )
    {
        DataSync::Fragment* fragment = aFragments.takeFirst();

        if( fragment->fragmentType == Fragment::FRAGMENT_HEADER )
        {
            HeaderParams* header = static_cast<HeaderParams*>(fragment);
            handleHeaderElement(header);
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_STATUS )
        {
            StatusParams* status = static_cast<StatusParams*>(fragment);
            handleStatusElement(status);
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_SYNC )
        {
            SyncParams* sync = static_cast<SyncParams*>(fragment);
            handleSyncElement(sync);
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_MAP )
        {
            MapParams* map = static_cast<MapParams*>(fragment);
            handleMapElement(map);
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_PUT )
        {
            PutParams* put = static_cast<PutParams*>(fragment);
            handlePutElement( put );
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_RESULTS )
        {
            ResultsParams* results = static_cast<ResultsParams*>(fragment);
            handleResultsElement(results);
        }
        else if( fragment->fragmentType == Fragment::FRAGMENT_COMMAND )
        {
            CommandParams* command = static_cast<CommandParams*>(fragment);

            if( command->commandType == CommandParams::COMMAND_ALERT )
            {
                handleAlertElement( command );
            }
            else if( command->commandType == CommandParams::COMMAND_GET )
            {
                handleGetElement( command );
            }
            else
            {
                LOG_WARNING( "Unsupported command. Command Id:" << command->cmdId );
                iCommandHandler.rejectCommand( *command, getResponseGenerator(), NOT_IMPLEMENTED );
                delete command;
                command = 0;
            }
        }
        else
        {
            Q_ASSERT(0);
        }
    }

    if( aLastMessageInPackage )
    {
        handleFinal();
    }

    iProcessing = false;
    LOG_DEBUG( "Received message processed" );

    handleEndOfMessage();

}

void SessionHandler::handleParserErrors( DataSync::ParserError aError )
{
    FUNCTION_CALL_TRACE;

    switch (aError) {
        case PARSER_ERROR_INCOMPLETE_DATA:
        {
            abortSync( INVALID_SYNCML_MESSAGE, "Parser error: incomplete data" );
            break;
        }
        case PARSER_ERROR_UNEXPECTED_DATA:
        {
            abortSync( INVALID_SYNCML_MESSAGE, "Parser error: unexpected data" );
            break;
        }
        case PARSER_ERROR_INVALID_DATA:
        {
            abortSync( INVALID_SYNCML_MESSAGE, "Parser error: invalid data" );
            break;
        }
        case PARSER_ERROR_INVALID_CHARS:
        {
            // Emit signal to transport to remove any invalid characters
            // from the buffer
            emit purgeAndResendBuffer();
            break;
        }
        default:
        {
            abortSync( INVALID_SYNCML_MESSAGE, "Unspecified error" );
            break;
        }
    }
}

void SessionHandler::SANPackageReceived( QIODevice* aDevice )
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED( aDevice );

    // SAN packages should never be received in-session! They are allowed
    // only when listening for request, which is before any session has been
    // established.
    LOG_CRITICAL( "Received unexpected 1.2 SAN message, aborting..." );
    abortSync( INVALID_SYNCML_MESSAGE, "Received unexpected 1.2 SAN message" );
}

void SessionHandler::handleHeaderElement( DataSync::HeaderParams* aHeaderParams )
{
    FUNCTION_CALL_TRACE;

    // Common handling for header parameters

    // Save message id of the remote party
    getResponseGenerator().setRemoteMsgId( aHeaderParams->msgID );

    // If remote party sent max message size, save it.
    // If remote partys max message size is smaller than ours, reduce our
    // max message size to match
    if( aHeaderParams->meta.maxMsgSize > 0 )
    {
        params().setRemoteMaxMsgSize( aHeaderParams->meta.maxMsgSize );

        if( params().remoteMaxMsgSize() < params().localMaxMsgSize() )
        {
            params().setLocalMaxMsgSize( params().remoteMaxMsgSize() );
        }
    }

    // Ignore sending of all command statuses if we are instructed so
    getResponseGenerator().ignoreStatuses( aHeaderParams->noResp );

    // Dedicated handling for server/client
    messageReceived( *aHeaderParams );

    SessionAuthentication::HeaderStatus status = authentication().analyzeHeader( *aHeaderParams,
                                                                                 getResponseGenerator() );

    if( status == SessionAuthentication::HEADER_HANDLED_ABORT )
    {
        abortSync( AUTHENTICATION_FAILURE, authentication().getLastError() );
    }
    else if( status == SessionAuthentication::HEADER_NOT_HANDLED )
    {
        // Everything OK
        getResponseGenerator().addStatus( *aHeaderParams, SUCCESS );
    }
    // else: SessionAuthentication::HEADER_HANDLED_OK

    // If remote party sent redirect URI, set it to transport
    // and also modify the target LocURI
    if( !aHeaderParams->respURI.isEmpty() ) {
        setRemoteLocURI( aHeaderParams->respURI );
        HeaderParams headerParams = getLocalHeaderParams();
        headerParams.targetDevice = aHeaderParams->respURI;
        setLocalHeaderParams( headerParams );
    }

    delete aHeaderParams;
    aHeaderParams = NULL;
}

void SessionHandler::handleStatusElement( StatusParams* aStatusParams )
{
    FUNCTION_CALL_TRACE;

    // Add the response from this status to the command we sent to our map
    // cmd here corresponds to the syncml command for which we are handling this
    // status element sent by the remote device, which would have the response.
    QString cmd = (aStatusParams->cmd).toLower();
    cmdRespMap[cmd] = aStatusParams->data;
    LOG_DEBUG( "Remote device responded with " << aStatusParams->data << " for cmd " << cmd );

    // Common status handling
    if( aStatusParams->cmdRef == 0 )
    {

        SessionAuthentication::StatusStatus status = authentication().analyzeHeaderStatus( *aStatusParams,
                                                                                           getDatabaseHandler(),
                                                                                           params().localDeviceName(),
                                                                                           params().remoteDeviceName() );

        if( status == SessionAuthentication::STATUS_HANDLED_ABORT )
        {
            abortSync( AUTHENTICATION_FAILURE, authentication().getLastError() );
        }
        else if( status == SessionAuthentication::STATUS_HANDLED_RESEND )
        {
            resendPackage();
        }
        else if( status == SessionAuthentication::STATUS_NOT_HANDLED )
        {
            if( aStatusParams->data == SUCCESS )
            {
                // @todo: should we handle SUCCESS with auths where we should resend auth in every syncml message?
            }
            else if ( aStatusParams->data == IN_PROGRESS ){
                // remote reported busy
                // request for the results
                iRemoteReportedBusy = true;
            } else {
                // Unknown code for header, abort
                abortSync( INTERNAL_ERROR, "Unknown status code received for SyncHdr" );
            }
        }
        // else: SessionAuthentication::STATUS_HANDLED_OK

    }
    else
    {

        // Support only server-layer authentication, do not allow
        // challenges anywhere else but header status
        if( !aStatusParams->chal.meta.type.isEmpty() ) {
            abortSync( AUTHENTICATION_FAILURE, "Database-layer authentication is not supported" );
        }
        else if( aStatusParams->cmd == SYNCML_ELEMENT_ALERT ){

            // Reverting to slow sync occured
            if( aStatusParams->data == REFRESH_REQUIRED ) {
                QString sourceDb = aStatusParams->sourceRef;

                SyncTarget* target = getSyncTarget( sourceDb );

                if( target ) {
                    target->revertSyncMode();
                }

            }

        }
        else {
            iCommandHandler.handleStatus( aStatusParams );
        }

    }

    delete aStatusParams;
    aStatusParams = NULL;
}

void SessionHandler::handleSyncElement( SyncParams* aSyncParams )
{
    FUNCTION_CALL_TRACE;

    QSharedPointer<SyncParams> params( aSyncParams );

    // Don't process Sync elements if remote device has not authenticated
    if( !authentication().remoteIsAuthed() ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, INVALID_CRED  );
        return;
    }

    if( !syncReceived() ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, COMMAND_NOT_ALLOWED );
        return;
    }

    SyncTarget* target = getSyncTarget( aSyncParams->target );

    if( !target ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator,NOT_FOUND );
        return;
    }

    if( !target->discoverLocalChanges( iRole ) ) {
        LOG_CRITICAL( "Failed to discover local changes for source db" << target->getSourceDatabase() );
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, COMMAND_FAILED );
        return;
    }

    ConflictResolutionPolicy policy = PREFER_LOCAL_CHANGES;

    ConflictResolutionPolicy confValue = static_cast<ConflictResolutionPolicy>( getConfig()->getAgentProperty( CONFLICTRESOLUTIONPOLICYPROP ).toInt() );

    if( confValue > 0 )
    {
        policy = confValue;
    }

    ConflictResolver conflictResolver( *target->getLocalChanges(),
                                       policy );

    bool fastMapsSend = false;

    int configValue = getConfig()->getAgentProperty( FASTMAPSSENDPROP ).toInt();

    if( configValue > 0 )
    {
        fastMapsSend = true;
    }

    iCommandHandler.handleSync( *aSyncParams, *target, iStorageHandler,
                                iResponseGenerator, conflictResolver,
                                fastMapsSend );

}

void SessionHandler::handleAlertElement( CommandParams* aAlertParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status;

    if( authentication().remoteIsAuthed() ) {

        SyncMode syncMode( aAlertParams->data.toInt() );

        if( syncMode.isValid() ) {
            status = syncAlertReceived( syncMode, *aAlertParams );
        }
        else {
            status = handleInformativeAlert( *aAlertParams );
        }
    }
    else {
        status = INVALID_CRED;
    }

    if( !aAlertParams->noResp )
    {
        getResponseGenerator().addStatus( *aAlertParams, status, true );
    }

    delete aAlertParams;
    aAlertParams = NULL;

}

void SessionHandler::handleGetElement( DataSync::CommandParams* aGetParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !authentication().remoteIsAuthed() )
    {
        code = INVALID_CRED;
    }
    else if( aGetParams->meta.type == SYNCML_CONTTYPE_DEVINF_XML )
    {
        code = getDevInfHandler().handleGet( *aGetParams, getProtocolVersion(),
                                             getStorages(),
                                             iRole,
                                             getResponseGenerator() );

    }
    else
    {
        code = NOT_IMPLEMENTED;
    }

    if( !aGetParams->noResp )
    {
        getResponseGenerator().addStatus( *aGetParams, code, true );
    }


    delete aGetParams;
    aGetParams = NULL;
}

void SessionHandler::handlePutElement( DataSync::PutParams* aPutParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !authentication().remoteIsAuthed() )
    {
        code = INVALID_CRED;
    }
    else if( aPutParams->meta.type == SYNCML_CONTTYPE_DEVINF_XML )
    {
        code = getDevInfHandler().handlePut( *aPutParams, getProtocolVersion() );
    }
    else
    {
        code = NOT_IMPLEMENTED;
    }

    if( !aPutParams->noResp )
    {
        getResponseGenerator().addStatus( *aPutParams, code );
    }


    delete aPutParams;
    aPutParams = NULL;

}

void SessionHandler::handleResultsElement(DataSync::ResultsParams* aResults)
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !authentication().remoteIsAuthed() )
    {
        code = INVALID_CRED;
    }
    else if( aResults->meta.type == SYNCML_CONTTYPE_DEVINF_XML )
    {
        code = getDevInfHandler().handleResults( *aResults, getProtocolVersion() );
    }
    else
    {
        code = NOT_IMPLEMENTED;
    }

    // noResp cannot be specified with RESULTS, so always send back status
    getResponseGenerator().addStatus( *aResults, code );

    delete aResults;
    aResults = NULL;

}

void SessionHandler::handleMapElement( DataSync::MapParams* aMapParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status = NOT_FOUND;
    SyncTarget* target = NULL;

    if (!authentication().remoteIsAuthed() ) {
        status = INVALID_CRED;
    }
    else if (!mapReceived()) {
        status = COMMAND_NOT_ALLOWED;
    }
    else {
        target = getSyncTarget( aMapParams->target );
    }

    if( target != NULL ) {
        status = iCommandHandler.handleMap( *aMapParams, *target );
    } // no else

    // Maps do not have noResp
    getResponseGenerator().addStatus( *aMapParams, status );

    delete aMapParams;
    aMapParams = NULL;
}

void SessionHandler::handleFinal()
{
    FUNCTION_CALL_TRACE;

    if( authentication().authedToRemote() ) {
        finalReceived();
    }

}

void SessionHandler::handleEndOfMessage()
{
    FUNCTION_CALL_TRACE;

    messageParsed();
    if( iSyncFinished ) {
        exitSync();
    }

}

void SessionHandler::sendNextMessage()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Sending next message...");

    // If have nothing to send in response other than status codes, we must
    // request remote side to send data by using alert 222 ( NEXT MESSAGE )
    if( iResponseGenerator.packageQueueEmpty() ) {

        foreach( const SyncTarget* syncTarget, getSyncTargets() ) {

            iResponseGenerator.addPackage( new AlertPackage( NEXT_MESSAGE,
                                                             syncTarget->getSourceDatabase(),
                                                             syncTarget->getTargetDatabase() ) );
        }

    }

    // @todo: what if message generation fails?

    SyncMLMessage* message = iResponseGenerator.generateNextMessage( params().remoteMaxMsgSize(),
                                                                     getProtocolVersion(),
                                                                     getTransport().usesWbXML() );

    // @todo: what if sending fails?

    getTransport().sendSyncML( message );

    if( getConfig()->extensionEnabled( EMITAGSEXTENSION ) )
    {
        clearEMITags();
    }

    LOG_DEBUG( "Next message sent" );

}

ProtocolVersion SessionHandler::getProtocolVersion() const
{
    return iProtocolVersion;
}

void SessionHandler::setProtocolVersion( const ProtocolVersion& aProtocolVersion )
{
    iProtocolVersion = aProtocolVersion;
}

const QString& SessionHandler::getLocalNextAnchor() const
{
    return iLocalNextAnchor;
}

void SessionHandler::setLocalNextAnchor( const QString& aLocalNextAnchor )
{
    iLocalNextAnchor = aLocalNextAnchor;
}

void SessionHandler::exitSync()
{
    FUNCTION_CALL_TRACE;

    if(!iSessionClosed) {

    	iSessionClosed = true;

        // Transport can be closed before doing cleaning
        getTransport().close();

        // In case of successful session, save sync anchors
        if( iSyncState == SYNC_FINISHED )
        {
            saveSession();
        }

        // Release everything
    	releaseStoragesAndTargets();

        emit syncFinished( params().remoteDeviceName(), iSyncState, iSyncError);

    }
}

void SessionHandler::releaseStoragesAndTargets()
{
    FUNCTION_CALL_TRACE;

    StorageProvider* provider = NULL;

    if (iConfig != NULL) {
        provider = iConfig->getStorageProvider();
    }

    if (provider != NULL) {

        for( int i = 0; i < iStorages.count(); ++i ) {
            provider->releaseStorage( iStorages[i] );
        }

        iStorages.clear();
    }


    qDeleteAll( iSyncTargets );
    iSyncTargets.clear();
}


void DataSync::SessionHandler::connectSignals()
{
    FUNCTION_CALL_TRACE;

    connect( &iParser, SIGNAL(parsingComplete(bool)),
             this, SLOT(handleParsingComplete(bool)), Qt::QueuedConnection );

    connect( &iParser, SIGNAL( parsingError(DataSync::ParserError)),
            this, SLOT(handleParserErrors(DataSync::ParserError)));

    connect( &iCommandHandler, SIGNAL( itemAcknowledged( int, int, SyncItemKey ) ),
             this, SLOT( processItemStatus( int, int, SyncItemKey ) ) );

    connect( &iStorageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString ,QString, int ) ),
             this, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString ,QString, int) ) );

}

ResponseStatusCode SessionHandler::handleInformativeAlert( const CommandParams& aAlertParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status;

    // Do not implement: RESULT_ALERT, DISPLAY
    // @todo: implement NO_END_OF_DATA, ALERT_SUSPEND, ALERT_RESUME
    qint32 alertCode = aAlertParams.data.toInt();
    switch( alertCode ) {
        case DISPLAY:
        case RESULT_ALERT:
        {
            status = NOT_IMPLEMENTED;
            break;
        }
        case NEXT_MESSAGE:
        {
            status = SUCCESS;
            break;
        }
        default:
        {
            status = NOT_IMPLEMENTED;
            break;
        }
    }

    return status;

}

bool SessionHandler::anchorMismatch( const SyncMode& aSyncMode, const SyncTarget& aTarget,
                                     const QString& aRemoteLastAnchor ) const
{
    FUNCTION_CALL_TRACE;

    if( aSyncMode.syncType() != TYPE_FAST )
    {
        LOG_DEBUG( "Slow sync mode, not checking anchors of remote database" << aTarget.getTargetDatabase() );
        return false;
    }

    LOG_DEBUG( "Fast sync mode, checking anchors of remote database" << aTarget.getTargetDatabase() );
    LOG_DEBUG( "Stored LAST anchor:" << aTarget.getRemoteLastAnchor() );
    LOG_DEBUG( "LAST anchor reported by remote device:" << aRemoteLastAnchor );

    if( aRemoteLastAnchor.isEmpty() || aTarget.getRemoteLastAnchor() != aRemoteLastAnchor )
    {
        LOG_DEBUG( "Anchor mismatch!" );
        return true;
    }
    else
    {
        LOG_DEBUG( "Anchors match" );
        return false;
    }
}

void SessionHandler::composeLocalChanges()
{
    FUNCTION_CALL_TRACE;

    int maxChangesPerMessage = DEFAULT_MAX_CHANGES_TO_SEND;

    int configValue = getConfig()->getTransportProperty( MAXCHANGESPERMESSAGEPROP ).toInt();
    if( configValue > 0 )
    {
        maxChangesPerMessage = configValue;
    }

    LOG_DEBUG( "Setting number of changes to send per message to" << maxChangesPerMessage );

    int largeObjectThreshold = qMax( static_cast<int>( MAXMSGOVERHEADRATIO * params().remoteMaxMsgSize()), MINMSGOVERHEADBYTES );

    const QList<SyncTarget*>& targets = getSyncTargets();
    foreach( const SyncTarget* syncTarget, targets ) {
        const LocalChanges* localChanges = syncTarget->getLocalChanges();
        LocalChangesPackage* localChangesPackage = new LocalChangesPackage( *syncTarget,
                                                                            *localChanges,
                                                                            largeObjectThreshold,
                                                                            iRole,
                                                                            maxChangesPerMessage );
        iResponseGenerator.addPackage(localChangesPackage);

        connect( localChangesPackage, SIGNAL( newItemWritten( int, int, SyncItemKey, ModificationType, QString, QString, QString ) ),
                 this, SLOT( newItemReference( int, int, SyncItemKey, ModificationType, QString, QString, QString ) ) );

    }

}

void SessionHandler::setupSession( HeaderParams& aHeaderParams )
{
    FUNCTION_CALL_TRACE;

    params().setSessionId( aHeaderParams.sessionID );

    // If remote party sent unknown device id, identify ourselves in the response
    if( aHeaderParams.targetDevice == SYNCML_UNKNOWN_DEVICE ) {

        if( !getConfig()->getLocalDeviceName().isEmpty() ) {
            aHeaderParams.targetDevice = getConfig()->getLocalDeviceName();
        }
        else {
            aHeaderParams.targetDevice = getDevInfHandler().getLocalDeviceInfo().getDeviceID();
        }
    }

    params().setLocalDeviceName( aHeaderParams.targetDevice );
    params().setRemoteDeviceName( aHeaderParams.sourceDevice );

    QString verDTD;
    QString verProto;

    if( aHeaderParams.verDTD == SYNCML_DTD_VERSION_1_1 ) {
        LOG_DEBUG("Setting SyncML 1.1 protocol version");
        setProtocolVersion( SYNCML_1_1 );
        verDTD = SYNCML_DTD_VERSION_1_1;
        verProto = DS_VERPROTO_1_1;
    }
    else if( aHeaderParams.verDTD == SYNCML_DTD_VERSION_1_2 ) {
        LOG_DEBUG("Setting SyncML 1.2 protocol version");
        setProtocolVersion( SYNCML_1_2 );
        verDTD = SYNCML_DTD_VERSION_1_2;
        verProto = DS_VERPROTO_1_2;
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
        handleEMITags( aHeaderParams, headerParams );
    }

    setLocalHeaderParams( headerParams );

}

void SessionHandler::setupSession( const QString& aSessionId )
{
    FUNCTION_CALL_TRACE;

    params().setSessionId( aSessionId );

    if( !getConfig()->getLocalDeviceName().isEmpty() ) {
        params().setLocalDeviceName( getConfig()->getLocalDeviceName() );
    }
    else {
        params().setLocalDeviceName( getDevInfHandler().getLocalDeviceInfo().getDeviceID() );
    }

    if( !getConfig()->getRemoteDeviceName().isEmpty() ) {
        params().setRemoteDeviceName( getConfig()->getRemoteDeviceName() );
    }
    else {
        params().setRemoteDeviceName( SYNCML_UNKNOWN_DEVICE );
    }

    setProtocolVersion( getConfig()->getProtocolVersion() );

    if( getConfig()->extensionEnabled( SYNCWITHOUTINITPHASEEXTENSION ) ) {
        setSyncWithoutInitPhase( true );
    }

    QString verDTD;
    QString verProto;

    if( getProtocolVersion() == SYNCML_1_1 )
    {
        LOG_DEBUG("Setting SyncML 1.1 protocol version");
        verDTD = SYNCML_DTD_VERSION_1_1;
        verProto = DS_VERPROTO_1_1;
    }
    else if( getProtocolVersion() == SYNCML_1_2 )
    {
        LOG_DEBUG("Setting SyncML 1.2 protocol version");
        verDTD = SYNCML_DTD_VERSION_1_2;
        verProto = DS_VERPROTO_1_2;
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
        insertEMITagsToken(headerParams);
    }

    setLocalHeaderParams( headerParams );

}

void SessionHandler::saveSession()
{
    FUNCTION_CALL_TRACE;

    // Store last sync time with accuracy of 1 second, ceiling it to the
    // next full second.
    QDateTime dateTime = QDateTime::currentDateTime();
    QTime time = dateTime.time();
    time.addSecs( 1 );
    time.setHMS( time.hour(), time.minute(), time.second() , 0 );
    dateTime.setTime( time );

    DatabaseHandler& handler = getDatabaseHandler();

    foreach( SyncTarget* syncTarget, getSyncTargets()) {
        syncTarget->saveSession( handler, dateTime );
    }

}

StoragePlugin* SessionHandler::createStorageByURI( const QString& aURI )
{
    FUNCTION_CALL_TRACE;

    StoragePlugin* plugin = NULL;

    for( int i = 0; i < iStorages.count(); ++i ) {
        if( iStorages[i]->getSourceURI() == aURI ) {
            plugin = iStorages[i];
            break;
        }
    }

    if( plugin == NULL ) {
        const SyncAgentConfig* config = getConfig();
        StorageProvider* storageProvider = NULL;

        if (config != NULL) {
            storageProvider = config->getStorageProvider();
        }

        if (storageProvider != NULL) {
            plugin = storageProvider->acquireStorageByURI( aURI );
        }

        if (plugin != NULL) {
            iStorages.append( plugin );
            emit storageAccquired (plugin->getFormatInfo().getPreferredTx().iType);
        }
    }

    return plugin;

}

StoragePlugin* SessionHandler::createStorageByMIME( const QString& aMIME )
{
    FUNCTION_CALL_TRACE;

    StoragePlugin* plugin = NULL;

    for( int i = 0; i < iStorages.count(); ++i ) {
        if( iStorages[i]->getFormatInfo().getPreferredRx().iType == aMIME ) {
            plugin = iStorages[i];
            break;
        }
    }

    if( !plugin ) {
        plugin = getConfig()->getStorageProvider()->acquireStorageByMIME( aMIME );
        if (plugin) {
            iStorages.append( plugin );
	    emit storageAccquired (aMIME);
        }
    }

    return plugin;

}

const QList<StoragePlugin*>& SessionHandler::getStorages() const
{
    return iStorages;
}

SyncTarget* SessionHandler::createSyncTarget( StoragePlugin& aPlugin, const SyncMode& aSyncMode )
{
    FUNCTION_CALL_TRACE;

    SyncTarget* target = getSyncTarget( aPlugin.getSourceURI() );

    if( !target ) {

        ChangeLog* changelog = new ChangeLog( params().remoteDeviceName(),
                                              aPlugin.getSourceURI(),
                                              aSyncMode.syncDirection() );

        if( !changelog->load( getDatabaseHandler().getDbHandle() ) ) {
            LOG_WARNING( "Could not load change log information" );
        }

        target = new SyncTarget( changelog, &aPlugin, aSyncMode, getLocalNextAnchor() );

    }

    return target;

}

void SessionHandler::addSyncTarget( SyncTarget* aTarget )
{
    FUNCTION_CALL_TRACE;

    if( !iSyncTargets.contains( aTarget ) ) {
        iSyncTargets.append( aTarget );
    }

}

SyncTarget* SessionHandler::getSyncTarget( const QString& aSourceURI ) const
{
    FUNCTION_CALL_TRACE;

    SyncTarget* target = NULL;

    for( int i = 0; i < iSyncTargets.count(); ++i ) {
        if( aSourceURI.compare(iSyncTargets[i]->getSourceDatabase(),Qt::CaseInsensitive)  == 0 ) {
            target = iSyncTargets[i];
            break;
        }

    }

    return target;

}

const QList<SyncTarget*>& SessionHandler::getSyncTargets() const
{
    return iSyncTargets;
}

void SessionHandler::setLocalHeaderParams( const HeaderParams& aHeaderParams )
{
    iResponseGenerator.setHeaderParams(aHeaderParams);
}

const HeaderParams& SessionHandler::getLocalHeaderParams() const
{
    return iResponseGenerator.getHeaderParams();
}

void SessionHandler::setRemoteLocURI( const QString& aURI )
{
    getTransport().setRemoteLocURI(aURI);
}

SessionAuthentication& SessionHandler::authentication()
{
    return iSessionAuth;
}

SessionParams& SessionHandler::params()
{
    return iSessionParams;
}

const SyncAgentConfig* SessionHandler::getConfig() const
{
    return iConfig;
}

Transport& SessionHandler::getTransport()
{
    return *iConfig->getTransport();
}

ResponseGenerator& SessionHandler::getResponseGenerator()
{
    return iResponseGenerator;
}

DatabaseHandler& SessionHandler::getDatabaseHandler()
{
    return iDatabaseHandler;
}

void SessionHandler::newItemReference( int aMsgId, int aCmdId, SyncItemKey aKey,
                                       ModificationType aModificationType,
                                       QString aLocalDatabase, QString aRemoteDatabase,
                                       QString aMimeType )
{
    FUNCTION_CALL_TRACE;

    ItemReference reference;

    reference.iMsgId = aMsgId;
    reference.iCmdId = aCmdId;
    reference.iKey = aKey;
    reference.iModificationType = aModificationType;
    reference.iLocalDatabase = aLocalDatabase;
    reference.iRemoteDatabase = aRemoteDatabase;
    reference.iMimeType = aMimeType;

    iItemReferences.append( reference );

    LOG_DEBUG("Adding reference to item:" << aKey );
}

void SessionHandler::processItemStatus( int aMsgRef, int aCmdRef, SyncItemKey aKey )
{
    FUNCTION_CALL_TRACE;

    quint32 count = iItemReferences.count();

    for( int i = 0; i < iItemReferences.count(); ++i ) {

        const ItemReference& reference = iItemReferences[i];

        if( reference.iMsgId == aMsgRef &&
            reference.iCmdId == aCmdRef &&
            reference.iKey == aKey ) {

            emit itemProcessed( reference.iModificationType, MOD_REMOTE_DATABASE, reference.iLocalDatabase,
                                reference.iMimeType, count );
            iItemReferences.removeAt( i );

            break;

        }

    }

}

bool DataSync::SessionHandler::isRemoteBusyStatusSet() const
{
	return iRemoteReportedBusy;
}

void DataSync::SessionHandler::resetRemoteBusyStatus()
{
	iRemoteReportedBusy = false;
}

DevInfHandler& SessionHandler::getDevInfHandler()
{
    return iDevInfHandler;
}

void SessionHandler::insertEMITagsToken( HeaderParams& aLocalHeader )
{
    FUNCTION_CALL_TRACE;

    QVariant data = getConfig()->getExtensionData( EMITAGSEXTENSION );

    QStringList tags = data.toStringList();

    LOG_DEBUG( "EMI tags extension: adding token" << tags[0] );

    aLocalHeader.meta.EMI.append( tags[0] );


}

void SessionHandler::handleEMITags( const HeaderParams& aRemoteHeader, HeaderParams& aLocalHeader )
{
    FUNCTION_CALL_TRACE;

    QVariant data = getConfig()->getExtensionData( EMITAGSEXTENSION );

    QStringList tags = data.toStringList();

    if( aRemoteHeader.meta.EMI.contains( tags[0] ) )
    {
        LOG_DEBUG( "EMI tags extension: responding to" << tags[0] << "with" << tags[1] );
        aLocalHeader.meta.EMI.append( tags[1] );
    }

}

void SessionHandler::clearEMITags()
{
    FUNCTION_CALL_TRACE;

    QVariant data = getConfig()->getExtensionData( EMITAGSEXTENSION );

    QStringList tags = data.toStringList();

    HeaderParams headerParams = getLocalHeaderParams();

    for( int i = 0; i < tags.count(); ++i )
    {
        headerParams.meta.EMI.removeOne( tags[i] );
    }

    setLocalHeaderParams( headerParams );

}

void SessionHandler::setSyncWithoutInitPhase( bool aSyncWithoutInitPhase )
{
    iSyncWithoutInitPhase = aSyncWithoutInitPhase;
}

bool SessionHandler::isSyncWithoutInitPhase() const
{
    return iSyncWithoutInitPhase;
}

QString SessionHandler::generateSessionID()
{
    // A common way to create unique session ID is to use UNIX timestamp.
    // However, it needs to be converted to string format and for compatibility
    // reasons only 4 rightmost digits are used.
    uint timeStamp = QDateTime::currentDateTime().toTime_t();
    QString sessionId = QString::number(timeStamp).right( 4 );

    return sessionId;
}

SyncState SessionHandler::getLastError( QString &aErrorMsg )
{
    // Try to figure out the last erroneous sync state, if any
    SyncState state = iSyncState;
    aErrorMsg = "";

    if( iSyncState != SYNC_FINISHED )
    {
        // Check alerts first - checking if the remote device responded with an error
        // for the sync alert that we sent last.
         switch(cmdRespMap["alert"])
        {
            case NOT_SUPPORTED:
            {
                state = UNSUPPORTED_SYNC_TYPE;
                aErrorMsg = "Unsupported sync type";
                break;
            }
            case NOT_FOUND:
            {
                state = UNSUPPORTED_STORAGE_TYPE;
                aErrorMsg = "Unsupported storage type";
                break;
            }
            default:
            {
                //TODO Do we check more codes
		state = INTERNAL_ERROR;
                aErrorMsg = "Internal error";
                break;
            }
        }
        //TODO Do we check responses to messages other than alerts?
    }
    return state;
}
