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

#include "OBEXTransport.h"

#include "datatypes.h"
#include "SyncMLMessage.h"
#include "SyncAgentConfigProperties.h"
#include "OBEXConnection.h"

#include "LogMacros.h"

// Default MTU, recommended by OpenOBEX
#define DEFAULT_MTU 1024

using namespace DataSync;

OBEXTransport::OBEXTransport( OBEXConnection& aConnection, Mode aOpMode,
                              int aTimeOut, ConnectionTypeHint aTypeHint,
                              QObject* aParent )
: BaseTransport( aParent ), iConnection( aConnection ), iMode( aOpMode ),
  iTimeOut( aTimeOut ), iTypeHint( aTypeHint ), iWorkerThread( 0 ),
  iWorker( 0 ), iMTU( DEFAULT_MTU ), iMessage( 0 )
{

    FUNCTION_CALL_TRACE;
}

OBEXTransport::~OBEXTransport()
{
    FUNCTION_CALL_TRACE;

    close();
}

void OBEXTransport::setProperty( const QString& aProperty, const QString& aValue )
{
    FUNCTION_CALL_TRACE;

    //const QString BTOBEXMTU( "bt-obex-mtu" );
    //const QString USBOBEXMTU( "usb-obex-mtu" );

    //bt-obex-mtu:
    if( aProperty == BTOBEXMTUPROP && iTypeHint == TYPEHINT_BT )
    {
        LOG_DEBUG( "Setting property" << aProperty <<":" << aValue );
        iMTU = aValue.toInt();
    }
    //usb-obex-mtu:
    else if( aProperty == USBOBEXMTUPROP && iTypeHint == TYPEHINT_USB )
    {
        LOG_DEBUG( "Setting property" << aProperty <<":" << aValue );
        iMTU = aValue.toInt();
    }

}

bool OBEXTransport::init()
{
    FUNCTION_CALL_TRACE;

    int fd = iConnection.connect();

    if( !iConnection.isConnected() )
    {
        return false;
    }

    if( iMode == MODE_OBEX_CLIENT )
    {
        setupClient( fd );
    }
    else if( iMode == MODE_OBEX_SERVER )
    {
        setupServer( fd );
    }
    else
    {
        Q_ASSERT(0);
    }

    return true;

}

void OBEXTransport::close()
{
    FUNCTION_CALL_TRACE;

    if( iWorkerThread && iWorkerThread->isRunning() )
    {

        if( iWorker->isConnected() )
        {
            if( iMode == MODE_OBEX_CLIENT )
            {
                QMetaObject::invokeMethod( iWorker, "disconnect", Qt::BlockingQueuedConnection );
            }
            else if( iMode == MODE_OBEX_SERVER )
            {
                QMetaObject::invokeMethod( iWorker, "waitForDisconnect", Qt::BlockingQueuedConnection );
            }
            else
            {
                Q_ASSERT(0);
            }
        }

        iWorkerThread->exit();

        // Wait for the thread to finish. After it, force it to terminate to
        // make sure it doesn't stay running.
        if( !iWorkerThread->wait( iTimeOut * 1000 ) )
        {
            iWorkerThread->terminate();
        }
    }

    delete iWorkerThread;
    iWorkerThread = 0;

    delete iMessage;
    iMessage = 0;

    if( iConnection.isConnected() )
    {
        iConnection.disconnect();
    }

}

void OBEXTransport::setupClient( int aFd )
{
    FUNCTION_CALL_TRACE;

    OBEXClientWorker* worker = new OBEXClientWorker( aFd, iMTU, iTimeOut );

    connect( worker, SIGNAL(incomingData(QByteArray,QString) ),
             this, SLOT(incomingData(QByteArray,QString) ) );
    connect( worker, SIGNAL(connectionFailed()),
             this, SLOT(connectionFailed()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionTimeout()),
             this, SLOT(connectionTimeout()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionError()),
             this, SLOT(connectionError()), Qt::QueuedConnection );
    connect( worker, SIGNAL(sessionRejected()),
             this, SLOT(sessionRejected()), Qt::QueuedConnection );

    iWorkerThread = new OBEXWorkerThread( worker );
    iWorker = worker;

    iWorkerThread->start();
}

void OBEXTransport::setupServer( int aFd )
{
    FUNCTION_CALL_TRACE;

    OBEXServerWorker* worker = new OBEXServerWorker( *this, aFd, iMTU, iTimeOut );

    connect( worker, SIGNAL(incomingData(QByteArray,QString) ),
             this, SLOT(incomingData(QByteArray,QString) ) );
    connect( worker, SIGNAL(connectionFailed()),
             this, SLOT(connectionFailed()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionTimeout()),
             this, SLOT(connectionTimeout()), Qt::QueuedConnection );
    connect( worker, SIGNAL(connectionError()),
             this, SLOT(connectionError()), Qt::QueuedConnection );

    iWorkerThread = new OBEXWorkerThread( worker );
    iWorker = worker;

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

        QMetaObject::invokeMethod( iWorker, "waitForGet", Qt::QueuedConnection );

        return true;
    }
    else
    {
        return BaseTransport::sendSyncML( aMessage );
    }
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

        if( !iWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iWorker, "connect", Qt::BlockingQueuedConnection );
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
        QMetaObject::invokeMethod( iWorker, "send", Qt::QueuedConnection,
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
        QMetaObject::invokeMethod( iWorker, "receive", Qt::QueuedConnection,
                                   Q_ARG( QString, aContentType ) );
    }
    else if( iMode == MODE_OBEX_SERVER )
    {

        if( !iWorker->isConnected() )
        {
            QMetaObject::invokeMethod( iWorker, "waitForConnect", Qt::BlockingQueuedConnection );
        }

        QMetaObject::invokeMethod( iWorker, "waitForPut", Qt::QueuedConnection );

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

void OBEXTransport::sessionRejected()
{
    emit sendEvent( TRANSPORT_SESSION_REJECTED, "" );
}

OBEXWorkerThread::OBEXWorkerThread( OBEXWorker* worker)
 : iWorker( worker )
{
    iWorker->moveToThread( this );
}

OBEXWorkerThread::~OBEXWorkerThread()
{
}

void OBEXWorkerThread::run()
{
    LOG_DEBUG( "Starting OBEX thread..." );

    exec();

    delete iWorker;
    iWorker = 0;

    LOG_DEBUG( "Stopping OBEX thread..." );
}
