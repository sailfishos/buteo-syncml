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
#include "TestLoader.h"
#include "internals.h"
#include "Mock.h"
#include "SyncAgent.h"
#include "TestUtils.h"
#include "ServerAlertedNotification.h"
#include "SyncAgentConfigProperties.h"

using namespace DataSync;

QString NB153701DB( "/tmp/NB153701.db" );
QString NB153701UNKNOWNDEVICE( "/" );
QString NB153701SOURCEDEVICE( "IMEI:000000000000000" );
QString NB153701TARGETDEVICE( "IMEI:000000000000001" );
QString NB153701FORCEDEVICE( "IMEI:000000000000002" );


class DummySessionHandler: public SessionHandler
{
public:
    enum FuncType {
        FUNC_NONE,
        FUNC_INITIATE,
        FUNC_START,
        FUNC_SUSPEND,
        FUNC_RESUME,
        FUNC_ABORT,
        FUNC_SAN_PACKAGE_RECEIVED,
        FUNC_MESSAGE_RECEIVED,
        FUNC_ALERT_RECEIVED,
        FUNC_SYNC_RECEIVED,
        FUNC_MAP_RECEIVED,
        FUNC_FINAL_RECEIVED,
        FUNC_MESSAGE_PARSED,
        FUNC_RESEND_PACKAGE,
        FUNC_HANDLE_SYNC_ALERT,
        FUNC_HANDLE_INFORMATIVE_ALERT
    };

    DummySessionHandler(const SyncAgentConfig* aConfig,
                        QObject* aParent)
    :   SessionHandler(aConfig, ROLE_CLIENT, aParent),
        iLatestFunction(FUNC_NONE) {}

    virtual void initiateSync() { iLatestFunction = FUNC_INITIATE; }

    virtual void startSync() { iLatestFunction = FUNC_START; }

    virtual void suspendSync() { iLatestFunction = FUNC_SUSPEND; }

    virtual void resumeSync() { iLatestFunction = FUNC_RESUME; }
    virtual void userAbort() { iLatestFunction = FUNC_ABORT; }

    virtual void SANPackageReceived( QIODevice* /*aDevice*/ ) { iLatestFunction = FUNC_SAN_PACKAGE_RECEIVED; }

    void clearLatestFunction() { iLatestFunction = FUNC_NONE; }

    FuncType getLatestFunction() const { return iLatestFunction; }

protected:
    virtual void messageReceived( HeaderParams& ) { iLatestFunction = FUNC_MESSAGE_RECEIVED; }


    virtual ResponseStatusCode syncAlertReceived( const SyncMode&, AlertParams& ) { iLatestFunction = FUNC_ALERT_RECEIVED; return SUCCESS; }

    virtual bool syncReceived() { iLatestFunction = FUNC_SYNC_RECEIVED; return true; }

    virtual bool mapReceived() { iLatestFunction = FUNC_MAP_RECEIVED; return true; }

    virtual void finalReceived() { iLatestFunction = FUNC_FINAL_RECEIVED; }

    virtual void messageParsed() { iLatestFunction = FUNC_FINAL_RECEIVED; }

    virtual void resendPackage() { iLatestFunction = FUNC_RESEND_PACKAGE; }


private:
    virtual void handleSyncAlert( const SyncMode&, AlertParams& ) {
        iLatestFunction = FUNC_HANDLE_SYNC_ALERT; }

    virtual void handleInformativeAlert( const AlertParams& ) {
        iLatestFunction = FUNC_HANDLE_INFORMATIVE_ALERT; }

    FuncType iLatestFunction;

    //friend class SessionHandlerTest;
};

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

void SessionHandlerTest::testBase()
{
    // Get metatypes registered.
    SyncAgent agent(NULL);

    MockTransport transport("transport");
    const QString DB = "target";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.setDatabaseFilePath( "/tmp/sessionhandlertest.db" );
    config.addSyncTarget( "storage", DB );

    // Create session handler and prepare.
    DummySessionHandler session_handler(&config, NULL);
    session_handler.prepareSync();
    QCOMPARE(session_handler.getSyncState(), PREPARED );

    // Status change emits a signal.
    session_handler.setSyncState(NOT_PREPARED);
    QSignalSpy status_spy(&session_handler, SIGNAL(syncStateChanged(DataSync::SyncState)));
    session_handler.setSyncState(PREPARED);
    QCOMPARE(status_spy.count(), 1);
    QCOMPARE(qvariant_cast<SyncState>(status_spy.at(0).at(0)), PREPARED);

    // Setting same state does not emit a signal.
    session_handler.setSyncState(PREPARED);
    QCOMPARE(status_spy.count(), 1);

    session_handler.startSync();
    QCOMPARE(session_handler.getLatestFunction(), DummySessionHandler::FUNC_START);

    StatusParams* status_params = new StatusParams();
    session_handler.handleStatusElement(status_params);
    status_params = NULL;

    ResultsParams* results_params = new ResultsParams();
    session_handler.handleResultsElement(results_params);
    results_params = NULL;

}

void SessionHandlerTest::testErrorStatuses()
{
    MockTransport transport("transport");
    const QString DB = "target";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.setDatabaseFilePath( "/tmp/sessionhandlertest.db" );
    config.addSyncTarget( "storage", "target" );

    // Create session handler and prepare.
    DummySessionHandler session_handler(&config, NULL);
    session_handler.prepareSync();
    QCOMPARE(session_handler.getSyncState(), PREPARED);

    // Parser errors.
    session_handler.iSessionClosed = false;
    session_handler.handleParserErrors(PARSER_ERROR_INCOMPLETE_DATA);
    QCOMPARE(session_handler.getSyncState(), INVALID_SYNCML_MESSAGE);

    session_handler.setSyncState(PREPARED);
    session_handler.iSessionClosed = false;
    session_handler.handleParserErrors(PARSER_ERROR_UNEXPECTED_DATA);
    QCOMPARE(session_handler.getSyncState(), INVALID_SYNCML_MESSAGE);

    session_handler.setSyncState(PREPARED);
    session_handler.iSessionClosed = false;
    session_handler.handleParserErrors(PARSER_ERROR_INVALID_DATA);
    QCOMPARE(session_handler.getSyncState(), INVALID_SYNCML_MESSAGE);

    // Transport errors.
    session_handler.setSyncState(PREPARED);
    session_handler.iSessionClosed = false;
    session_handler.setTransportStatus(TRANSPORT_DATA_INVALID_CONTENT);
    QCOMPARE(session_handler.getSyncState(), INVALID_SYNCML_MESSAGE);

    session_handler.setSyncState(PREPARED);
    session_handler.iSessionClosed = false;
    session_handler.setTransportStatus(TRANSPORT_CONNECTION_FAILED);
    QCOMPARE(session_handler.getSyncState(), CONNECTION_ERROR);

    session_handler.setSyncState(PREPARED);
    session_handler.iSessionClosed = false;
    session_handler.setTransportStatus(TRANSPORT_CONNECTION_ABORTED);
    QCOMPARE(session_handler.getSyncState(), CONNECTION_ERROR);

}

void SessionHandlerTest::testClientWithClientInitiated()
{
    MockTransport transport("transport");
    const QString DB = "calendar";

    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "calendar", "calendar" );
    config.setDatabaseFilePath("/tmp/sessionhandler.db");

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
    hp1->maxMsgSize = 30000;
    session_handler.handleHeaderElement(hp1);
    hp1 = NULL;

    // Fake status
    StatusParams* sp1 = new StatusParams();
    sp1->cmdID = 1;
    sp1->msgRef = 1;
    sp1->cmdRef = 0;
    sp1->cmd = SYNCML_ELEMENT_SYNCHDR;
    sp1->data = AUTH_ACCEPTED;
    session_handler.handleStatusElement( sp1 );

    // Fake alert.
    AlertParams* ap1 = new AlertParams();
    ap1->cmdID = 2;
    ap1->sourceDatabase = DB;
    ap1->targetDatabase = DB;
    ap1->nextAnchor = "something";
    session_handler.handleAlertElement(ap1);
    ap1 = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    // Step through different states.
    session_handler.handleEndOfMessage();
    SyncParams* sync = new SyncParams();
    sync->cmdID = 1;
    sync->sourceDatabase = DB;
    sync->targetDatabase = DB;
    session_handler.handleSyncElement( sync );
    QCOMPARE(session_handler.getSyncState(), RECEIVING_ITEMS);
    SyncActionData* data = new SyncActionData;
    data->action = SYNCML_GET;
    data->cmdID = 1;
    session_handler.handleGetElement(data);
    data = NULL;
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
    config.setSyncParams( "", DS_1_2, SyncMode(DIRECTION_FROM_CLIENT, INIT_SERVER) );
    config.addSyncTarget( "contacts", DB );
    config.setDatabaseFilePath("/tmp/sessionhandler.db");

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
    AlertParams* ap1 = new AlertParams();
    ap1->cmdID = 0;
    ap1->data = ONE_WAY_FROM_CLIENT_BY_SERVER;
    ap1->type = "text/x-vcard";
    ap1->sourceDatabase = DB;
    ap1->targetDatabase = DB;
    ap1->nextAnchor = "kala";
    fragments.append(ap1);

    session_handler.handleNotificationXML( fragments );

    QVERIFY( fragments.isEmpty() );
    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    SyncActionData* data = new SyncActionData;
    data->action = SYNCML_PUT;
    data->cmdID = 1;
    fragments.append(data);

    // Fake alert.
    ap1 = new AlertParams();
    ap1->cmdID = 1;
    ap1->sourceDatabase = DB;
    ap1->targetDatabase = DB;
    ap1->nextAnchor = "something";
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
    config.setDatabaseFilePath( "/tmp/sessionhandler.db" );
    config.setLocalDeviceName( "Local device" );
    config.setAuthParams( AUTH_BASIC, "user", "password" );

    // Prepare
    ServerSessionHandler sessionHandler( &config, NULL );
    sessionHandler.prepareSync();
    QCOMPARE( sessionHandler.getSyncState(), PREPARED );

    // Start.
    sessionHandler.prepareSync();
    QCOMPARE( sessionHandler.getSyncState(), PREPARED );

    // Fake header
    HeaderParams* hp1 = new HeaderParams();
    hp1->verDTD = SYNCML_DTD_VERSION_1_2;
    hp1->sourceDevice = "Source device";
    hp1->targetDevice = "Target device";
    hp1->sessionID = "1";
    hp1->msgID = 1;
    hp1->maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    hp1->cred.meta.type = SYNCML_FORMAT_AUTH_BASIC;
    hp1->cred.meta.format = SYNCML_FORMAT_ENCODING_B64;
    hp1->cred.data = QByteArray( "user:password" ).toBase64();

    sessionHandler.handleHeaderElement( hp1 );
    hp1 = NULL;

    // Fake alert
    AlertParams* ap1 = new AlertParams();
    ap1->cmdID = 1;
    ap1->sourceDatabase = "calendar";
    ap1->targetDatabase = "calendar";
    ap1->nextAnchor = "nextAnchor";
    ap1->data = SLOW_SYNC;

    sessionHandler.handleAlertElement( ap1 );
    ap1 = NULL;

    QCOMPARE( sessionHandler.getSyncState(), REMOTE_INIT );

    sessionHandler.handleFinal();
    QCOMPARE( sessionHandler.getSyncState(), LOCAL_INIT );

    sessionHandler.handleEndOfMessage();

    SyncParams* sync = new SyncParams();
    sync->cmdID = 1;
    sync->sourceDatabase = "calendar";
    sync->targetDatabase = "calendar";

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
    config.setDatabaseFilePath("/tmp/sessionhandler.db");
    config.setSyncParams( "", DS_1_2, SyncMode(DIRECTION_FROM_CLIENT, INIT_SERVER) );

    QStringList mappings;
    mappings << "Contacts" << "text/x-vcard";
    config.setExtension( SANMAPPINGSEXTENSION, QVariant( mappings ) );

    QByteArray message;
    QVERIFY( readFile( "testfiles/SAN01.bin", message ) );

    // Start.
    ClientSessionHandler session_handler(&config, NULL);

    // Fake SAN

    SANHandler parser;
    SANData SANdata;

    QVERIFY( parser.parseSANMessage( message, SANdata ) );

    session_handler.handleNotificationPackage( SANdata );

    QCOMPARE(session_handler.getSyncState(), LOCAL_INIT);

    // Fake alert.
    AlertParams* ap1 = new AlertParams();
    ap1->cmdID = 2;
    ap1->sourceDatabase = DB;
    ap1->targetDatabase = DB;
    ap1->nextAnchor = "something";
    session_handler.handleAlertElement(ap1);
    ap1 = NULL;

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_ITEMS);

    // Step through different states.
    session_handler.handleEndOfMessage();
    SyncParams* sync = new SyncParams();
    sync->cmdID = 1;
    sync->sourceDatabase = DB;
    sync->targetDatabase = DB;
    session_handler.handleSyncElement( sync );
    QCOMPARE(session_handler.getSyncState(), RECEIVING_ITEMS);
    SyncActionData* data = new SyncActionData;
    data->action = SYNCML_GET;
    data->cmdID = 1;
    session_handler.handleGetElement(data);
    data = NULL;
    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), SENDING_MAPPINGS);
    session_handler.handleEndOfMessage();

    session_handler.handleFinal();
    QCOMPARE(session_handler.getSyncState(), FINALIZING);

    session_handler.handleEndOfMessage();
    QCOMPARE(session_handler.getSyncState(), SYNC_FINISHED);

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
    config.setDatabaseFilePath( NB153701DB );
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
    hp1->maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QCOMPARE( sessionHandler.getDevInfHandler().getDeviceInfo().getDeviceID(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.getLocalDeviceName(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.getRemoteDeviceName(), NB153701TARGETDEVICE );
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
    config.setDatabaseFilePath( NB153701DB );
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
    hp1->maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QCOMPARE( sessionHandler.getDevInfHandler().getDeviceInfo().getDeviceID(), NB153701SOURCEDEVICE );
    QCOMPARE( sessionHandler.getLocalDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getRemoteDeviceName(), NB153701TARGETDEVICE );
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
    config.setDatabaseFilePath( NB153701DB );
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
    hp1->maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QVERIFY( sessionHandler.getDevInfHandler().getDeviceInfo().getDeviceID().isEmpty() );
    QCOMPARE( sessionHandler.getLocalDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getRemoteDeviceName(), NB153701TARGETDEVICE );
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
    config.setDatabaseFilePath( NB153701DB );
    config.setLocalDeviceName( NB153701SOURCEDEVICE );
    config.setSyncParams( "", DS_1_1, SyncMode(DIRECTION_TWO_WAY, INIT_SERVER, TYPE_FAST) );
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
    hp1->maxMsgSize = DEFAULT_MAX_MESSAGESIZE;
    fragments.append(hp1);

    sessionHandler.handleNotificationXML(fragments);
    QVERIFY(fragments.isEmpty());

    QVERIFY( sessionHandler.getDevInfHandler().getDeviceInfo().getDeviceID().isEmpty() );
    QCOMPARE( sessionHandler.getLocalDeviceName(), NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().sourceDevice, NB153701FORCEDEVICE );
    QCOMPARE( sessionHandler.getRemoteDeviceName(), NB153701TARGETDEVICE );
    QCOMPARE( sessionHandler.getResponseGenerator().getHeaderParams().targetDevice, NB153701TARGETDEVICE );

}

TESTLOADER_ADD_TEST(SessionHandlerTest);
