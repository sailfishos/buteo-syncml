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

#include "ClientWorkerTest.h"

#include <QTest>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSignalSpy>

#include "TestLoader.h"
#include "TestUtils.h"

#include "OBEXClientWorker.h"

#include "LogMacros.h"

using namespace DataSync;

const int MTU = 1024;
const int TIMEOUT = 5;


ClientWorkerTest::ClientWorkerTest() : iServerThread( 0 ), iClientSocket( 0 )
{

}

ClientWorkerTest::~ClientWorkerTest()
{

    delete iClientSocket;
    iClientSocket = 0;

    delete iServerThread;
    iServerThread = 0;
}

void ClientWorkerTest::init()
{

    iServerThread = new ServerThread;
    iServerThread->start();

    while( iServerThread->port() == 0 )
    {

    }

    iClientSocket = new QTcpSocket;
    iClientSocket->connectToHost( "127.0.0.1", iServerThread->port() );

    QVERIFY( iClientSocket->waitForConnected() );

}
void ClientWorkerTest::cleanup()
{

    iClientSocket->close();

    delete iClientSocket;
    iClientSocket = 0;

    iServerThread->exit();
    iServerThread->wait();

    delete iServerThread;
    iServerThread = 0;

}

void ClientWorkerTest::testConnectSuccess()
{

    // Case to test successful OBEX CONNECT

    QByteArray rsp1;
    QVERIFY( readFile( "data/obexresp01.bin", rsp1 ) );
    iServerThread->addResponse( rsp1 );

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );
}

void ClientWorkerTest::testConnectLinkFailure()
{

    // Case to test unsuccessful OBEX CONNECT due to link failure.
    // As connection is not up yet, stack should not send error signals

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( !worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

}

void ClientWorkerTest::testConnectRefused()
{

    // Case to test unsuccessful OBEX CONNECT due to remote device refusing the
    // request
    // As connection is not up yet, stack should not send error signals

    QByteArray rsp1;
    QVERIFY( readFile( "data/obexresp02.bin", rsp1 ) );
    iServerThread->addResponse( rsp1 );

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( !worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );
}

void ClientWorkerTest::testDisconnectSuccess()
{

    // Case to test successful OBEX DISCONNECT

    QByteArray rsp1;
    QVERIFY( readFile( "data/obexresp01.bin", rsp1 ) );
    iServerThread->addResponse( rsp1 );

    QByteArray rsp2;
    QVERIFY( readFile( "data/obexresp03.bin", rsp2 ) );
    iServerThread->addResponse( rsp2 );

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

    worker.disconnect();

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

}

void ClientWorkerTest::testDisconnectLinkFailure()
{

    // Case to test unsuccessful OBEX DISCONNECT due to link failure.
    // As connection is being broken down, stack should not send error signal
    // about the failure

    QByteArray rsp1;
    QVERIFY( readFile( "data/obexresp01.bin", rsp1 ) );
    iServerThread->addResponse( rsp1 );

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

    worker.disconnect();

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

}

void ClientWorkerTest::testDisconnectRefused()
{

    // Case to test unsuccessful OBEX DISCONNECT due to remote device refusing the
    // request

    QByteArray rsp1;
    QVERIFY( readFile( "data/obexresp01.bin", rsp1 ) );
    iServerThread->addResponse( rsp1 );

    QByteArray rsp2;
    QVERIFY( readFile( "data/obexresp02.bin", rsp2 ) );
    iServerThread->addResponse( rsp2 );

    OBEXClientWorker worker( iClientSocket->socketDescriptor(), MTU, TIMEOUT );

    QSignalSpy dataSpy( &worker, SIGNAL(incomingData( QByteArray, QString )) );
    QSignalSpy connFailureSpy( &worker, SIGNAL(connectionFailed()) );
    QSignalSpy connTimeoutSpy( &worker, SIGNAL(connectionTimeout()) );
    QSignalSpy connErrorSpy( &worker, SIGNAL(connectionError()) );

    worker.connect();
    QVERIFY( worker.isConnected() );

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

    worker.disconnect();

    QCOMPARE( dataSpy.count(), 0 );
    QCOMPARE( connFailureSpy.count(), 0 );
    QCOMPARE( connTimeoutSpy.count(), 0 );
    QCOMPARE( connErrorSpy.count(), 0 );

}

ServerThread::ServerThread()
 : iPort( 0 ), iServerSocket( 0 )
{

}

ServerThread::~ServerThread()
{
    delete iServerSocket;
    iServerSocket = 0;
}

quint16 ServerThread::port() const
{
    return iPort;
}

void ServerThread::addResponse( const QByteArray& aResponse )
{
    iResponses.append(aResponse);
}

void ServerThread::run()
{
    QTcpServer server;

    if( !server.listen( QHostAddress::LocalHost, iPort ) )
    {
        LOG_CRITICAL( "Could not start listening");
        return;
    }

    iPort = server.serverPort();
    LOG_DEBUG( "Listening port" << iPort );

    if( !server.waitForNewConnection(1000) )
    {
        LOG_CRITICAL( "No connection received");
        return;
    }

    iServerSocket = server.nextPendingConnection();

    connect( iServerSocket, SIGNAL( readyRead() ),
             this, SLOT( readData() ), Qt::DirectConnection );

    LOG_DEBUG( "Entering event loop" );

    exec();

    LOG_DEBUG( "Exiting event loop" );

    delete iServerSocket;
    iServerSocket = 0;

    server.close();

}

void ServerThread::readData()
{
    QByteArray request = iServerSocket->readAll();
    LOG_DEBUG( "Received request:" << request.toHex() );

    if( !iResponses.isEmpty() )
    {
        QByteArray response = iResponses.takeFirst();
        LOG_DEBUG( "Writing response:" << response.toHex() );
        iServerSocket->write( response );
    }
    else
    {
        LOG_DEBUG( "No responses, exiting" );
        exit();
    }

}

TESTLOADER_ADD_TEST(ClientWorkerTest);
