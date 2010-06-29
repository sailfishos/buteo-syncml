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

#include "OBEXTransport.h"

#include "datatypes.h"
#include "SyncMLMessage.h"

#include "OBEXBTConnection.h"
#include "OBEXUsbConnection.h"

#include "LogMacros.h"

using namespace DataSync;

OBEXTransport::OBEXTransport( Mode aOpMode, Type aType, int aFd, int aTimeOut,
                             QObject* aParent )
: BaseTransport( aParent ), iMode( aOpMode ), iWorkerThread( 0 ), iClientWorker( 0 ),
  iServerWorker( 0 ), iTimeOut( aTimeOut ), iMessage( 0 )
{

    OBEXConnection* connection = 0;

    if( aType == TYPE_BT )
    {
        connection = new OBEXBTConnection( aFd );
    }
    else if( aType == TYPE_USB )
    {
        connection = new OBEXUsbConnection( aFd );
    }
    else
    {
        Q_ASSERT(0);
    }

    if( iMode == MODE_OBEX_CLIENT )
    {
        setupClient( connection );
    }
    else if( iMode == MODE_OBEX_SERVER )
    {
        setupServer( connection );
    }
    else
    {
        Q_ASSERT(0);
    }

}

OBEXTransport::OBEXTransport( const QString& aBTAddress, const QString& aServiceUUID,
                              int aTimeOut, QObject* aParent )
 : BaseTransport( aParent ), iMode( MODE_OBEX_CLIENT ), iWorkerThread( 0 ), iClientWorker( 0 ),
   iServerWorker( 0 ), iTimeOut( aTimeOut ), iMessage( 0 )
{
    OBEXConnection* connection = new OBEXBTConnection( aBTAddress, aServiceUUID );
    setupClient(connection);
}



OBEXTransport::~OBEXTransport()
{
    FUNCTION_CALL_TRACE;

    if( iMode == MODE_OBEX_CLIENT )
    {

        if( iClientWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iClientWorker, "disconnect", Qt::BlockingQueuedConnection );
        }

    }
    else if( iMode == MODE_OBEX_SERVER )
    {

        if( iServerWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iServerWorker, "waitForDisconnect", Qt::BlockingQueuedConnection );
        }

    }
    else
    {
        Q_ASSERT(0);
    }

    iWorkerThread->exit();

    // Wait for the thread to finish. After it, force it to terminate to
    // make sure it doesn't stay running.
    if( !iWorkerThread->wait( iTimeOut * 1000 ) )
    {
        iWorkerThread->terminate();
    }

    delete iWorkerThread;
    iWorkerThread = 0;

    delete iMessage;
    iMessage = 0;
}

void OBEXTransport::setupClient( OBEXConnection* connection )
{
    FUNCTION_CALL_TRACE;

    OBEXClientWorker* worker = new OBEXClientWorker( connection, iTimeOut );

    connect( worker, SIGNAL(incomingData(QByteArray,QString) ),
             this, SLOT(incomingData(QByteArray,QString) ) );
    connect( worker, SIGNAL(connectionFailed()),
             this, SLOT(connectionFailed()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionTimeout()),
             this, SLOT(connectionTimeout()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionError()),
             this, SLOT(connectionError()), Qt::QueuedConnection );

    iWorkerThread = new OBEXWorkerThread( connection, worker );
    iClientWorker = worker;

    iWorkerThread->start();
}

void OBEXTransport::setupServer( OBEXConnection* connection )
{
    FUNCTION_CALL_TRACE;

    OBEXServerWorker* worker = new OBEXServerWorker( connection, *this, iTimeOut );

    connect( worker, SIGNAL(incomingData(QByteArray,QString) ),
             this, SLOT(incomingData(QByteArray,QString) ) );
    connect( worker, SIGNAL(connectionFailed()),
             this, SLOT(connectionFailed()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionTimeout()),
             this, SLOT(connectionTimeout()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionError()),
             this, SLOT(connectionError()), Qt::QueuedConnection );

    iWorkerThread = new OBEXWorkerThread( connection, worker );
    iServerWorker = worker;

    iWorkerThread->start();
}

bool OBEXTransport::sendSyncML( SyncMLMessage* aMessage )
{
    FUNCTION_CALL_TRACE;

    // Normally when sending a SyncML message, the message is pushed
    // to the remote device. When in OBEX server mode, the message is
    // instead pulled by the remote device with OBEX GET. Also the
    // encoding (XML/WbXML) to be used is known only when the GET
    // command has been received, and not immediately when the message
    // can be pushed. For this reason, BaseTransport::sendSyncML()
    // implementation cannot be used in OBEX server mode. Instead we
    // must save the message, and supply it to the OBEX server worker
    // when the data is being pulled (with the encoding specified by
    // GET command )
    if( iMode == MODE_OBEX_SERVER )
    {
        delete iMessage;
        iMessage = aMessage;

        QMetaObject::invokeMethod( iServerWorker, "waitForGet", Qt::QueuedConnection );

        return true;
    }
    else
    {
        return BaseTransport::sendSyncML( aMessage );
    }
}

qint64 OBEXTransport::getMaxTxSize()
{
    return OBEX_MAX_MESSAGESIZE;
}

qint64 OBEXTransport::getMaxRxSize()
{
    return OBEX_MAX_MESSAGESIZE;
}

bool OBEXTransport::getData( const QString& aContentType, QByteArray& aData )
{
    FUNCTION_CALL_TRACE;

    if( !iMessage )
    {
        return false;
    }

    bool success = false;

    if( aContentType == SYNCML_CONTTYPE_WBXML )
    {
        setWbXml(true);
        encodeMessage(*iMessage, aData );
        success = true;
    }
    else if( aContentType == SYNCML_CONTTYPE_XML )
    {
        setWbXml(false);
        encodeMessage(*iMessage, aData );
        success = true;
    }
    else
    {
        LOG_CRITICAL( "Unsupported content type:" << aContentType );
    }

    delete iMessage;
    iMessage = 0;

    return success;
}

bool OBEXTransport::prepareSend()
{
    FUNCTION_CALL_TRACE;

    if( iMode == MODE_OBEX_CLIENT )
    {

        if( !iClientWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iClientWorker, "connect", Qt::BlockingQueuedConnection );
        }

    }
    else if( iMode == MODE_OBEX_SERVER )
    {
        // Don't need to do anything in server mode
    }
    else
    {
        Q_ASSERT(0);
    }

    return true;

}

bool OBEXTransport::doSend( const QByteArray& aData, const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    if( iMode == MODE_OBEX_CLIENT )
    {
        QMetaObject::invokeMethod( iClientWorker, "send", Qt::QueuedConnection,
                                   Q_ARG( QByteArray, aData ),
                                   Q_ARG( QString, aContentType ) );
    }
    else if( iMode == MODE_OBEX_SERVER )
    {
        Q_ASSERT(0);
    }
    else
    {
        Q_ASSERT(0);
    }

    return true;
}

bool OBEXTransport::doReceive( const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    if( iMode == MODE_OBEX_CLIENT )
    {
        QMetaObject::invokeMethod( iClientWorker, "receive", Qt::QueuedConnection,
                                   Q_ARG( QString, aContentType ) );
    }
    else if( iMode == MODE_OBEX_SERVER )
    {

        if( !iServerWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iServerWorker, "waitForConnect", Qt::BlockingQueuedConnection );
        }

        QMetaObject::invokeMethod( iServerWorker, "waitForPut", Qt::QueuedConnection );

    }
    else
    {
        Q_ASSERT(0);
    }

    return true;
}

void OBEXTransport::incomingData( QByteArray aData, QString aContentType )
{
    receive( aData, aContentType );
}

void OBEXTransport::connectionFailed()
{
    emit sendEvent( TRANSPORT_CONNECTION_FAILED, "" );
}

void OBEXTransport::connectionTimeout()
{
    emit sendEvent( TRANSPORT_CONNECTION_TIMEOUT, "" );
}

void OBEXTransport::connectionError()
{
    emit sendEvent( TRANSPORT_CONNECTION_ABORTED, "" );
}

OBEXWorkerThread::OBEXWorkerThread( OBEXConnection* connection, OBEXClientWorker* worker)
 : iConnection( connection ), iClientWorker( worker ), iServerWorker( 0 )
{
    iConnection->moveToThread( this );
    iClientWorker->moveToThread( this );
}

OBEXWorkerThread::OBEXWorkerThread( OBEXConnection* connection, OBEXServerWorker* worker)
 : iConnection( connection ), iClientWorker( 0 ), iServerWorker( worker )
{
    iConnection->moveToThread( this );
    iServerWorker->moveToThread( this );
}

OBEXWorkerThread::~OBEXWorkerThread()
{
}

void OBEXWorkerThread::run()
{
    LOG_DEBUG( "Starting OBEX thread..." );

    exec();

    delete iConnection;
    iConnection = 0;

    delete iClientWorker;
    iClientWorker = 0;

    delete iServerWorker;
    iServerWorker = 0;

    LOG_DEBUG( "Stopping OBEX thread..." );
}
