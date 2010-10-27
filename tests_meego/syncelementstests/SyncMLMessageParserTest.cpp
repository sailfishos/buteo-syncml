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
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_HEADER );

    HeaderParams* header = static_cast<HeaderParams*>( fragment );

    QCOMPARE( header->sessionID, QString("1230022352") );
    QCOMPARE( header->msgID, 5 );
    QCOMPARE( header->targetDevice, QString("sc-pim-75af2bff-3a5e-4cba-affb-4c555b9ee740"));
    QCOMPARE( header->sourceDevice, QString("http://82.77.123.71:8080/services/syncml"));
    QCOMPARE( header->respURI, QString("http://82.77.123.71:8080/services/syncml?s=RYHmRQAA&srv_id=002"));

    delete header;
    header = NULL;

    // Results
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_RESULTS );

    ResultsParams* results = static_cast<ResultsParams*>( fragment );
    delete results;
    results = NULL;

    // Status
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_STATUS );

    StatusParams* status = static_cast<StatusParams*>( fragment );

    QCOMPARE(status->cmdId,1);
    QCOMPARE(status->data,SUCCESS);
    QCOMPARE(status->msgRef,5);
    QCOMPARE(status->cmdRef,0);
    QCOMPARE(status->cmd,QString(SYNCML_ELEMENT_SYNCHDR));
    QCOMPARE(status->targetRef,QString("http://localhost:8080/funambol/ds/card"));
    QCOMPARE(status->sourceRef,QString("sc-pim-75af2bff-3a5e-4cba-affb-4c555b9ee740"));
    QVERIFY(!status->items.isEmpty());
    QCOMPARE(status->items.at(0).target,QString("note"));
    QCOMPARE(status->items.at(0).source,QString("memo"));
    QCOMPARE(status->chal.meta.type,QString("syncml:auth-md5"));
    QCOMPARE(status->chal.meta.format,QString("b64"));
    QCOMPARE(status->chal.meta.nextNonce,QString("ZG9iZWhhdmUNCg=="));

    delete status;
    status = NULL;

    // Alert
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_COMMAND );

    CommandParams* command = static_cast<CommandParams*>( fragment );
    QVERIFY( command->commandType == CommandParams::COMMAND_ALERT );

    QCOMPARE(command->cmdId,2);
    QCOMPARE(command->data,QString( "201" ) );
    QVERIFY(!command->items.isEmpty());
    QCOMPARE(command->items.first().target,QString("addressbook"));
    QCOMPARE(command->items.first().source,QString("card"));
    QCOMPARE(command->items.first().meta.anchor.next,QString("1232981790235"));
    QCOMPARE(command->items.first().meta.anchor.last,QString("1232366487448"));

    delete command;
    command = NULL;

    // Sync
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_SYNC );

    SyncParams* sync = static_cast<SyncParams*>( fragment );

    QCOMPARE(sync->cmdId,3);
    QCOMPARE(sync->target,QString("addressbook"));
    QCOMPARE(sync->source,QString("card"));
    QCOMPARE( sync->commands.count(), 3 );

    verifyAdd( sync->commands.at(0) );
    verifyReplace( sync->commands.at(1) );
    verifyDelete( sync->commands.at(2) );

    delete sync;
    sync = NULL;

    // Map
    QVERIFY( !fragments.isEmpty() );
    fragment = fragments.takeFirst();
    QCOMPARE( fragment->fragmentType, Fragment::FRAGMENT_MAP );

    MapParams* map = static_cast<MapParams*>( fragment );

    QCOMPARE(map->cmdId,7);
    QCOMPARE(map->target,QString("http://www.datasync.org/servlet/syncit"));
    QCOMPARE(map->source,QString("IMEI:001004FF1234567"));

    QCOMPARE(map->mapItems.count(), 3 );
    QCOMPARE(map->mapItems.at(0).target,QString("./0123456789ABCDEF"));
    QCOMPARE(map->mapItems.at(0).source,QString("./01"));

    QCOMPARE(map->mapItems.at(1).target,QString("./0123456789ABCDF0"));
    QCOMPARE(map->mapItems.at(1).source,QString("./02"));

    QCOMPARE(map->mapItems.at(2).target,QString("./0123456789ABCDF1"));
    QCOMPARE(map->mapItems.at(2).source,QString("./03"));

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

void SyncMLMessageParserTest::verifyAdd( const DataSync::CommandParams& aData )
{
    QCOMPARE(aData.cmdId,4);
    QCOMPARE(aData.meta.type,QString("text/x-vcard"));
    QCOMPARE(aData.items.count(), 1 );
    QCOMPARE(aData.items[0].source, QString( "0" ) );
    QCOMPARE(aData.items[0].sourceParent, QString( "1" ) );
    QCOMPARE(aData.items[0].data.simplified(), QString( "BEGIN:VCARD VERSION:2.1 N:Lahtela;Tatu;;; FN:Lahtela, Tatu TEL;TYPE=PREF:+35840 7532165 EMAIL;INTERNET:tatu.lahtela TITLE: ORG:; END:VCARD") );
}

void SyncMLMessageParserTest::verifyReplace( const DataSync::CommandParams& aData )
{
    QCOMPARE(aData.cmdId,5);
    QCOMPARE(aData.meta.type,QString("x-type/x-subtype"));
    QCOMPARE(aData.items.count(), 1);
    QCOMPARE(aData.items.at(0).target,QString("244"));
    QCOMPARE(aData.items.at(0).targetParent,QString("245"));
    QCOMPARE(aData.items.at(0).data,QString("ReplaceData"));
}


void SyncMLMessageParserTest::verifyDelete( const DataSync::CommandParams& aData )
{
    QCOMPARE(aData.cmdId,6);
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

    const DeviceInfo& devInf = params.devInfo.deviceInfo();
    QCOMPARE( devInf.getManufacturer(), QString( "FooManufacturer" ) );
    QCOMPARE( devInf.getModel(), QString( "FooModel" ) );
    QCOMPARE( devInf.getOEM(), QString( "FooOEM" ) );
    QCOMPARE( devInf.getFirmwareVersion(), QString( "FwVersion" ) );
    QCOMPARE( devInf.getSoftwareVersion(), QString( "v 21.0.045") );
    QCOMPARE( devInf.getHardwareVersion(), QString( "HwVersion" ) );
    QCOMPARE( devInf.getDeviceID(), QString( "IMEI:356064034969473" ) );
    QVERIFY( params.devInfo.getSupportsLargeObjs() );
    QVERIFY( params.devInfo.getSupportsNumberOfChanges() );
    QVERIFY( params.devInfo.getSupportsUTC() );

    const QList<Datastore>& datastores = params.devInfo.datastores();
    QCOMPARE( datastores.count(), 3 );

    QCOMPARE( datastores[0].getSourceURI(), QString( "./Contacts" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredRx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredRx().iVersion, QString( "2.1" ) );
    QCOMPARE( datastores[0].formatInfo().rx().count(), 1 );
    QCOMPARE( datastores[0].formatInfo().rx().at(0).iType, QString( "text/vcard" ) );
    QCOMPARE( datastores[0].formatInfo().rx().at(0).iVersion, QString( "3.0" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredTx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredTx().iVersion, QString( "2.1" ) );
    QVERIFY( !datastores[0].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[0].formatInfo().tx().count(), 0 );
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
    QCOMPARE( datastores[1].formatInfo().getPreferredRx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].formatInfo().getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[1].formatInfo().rx().count(), 0 );
    QCOMPARE( datastores[1].formatInfo().getPreferredTx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].formatInfo().getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[1].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[1].formatInfo().tx().count(), 0 );
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
    QCOMPARE( datastores[2].formatInfo().getPreferredRx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].formatInfo().rx().count(), 1 );
    QCOMPARE( datastores[2].formatInfo().rx().at(0).iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().rx().at(0).iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredTx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[2].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[2].formatInfo().tx().count(), 0 );
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
    QCOMPARE( datastores[0].formatInfo().getPreferredRx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredRx().iVersion, QString( "2.1" ) );
    QCOMPARE( datastores[0].formatInfo().rx().count(), 1 );
    QCOMPARE( datastores[0].formatInfo().rx().at(0).iType, QString( "text/vcard" ) );
    QCOMPARE( datastores[0].formatInfo().rx().at(0).iVersion, QString( "3.0" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredTx().iType, QString( "text/x-vcard" ) );
    QCOMPARE( datastores[0].formatInfo().getPreferredTx().iVersion, QString( "2.1" ) );
    QVERIFY( datastores[0].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[0].formatInfo().tx().count(), 0 );
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
    QCOMPARE( datastores[1].formatInfo().getPreferredRx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].formatInfo().getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[1].formatInfo().rx().count(), 0 );
    QCOMPARE( datastores[1].formatInfo().getPreferredTx().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].formatInfo().getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( !datastores[1].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[1].formatInfo().tx().count(), 0 );
    QCOMPARE( datastores[1].syncCaps().count(), 5 );
    QVERIFY( datastores[1].syncCaps().at(0) == SYNCTYPE_TWOWAY );
    QVERIFY( datastores[1].syncCaps().at(1) == SYNCTYPE_TWOWAYSLOW );
    QVERIFY( datastores[1].syncCaps().at(2) == SYNCTYPE_FROMCLIENT );
    QVERIFY( datastores[1].syncCaps().at(3) == SYNCTYPE_FROMSERVER );
    QVERIFY( datastores[1].syncCaps().at(4) == SYNCTYPE_SERVERALERTED );

    QCOMPARE( datastores[1].ctCaps().count(), 1 );
    QCOMPARE( datastores[1].ctCaps().at(0).getFormat().iType, QString( "text/x-vcalendar" ) );
    QCOMPARE( datastores[1].ctCaps().at(0).properties().count(), 25 );
    QVERIFY( !datastores[1].ctCaps().at(0).properties().at(0).iNoTruncate );

    QCOMPARE( datastores[2].getSourceURI(), QString( "./Notepad" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredRx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredRx().iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].formatInfo().rx().count(), 1 );
    QCOMPARE( datastores[2].formatInfo().rx().at(0).iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().rx().at(0).iVersion, QString( "1.0" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredTx().iType, QString( "text/plain" ) );
    QCOMPARE( datastores[2].formatInfo().getPreferredTx().iVersion, QString( "1.0" ) );
    QVERIFY( datastores[2].getSupportsHierarchicalSync() );
    QCOMPARE( datastores[2].formatInfo().tx().count(), 0 );
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
    QCOMPARE( datastores[2].ctCaps().at(0).properties().at(0).iMaxOccur, 5 );
    QVERIFY( datastores[2].ctCaps().at(0).properties().at(0).iNoTruncate );

}

void SyncMLMessageParserTest::testSubcommands()
{
    QByteArray data;
    QVERIFY( readFile( "testfiles/subcommands01.txt", data ) );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( 0 );
    SyncMLMessageParser parser;

    parser.parseResponse( &buffer, true );
    QCOMPARE( parser.iError, PARSER_ERROR_LAST );

    QList<Fragment*> fragments = parser.takeFragments();

    QCOMPARE( fragments.count(), 6 );

    QVERIFY( fragments[0]->fragmentType == Fragment::FRAGMENT_HEADER );

    QVERIFY( fragments[1]->fragmentType == Fragment::FRAGMENT_COMMAND );
    QVERIFY( static_cast<CommandParams*>(fragments[1])->commandType == CommandParams::COMMAND_ALERT );

    QVERIFY( fragments[2]->fragmentType == Fragment::FRAGMENT_COMMAND );
    CommandParams* get = static_cast<CommandParams*>(fragments[2]);
    QVERIFY( get->commandType == CommandParams::COMMAND_GET );

    QVERIFY( fragments[3]->fragmentType == Fragment::FRAGMENT_COMMAND );
    CommandParams* atomic = static_cast<CommandParams*>(fragments[3]);
    QVERIFY( atomic->commandType == CommandParams::COMMAND_ATOMIC );

    QCOMPARE( atomic->subCommands.count(), 2 );
    QVERIFY( atomic->subCommands[0].commandType == CommandParams::COMMAND_ALERT );
    QVERIFY( atomic->subCommands[1].commandType == CommandParams::COMMAND_ADD );

    QVERIFY( fragments[4]->fragmentType == Fragment::FRAGMENT_COMMAND );
    CommandParams* sequence = static_cast<CommandParams*>(fragments[4]);
    QVERIFY( sequence->commandType == CommandParams::COMMAND_SEQUENCE );

    QCOMPARE( sequence->subCommands.count(), 2 );
    QVERIFY( sequence->subCommands[0].commandType == CommandParams::COMMAND_ALERT );
    QVERIFY( sequence->subCommands[1].commandType == CommandParams::COMMAND_ADD );

    QVERIFY( fragments[5]->fragmentType == Fragment::FRAGMENT_SYNC );
    SyncParams* sync = static_cast<SyncParams*>(fragments[5]);

    QCOMPARE( sync->commands.count(), 3 );
    QVERIFY( sync->commands[0].commandType == CommandParams::COMMAND_ADD );
    QVERIFY( sync->commands[1].commandType == CommandParams::COMMAND_ADD );
    QVERIFY( sync->commands[2].commandType == CommandParams::COMMAND_ATOMIC );

    atomic = &sync->commands[2];

    QCOMPARE( atomic->subCommands.count(), 2 );
    QVERIFY( atomic->subCommands[0].commandType == CommandParams::COMMAND_ADD );
    QVERIFY( atomic->subCommands[1].commandType == CommandParams::COMMAND_ADD );

    qDeleteAll(fragments);
    fragments.clear();
}

void SyncMLMessageParserTest::testEmbeddedXML()
{
    QByteArray data;
    QVERIFY( readFile( "testfiles/resp2.txt", data ) );
    QBuffer buffer( &data );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( 0 );
    SyncMLMessageParser parser;
    const QString expected( "<Anchor xmlns=\"syncml:metinf\"><Next>276</Next></Anchor>" );

    parser.parseResponse( &buffer, true );
    QCOMPARE( parser.iError, PARSER_ERROR_LAST );

    QList<Fragment*> fragments = parser.takeFragments();

    QCOMPARE( fragments.count(), 2 );

    QVERIFY( fragments[0]->fragmentType == Fragment::FRAGMENT_HEADER );

    QVERIFY( fragments[1]->fragmentType == Fragment::FRAGMENT_STATUS );
    StatusParams* status = static_cast<StatusParams*>( fragments[1] );
    QCOMPARE( status->items.count(), 1 );
    QCOMPARE( status->items.first().data, expected );
}

TESTLOADER_ADD_TEST(SyncMLMessageParserTest);
