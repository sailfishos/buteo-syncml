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

#include "SessionHandlerTest.h"

#include <QSignalSpy>

#include "SessionHandler.h"
#include "ClientSessionHandler.h"
#include "ServerSessionHandler.h"
#include "Mock.h"
#include "SyncAgent.h"
#include "TestUtils.h"
#include "ServerAlertedNotification.h"
#include "SyncAgentConfigProperties.h"
#include "SyncCommonDefs.h"


using namespace DataSync;

QString DBFILE( "/tmp/sessionhandlertest.db" );
QString NB153701UNKNOWNDEVICE( "/" );
QString NB153701SOURCEDEVICE( "IMEI:000000000000000" );
QString NB153701TARGETDEVICE( "IMEI:000000000000001" );
QString NB153701FORCEDEVICE( "IMEI:000000000000002" );




bool SessionHandlerTest::getStorageContentFormatInfo( const QString& aURI,
                                                      StorageContentFormatInfo& aInfo )
{
    Q_UNUSED( aURI );
    MockStorage storage( "storage" );
    aInfo = storage.getFormatInfo();
    return true;
}

StoragePlugin* SessionHandlerTest::acquireStorageByURI( const QString& /*aURI*/ )
{
    return new MockStorage( "storage" );
}

StoragePlugin* SessionHandlerTest::acquireStorageByMIME( const QString& /*aMIME*/ )
{
    return new MockStorage( "storage" );
}

void SessionHandlerTest::releaseStorage( StoragePlugin* aStorage )
{
    delete aStorage;
}

void SessionHandlerTest::init()
{
    QFile::remove( DBFILE );
}

void SessionHandlerTest::cleanup()
{
    QFile::remove( DBFILE );
}

void SessionHandlerTest::testClientWithClientInitiated()
{
    TestTransport transport( false );
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_BASIC, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    // First message has been sent to the server.
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "Source device";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->targetDevice = SYNCML_UNKNOWN_DEVICE;
    hp1->respURI = "redirect URI";
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = AUTH_ACCEPTED;
    session_handler.handleStatusElement( sp1 );

    // Fake alert.
    CommandParams* ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 2;
    ap1->data = QString::number( SLOW_SYNC );
    ItemParams item;
    item.source = DB;
    item.target = DB;
    item.meta.anchor.next = "something";
    ap1->items.append(item);
    session_handler.handleAlertElement(ap1);
    ap1 = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    // Step through different states.
    session_handler.handleEndOfMessage();
    SyncParams* sync = new SyncParams();
    sync->cmdId = 1;
    sync->source = DB;
    sync->target = DB;
    session_handler.handleSyncElement( sync );
    QCOMPARE(session_handler.getSyncState(), RECEIVING_ITEMS);

    CommandParams* get = new CommandParams( CommandParams::COMMAND_GET );
    get->cmdId = 1;
    session_handler.handleGetElement(get);
    get = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_MAPPINGS);
    session_handler.handleEndOfMessage();

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), FINALIZING);

    session_handler.handleEndOfMessage();
    QCOMPARE(session_handler.getSyncState(), SYNC_FINISHED);
}


void SessionHandlerTest::testNoRespSyncElement()
{
    TestTransport transport( false );
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_BASIC, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "Source device";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->targetDevice = SYNCML_UNKNOWN_DEVICE;
    hp1->respURI = "redirect URI";
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = AUTH_ACCEPTED;
    session_handler.handleStatusElement( sp1 );

    // Fake alert.
    CommandParams* ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 2;
    ap1->data = QString::number( SLOW_SYNC );
    ItemParams item;
    item.source = DB;
    item.target = DB;
    item.meta.anchor.next = "something";
    ap1->items.append(item);
    session_handler.handleAlertElement(ap1);
    ap1 = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    // Step through different states.
    session_handler.handleEndOfMessage();
    SyncParams* sync = new SyncParams();
    sync->cmdId = 1;
    sync->source = DB;
    sync->target = DB;
    sync->noResp = true;
    session_handler.handleSyncElement( sync );
    QCOMPARE(session_handler.getSyncState(), RECEIVING_ITEMS);

    CommandParams* get = new CommandParams( CommandParams::COMMAND_GET );
    get->cmdId = 1;
    session_handler.handleGetElement(get);
    get = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_MAPPINGS);
    session_handler.handleEndOfMessage();

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), FINALIZING);

    session_handler.handleEndOfMessage();
    QCOMPARE(session_handler.getSyncState(), SYNC_FINISHED);

}

void SessionHandlerTest::testClientWithServerInitiated()
{
    MockTransport transport("transport");
    const QString DB = "contacts";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.setSyncParams( "", SYNCML_1_2, SyncMode(DIRECTION_FROM_CLIENT, INIT_SERVER) );
    config.addSyncTarget( "contacts", DB );
    config.setDatabaseFilePath( DBFILE );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);

    QList<Fragment*> fragments;
    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "Source device";
    hp1->sessionID = "1";
    hp1->msgID = 1;

    fragments.append(hp1);

    // Fake alert.
    CommandParams* ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 1;
    ap1->data = QString::number( ONE_WAY_FROM_CLIENT_BY_SERVER );
    ItemParams item1;
    item1.meta.type = "text/x-vcard";
    item1.source = DB;
    item1.target = DB;
    item1.meta.anchor.next = "kala";
    ap1->items.append(item1);
    fragments.append(ap1);

    session_handler.handleNotificationXML( fragments );

    QVERIFY( fragments.isEmpty() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    PutParams* put = new PutParams;
    put->cmdId = 1;
    fragments.append(put);

    // Fake alert.
    ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 1;
    ap1->data = QString::number( SLOW_SYNC );
    ItemParams item2;
    item2.source = DB;
    item2.target = DB;
    item2.meta.anchor.next = "something";
    ap1->items.append(item2);
    fragments.append(ap1);

    session_handler.processMessage( fragments, true );
    QVERIFY( fragments.isEmpty() );

    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    session_handler.processMessage( fragments, true );

    // One way sync, no RECEIVING_ITEMS state.
    QCOMPARE(session_handler.getSyncState(), SYNC_FINISHED);

}

void SessionHandlerTest::testServerWithClientInitiated()
{

    MockTransport transport( "transport" );

    SyncAgentConfig config;
    config.setTransport( &transport );
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar" );
    config.setDatabaseFilePath( DBFILE );
    config.setLocalDeviceName( "Local device" );

    // Prepare
    ServerSessionHandler sessionHandler( &config, NULL );
    QVERIFY(sessionHandler.prepareSync());
    QCOMPARE( sessionHandler.getSyncState(), PREPARED );

    // Start.
    QVERIFY(sessionHandler.prepareSync());
    QCOMPARE( sessionHandler.getSyncState(), PREPARED );

    // Fake header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "Source device";
    hp1->targetDevice = "Target device";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    sessionHandler.handleHeaderElement( hp1 );
    hp1 = NULL;

    // Fake alert
    CommandParams* ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 1;
    ap1->data = QString::number( SLOW_SYNC );
    ItemParams item;
    item.source = "calendar";
    item.target = "calendar";
    item.meta.anchor.next = "nextAnchor";
    ap1->items.append(item);

    sessionHandler.handleAlertElement( ap1 );
    ap1 = NULL;

    QCOMPARE( sessionHandler.getSyncState(), REMOTE_INIT );

    sessionHandler.handleFinal();
    QCOMPARE( sessionHandler.getSyncState(), LOCAL_INIT );

    sessionHandler.handleEndOfMessage();

    SyncParams* sync = new SyncParams();
    sync->cmdId = 1;
    sync->source = "calendar";
    sync->target = "calendar";

    sessionHandler.handleSyncElement( sync );
    sync = NULL;

    QCOMPARE( sessionHandler.getSyncState(), RECEIVING_ITEMS );

    sessionHandler.handleFinal();
    QCOMPARE( sessionHandler.getSyncState(), SENDING_ITEMS );

    sessionHandler.handleEndOfMessage();

    sessionHandler.handleFinal();
    QCOMPARE( sessionHandler.getSyncState(), FINALIZING );

    sessionHandler.handleEndOfMessage();
    QCOMPARE( sessionHandler.getSyncState(), SYNC_FINISHED );

}

void SessionHandlerTest::testClientWithServerInitiatedSAN()
{
    MockTransport transport("transport");
    const QString DB = "contacts";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "Contacts", DB );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "", SYNCML_1_2, SyncMode(DIRECTION_FROM_CLIENT, INIT_SERVER) );

    QStringList mappings;
    mappings << "Contacts" << "text/x-vcard";
    config.setExtension( SANMAPPINGSEXTENSION, QVariant( mappings ) );

    QByteArray message;
    QVERIFY( readFile( "data/SAN01.bin", message ) );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);

    // Fake SAN

    SANHandler parser;
    SANDS SANdata;

    QVERIFY( parser.parseSANMessageDS( message, SANdata ) );

    session_handler.handleNotificationPackage( SANdata );

    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    // Fake alert.
    CommandParams* ap1 = new CommandParams( CommandParams::COMMAND_ALERT );
    ap1->cmdId = 2;
    ap1->data = QString::number( SLOW_SYNC );
    ItemParams item;
    item.source = DB;
    item.target = DB;
    item.meta.anchor.next = "something";
    ap1->items.append( item );
    session_handler.handleAlertElement(ap1);
    ap1 = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    // Step through different states.
    session_handler.handleEndOfMessage();
    SyncParams* sync = new SyncParams();
    sync->cmdId = 1;
    sync->source = DB;
    sync->target = DB;
    session_handler.handleSyncElement( sync );
    QCOMPARE(session_handler.getSyncState(), RECEIVING_ITEMS);

    CommandParams* get = new CommandParams( CommandParams::COMMAND_GET );
    get->cmdId = 1;
    session_handler.handleGetElement(get);
    get = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_MAPPINGS);
    session_handler.handleEndOfMessage();

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), FINALIZING);

    session_handler.handleEndOfMessage();
    QCOMPARE(session_handler.getSyncState(), SYNC_FINISHED);

}

void SessionHandlerTest::testClientAuthNone()
{
    // Test that no authentication info is sent if authentication type
    // is AUTH_NONE, even if credenticals would be supplied

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_NONE, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray data = transport.iData;

    QVERIFY( !data.contains( SYNCML_ELEMENT_CRED ) );

}

void SessionHandlerTest::testClientAuthNoneFailed()
{
    // Test that session is aborted if authentication type is AUTH_NONE
    // and we receive 401 for authentication

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setLocalDeviceName( "client" );
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_NONE, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray data = transport.iData;

    QVERIFY( !data.contains( SYNCML_ELEMENT_CRED ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "client";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE( session_handler.getSyncState(), AUTHENTICATION_FAILURE );
}

void SessionHandlerTest::testClientAuthNoneChal1()
{
    // Test that session is aborted if authentication type is AUTH_NONE
    // and we receive 407 for authentication with a challenge for
    // basic auth

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setLocalDeviceName( "client" );
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_NONE, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray data = transport.iData;

    QVERIFY( !data.contains( SYNCML_ELEMENT_CRED ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "client";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_BASIC;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE( session_handler.getSyncState(), AUTHENTICATION_FAILURE );
}

void SessionHandlerTest::testClientAuthNoneChal2()
{
    // Test that session is aborted if authentication type is AUTH_NONE
    // and we receive 407 for authentication with a challenge without
    // unspecified type (should default to basic auth)

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setLocalDeviceName( "client" );
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_NONE, "user", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray data = transport.iData;

    QVERIFY( !data.contains( SYNCML_ELEMENT_CRED ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "client";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE( session_handler.getSyncState(), AUTHENTICATION_FAILURE );
}

void SessionHandlerTest::testClientAuthBasicNoCreds()
{
    // Test that basic authentication fails if no credentials are
    // supplied

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_BASIC, "", "" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), AUTHENTICATION_FAILURE);

}


void SessionHandlerTest::testClientAuthBasic()
{
    // Test that basic authentication is processed properly when
    // server accepts the credentials

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_BASIC, "userid", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_BASIC ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );
    QVERIFY( message.contains( "dXNlcmlkOnBhc3N3b3Jk" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "client";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = AUTH_ACCEPTED;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( session_handler.authentication().authedToRemote() );
}

void SessionHandlerTest::testClientAuthBasicChalToMD5WithoutNonce()
{
    // Test that session is aborted if we try to use basic auth but
    // get challenged with MD5 without us having a nonce

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthBasicChalToMD5WithoutNonce" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_BASIC, "userid", "password" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_BASIC ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );
    QVERIFY( message.contains( "dXNlcmlkOnBhc3N3b3Jk" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "client";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = MISSING_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_MD5;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), AUTHENTICATION_FAILURE);
}

void SessionHandlerTest::testClientAuthBasicChalToMD5WithNonce()
{
    // Test that reauthentication is attempted if basic authentication
    // gets rejected, but MD5 auth challenge is issued with a nonce

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthBasicChalToMD5WithNonce" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_BASIC, "Bruce2", "OhBehave" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_BASIC ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );
    QVERIFY( message.contains( "QnJ1Y2UyOk9oQmVoYXZl" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthBasicChalToMD5WithNonce";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = MISSING_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_MD5;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    sp1->chal.meta.nextNonce = "Tm9uY2U="; // "Nonce" in B64
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );

    session_handler.handleFinal();
    session_handler.handleEndOfMessage();
    QCOMPARE( session_handler.getSyncState(), LOCAL_INIT );

    message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_MD5 ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with nonce "Nonce"
    QVERIFY( message.contains( "Zz6EivR3yeaaENcRN6lpAQ==" ) );

}

void SessionHandlerTest::testClientAuthBasicContinuous()
{

    // Test that basic authentication information is continued to be sent
    // if server responds with 200.

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QSKIP( "Unimplemented!" );
#else
    QSKIP( "Unimplemented!", SkipAll );
#endif
}

void SessionHandlerTest::testClientAuthMD5NoCreds()
{
    // Test that MD5 authentication fails if no credentials are
    // supplied

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setAuthParams( AUTH_MD5, "", "" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), AUTHENTICATION_FAILURE);

}

void SessionHandlerTest::testClientAuthMD5WithoutNonce1()
{
    // Check that if we don't have a known nonce & MD5 is to be used,
    // that we first try to send message by using an empty nonce. If it is rejected
    // without sending NextNonce back to us in MD5 challenge, session should abort

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthMD5WithoutNonce1" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_MD5, "Bruce2", "OhBehave" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_MD5 ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with a null nonce
    QVERIFY( message.contains( "lEf25jyjHbCcTbj4p/1pUw==" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthMD5WithoutNonce1";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_MD5;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), AUTHENTICATION_FAILURE);
}

void SessionHandlerTest::testClientAuthMD5WithoutNonce2()
{
    // Check that if we don't have a known nonce & MD5 is to be used,
    // that we first try to send message by using an empty nonce. If basic challenge
    // comes, session should abort

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthMD5WithoutNonce2" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_MD5, "Bruce2", "OhBehave" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_MD5 ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with a null nonce
    QVERIFY( message.contains( "lEf25jyjHbCcTbj4p/1pUw==" ) );


    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthMD5WithoutNonce2";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_BASIC;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), AUTHENTICATION_FAILURE);
}

void SessionHandlerTest::testClientAuthMD5WithoutNonce3()
{
    // Check that if we don't have a known nonce & MD5 is to be used,
    // that we first try to send message by using an empty nonce. If MD5 challenge
    // comes with a nonce, continue the session

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthMD5WithoutNonce3" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_MD5, "Bruce2", "OhBehave" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_MD5 ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with a null nonce
    QVERIFY( message.contains( "lEf25jyjHbCcTbj4p/1pUw==" ) );


    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthMD5WithoutNonce3";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_MD5;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    sp1->chal.meta.nextNonce = "Tm9uY2U=";
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );

    session_handler.handleFinal();
    session_handler.handleEndOfMessage();
    QCOMPARE( session_handler.getSyncState(), LOCAL_INIT );

    message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );
    QVERIFY( message.contains( SYNCML_FORMAT_AUTH_MD5 ) );
    QVERIFY( message.contains( SYNCML_FORMAT_ENCODING_B64 ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with nonce "Nonce"
    QVERIFY( message.contains( "Zz6EivR3yeaaENcRN6lpAQ==" ) );

}

void SessionHandlerTest::testClientAuthMD5WithNonce()
{
    // Test that MD5 authentication is processed properly when
    // server accepts the credentials

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthMD5WithNonce" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_MD5, "Bruce2", "OhBehave", "Nonce" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with nonce "Nonce"
    QVERIFY( message.contains( "Zz6EivR3yeaaENcRN6lpAQ==" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthMD5WithNonce";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = SUCCESS;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( session_handler.authentication().authedToRemote() );
    QCOMPARE( session_handler.getSyncState(), LOCAL_INIT );
}

void SessionHandlerTest::testClientAuthMD5ChalToBasic()
{
    // Test that session is aborted if we want to use MD5 authentication and
    // server responds with challenge for basic

    TestTransport transport(false);
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setLocalDeviceName( "testClientAuthMD5ChalToBasic" );
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath( DBFILE );

    config.setSyncParams( "server", SYNCML_1_2, SyncMode() );
    config.setAuthParams( AUTH_MD5, "Bruce2", "OhBehave", "Nonce" );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);
    session_handler.initiateSync();

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    QByteArray message = transport.iData;
    QVERIFY( message.contains( SYNCML_ELEMENT_CRED ) );

    // MD5 hash of username "Bruce2", password "OhBehave", with nonce "Nonce"
    QVERIFY( message.contains( "Zz6EivR3yeaaENcRN6lpAQ==" ) );

    // Fake header.
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "server";
    hp1->targetDevice = "testClientAuthMD5ChalToBasic";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdId = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = INVALID_CRED;
    sp1->hasChal = true;
    sp1->chal.meta.type = SYNCML_FORMAT_AUTH_BASIC;
    sp1->chal.meta.format = SYNCML_FORMAT_ENCODING_B64;
    session_handler.handleStatusElement( sp1 );

    QVERIFY( !session_handler.authentication().authedToRemote() );
    QCOMPARE( session_handler.getSyncState(), AUTHENTICATION_FAILURE );
}

void SessionHandlerTest::testClientAuthMD5Continuous()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QSKIP( "Unimplemented!" );
#else
    QSKIP( "Unimplemented!", SkipAll );
#endif
}

void SessionHandlerTest::regression_NB153701_01()
{
    // regression_NB153701_01:
    // Test handling of source device when receiving server initiated
    // sync message with unknown source device and local device name is not set

    MockTransport transport( "transport" );

    SyncAgentConfig config;
    DeviceInfo deviceInfo;
    config.setTransport( &transport );
    config.setStorageProvider( this );
    config.setDatabaseFilePath( DBFILE );
    deviceInfo.setDeviceID( NB153701SOURCEDEVICE );
    config.setDeviceInfo(deviceInfo);

    // Prepare
    ClientSessionHandler sessionHandler( &config, NULL );
    QList<Fragment*> fragments;

    // Receive header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = NB153701TARGETDEVICE;
    hp1->targetDevice = NB153701UNKNOWNDEVICE;
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QCOMPARE( sessionHandler.getDevInfHandler().getLocalDeviceInfo().getDeviceID(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.params().localDeviceName(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.params().remoteDeviceName(), NB153701TARGETDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().targetDevice, NB153701TARGETDEVICE );

}

void SessionHandlerTest::regression_NB153701_02()
{
    // regression_NB153701_02:
    // Test handling of source device when receiving server initiated
    // sync message with unknown source device and local device name is set

    MockTransport transport( "transport" );

    SyncAgentConfig config;
    DeviceInfo deviceInfo;
    config.setTransport( &transport );
    config.setStorageProvider( this );
    config.setDatabaseFilePath( DBFILE );
    deviceInfo.setDeviceID( NB153701SOURCEDEVICE );
    config.setDeviceInfo(deviceInfo);
    config.setLocalDeviceName( NB153701FORCEDEVICE );

    // Prepare
    ClientSessionHandler sessionHandler( &config, NULL );
    QList<Fragment*> fragments;

    // Receive header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = NB153701TARGETDEVICE;
    hp1->targetDevice = NB153701UNKNOWNDEVICE;
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QCOMPARE( sessionHandler.getDevInfHandler().getLocalDeviceInfo().getDeviceID(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.params().localDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.params().remoteDeviceName(), NB153701TARGETDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().targetDevice, NB153701TARGETDEVICE );

}

void SessionHandlerTest::regression_NB153701_03()
{

    // regression_NB153701_03:
    // Test handling of device id when receiving server initiated
    // sync message that includes a source device

    MockTransport transport( "transport" );

    SyncAgentConfig config;
    config.setTransport( &transport );
    config.setStorageProvider( this );
    config.setDatabaseFilePath( DBFILE );
    config.setLocalDeviceName( NB153701SOURCEDEVICE );

    // Prepare
    ClientSessionHandler sessionHandler( &config, NULL );
    QList<Fragment*> fragments;

    // Receive header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = NB153701TARGETDEVICE;
    hp1->targetDevice = NB153701FORCEDEVICE;
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QVERIFY( sessionHandler.getDevInfHandler().getLocalDeviceInfo().getDeviceID().isEmpty() );
    QCOMPARE( sessionHandler.params().localDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.params().remoteDeviceName(), NB153701TARGETDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().targetDevice, NB153701TARGETDEVICE );

}

void SessionHandlerTest::regression_NB153701_04()
{
    // regression_NB153701_04:
    // Test handling of target device after sent server initiated
    // sync message

    MockTransport transport( "transport" );

    SyncAgentConfig config;
    config.setTransport( &transport );
    config.setStorageProvider( this );
    config.setDatabaseFilePath( DBFILE );
    config.setLocalDeviceName( NB153701SOURCEDEVICE );
    config.setSyncParams( "", SYNCML_1_1, SyncMode(DIRECTION_TWO_WAY, INIT_SERVER, TYPE_FAST) );
    config.addSyncTarget( "./calendar", "./calendar" );

    ClientSessionHandler sessionHandler( &config, NULL );

    sessionHandler.initiateSync();

    QList<Fragment*> fragments;

    // Receive header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_1;
    hp1->sourceDevice = NB153701TARGETDEVICE;
    hp1->targetDevice = NB153701FORCEDEVICE;
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->meta.maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QVERIFY( sessionHandler.getDevInfHandler().getLocalDeviceInfo().getDeviceID().isEmpty() );
    QCOMPARE( sessionHandler.params().localDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.params().remoteDeviceName(), NB153701TARGETDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().targetDevice, NB153701TARGETDEVICE );

}

QTEST_MAIN(SessionHandlerTest)
