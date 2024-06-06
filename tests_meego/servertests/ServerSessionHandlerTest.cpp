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

#include "ServerSessionHandlerTest.h"

#include <QtTest>

#include "ServerSessionHandler.h"
#include "SyncAgentConfig.h"
#include "datatypes.h"
#include "Mock.h"
#include "ChangeLog.h"

#include "SyncMLLogging.h"

using namespace DataSync;

static QString DBFILE( QProcessEnvironment::systemEnvironment().value("TMPDIR", "/tmp") + "/serversessionhandler.db" );

void ServerSessionHandlerTest::initTestCase()
{
    iDbHandler = new DatabaseHandler( DBFILE );
    iClientId = QString("clientId");
    iConfig = new SyncAgentConfig();
    QString localDb = "localcontacts";
    iStorage = new MockStorage( localDb );
    iTransport = new MockTransport(QString("data/transport_initrequest_nohdr.txt"));
    iConfig->setTransport(iTransport);
    iConfig->setDatabaseFilePath( DBFILE );
    const SyncAgentConfig *tempConstConfig = iConfig;
    iHandler = new ServerSessionHandler(tempConstConfig);
}

void ServerSessionHandlerTest::cleanupTestCase()
{
    delete iHandler;
    iHandler = 0;

    delete iConfig;
    iConfig = 0;

    delete iStorage;
    iStorage = NULL;

    delete iTransport;
    iTransport = 0;

    delete iDbHandler;
    iDbHandler = 0;
}

void ServerSessionHandlerTest::testHandleResponseHeader()
{
    HeaderParams* params = new HeaderParams;
    params->sessionID = QString("sessionId");
    params->targetDevice = QString("targetDevice");
    params->sourceDevice= QString("sourceDevice");
    params->msgID = 1;

    iHandler->handleHeaderElement( params );


}

void ServerSessionHandlerTest::testSetGetSyncStatus()
{
    SyncState setSyncState = SENDING_ITEMS;

    iHandler->setSyncState(setSyncState);
    SyncState receivedSyncState = iHandler->getSyncState();

    QVERIFY(setSyncState == receivedSyncState);
}

void ServerSessionHandlerTest::testAddGetSyncTargets()
{
//     DatabaseHandler* dbHandler = new DatabaseHandler( "/tmp/serversessionhandlertest2.db");
//     iConfig = new SyncAgentConfig();
//     iStorageHandler = new StorageHandler();
//     iCommandHandler = new ServerCommandHandler( );
    QString remoteDevice = "remotedevice";
    QString localDb = "localcontacts";

    SyncMode syncMode;

    ChangeLog* changeLog = new ChangeLog( remoteDevice, localDb, DIRECTION_TWO_WAY );

    SyncTarget* target = new SyncTarget( changeLog, iStorage, syncMode, "fooanchor" );
    iHandler->addSyncTarget(target);

    QList<SyncTarget*> returnedTargets = iHandler->getSyncTargets();

    QVERIFY(!returnedTargets.isEmpty());


}

void ServerSessionHandlerTest::testSyncReceived()
{
    bool result;

    iHandler->setSyncState(PREPARED);
    result = iHandler->syncReceived();
    QVERIFY(result == false);

    iHandler->setSyncState(REMOTE_INIT);
    result = iHandler->syncReceived();
    QVERIFY(result == true);

    iHandler->setSyncState(LOCAL_INIT);
    result = iHandler->syncReceived();
    QVERIFY(result == true);

    iHandler->setSyncState(RECEIVING_ITEMS);
    result = iHandler->syncReceived();
    QVERIFY(result == true);
}

void ServerSessionHandlerTest::testMapReceived()
{
    bool result;

    iHandler->setSyncState(PREPARED);
    result = iHandler->mapReceived();
    QVERIFY(result == false);

    iHandler->setSyncState(SENDING_ITEMS);
    result = iHandler->mapReceived();
    QVERIFY(result == true);

    iHandler->setSyncState(RECEIVING_MAPPINGS);
    result = iHandler->mapReceived();
    QVERIFY(result == true);
}

void ServerSessionHandlerTest::testFinalReceived()
{
    iHandler->setSyncState(PREPARED);
    iHandler->finalReceived();

    iHandler->setSyncState(SENDING_ITEMS);
    iHandler->finalReceived();

    iHandler->setSyncState(RECEIVING_ITEMS);
    iHandler->finalReceived();

    iHandler->setSyncState(RECEIVING_MAPPINGS);
    iHandler->finalReceived();

    iHandler->setSyncState(REMOTE_INIT);
    iHandler->finalReceived();

    iHandler->setSyncState(FINALIZING);
    iHandler->finalReceived();
}

void ServerSessionHandlerTest::testMessageParsed()
{
    iHandler->setSyncState(PREPARED);
    iHandler->messageParsed();

    iHandler->setSyncState(SUSPENDING);
    iHandler->messageParsed();

    iHandler->setSyncState(FINALIZING);
    iHandler->messageParsed();

}

void ServerSessionHandlerTest::testResendPackage()
{
    iHandler->setSyncState(PREPARED);
    iHandler->resendPackage();

    iHandler->setSyncState(LOCAL_INIT);
    iHandler->resendPackage();

    iHandler->setSyncState(SENDING_ITEMS);
    iHandler->resendPackage();

    iHandler->setSyncState(FINALIZING);
    iHandler->resendPackage();
}

void ServerSessionHandlerTest::testComposeServerModificationsPackage()
{
    iHandler->composeServerModificationsPackage();
}

void ServerSessionHandlerTest::testSyncFinished()
{
    bool expected = iHandler->iSyncFinished;
    bool actual   = iHandler->syncFinished();
    QCOMPARE(expected, actual);
}

void ServerSessionHandlerTest::testHandleInformativeAlert()
{
    CommandParams alertParams( CommandParams::COMMAND_ALERT );

    alertParams.data = QString::number( DISPLAY );
    ResponseStatusCode response = iHandler->handleInformativeAlert(alertParams);
    QVERIFY(response == NOT_IMPLEMENTED);

    alertParams.data = QString::number( NEXT_MESSAGE );
    response = iHandler->handleInformativeAlert(alertParams);
    QVERIFY(response == SUCCESS);

    alertParams.data = QString::number( SLOW_SYNC );
    response = iHandler->handleInformativeAlert(alertParams);
    QVERIFY(response == NOT_IMPLEMENTED);

}

void ServerSessionHandlerTest::testSyncInitiate()
{
    ServerSessionHandler sessionHandler(iConfig);
    sessionHandler.initiateSync();
    QCOMPARE(sessionHandler.getSyncState(), PREPARED);
}

void ServerSessionHandlerTest::testUnsupportedCalls()
{
    iHandler->initiateSync();
    SyncState prevState = iHandler->getSyncState();
    // After unsupported calls the state should be the same as it was previously
    iHandler->suspendSync();
    QCOMPARE(iHandler->getSyncState(), prevState);
    iHandler->resumeSync();
    QCOMPARE(iHandler->getSyncState(), prevState);
}

void ServerSessionHandlerTest::testSyncAlert()
{
    CommandParams params;
    SyncMode syncMode(TWO_WAY_SYNC);

    iHandler->setSyncState(PREPARED);
    QCOMPARE(iHandler->syncAlertReceived(syncMode, params), INCOMPLETE_COMMAND);
    QCOMPARE(iHandler->getSyncState(), REMOTE_INIT);

    iHandler->setSyncState(LOCAL_INIT);
    QCOMPARE(iHandler->syncAlertReceived(syncMode, params), INCOMPLETE_COMMAND);
    QCOMPARE(iHandler->getSyncState(), LOCAL_INIT);

    iHandler->setSyncState(NOT_PREPARED);
    QCOMPARE(iHandler->syncAlertReceived(syncMode, params), COMMAND_NOT_ALLOWED);
    QCOMPARE(iHandler->getSyncState(), NOT_PREPARED);
}

void ServerSessionHandlerTest::regression_NB166841_01()
{
    // regression_NB166841_01: Test that target setup succeeds if source db uri,
    // target db uri and next anchor are present
    const QString sourceURI( "./target" );
    const QString targetURI( "./source" );
    const QString nextAnchor( "0" );

    SyncAgentConfig config;
    config.setDatabaseFilePath( DBFILE );
    ServerSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );

    sessionHandler.iStorages.append( storage );

    SyncMode syncMode(DIRECTION_TWO_WAY, INIT_CLIENT, TYPE_SLOW);
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.target = sourceURI;
    item.source = targetURI;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.setupTargetByClient(syncMode,alert);
    // Intentionally expecting REFRESH_REQUIRED inst. of SUCCESS. Would need
    // TYPE_FAST and valid last anchor otherwise.
    QCOMPARE( status, REFRESH_REQUIRED );
}

void ServerSessionHandlerTest::regression_NB166841_02()
{
    // regression_NB166841_02: Test that target setup succeeds if source db uri,
    // mime type and next anchor are present
    const QString sourceURI( "./target" );
    const QString mimeURI( "text/x-vcalendar" );
    const QString nextAnchor( "0" );

    SyncAgentConfig config;
    config.setDatabaseFilePath( DBFILE );
    ServerSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI, mimeURI );

    sessionHandler.iStorages.append( storage );

    SyncMode syncMode(DIRECTION_TWO_WAY, INIT_CLIENT, TYPE_SLOW);
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.source = sourceURI;
    item.meta.type = mimeURI;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.setupTargetByClient(syncMode,alert);
    // Intentionally expecting REFRESH_REQUIRED inst. of SUCCESS. Would need
    // TYPE_FAST and valid last anchor otherwise.
    QCOMPARE( status, REFRESH_REQUIRED );
}

void ServerSessionHandlerTest::regression_NB166841_03()
{
    // regression_NB166841_03: Test that target setup fails if source db uri and
    // mime type are absent
    const QString mimeURI( "text/x-vcalendar" );
    const QString nextAnchor( "0" );

    SyncAgentConfig config;
    config.setDatabaseFilePath( DBFILE );
    ServerSessionHandler sessionHandler(&config);

    SyncMode syncMode(DIRECTION_TWO_WAY, INIT_CLIENT, TYPE_SLOW);
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.setupTargetByClient(syncMode,alert);
    QCOMPARE( status, INCOMPLETE_COMMAND );
}

void ServerSessionHandlerTest::regression_NB166841_04()
{
    // regression_NB166841_01: Test that target setup fails if next anchor is absent
    const QString sourceURI( "./target" );
    const QString targetURI( "./source" );

    SyncAgentConfig config;
    config.setDatabaseFilePath( DBFILE );
    ServerSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );

    sessionHandler.iStorages.append( storage );

    SyncMode syncMode(DIRECTION_TWO_WAY, INIT_CLIENT, TYPE_SLOW);
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.target = sourceURI;
    item.source = targetURI;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.setupTargetByClient(syncMode,alert);
    QCOMPARE( status, INCOMPLETE_COMMAND );
}

void ServerSessionHandlerTest::testSetClientRefresh()
{
    QString remoteDevice = "aremotedevice";
    QString localDb = "localdb";
    const QString sourceURI( "./targetURI" );
    const QString targetURI( "./sourceURI" );
    const QString nextAnchor( "0" );
    SyncMode syncMode(DIRECTION_FROM_CLIENT,INIT_CLIENT,TYPE_SLOW);
    SyncMode nextSyncMode(DIRECTION_FROM_CLIENT,INIT_CLIENT,TYPE_FAST);

    SyncAgentConfig config;
    config.setDatabaseFilePath( DBFILE );
    ServerSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );
    sessionHandler.iStorages.append( storage );
    ChangeLog* changeLog = new ChangeLog( remoteDevice, localDb, DIRECTION_FROM_CLIENT );
    SyncTarget* target = new SyncTarget( changeLog, storage, syncMode, "lastanchor" );
    sessionHandler.addSyncTarget(target);

    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.target = sourceURI;
    item.source = targetURI;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);

    ResponseStatusCode status = sessionHandler.setupTargetByClient(nextSyncMode,alert);
    QCOMPARE( status, REFRESH_REQUIRED );
}

QTEST_MAIN(ServerSessionHandlerTest)
