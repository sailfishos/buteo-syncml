/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "SessionHandler.h"

#include "ChangeLog.h"
#include "SyncAgentConfig.h"
#include "CommandHandler.h"
#include "AlertPackage.h"
#include "SyncTarget.h"
#include "LocalChangesPackage.h"
#include "AuthenticationPackage.h"
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
	iCommandHandler( aRole ),
	iDevInfHandler( aConfig->getDeviceInfo() ),
    iConfig(aConfig),
    iSyncState( NOT_PREPARED ),
    iProtocolAttributes( LAST_PROTOCOL_ATTRIBUTE ),
    iRemoteMaxMsgSize( -1 ),
    iDatabaseHandler( aConfig->getDatabaseFilePath() ),
    iAuthenticationType( AUTH_NONE ),
    iNonceStorage( 0 ),
    iSyncFinished( false ),
    iSessionClosed(false) ,
    iSessionAuthenticated( false ),
    iAuthenticationPending( false ),
    iParsing( false ),
    iProtocolVersion( DS_1_2 ),
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

    delete iNonceStorage;
    iNonceStorage = NULL;

}

void SessionHandler::prepareSync()
{
    FUNCTION_CALL_TRACE;

    if( !iDatabaseHandler.isValid() ) {
        abortSync( INTERNAL_ERROR, "Could not open database file" );
    }

    // @todo: sanitize this

    // Configuration is valid, set up changelog
    iNonceStorage = new NonceStorage( getDatabaseHandler().getDbHandle() );
    bool nonces = iNonceStorage->createNonceTable();

    if( nonces ) {

        // Set up transport
        Transport& transport = getTransport();
        iRemoteMaxMsgSize = transport.getMaxTxSize();
        connect( &transport, SIGNAL(sendEvent(DataSync::TransportStatusEvent, QString )),
                 this, SLOT(setTransportStatus(DataSync::TransportStatusEvent , QString )));
        connect( &transport, SIGNAL(readXMLData(QIODevice *)) ,
                 &iParser, SLOT(parseResponse(QIODevice *)));
        connect( &transport, SIGNAL(readSANData(QIODevice *)) ,
                 this, SLOT(SANPackageReceived(QIODevice *)));

        setLocalNextAnchor( QString::number( QDateTime::currentDateTime().toTime_t() ) );
        connectSignals();
        iItemReferences.clear();
        iMapReferences.clear();
        setSyncState( PREPARED );
    }
    else {
        abortSync( INTERNAL_ERROR, "Failed to set up database tables" );
    }

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

        // If we are parsing a message, emit signals after whole message has been parsed
        // (and in server mode response has been sent). If we are not parsing a message, we must
        // emit signal right away.
        if( !iParsing ) {
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
        case TRANSPORT_CONNECTION_ABORTED:
        case TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED:
        {
            abortSync( CONNECTION_ERROR , aErrorString );
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

    while( !aFragments.isEmpty() )
    {
        DataSync::Fragment* fragment = aFragments.takeFirst();

        if( fragment->iType == Fragment::FRAGMENT_HEADER )
        {
            HeaderParams* header = static_cast<HeaderParams*>(fragment);
            handleHeaderElement(header);
        }
        else if( fragment->iType == Fragment::FRAGMENT_STATUS )
        {
            StatusParams* status = static_cast<StatusParams*>(fragment);
            handleStatusElement(status);
        }
        else if( fragment->iType == Fragment::FRAGMENT_ALERT )
        {
            AlertParams* alert = static_cast<AlertParams*>(fragment);
            handleAlertElement(alert);
        }
        else if( fragment->iType == Fragment::FRAGMENT_SYNC )
        {
            SyncParams* sync = static_cast<SyncParams*>(fragment);
            handleSyncElement(sync);
        }
        else if( fragment->iType == Fragment::FRAGMENT_MAP )
        {
            MapParams* map = static_cast<MapParams*>(fragment);
            handleMapElement(map);
        }
        else if( fragment->iType == Fragment::FRAGMENT_RESULTS )
        {
            ResultsParams* results = static_cast<ResultsParams*>(fragment);
            handleResultsElement(results);
        }
        else if( fragment->iType == Fragment::FRAGMENT_COMMAND )
        {
            SyncActionData* action = static_cast<SyncActionData*>(fragment);

            if( action->action == SYNCML_PUT )
            {
                handlePutElement(action);
            }
            else if( action->action == SYNCML_GET )
            {
                handleGetElement(action);
            }
        }
    }

    if( aLastMessageInPackage )
    {
        handleFinal();
    }

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

    // Message parsing started
    iParsing = true;

    // Common handling for header parameters

    // If remote party sent redirect URI, set it to transport
    if( !aHeaderParams->respURI.isEmpty() ) {
        setRemoteLocURI( aHeaderParams->respURI );
    }

    // Save message id of the remote party
    getResponseGenerator().setRemoteMsgId( aHeaderParams->msgID );

    // If remote party sent max message size, set it to transport
    if( aHeaderParams->maxMsgSize > 0 ) {
        setRemoteMaxMsgSize( aHeaderParams->maxMsgSize );
    }

    // Ignore sending of all command statuses if we are instructed so
    getResponseGenerator().ignoreStatuses( aHeaderParams->noResp );

    // Dedicated handling for server/client
    messageReceived( *aHeaderParams );

    // Handle authentication

    if( !aHeaderParams->cred.data.isEmpty() ) {

        // Authentication received

        authenticationInformationReceived( *aHeaderParams );
    }
    else if( !iAuthenticationPending && !getSessionAuthenticated() ) {

        // Expecting authentication, send challenge

        ChalParams challenge;
        challenge.meta.format = SYNCML_FORMAT_ENCODING_B64;

        if( getAuthenticationType() == AUTH_BASIC ) {
            challenge.meta.type = SYNCML_FORMAT_AUTH_BASIC;
        }
        else if( getAuthenticationType() == AUTH_MD5 ) {
            challenge.meta.type = SYNCML_FORMAT_AUTH_MD5;

            QByteArray nonce = iNonceStorage->generateNonce();

            challenge.meta.nextNonce = nonce.toBase64();

            iNonceStorage->addNonce( getLocalDeviceName(),
                                     getRemoteDeviceName(), nonce );

        }

        getResponseGenerator().addStatus( *aHeaderParams, challenge, MISSING_CRED );

    }
    else {

        // Everything OK

        getResponseGenerator().addStatus( *aHeaderParams, SUCCESS );

    }

    delete aHeaderParams;
    aHeaderParams = NULL;
}

void SessionHandler::handleStatusElement( StatusParams* aStatusParams )
{
    FUNCTION_CALL_TRACE;

    // Common status handling
    if( aStatusParams->cmdRef == 0 ) {

        QString remoteDevice = getRemoteDeviceName();
        QString localDevice = getLocalDeviceName();

        // Header status

        if( aStatusParams->data == AUTH_ACCEPTED ) {

            // Authenticated for session
            iAuthenticationPending = false;
            setSessionAuthenticated( true );

            if( getAuthenticationType() == AUTH_MD5 ) {
                // Clear possible nonce
                iNonceStorage->clearNonce( remoteDevice, localDevice );
            }

            // If remote party sent us a challenge when authentication was successful,
            // we must save the next nonce
            if( aStatusParams->chal.meta.type == SYNCML_FORMAT_AUTH_MD5 ) {
                QByteArray nonce = aStatusParams->chal.meta.nextNonce.toAscii();

                iNonceStorage->addNonce( remoteDevice, localDevice, nonce );
            }

        }
        else if( aStatusParams->data == MISSING_CRED ) {

            if( aStatusParams->chal.meta.type.isEmpty() ) {

                if( getAuthenticationType() == AUTH_MD5 ) {
                    // Clear possible nonce
                    iNonceStorage->clearNonce( remoteDevice, localDevice );
                }

                abortSync( AUTHENTICATION_FAILURE, "Authentication required" );
            }
            else {
                // Handle challenge
                handleChallenge( aStatusParams->chal );
            }

        }
        else if( aStatusParams->data == INVALID_CRED ) {

            // Already sent auth which failed, or auth failed without challenge
            // sent to us
            if( iAuthenticationPending || aStatusParams->chal.meta.type.isEmpty() ) {

                if( getAuthenticationType() == AUTH_MD5 ) {
                    // Clear possible nonce
                    iNonceStorage->clearNonce( remoteDevice, localDevice );
                }

                abortSync( AUTHENTICATION_FAILURE, "Authentication failed" );
            }
            else {
                // Handle challenge
                handleChallenge( aStatusParams->chal );
            }

        }
        else if( aStatusParams->data == SUCCESS ) {

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

    delete aStatusParams;
    aStatusParams = NULL;
}

void SessionHandler::handleSyncElement( SyncParams* aSyncParams )
{
    FUNCTION_CALL_TRACE;

    QSharedPointer<SyncParams> params( aSyncParams );

    // Don't process Sync elements if we are not authenticated
    if( !iSessionAuthenticated ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, INVALID_CRED  );
        return;
    }

    if( !syncReceived() ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, COMMAND_NOT_ALLOWED );
        return;
    }

    SyncTarget* target = getSyncTarget( aSyncParams->targetDatabase );

    if( !target ) {
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator,NOT_FOUND );
        return;
    }

    if( !target->discoverLocalChanges( iRole ) ) {
        LOG_CRITICAL( "Failed to discover local changes for source db" << target->getSourceDatabase() );
        iCommandHandler.rejectSync( *aSyncParams, iResponseGenerator, COMMAND_FAILED );
        return;
    }

    ConflictResolver conflictResolver( *target->getLocalChanges(),
                                       getConfig()->getConflictResolutionPolicy() );

    iCommandHandler.handleSync( *aSyncParams, *target, iStorageHandler,
                                iResponseGenerator, conflictResolver );

}

void SessionHandler::handleAlertElement( AlertParams* aAlertParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status;

    if( iSessionAuthenticated ) {

        SyncMode syncMode( aAlertParams->data );

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

    if( !aAlertParams->noResp ) {
        getResponseGenerator().addStatus( *aAlertParams, status );
    }

    delete aAlertParams;
    aAlertParams = NULL;

}

void SessionHandler::handleGetElement( DataSync::SyncActionData* aGetParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !iSessionAuthenticated )
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

    // if noResp specified, do not write status
    if( !aGetParams->noResp ) {
        getResponseGenerator().addStatus( *aGetParams, code );
    }


    delete aGetParams;
    aGetParams = NULL;
}

void SessionHandler::handlePutElement( DataSync::SyncActionData* aPutParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !iSessionAuthenticated )
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

    // if noResp specified, do not write status
    if( !aPutParams->noResp ) {
        getResponseGenerator().addStatus( *aPutParams, code );
    }


    delete aPutParams;
    aPutParams = NULL;

}

void SessionHandler::handleResultsElement(DataSync::ResultsParams* aResults)
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode code = NOT_IMPLEMENTED;

    if( !iSessionAuthenticated )
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

    if (!iSessionAuthenticated) {
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

    if( iSessionAuthenticated ) {
        finalReceived();
    }

}

void SessionHandler::handleEndOfMessage()
{
    FUNCTION_CALL_TRACE;

    // Message parsing ended
    messageParsed();
    if( iSyncFinished ) {
        exitSync();
    }
    iParsing = false;
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

    SyncMLMessage* message = iResponseGenerator.generateNextMessage( iRemoteMaxMsgSize, getProtocolVersion() );

    // @todo: what if sending fails?

    getTransport().sendSyncML( message );

    //FIXME! Add extra headers here

    LOG_DEBUG( "Next message sent" );

}

const QString& SessionHandler::getSessionId() const
{
    return iSessionId;
}

void SessionHandler::setSessionId( const QString& aSessionId )
{
    iSessionId = aSessionId;
}

ProtocolVersion SessionHandler::getProtocolVersion() const
{
    return iProtocolVersion;
}

void SessionHandler::setProtocolVersion( const ProtocolVersion& aProtocolVersion )
{
    iProtocolVersion = aProtocolVersion;
}

bool SessionHandler::getProtocolAttribute( int aAttribute ) const
{
    return iProtocolAttributes.at( aAttribute );
}

void SessionHandler::setProtocolAttribute( int aAttribute )
{
    iProtocolAttributes.setBit( aAttribute );
}

void SessionHandler::clearProtocolAttribute( int aAttribute )
{
    iProtocolAttributes.clearBit( aAttribute );
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

    	LOG_DEBUG("Tearing down the session started " << getRemoteDeviceName());

    	releaseStoragesAndTargets();
    	emit syncFinished( getRemoteDeviceName(), iSyncState, iSyncError);

    } else {
     	LOG_DEBUG("Sync Session Already Closed ");
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

    connect( &iCommandHandler, SIGNAL( mappingAcknowledged( int, int ) ),
             this, SLOT( processMapStatus( int, int ) ) );

    connect( &iStorageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString ,QString ) ),
             this, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString ,QString) ) );

}

ResponseStatusCode SessionHandler::handleInformativeAlert( const AlertParams& aAlertParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status;

    // Do not implement: RESULT_ALERT, DISPLAY
    // @todo: implement NO_END_OF_DATA, ALERT_SUSPEND, ALERT_RESUME
    switch( aAlertParams.data ) {
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

void SessionHandler::handleChallenge( const ChalParams& aChallenge )
{
    FUNCTION_CALL_TRACE;

    setSessionAuthenticated( false );

    // Support only b64-encoding in challenges
    if( aChallenge.meta.format != SYNCML_FORMAT_ENCODING_B64 ) {
        abortSync( AUTHENTICATION_FAILURE, "Unsupported encoding encounted in authentication challenge" );
    }
    // Support only basic and md5 authentication
    else if( aChallenge.meta.type == SYNCML_FORMAT_AUTH_BASIC ) {

        if( iAuthenticationPending && getAuthenticationType() == AUTH_BASIC ) {
            // We have already sent auth using basic authentication, re-challenge
            // means authentication has failed
            abortSync( AUTHENTICATION_FAILURE, "Authentication failed" );
        }
        else {
            setAuthenticationType( AUTH_BASIC );
            resendPackage();
        }
    }
    else if( aChallenge.meta.type == SYNCML_FORMAT_AUTH_MD5 ) {

        if( iAuthenticationPending && getAuthenticationType() == AUTH_MD5 ) {

            // We have already sent auth using md5 authentication, re-challenge
            // means authentication has failed if we already own a nonce

            QString nonce = iNonceStorage->retrieveNonce( getRemoteDeviceName(),
                                                          getLocalDeviceName() );

            if( nonce.isEmpty() ) {

                QByteArray nonce = QByteArray::fromBase64( aChallenge.meta.nextNonce.toAscii() );
                LOG_DEBUG("Found out nonce:" << nonce);

                iNonceStorage->addNonce( getRemoteDeviceName(),
                                         getLocalDeviceName(), nonce );

                resendPackage();
            }
            else {
                abortSync( AUTHENTICATION_FAILURE, "Challenged for MD5 authentication, but do not have a nonce!" );
            }
        }
        else {
            setAuthenticationType( AUTH_MD5 );

            QByteArray nonce = QByteArray::fromBase64( aChallenge.meta.nextNonce.toAscii() );
            LOG_DEBUG("Found out nonce:" << nonce);

            iNonceStorage->addNonce( getRemoteDeviceName(),
                                     getLocalDeviceName(), nonce );

            resendPackage();
        }
    }
    else {
        abortSync( AUTHENTICATION_FAILURE, "Unsupported authentication type encountered" );
    }

}

bool SessionHandler::anchorMismatch( const SyncMode& aSyncMode, const SyncTarget& aTarget,
                                     const QString& aRemoteLastAnchor ) const
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Remote Last Anchor:" << aRemoteLastAnchor );
    LOG_DEBUG("Target: DATABASE:" <<  aTarget.getTargetDatabase() );
    LOG_DEBUG("Stored Remote Last Anchor:" <<  aTarget.getRemoteLastAnchor());

    if( aSyncMode.syncType() == TYPE_FAST &&
        ( aRemoteLastAnchor.isEmpty() || aTarget.getRemoteLastAnchor() != aRemoteLastAnchor ) ) {
        return true;
    }
    else {
        return false;
    }
}

void SessionHandler::composeLocalChanges()
{
    FUNCTION_CALL_TRACE;

    const QList<SyncTarget*>& targets = getSyncTargets();
    foreach( const SyncTarget* syncTarget, targets ) {
        const LocalChanges* localChanges = syncTarget->getLocalChanges();
        LocalChangesPackage* localChangesPackage = new LocalChangesPackage( *syncTarget,
                                                                            *localChanges,
                                                                            getRemoteMaxMsgSize(),
                                                                            iRole,
                                                                            getConfig()->getMaxChangesToSend());
        iResponseGenerator.addPackage(localChangesPackage);

        connect( localChangesPackage, SIGNAL( newItemWritten( int, int, SyncItemKey, ModificationType, QString, QString, QString ) ),
                 this, SLOT( newItemReference( int, int, SyncItemKey, ModificationType, QString, QString, QString ) ) );

    }

}

void SessionHandler::composeAuthentication()
{
    FUNCTION_CALL_TRACE;

    AuthenticationType authType = getAuthenticationType();

    if( authType == AUTH_BASIC ){

        getResponseGenerator().addPackage( new AuthenticationPackage( getConfig()->getUsername(),
                                                                      getConfig()->getPassword() ) );
        iAuthenticationPending = true;

    }
    else if( authType == AUTH_MD5 ) {

        QByteArray nonce = iNonceStorage->retrieveNonce( getRemoteDeviceName(),
                                                         getLocalDeviceName() );

        // If we're set to MD5 but we have no nonce, don't attempt to send auth as it will fail anyway.
        // Remote side will surely challenge us and then we receive the nonce

        if( !nonce.isEmpty() ) {
            getResponseGenerator().addPackage( new AuthenticationPackage( getConfig()->getUsername(),
                                                                          getConfig()->getPassword(),
                                                                          nonce ) );
        }

        iAuthenticationPending = true;
    }


}

void SessionHandler::authenticationInformationReceived( const HeaderParams& aHeaderParams )
{
    FUNCTION_CALL_TRACE;

    AuthenticationType authType = getAuthenticationType();

    if( authType == AUTH_NONE ) {
        // Set to require no authentication -> auto-accept
        setSessionAuthenticated( true );
        getResponseGenerator().addStatus( aHeaderParams, AUTH_ACCEPTED );
    }
    else if( authType == AUTH_BASIC ) {

        if( aHeaderParams.cred.meta.type != SYNCML_FORMAT_AUTH_BASIC ||
            aHeaderParams.cred.meta.format != SYNCML_FORMAT_ENCODING_B64 ) {

            // Expecting basic authentication, send a challenge back
            ChalParams challenge;
            challenge.meta.type = SYNCML_FORMAT_AUTH_BASIC;
            challenge.meta.format = SYNCML_FORMAT_ENCODING_B64;
            getResponseGenerator().addStatus( aHeaderParams, challenge, MISSING_CRED );

        }
        else {

            AuthHelper helper;
            AuthHelper::AuthData data;

            if( helper.decodeBasicB64EncodedAuth( aHeaderParams.cred.data.toAscii(), data ) ) {

                if( data.iUsername == getConfig()->getUsername() &&
                    data.iPassword == getConfig()->getPassword() ) {

                    // Authenticated
                    setSessionAuthenticated( true );
                    getResponseGenerator().addStatus( aHeaderParams, AUTH_ACCEPTED );

                }
                else {

                    // Invalid credentials
                    getResponseGenerator().addStatus( aHeaderParams, INVALID_CRED );

                }

            }
            else {
                // Decoding failure
                getResponseGenerator().addStatus( aHeaderParams, PROCESSING_ERROR );
            }

        }

    }
    else if( authType == AUTH_MD5 ) {

        if( aHeaderParams.cred.meta.type != SYNCML_FORMAT_AUTH_MD5 ||
            aHeaderParams.cred.meta.format != SYNCML_FORMAT_ENCODING_B64 ) {

            // Expecting md5 authentication, send a challenge back
            ChalParams challenge;
            challenge.meta.type = SYNCML_FORMAT_AUTH_MD5;
            challenge.meta.format = SYNCML_FORMAT_ENCODING_B64;

            QByteArray nonce = iNonceStorage->generateNonce();

            challenge.meta.nextNonce = nonce.toBase64();

            iNonceStorage->addNonce( getLocalDeviceName(),
                                     getRemoteDeviceName(), nonce );

            getResponseGenerator().addStatus( aHeaderParams, challenge, MISSING_CRED );
        }
        else {

            AuthHelper helper;
            AuthHelper::AuthData data;
            data.iUsername = getConfig()->getUsername();
            data.iPassword = getConfig()->getPassword();

            QByteArray nonce = iNonceStorage->retrieveNonce( getLocalDeviceName(),
                                                             getRemoteDeviceName() );

            QByteArray hash = helper.encodeMD5B64Auth( data, nonce );

            if( hash == aHeaderParams.cred.data.toAscii() ) {

                // Authenticated
                setSessionAuthenticated( true );
                getResponseGenerator().addStatus( aHeaderParams, AUTH_ACCEPTED );

            }
            else {

                // Invalid credentials
                getResponseGenerator().addStatus( aHeaderParams, INVALID_CRED );

            }

        }
    }
    else {
        abortSync( INTERNAL_ERROR, "Unknown authentication type" );
    }
}


void SessionHandler::setupSession( HeaderParams& aHeaderParams )
{
    FUNCTION_CALL_TRACE;

    setSessionId( aHeaderParams.sessionID );

    // If remote party sent unknown device id, identify ourselves in the response
    if( aHeaderParams.targetDevice == SYNCML_UNKNOWN_DEVICE ) {

        if( !getConfig()->getLocalDevice().isEmpty() ) {
            aHeaderParams.targetDevice = getConfig()->getLocalDevice();
        }
        else {
            aHeaderParams.targetDevice = getDevInfHandler().getDeviceInfo().getDeviceID();
        }
    }

    setLocalDeviceName( aHeaderParams.targetDevice );
    setRemoteDeviceName( aHeaderParams.sourceDevice );

    setAuthenticationType( getConfig()->getAuthenticationType() );

    if( aHeaderParams.verDTD == SYNCML_DTD_VERSION_1_1 ) {
        LOG_DEBUG("Setting SyncML 1.1 protocol version");
        setProtocolVersion( DS_1_1 );
    }
    else if( aHeaderParams.verDTD == SYNCML_DTD_VERSION_1_2 ) {
        LOG_DEBUG("Setting SyncML 1.2 protocol version");
        setProtocolVersion( DS_1_2 );
    }

    if( getAuthenticationType() == AUTH_NONE ) {
        setSessionAuthenticated( true );
    }
    else {
        setSessionAuthenticated( false );
    }

    HeaderParams headerParams;
    headerParams.sessionID = getSessionId();
    headerParams.sourceDevice = getLocalDeviceName();
    headerParams.targetDevice = getRemoteDeviceName();
    headerParams.maxMsgSize = getLocalMaxMsgSize();

    //FIXME! Add extra headers here

    setLocalHeaderParams( headerParams );

}

void SessionHandler::setupSession( const QString& aSessionId )
{
    FUNCTION_CALL_TRACE;

    setSessionId( aSessionId );

    if( !getConfig()->getLocalDevice().isEmpty() ) {
        setLocalDeviceName( getConfig()->getLocalDevice() );
    }
    else {
        setLocalDeviceName( getDevInfHandler().getDeviceInfo().getDeviceID() );
    }

    setRemoteDeviceName( getConfig()->getRemoteDevice() );

    setProtocolVersion( getConfig()->getProtocolVersion() );

    if( getConfig()->getProtocolAttribute( NO_INIT_PHASE ) ) {
        setProtocolAttribute( NO_INIT_PHASE );
    }

    setAuthenticationType( getConfig()->getAuthenticationType() );

    HeaderParams headerParams;
    headerParams.sessionID = getSessionId();
    headerParams.sourceDevice = getLocalDeviceName();
    headerParams.targetDevice = getRemoteDeviceName();
    headerParams.maxMsgSize = getLocalMaxMsgSize();

    //FIXME! Add extra headers here

    if( getAuthenticationType() == AUTH_NONE ) {
        setSessionAuthenticated( true );
    }
    else {
        setSessionAuthenticated( false );
    }

    setLocalHeaderParams( headerParams );

}

void SessionHandler::saveSession()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Saving Sync Session ");

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
	    emit storageAccquired (plugin->getPreferredFormat().iType);
        }
    }

    return plugin;

}

StoragePlugin* SessionHandler::createStorageByMIME( const QString& aMIME )
{
    FUNCTION_CALL_TRACE;

    StoragePlugin* plugin = NULL;

    for( int i = 0; i < iStorages.count(); ++i ) {
        if( iStorages[i]->getPreferredFormat().iType == aMIME ) {
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
    FUNCTION_CALL_TRACE;

    return iStorages;
}

SyncTarget* SessionHandler::createSyncTarget( StoragePlugin& aPlugin, const SyncMode& aSyncMode )
{
    FUNCTION_CALL_TRACE;

    SyncTarget* target = getSyncTarget( aPlugin.getSourceURI() );

    if( !target ) {

        ChangeLog* changelog = new ChangeLog( getRemoteDeviceName(),
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
    FUNCTION_CALL_TRACE;
    return iSyncTargets;
}

void SessionHandler::setLocalHeaderParams( const HeaderParams& aHeaderParams )
{
    FUNCTION_CALL_TRACE;
    iResponseGenerator.setHeaderParams(aHeaderParams);
}

const HeaderParams& SessionHandler::getLocalHeaderParams() const
{
    FUNCTION_CALL_TRACE;
    return iResponseGenerator.getHeaderParams();
}

void SessionHandler::setRemoteMaxMsgSize( int aMaxMsgSize )
{
    FUNCTION_CALL_TRACE;
    iRemoteMaxMsgSize = aMaxMsgSize;
}

int SessionHandler::getRemoteMaxMsgSize() const
{
    return iRemoteMaxMsgSize;
}

int SessionHandler::getLocalMaxMsgSize()
{
    FUNCTION_CALL_TRACE;
    return getTransport().getMaxTxSize();
}

void SessionHandler::setRemoteLocURI( const QString& aURI )
{
    FUNCTION_CALL_TRACE;
    getTransport().setRemoteLocURI(aURI);
}

bool SessionHandler::getSessionAuthenticated() const
{
    FUNCTION_CALL_TRACE;
    return iSessionAuthenticated;
}

void SessionHandler::setSessionAuthenticated( bool aAuthenticated )
{
    FUNCTION_CALL_TRACE;
    iSessionAuthenticated = aAuthenticated;
}

void SessionHandler::setAuthenticationType( const AuthenticationType& aAuthenticationType )
{
    FUNCTION_CALL_TRACE;
    iAuthenticationType = aAuthenticationType;
}

AuthenticationType SessionHandler::getAuthenticationType() const
{
    FUNCTION_CALL_TRACE;
    return iAuthenticationType;
}

const SyncAgentConfig* SessionHandler::getConfig() const
{
    FUNCTION_CALL_TRACE;
    return iConfig;
}

Transport& SessionHandler::getTransport()
{
    FUNCTION_CALL_TRACE;

    return *iConfig->getTransport();
}

ResponseGenerator& SessionHandler::getResponseGenerator()
{
    FUNCTION_CALL_TRACE;
    return iResponseGenerator;
}

DatabaseHandler& SessionHandler::getDatabaseHandler()
{
    FUNCTION_CALL_TRACE;
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

void SessionHandler::newMapReference( int aMsgId, int aCmdId,
                                      const QString& aLocalDatabase,
                                      const QString& aRemoteDatabase )
{
    FUNCTION_CALL_TRACE;

    MapReference reference;

    reference.iMsgId = aMsgId;
    reference.iCmdId = aCmdId;
    reference.iLocalDatabase = aLocalDatabase;
    reference.iRemoteDatabase = aRemoteDatabase;

    iMapReferences.append( reference );

    LOG_DEBUG("Adding reference to map:" << aLocalDatabase << "->" << aRemoteDatabase );
}


void SessionHandler::processItemStatus( int aMsgRef, int aCmdRef, SyncItemKey aKey )
{
    FUNCTION_CALL_TRACE;

    for( int i = 0; i < iItemReferences.count(); ++i ) {

        const ItemReference& reference = iItemReferences[i];

        if( reference.iMsgId == aMsgRef &&
            reference.iCmdId == aCmdRef &&
            reference.iKey == aKey ) {

            iItemReferences.removeAt( i );

            emit itemProcessed( reference.iModificationType, MOD_REMOTE_DATABASE, reference.iLocalDatabase,
                                reference.iMimeType );

            break;

        }

    }

}

void DataSync::SessionHandler::processMapStatus( int aMsgRef, int aCmdRef)
{
    FUNCTION_CALL_TRACE;

    for( int i = 0; i < iMapReferences.count(); ++i ) {
        const MapReference& reference = iMapReferences[i];

        if( reference.iMsgId == aMsgRef &&
            reference.iCmdId == aCmdRef ) {

            iMapReferences.removeAt( i );

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

void SessionHandler::setLocalDeviceName( const QString& aLocalDeviceName )
{
    iLocalDeviceName = aLocalDeviceName;
}

const QString& SessionHandler::getLocalDeviceName() const
{
    return iLocalDeviceName;
}

void SessionHandler::setRemoteDeviceName( const QString& aRemoteDeviceName )
{
    iRemoteDeviceName = aRemoteDeviceName;
}

const QString& SessionHandler::getRemoteDeviceName() const
{
    return iRemoteDeviceName;
}

DevInfHandler& SessionHandler::getDevInfHandler()
{
    return iDevInfHandler;
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
