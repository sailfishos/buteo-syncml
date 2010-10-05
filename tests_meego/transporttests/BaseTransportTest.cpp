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

#include "BaseTransportTest.h"

#include "SyncMLMessage.h"
#include "TestLoader.h"
#include "TestUtils.h"
#include "Fragments.h"
#include "Mock.h"

#include <QSignalSpy>

#define SYNCML_CONTTYPE_XML "application/vnd.syncml+xml"
#define SYNCML_CONTTYPE_WBXML "application/vnd.syncml+wbxml"

Q_DECLARE_METATYPE( QIODevice* );

void BaseTransportTest::initTestCase()
{
    qRegisterMetaType<DataSync::TransportStatusEvent>( "DataSync::TransportStatusEvent" );
    qRegisterMetaType<QIODevice*>( "QIODevice*" );
}

void BaseTransportTest::cleanupTestCase()
{

}

void BaseTransportTest::testBasicXMLSend()
{
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readData( &transport, SIGNAL( readXMLData( QIODevice*, bool ) ) );

    transport.setWbXml( false );

    HeaderParams params;

    params.verDTD = SYNCML_DTD_VERSION_1_2;
    params.verProto = DS_VERPROTO_1_2;
    params.msgID = 1;
    params.targetDevice = "targetDevice";
    params.sourceDevice = "sourceDevice";

    SyncMLMessage* message = new  SyncMLMessage( params, SYNCML_1_2 );

    QVERIFY( transport.sendSyncML( NULL ) == false );
    QVERIFY( sendEvent.count() == 0  );
    QVERIFY( readData.count() == 0 );

    QVERIFY( transport.sendSyncML( message ) == true );
    QVERIFY( sendEvent.count() == 0  );
    QVERIFY( readData.count() == 0 );

    QByteArray correctOutput;
    QVERIFY( readFile( "testfiles/basicbasetransport.txt", correctOutput ) );

    QVERIFY( transport.iContentType == SYNCML_CONTTYPE_XML );
    QVERIFY( transport.iData == correctOutput );

}


void BaseTransportTest::testBasicXMLReceive()
{
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readData( &transport, SIGNAL( readXMLData( QIODevice*, bool ) ) );

    transport.setWbXml( false );

    transport.iContentType = SYNCML_CONTTYPE_XML;
    QVERIFY( readFile( "testfiles/basicbasetransport.txt", transport.iData ) );

    QVERIFY( transport.receive() == true );

    QVERIFY( sendEvent.count() == 0 );
    QVERIFY( readData.count() == 1 );

    QIODevice* dev = qvariant_cast<QIODevice*>( readData.at(0).at(0) );

    QByteArray data = dev->readAll();

    QCOMPARE( data, transport.iData );

}




void BaseTransportTest::testBasicWbXMLSend()
{
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readData( &transport, SIGNAL( readXMLData( QIODevice* ) ) );

    transport.setWbXml( true );

    HeaderParams params;

    params.msgID = 1;
    params.targetDevice = "targetDevice";
    params.sourceDevice = "sourceDevice";

    SyncMLMessage* message = new  SyncMLMessage( params, SYNCML_1_2 );

    QVERIFY( transport.sendSyncML( NULL ) == false );
    QVERIFY( sendEvent.count() == 0  );
    QVERIFY( readData.count() == 0 );

    QVERIFY( transport.sendSyncML( message ) == true ); // Ownership of message is transferred.
    message = 0;
    QVERIFY( sendEvent.count() == 0  );
    QVERIFY( readData.count() == 0 );

    QByteArray correctOutput;
    QVERIFY( readFile( "testfiles/basicbasetransport.bin", correctOutput ) == true );

    QVERIFY( transport.iContentType == SYNCML_CONTTYPE_WBXML );


    // These tests are failing outside scratchbox. This is probably due to differences
    // in the libwbxml2 output because different versions of the library are used.
    // Disabling these for now until we can figure out can we test this some other way.
    //QVERIFY( transport.iData == correctOutput );

}


void BaseTransportTest::testBasicWbXMLReceive()
{
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readData( &transport, SIGNAL( readXMLData( QIODevice*, bool ) ) );

    transport.setWbXml( true );

    transport.iContentType = SYNCML_CONTTYPE_WBXML;
    QVERIFY( readFile( "testfiles/basicbasetransport.bin", transport.iData ) );

    QVERIFY( transport.receive() == true );

    QVERIFY( sendEvent.count() == 0 );
    QVERIFY( readData.count() == 1 );

    QIODevice* dev = qvariant_cast<QIODevice*>( readData.at(0).at(0) );

    QByteArray data = dev->readAll();

    QByteArray correctOutput;
    QVERIFY( readFile( "testfiles/basicbasetransport2.txt", correctOutput ) );

    // These tests are failing outside scratchbox. This is probably due to differences
    // in the libwbxml2 output because different versions of the library are used.
    // Disabling these for now until we can figure out can we test this some other way.
    //QCOMPARE( data, correctOutput );

}

void BaseTransportTest::testSANReceive01()
{

    // testSANReceive01: test SAN receive with correct content type
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readXMLData( &transport, SIGNAL( readXMLData( QIODevice* ) ) );
    QSignalSpy readSANData( &transport, SIGNAL( readSANData( QIODevice* ) ) );

    QByteArray contentType = SYNCML_CONTTYPE_SAN;
    QByteArray originalData;

    QVERIFY( readFile( "testfiles/SAN01.bin", originalData ) );

    transport.iContentType = contentType;
    transport.iData = originalData;

    QVERIFY( transport.receive() == true );

    QVERIFY( sendEvent.count() == 0 );
    QVERIFY( readXMLData.count() == 0 );
    QVERIFY( readSANData.count() == 1 );

    QIODevice* dev = qvariant_cast<QIODevice*>( readSANData.at(0).at(0) );

    QByteArray data = dev->readAll();

    QCOMPARE( originalData, data );

}

void BaseTransportTest::testSANReceive02()
{
    // testSANReceive01: test SAN receive with bad content type. Nokia SW seems to
    // use wbxml content type with SAN, even though standard specifies a distinct
    // content type for it
    TestTransport transport( true );

    QSignalSpy sendEvent( &transport, SIGNAL( sendEvent( DataSync::TransportStatusEvent, const QString& ) ) );
    QSignalSpy readXMLData( &transport, SIGNAL( readXMLData( QIODevice* ) ) );
    QSignalSpy readSANData( &transport, SIGNAL( readSANData( QIODevice* ) ) );

    QByteArray contentType = SYNCML_CONTTYPE_WBXML;
    QByteArray originalData;

    QVERIFY( readFile( "testfiles/SAN01.bin", originalData ) );

    transport.iContentType = contentType;
    transport.iData = originalData;

    QVERIFY( transport.receive() == true );

    QVERIFY( sendEvent.count() == 0 );
    QVERIFY( readXMLData.count() == 0 );
    QVERIFY( readSANData.count() == 1 );

    QIODevice* dev = qvariant_cast<QIODevice*>( readSANData.at(0).at(0) );

    QByteArray data = dev->readAll();

    QCOMPARE( originalData, data );

}

TESTLOADER_ADD_TEST(BaseTransportTest);
