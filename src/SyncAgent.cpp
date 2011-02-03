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

#include "SyncAgent.h"

#include <QMetaType>

#include "ChangeLog.h"
#include "SyncAgentConfig.h"
#include "SyncResults.h"
#include "ClientSessionHandler.h"
#include "ServerSessionHandler.h"
#include "RequestListener.h"

#include "LogMacros.h"

using namespace DataSync;

SyncAgent::SyncAgent(QObject* aParent)
: QObject(aParent), iListener(0), iHandler(0), iConfig(0)
{
    FUNCTION_CALL_TRACE
    // Register the struct as a type for the signals
    qRegisterMetaType<DataSync::SyncState>("DataSync::SyncState");
    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
}


SyncAgent::~SyncAgent()
{
    FUNCTION_CALL_TRACE

    abortListen();

    cleanSession();

}

// Bindings for dynamic linking
extern "C" {

SyncAgent* createSyncAgent(QObject* aParent)
{
    return new SyncAgent(aParent);
}

void destroySyncAgent(SyncAgent* aObj)
{
    delete aObj;
    aObj = 0;
}

}

bool SyncAgent::startSync( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    if( !iHandler && !iListener) {
        return initiateSession( aConfig );
    }
    else {
        LOG_CRITICAL( "SyncAgent: Listening for requests, or synchronization in progress" );
        return false;
    }

}

bool SyncAgent::isSyncing() const
{
    if( iHandler ) {
        return true;
    }
    else {
        return false;
    }
}

bool SyncAgent::listen( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    if( !iHandler && !iListener ) {
        return initiateListen( aConfig );
    }
    else {
        LOG_CRITICAL( "SyncAgent: Already listening for requests, or synchronization in progress" );
        return false;
    }
}

bool SyncAgent::isListening() const
{
    if( iListener ) {
        return true;
    }
    else {
        return false;
    }
}

bool SyncAgent::pauseSync()
{
    FUNCTION_CALL_TRACE

    if( iHandler ) {
        QTimer::singleShot( 0, iHandler, SLOT(suspendSync()) );
        return true;
    }
    else {
        LOG_CRITICAL( "SyncAgent: Nothing to pause!");
        return false;
    }

}

bool SyncAgent::resumeSync()
{
    FUNCTION_CALL_TRACE

    if( iHandler ) {
        QTimer::singleShot( 0, iHandler, SLOT(resumeSync()) );
        return true;
    }
    else {
        LOG_CRITICAL( "SyncAgent: Nothing to resume!" );
        return false;
    }

}

bool SyncAgent::abort(DataSync::SyncState aState)
{
    FUNCTION_CALL_TRACE

    if( iHandler ) {
        abortSession(aState);
        return true;
    }
    else if( iListener ) {
        abortListen();
        return true;
    }
    else {
        LOG_CRITICAL( "SyncAgent: Nothing to abort!");
        return false;
    }
}

const SyncResults& SyncAgent::getResults() const
{
    FUNCTION_CALL_TRACE
    return iResults;
}

bool  SyncAgent::cleanUp(const SyncAgentConfig* aConfig)
{
    FUNCTION_CALL_TRACE

    QList<QString> sourceDBs = aConfig->getSourceDbs();
    sourceDBs.append (aConfig->getDisabledSourceDbs());
    SyncDirection iDirection = aConfig->getSyncMode().syncDirection();

    const QString remoteId = aConfig->getRemoteDeviceName();
    QString dbPath = aConfig->getDatabaseFilePath();

    LOG_DEBUG("SyncAgent: Remote Id: " << remoteId);
    LOG_DEBUG("SyncAgent: DB Path : " << dbPath);

    if (remoteId.isEmpty() || dbPath.isEmpty()) {
	 return false;
    }

    LOG_DEBUG("SyncAgent: Sync Direction: " << iDirection);


    bool success = true;
    for (int i = 0; i < sourceDBs.size(); ++i) {
	   LOG_DEBUG("SyncAgent: Removing anchors for source DB: " << sourceDBs.at(i));
	   ChangeLog changeLog(remoteId, sourceDBs.at(i), iDirection);
    	   success = changeLog.remove (dbPath);
	   if (!success){
	   	LOG_WARNING("SyncAgent: Error Removing anchors for source DB: " << sourceDBs.at(i));
	   }
    }

    return success;
}

void SyncAgent::receiveStateChanged( DataSync::SyncState aState )
{
    FUNCTION_CALL_TRACE

    LOG_DEBUG( "SyncAgent: Synchronization status changed to:" << aState );

    iResults.setState( aState );

    emit stateChanged( aState );
}

void SyncAgent::receiveSyncFinished( const QString& aDevId,
                                     DataSync::SyncState aState,
                                     const QString& aErrorString )
{
    FUNCTION_CALL_TRACE

    iResults.setRemoteDeviceId( aDevId );

    cleanSession();

    finishSync( aState, aErrorString );

}

void SyncAgent::accquiredStorage( const QString& aStorageMimeType )
{
    FUNCTION_CALL_TRACE

    LOG_DEBUG("SyncAgent:Mime Type acquired : " << aStorageMimeType);

    emit storageAccquired(aStorageMimeType );
}

void SyncAgent::receiveItemProcessed( DataSync::ModificationType aModificationType,
                                      DataSync::ModifiedDatabase aModifiedDatabase,
                                      const QString aLocalDatabase,
                                      const QString aMimeType, int aCommittedItems )
{
    FUNCTION_CALL_TRACE

    LOG_DEBUG("SyncAgent: Item processed");

    if( (aModifiedDatabase == MOD_LOCAL_DATABASE) || ( aModifiedDatabase == MOD_REMOTE_DATABASE )) {
        iResults.addProcessedItem( aModificationType, aModifiedDatabase, aLocalDatabase );
        emit itemProcessed( aModificationType, aModifiedDatabase, aLocalDatabase , aMimeType, aCommittedItems );
    }
    else {
        Q_ASSERT( 0 );
    }


}

void SyncAgent::finishSync( DataSync::SyncState aState, const QString& aErrorString )
{
    FUNCTION_CALL_TRACE

    iResults.setState( aState );
    iResults.setErrorString( aErrorString );

    LOG_DEBUG("SyncAgent: Synchronization finished with state:" << aState );

    emit syncFinished( aState );
}

bool SyncAgent::initiateSession( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( !iHandler );

    LOG_DEBUG( "SyncAgent: Preparing for synchronization..." );

    // * Validate critical configuration

    if( !aConfig.getTransport() ) {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, transport is NULL");
        return false;
    }

    if( !aConfig.getStorageProvider() ) {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, storage provider is NULL");
        return false;
    }

    if( aConfig.getSourceDbs().isEmpty() ) {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, could not find any source databases to sync" );
        return false;
    }

    // * Clear results of previous sync
    iResults.reset();

    // * Determine type of sync
    const SyncMode& syncMode = aConfig.getSyncMode();

    if( syncMode.syncInitiator() == INIT_CLIENT )
    {
        // * Start client-initiated sync session
        return startClientInitiatedSession( aConfig );
    }
    else if( syncMode.syncInitiator() == INIT_SERVER )
    {
        // * Start server-initiated sync session
        return startServerInitiatedSession( aConfig );
    }
    else
    {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, could not interpret SyncMode" );
        return false;
    }

}

bool SyncAgent::startClientInitiatedSession( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( !iHandler );

    ClientSessionHandler* handler = new ClientSessionHandler( &aConfig, this );

    connect( handler, SIGNAL(syncStateChanged(DataSync::SyncState )),
             this, SLOT(receiveStateChanged(DataSync::SyncState)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL(syncFinished(QString, DataSync::SyncState, QString )),
             this, SLOT(receiveSyncFinished(QString, DataSync::SyncState, QString)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL(storageAccquired(QString )),
             this, SLOT(accquiredStorage(QString)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL( itemProcessed( DataSync::ModificationType,
             DataSync::ModifiedDatabase,QString,QString,int ) ),
             this, SLOT( receiveItemProcessed( DataSync::ModificationType,
             DataSync::ModifiedDatabase,QString,QString,int ) ),
             Qt::QueuedConnection );

    LOG_DEBUG( "SyncAgent: Everything OK, starting synchronization..." );

    // * Begin synchronization session
    QTimer::singleShot(0, handler, SLOT(initiateSync()));

    iHandler = handler;
    iConfig = &aConfig;

    return true;
}

bool SyncAgent::startServerInitiatedSession( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( !iHandler );

    ServerSessionHandler* handler = new ServerSessionHandler( &aConfig, this );

    connect( handler, SIGNAL(syncStateChanged(DataSync::SyncState )),
             this, SLOT(receiveStateChanged(DataSync::SyncState)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL(syncFinished(QString, DataSync::SyncState, QString )),
             this, SLOT(receiveSyncFinished(QString, DataSync::SyncState, QString)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL(storageAccquired(QString )),
             this, SLOT(accquiredStorage(QString)),
             Qt::QueuedConnection );

    connect( handler, SIGNAL( itemProcessed( DataSync::ModificationType,
             DataSync::ModifiedDatabase,QString,QString,int ) ),
             this, SLOT( receiveItemProcessed( DataSync::ModificationType,
             DataSync::ModifiedDatabase,QString,QString,int ) ),
             Qt::QueuedConnection );

    LOG_DEBUG( "SyncAgent: Everything OK, starting synchronization..." );

    // * Begin synchronization session
    QTimer::singleShot(0, handler, SLOT(initiateSync()));

    iHandler = handler;
    iConfig = &aConfig;

    return true;
}

void SyncAgent::abortSession(DataSync::SyncState aState)
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( iHandler );

    QMetaObject::invokeMethod( iHandler, "abortSync", Q_ARG( DataSync::SyncState, aState ),
                               Q_ARG( QString, "User aborted synchronization" ) );
}

void SyncAgent::cleanSession()
{
    FUNCTION_CALL_TRACE

    delete iHandler;
    iHandler = NULL;
}

bool SyncAgent::initiateListen( const SyncAgentConfig& aConfig )
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( !iListener );

    LOG_DEBUG( "SyncAgent: Preparing for listening requests..." );

    // * Validate critical configuration

    if( !aConfig.getTransport() ) {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, transport is NULL");
        return false;
    }

    if( !aConfig.getStorageProvider() ) {
        LOG_CRITICAL( "SyncAgent: Invalid configuration, storage provider is NULL");
        return false;
    }

    if( !aConfig.getTransport()->init() )
    {
        LOG_CRITICAL( "SyncAgent: Could not initiate transport" );
        return false;
    }

    // * Create & start request listener object

    RequestListener* listener = new RequestListener(this);

    connect( listener, SIGNAL(newPendingRequest()),
             this, SLOT(listenEvent()) );
    connect( listener, SIGNAL(error(DataSync::SyncState,QString)),
             this, SLOT(listenError(DataSync::SyncState,QString)) );

    if( listener->start( aConfig.getTransport() ) )
    {
        LOG_DEBUG( "SyncAgent: Now listening for requests" );
        iListener = listener;
        iConfig = &aConfig;
        return true;
    }
    else
    {
        LOG_CRITICAL( "SyncAgent: Could not start listening for requests" );
        delete listener;
        listener = 0;
        return false;
    }

}

void SyncAgent::listenEvent()
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( iListener );

    LOG_DEBUG( "SyncAgent: Processing incoming request..." );

    RequestListener::RequestData data = iListener->takeRequestData();

    iListener->stop();

    cleanListen();

    if( data.iType == RequestListener::REQUEST_CLIENT )
    {
        LOG_DEBUG( "SyncAgent: Remote client requesting session with server");

        ServerSessionHandler* handler = new ServerSessionHandler( iConfig, this );

        connect( handler, SIGNAL(syncStateChanged(DataSync::SyncState )),
                 this, SLOT(receiveStateChanged(DataSync::SyncState)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(syncFinished(QString, DataSync::SyncState, QString )),
                 this, SLOT(receiveSyncFinished(QString, DataSync::SyncState, QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(storageAccquired(QString )),
                 this, SLOT(accquiredStorage(QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL( itemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 this, SLOT( receiveItemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 Qt::QueuedConnection );

        LOG_DEBUG( "SyncAgent: Everything OK, starting synchronization..." );

        // * Begin synchronization session
        handler->serveRequest( data.iFragments );

        iHandler = handler;
    }
    else if( data.iType == RequestListener::REQUEST_SAN_XML )
    {
        LOG_DEBUG( "SyncAgent: Remote server notifying client with OMA DS XML message");

        ClientSessionHandler* handler = new ClientSessionHandler( iConfig, this );

        connect( handler, SIGNAL(syncStateChanged(DataSync::SyncState )),
                 this, SLOT(receiveStateChanged(DataSync::SyncState)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(syncFinished(QString, DataSync::SyncState, QString )),
                 this, SLOT(receiveSyncFinished(QString, DataSync::SyncState, QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(storageAccquired(QString )),
                 this, SLOT(accquiredStorage(QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL( itemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 this, SLOT( receiveItemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 Qt::QueuedConnection );

        LOG_DEBUG( "SyncAgent: Everything OK, starting synchronization..." );

        // * Begin synchronization session
        handler->handleNotificationXML( data.iFragments );

        iHandler = handler;
    }
    else if( data.iType == RequestListener::REQUEST_SAN_PACKAGE )
    {
        LOG_DEBUG( "SyncAgent: Remote server notifying client with OMA DS SAN package");

        ClientSessionHandler* handler = new ClientSessionHandler( iConfig, this );

        connect( handler, SIGNAL(syncStateChanged(DataSync::SyncState )),
                 this, SLOT(receiveStateChanged(DataSync::SyncState)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(syncFinished(QString, DataSync::SyncState, QString )),
                 this, SLOT(receiveSyncFinished(QString, DataSync::SyncState, QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL(storageAccquired(QString )),
                 this, SLOT(accquiredStorage(QString)),
                 Qt::QueuedConnection );

        connect( handler, SIGNAL( itemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 this, SLOT( receiveItemProcessed( DataSync::ModificationType,
                 DataSync::ModifiedDatabase,QString,QString,int ) ),
                 Qt::QueuedConnection );

        LOG_DEBUG( "SyncAgent: Everything OK, starting synchronization..." );

        // * Begin synchronization session
        handler->handleNotificationPackage( data.iSANData );

        iHandler = handler;
    }
    else
    {
        LOG_CRITICAL( "SyncAgent: Unknown listen event" );
        finishSync( INTERNAL_ERROR, "Unknown listen event" );
    }
}

void SyncAgent::listenError( DataSync::SyncState aState, QString aErrorString )
{
    FUNCTION_CALL_TRACE

    Q_ASSERT( iListener );

    abortListen();

    LOG_CRITICAL( "SyncAgent: Error while listening for incoming requests" );

    finishSync( aState, aErrorString );
}

void SyncAgent::abortListen()
{
    FUNCTION_CALL_TRACE

    if( iListener )
    {
        iListener->stop();
    }

    cleanListen();
}

void SyncAgent::cleanListen()
{
    FUNCTION_CALL_TRACE

    delete iListener;
    iListener = NULL;
}
