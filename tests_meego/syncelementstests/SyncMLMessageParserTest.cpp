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

#include "SyncMLMessageParserTest.h"

#include <QTest>
#include <QSignalSpy>
#include <QBuffer>

#include "SyncMLMessageParser.h"
#include "TestLoader.h"
#include "TestUtils.h"
#include "RemoteDeviceInfo.h"

using namespace DataSync;

void SyncMLMessageParserTest::testResp1()
{

    SyncMLMessageParser parser;

    QByteArray data;

    QVERIFY( readFile( "testfiles/resp.txt", data ) );

    QBuffer buffer(&data );
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );

    QSignalSpy parsingSpy( &parser, SIGNAL(parsingComplete(bool)) );
    QSignalSpy errorSpy( &parser, SIGNAL(parsingError(DataSync::ParserError)) );

    parser.parseResponse( &buffer, true );

    buffer.close();

    QCOMPARE( parsingSpy.count(), 1 );
    QCOMPARE( errorSpy.count(), 0 );

    QList<DataSync::Fragment*> fragments = parser.takeFragments();


    // Header
    QVERIFY( !fragments.isEmpty() );
    DataSync::Fragment* fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_HEADER );

    HeaderParams* header = static_cast<HeaderParams*>( fragment );

    QCOMPARE( header->sessionID, QString("1230022352") );
    QCOMPARE( header->msgID, 5 );
    QCOMPARE( header->targetDevice, QString("sc-pim-75af2bff-3a5e-4cba-affb-4c555b9ee740"));
    QCOMPARE( header->sourceDevice, QString("http://82.77.123.71:8080/services/syncml"));
    QCOMPARE( header->respURI, QString("http://82.77.123.71:8080/services/syncml?s=RYHmRQAA&srv_id=002"));

    delete header;
    header = NULL;

    // Status
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_STATUS );

    StatusParams* status = static_cast<StatusParams*>( fragment );

    QCOMPARE(status->cmdID,1);
    QCOMPARE(status->data,SUCCESS);
    QCOMPARE(status->msgRef,5);
    QCOMPARE(status->cmdRef,0);
    QCOMPARE(status->cmd,QString(SYNCML_ELEMENT_SYNCHDR));
    QCOMPARE(status->targetRef,QString("http://localhost:8080/funambol/ds/card"));
    QCOMPARE(status->sourceRef,QString("sc-pim-75af2bff-3a5e-4cba-affb-4c555b9ee740"));
    QCOMPARE(status->itemList.at(0).target,QString("note"));
    QCOMPARE(status->itemList.at(0).source,QString("memo"));
    QCOMPARE(status->chal.meta.type,QString("syncml:auth-md5"));
    QCOMPARE(status->chal.meta.format,QString("b64"));
    QCOMPARE(status->chal.meta.nextNonce,QString("ZG9iZWhhdmUNCg=="));

    delete status;
    status = NULL;

    // Alert
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_ALERT );

    AlertParams* alert = static_cast<AlertParams*>( fragment );

    QCOMPARE(alert->cmdID,2);
    QCOMPARE((int)alert->data,201);
    QCOMPARE(alert->targetDatabase,QString("addressbook"));
    QCOMPARE(alert->sourceDatabase,QString("card"));
    QCOMPARE(alert->nextAnchor,QString("1232981790235"));
    QCOMPARE(alert->lastAnchor,QString("1232366487448"));

    delete alert;
    alert = NULL;

    // Sync
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_SYNC );

    SyncParams* sync = static_cast<SyncParams*>( fragment );

    QCOMPARE(sync->cmdID,3);
    QCOMPARE(sync->targetDatabase,QString("addressbook"));
    QCOMPARE(sync->sourceDatabase,QString("card"));
    QCOMPARE( sync->actionList.count(), 3 );

    verifyAdd( sync->actionList.at(0) );
    verifyReplace( sync->actionList.at(1) );
    verifyDelete( sync->actionList.at(2) );

    delete sync;
    sync = NULL;

    // Results
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_RESULTS );

    ResultsParams* results = static_cast<ResultsParams*>( fragment );
    delete results;
    results = NULL;

    // Map
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->iType, Fragment::FRAGMENT_MAP );

    MapParams* map = static_cast<MapParams*>( fragment );

    QCOMPARE(map->cmdID,7);
    QCOMPARE(map->target,QString("http://www.datasync.org/servlet/syncit"));
    QCOMPARE(map->source,QString("IMEI:001004FF1234567"));

    QCOMPARE(map->mapItemList.count(), 3 );
    QCOMPARE(map->mapItemList.at(0).target,QString("./0123456789ABCDEF"));
    QCOMPARE(map->mapItemList.at(0).source,QString("./01"));

    QCOMPARE(map->mapItemList.at(1).target,QString("./0123456789ABCDF0"));
    QCOMPARE(map->mapItemList.at(1).source,QString("./02"));

    QCOMPARE(map->mapItemList.at(2).target,QString("./0123456789ABCDF1"));
    QCOMPARE(map->mapItemList.at(2).source,QString("./03"));

    delete map;
    map = NULL;

    // Final
    QVERIFY( fragments.isEmpty() );

    QList<QVariant> signal1 = parsingSpy.at(0);
    QCOMPARE( signal1.count(), 1 );

    bool final = signal1.at(0).toBool();
    QCOMPARE( final, true );
}

void SyncMLMessageParserTest::testInvalid1()
{

    SyncMLMessageParser parser;
    QSignalSpy parsingSpy( &parser, SIGNAL(parsingComplete(bool)) );
    QSignalSpy errorSpy( &parser, SIGNAL(parsingError(DataSync::ParserError)) );

    QByteArray data;
    QVERIFY( readFile( "testfiles/respinvalid1.txt", data ) );
    QBuffer buffer(&data );
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QVERIFY( readFile( "testfiles/respinvalid2.txt", data ) );
    buffer.setBuffer(&data);
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QVERIFY( readFile( "testfiles/respinvalid3.txt", data ) );
    buffer.setBuffer(&data);
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QVERIFY( readFile( "testfiles/respinvalid4.txt", data ) );
    buffer.setBuffer(&data);
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QVERIFY( readFile( "testfiles/respinvalid5.txt", data ) );
    buffer.setBuffer(&data);
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QVERIFY( readFile( "testfiles/respinvalid6.txt", data ) );
    buffer.setBuffer(&data);
    QVERIFY( buffer.open( QIODevice::ReadOnly ) );
    parser.parseResponse( &buffer, true );
    buffer.close();

    QCOMPARE( parsingSpy.count(), 0 );
    QCOMPARE( errorSpy.count(), 6 );
}

void SyncMLMessageParserTest::verifyAdd( const DataSync::SyncActionData& aData )
{
    QCOMPARE(aData.cmdID,4);
    QCOMPARE(aData.meta.type,QString("text/x-vcard"));
    QCOMPARE(aData.items.count(), 1 );
    QCOMPARE(aData.items[0].source, QString( "0" ) );
    QCOMPARE(aData.items[0].sourceParent, QString( "1" ) );
    QCOMPARE(aData.items[0].Data.simplified(), QString( "BEGIN:VCARD VERSION:2.1 N:Lahtela;Tatu;;; FN:Lahtela, Tatu TEL;TYPE=PREF:+35840 7532165 EMAIL;INTERNET:tatu.lahtela TITLE: ORG:; END:VCARD") );
}

void SyncMLMessageParserTest::verifyReplace( const DataSync::SyncActionData& aData )
{
    QCOMPARE(aData.cmdID,5);
    QCOMPARE(aData.meta.type,QString("x-type/x-subtype"));
    QCOMPARE(aData.items.count(), 1);
    QCOMPARE(aData.items.at(0).target,QString("244"));
    QCOMPARE(aData.items.at(0).targetParent,QString("245"));
    QCOMPARE(aData.items.at(0).Data,QString("ReplaceData"));
}


void SyncMLMessageParserTest::verifyDelete( const DataSync::SyncActionData& aData )
{
    QCOMPARE(aData.cmdID,6);
    QCOMPARE(aData.meta.type,QString("x-type/x-subtype2"));
    QCOMPARE(aData.items.at(0).target,QString("128"));
}

void SyncMLMessageParserTest::testDevInf11()
{
    QByteArray data;
    QVERIFY( readFile( "testfiles/devinf01.txt", data ) );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( 0 );
    SyncMLMessageParser parser;
    parser.iReader.setDevice( &buffer );

    DevInfItemParams params;
    parser.readDevInf( params );
    buffer.close();

    QCOMPARE( parser.iError, PARSER_ERROR_LAST );

    QCOMPARE( params.devInfo.deviceInfo().getManufacturer(), QString( "FooManufacturer" ) );
    QCOMPARE( params.devInfo.deviceInfo().getModel(), QString( "FooModel" ) );
    QCOMPARE( params.devInfo.deviceInfo().getOEM(), QString( "FooOEM" ) );
    QCOMPARE( params.devInfo.deviceInfo().getFirmwareVersion(), QString( "FwVersion" ) );
    QCOMPARE( params.devInfo.deviceInfo().getSoftwareVersion(), QString( "v 21.0.045") );
    QCOMPARE( params.devInfo.deviceInfo().getHardwareVersion(), QString( "HwVersion" ) );
    QCOMPARE( params.devInfo.deviceInfo().getDeviceID(), QString( "IMEI:356064034969473" ) );
    QVERIFY( params.devInfo.getSupportsLargeObjs() );
    QVERIFY( params.devInfo.getSupportsNumberOfChanges() );
    QVERIFY( params.devInfo.getSupportsUTC() );

    const QList<Datastore>& datastores = params.devInfo.datastores();
    QCOMPARE( datastores.count(), 3 );

    QCOMPARE( datastores[0].getSourceURI(), QString( "./Contacts" ) );
    QCOMPARE( datastores[0].getPreferredRx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].getPreferredRx().iVersion, QString( "2.1" ) );
    QCOMPARE( datastores[0].rx().count(), 1 );
    QCOMPARE( datastores[0].rx().at(0).iType, QString( "text/vcard" ) );
    QCOMPARE( datastores[0].rx().at(0).iVersion, QString( "3.0" ) );
    QCOMPARE( datastores[0].getPreferredTx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].getPreferredTx().iVersion, QString( "2.1" ) );
    QVERIFY( !datastores[0].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[0].tx().count(), 0 );
    QCOMPARE( datastores[0].syncCaps().count(), 5 );
    QVERIFY( datastores[0].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[0].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[0].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[0].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[0].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );
    QCOMPARE( datastores[0].ctCaps().count(), 1 );
    QCOMPARE( datastores[0].ctCaps().at(0).getFormat().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().count(), 28 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iName, QString( "TEL" ) );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.count(), 1 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.at(0).iValues.count(), 8 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.at(0).iValues.at(7), QString( "VIDEO") );

    QCOMPARE( datastores[1].getSourceURI(), QString( "./Calendar" ) );
    QCOMPARE( datastores[1].getPreferredRx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[1].rx().count(), 0 );
    QCOMPARE( datastores[1].getPreferredTx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[1].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[1].tx().count(), 0 );
    QCOMPARE( datastores[1].syncCaps().count(), 5 );
    QVERIFY( datastores[1].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[1].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[1].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[1].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[1].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );
    QCOMPARE( datastores[1].ctCaps().count(), 1 );
    QCOMPARE( datastores[1].ctCaps().at(0).getFormat().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].ctCaps().at(0).properties().count(), 35 );

    QCOMPARE( datastores[2].getSourceURI(), QString( "./Notepad" ) );
    QCOMPARE( datastores[2].getPreferredRx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].rx().count(), 1 );
    QCOMPARE( datastores[2].rx().at(0).iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].rx().at(0).iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].getPreferredTx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[2].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[2].tx().count(), 0 );
    QCOMPARE( datastores[2].syncCaps().count(), 5 );
    QVERIFY( datastores[2].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[2].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[2].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[2].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[2].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );
    QCOMPARE( datastores[2].ctCaps().count(), 1 );
    QCOMPARE( datastores[2].ctCaps().at(0).getFormat().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().count(), 1 );
    QVERIFY( datastores[2].ctCaps().at(0).properties().at(0).iName.isEmpty() );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iType, QString( "chr" ) );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iSize, 0 );

}

void SyncMLMessageParserTest::testDevInf12()
{
    QByteArray data;
    QVERIFY( readFile( "testfiles/devinf02.txt", data ) );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( 0 );
    SyncMLMessageParser parser;
    parser.iReader.setDevice( &buffer );

    DevInfItemParams params;
    parser.readDevInf( params );
    buffer.close();

    QCOMPARE( parser.iError, PARSER_ERROR_LAST );

    QCOMPARE( params.devInfo.deviceInfo().getManufacturer(), QString( "FooManufacturer" ) );
    QCOMPARE( params.devInfo.deviceInfo().getModel(), QString( "FooModel" ) );
    QCOMPARE( params.devInfo.deviceInfo().getOEM(), QString( "FooOEM" ) );
    QCOMPARE( params.devInfo.deviceInfo().getFirmwareVersion(), QString( "FwVersion" ) );
    QCOMPARE( params.devInfo.deviceInfo().getSoftwareVersion(), QString( "v 21.0.045") );
    QCOMPARE( params.devInfo.deviceInfo().getHardwareVersion(), QString( "HwVersion" ) );
    QCOMPARE( params.devInfo.deviceInfo().getDeviceID(), QString( "IMEI:356064034969473" ) );
    QVERIFY( params.devInfo.getSupportsLargeObjs() );
    QVERIFY( params.devInfo.getSupportsNumberOfChanges() );
    QVERIFY( params.devInfo.getSupportsUTC() );

    const QList<Datastore>& datastores = params.devInfo.datastores();
    QCOMPARE( datastores.count(), 3 );

    QCOMPARE( datastores[0].getSourceURI(), QString( "./Contacts" ) );
    QCOMPARE( datastores[0].getPreferredRx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].getPreferredRx().iVersion, QString( "2.1" ) );
    QCOMPARE( datastores[0].rx().count(), 1 );
    QCOMPARE( datastores[0].rx().at(0).iType, QString( "text/vcard" ) );
    QCOMPARE( datastores[0].rx().at(0).iVersion, QString( "3.0" ) );
    QCOMPARE( datastores[0].getPreferredTx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].getPreferredTx().iVersion, QString( "2.1" ) );
    QVERIFY( datastores[0].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[0].tx().count(), 0 );
    QCOMPARE( datastores[0].syncCaps().count(), 5 );
    QVERIFY( datastores[0].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[0].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[0].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[0].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[0].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );

    QCOMPARE( datastores[0].ctCaps().count(), 1 );
    QCOMPARE( datastores[0].ctCaps().at(0).getFormat().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().count(), 24 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iName, QString( "TEL" ) );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.count(), 1 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.at(0).iName, QString( "TYPE") );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.at(0).iValues.count(), 8 );
    QCOMPARE( datastores[0].ctCaps().at(0).properties().at(6).iParameters.at(0).iValues.at(7), QString( "CAR") );

    QCOMPARE( datastores[1].getSourceURI(), QString( "./Calendar" ) );
    QCOMPARE( datastores[1].getPreferredRx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[1].rx().count(), 0 );
    QCOMPARE( datastores[1].getPreferredTx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[1].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[1].tx().count(), 0 );
    QCOMPARE( datastores[1].syncCaps().count(), 5 );
    QVERIFY( datastores[1].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[1].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[1].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[1].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[1].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );

    QCOMPARE( datastores[1].ctCaps().count(), 1 );
    QCOMPARE( datastores[1].ctCaps().at(0).getFormat().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].ctCaps().at(0).properties().count(), 25 );

    QCOMPARE( datastores[2].getSourceURI(), QString( "./Notepad" ) );
    QCOMPARE( datastores[2].getPreferredRx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].rx().count(), 1 );
    QCOMPARE( datastores[2].rx().at(0).iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].rx().at(0).iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].getPreferredTx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( datastores[2].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[2].tx().count(), 0 );
    QCOMPARE( datastores[2].syncCaps().count(), 5 );
    QVERIFY( datastores[2].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[2].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[2].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[2].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[2].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );

    QCOMPARE( datastores[2].ctCaps().count(), 1 );
    QCOMPARE( datastores[2].ctCaps().at(0).getFormat().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().count(), 1 );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iName, QString( "Note" ) );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iType, QString( "chr" ) );
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iSize, 0 );

}

TESTLOADER_ADD_TEST(SyncMLMessageParserTest);
