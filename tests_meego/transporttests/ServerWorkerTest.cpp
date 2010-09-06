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

#include "ServerWorkerTest.h"

#include <QTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSignalSpy>

#include "TestLoader.h"

#include "OBEXServerWorker.h"

#include "LogMacros.h"

using namespace DataSync;

const int MTU = 1024;
const int TIMEOUT = 5;

ServerWorkerTest::ServerWorkerTest()
 : iServer( 0 ), iClientThread( 0 ), iServerSocket( 0 )
{

}

ServerWorkerTest::~ServerWorkerTest()
{

}

bool ServerWorkerTest::getData( const QString& aContentType, QByteArray& aData )
{
    Q_UNUSED(aContentType);
    Q_UNUSED(aData);

    Q_ASSERT(0);

    return false;
}

void ServerWorkerTest::init()
{

    //Logger::createInstance();

    iServer = new QTcpServer;

    QVERIFY( iServer->listen() );

    LOG_DEBUG( "Listening port" << iServer->serverPort() );

    iClientThread = new ClientThread( iServer->serverPort() );
    iClientThread->start();

    QVERIFY( iServer->waitForNewConnection(1000) );

    iServerSocket = iServer->nextPendingConnection();

}

void ServerWorkerTest::cleanup()
{

    iServerSocket->close();

    delete iServerSocket;
    iServerSocket = 0;

    iServer->close();

    delete iServer;
    iServer = 0;

    iClientThread->exit();
    iClientThread->wait();

    delete iClientThread;
    iClientThread = 0;

    //Logger::deleteInstance();
}


// @todo: commented out until implementation finished
/*
void ServerWorkerTest::testConnectSuccess()
{
    // Case to test successful OBEX CONNECT

    QByteArray rqst;
    iClientThread->addRequest( "foo" );

    OBEXUsbConnection connection( iServerSocket->socketDescriptor() );
    OBEXServerWorker worker( &connection, *this, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );


    //QTimer::singleShot( 2000, iClientThread, SLOT( begin() ) )

    QMetaObject::invokeMethod( iClientThread, "begin", Qt::QueuedConnection );
    //iClientThread->begin();

    worker.waitForConnect();
    QVERIFY( worker.isConnected() );

}
*/

ClientThread::ClientThread( quint16 aPort )
 : iPort( aPort ), iClientSocket( 0 )
{

}

ClientThread::~ClientThread()
{
    delete iClientSocket;
    iClientSocket = 0;
}

void ClientThread::addRequest( const QByteArray& aRequest )
{
    iRequests.append(aRequest);
}

void ClientThread::begin()
{
    sendNextRequest();
}

void ClientThread::run()
{

    iClientSocket = new QTcpSocket;
    iClientSocket->connectToHost( "127.0.0.1", iPort );

    if( !iClientSocket->waitForDisconnected() )
    {
        return;
    }

    connect( iClientSocket, SIGNAL( readyRead() ),
             this, SLOT( readData() ), Qt::DirectConnection );

    exec();

    iClientSocket->close();

    delete iClientSocket;
    iClientSocket = 0;

}


void ClientThread::readData()
{
    QByteArray response = iClientSocket->readAll();
    LOG_DEBUG( "Received response:" << response.toHex() );

    sendNextRequest();
}

void ClientThread::sendNextRequest()
{
    if( !iRequests.isEmpty() )
    {
        QByteArray request = iRequests.takeFirst();
        LOG_DEBUG( "Writing request:" << request.toHex() );
        iClientSocket->write( request );
    }
    else
    {
        LOG_DEBUG( "No requests, exiting" );
        exit();
    }
}

TESTLOADER_ADD_TEST(ServerWorkerTest);
