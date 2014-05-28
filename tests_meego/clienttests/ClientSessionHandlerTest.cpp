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

#include "ClientSessionHandlerTest.h"

#include <QtTest>

#include "ClientSessionHandler.h"
#include "SyncAgentConfig.h"
#include "datatypes.h"
#include "Mock.h"
#include "ChangeLog.h"

#include "LogMacros.h"

using namespace DataSync;

void ClientSessionHandlerTest::initTestCase()
{
    iDbHandler = new DatabaseHandler( "/tmp/clientsessionhandler.db");
    iClientId = QString("clientId");
    iConfig = new SyncAgentConfig();
    iTransport = new MockTransport(QString("data/transport_initrequest_nohdr.txt"));
    iConfig->setTransport(iTransport);
    const SyncAgentConfig *tempConstConfig = iConfig;
    iHandler = new ClientSessionHandler(tempConstConfig);
}

void ClientSessionHandlerTest::cleanupTestCase()
{

    delete iTransport;
    delete iHandler;
    delete iDbHandler;
    delete iConfig;
}

void ClientSessionHandlerTest::testMapReceived()
{
    bool result = iHandler->mapReceived();
    QVERIFY(result == false);
}


void ClientSessionHandlerTest::testResendPackage()
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

void ClientSessionHandlerTest::testSyncReceived()
{
    bool result;

    iHandler->setSyncState(PREPARED);
    result = iHandler->syncReceived();
    QVERIFY(result == false);

    iHandler->setSyncState(REMOTE_INIT);
    result = iHandler->syncReceived();
    QVERIFY(result == false);
    iHandler->setSyncWithoutInitPhase( true );
    iHandler->setSyncState(REMOTE_INIT);
    result = iHandler->syncReceived();
    QVERIFY(result == true);

    iHandler->setSyncState(LOCAL_INIT);
    result = iHandler->syncReceived();
    QVERIFY(result == false);

    iHandler->setSyncState(SENDING_ITEMS);
    result = iHandler->syncReceived();
    QVERIFY(result == true);

    iHandler->setSyncState(RECEIVING_ITEMS);
    result = iHandler->syncReceived();
    QVERIFY(result == true);
}

void ClientSessionHandlerTest::testFinalReceived()
{
    iHandler->setSyncState(PREPARED);
    iHandler->finalReceived();

    iHandler->setSyncState(SENDING_ITEMS);
    iHandler->finalReceived();

    iHandler->setSyncState(RECEIVING_ITEMS);
    iHandler->finalReceived();
    iHandler->setSyncWithoutInitPhase( true );
    iHandler->finalReceived();

    iHandler->setSyncState(RECEIVING_MAPPINGS);
    iHandler->finalReceived();

    iHandler->setSyncState(REMOTE_INIT);
    iHandler->finalReceived();

    iHandler->setSyncState(FINALIZING);
    iHandler->finalReceived();
}

void ClientSessionHandlerTest::testBusyStatusReceived()
{
	StatusParams *busyStatusParams = new StatusParams;

    busyStatusParams->data = IN_PROGRESS;
	busyStatusParams->cmdRef = 0;

    iHandler->setSyncState(PREPARED);
    iHandler->finalReceived();

    iHandler->setSyncState(SENDING_ITEMS);
	iHandler->handleStatusElement(busyStatusParams);

	QVERIFY(iHandler->isRemoteBusyStatusSet() == true);

	iHandler->messageParsed();

	QVERIFY(iHandler->isRemoteBusyStatusSet() == false);
}

void ClientSessionHandlerTest::regression_NB166841_01()
{

    // regression_NB166841_01: Test that target acknowledgement succeeds if
    // target db uri and next anchor are present
    const QString sourceURI( "./storage" );
    const QString nextAnchor( "0" );
    const SyncDirection direction( DIRECTION_TWO_WAY );
    SyncAgentConfig config;
    ClientSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );
    ChangeLog* changeLog = new ChangeLog( "", sourceURI, direction );
    MockSyncTarget* target = new MockSyncTarget( changeLog, storage, direction, nextAnchor );

    sessionHandler.iStorages.append( storage );
    sessionHandler.iSyncTargets.append( target );

    SyncMode syncMode;

    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.target = sourceURI;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.acknowledgeTarget( syncMode, alert );
    QCOMPARE( status, SUCCESS );

}

void ClientSessionHandlerTest::regression_NB166841_02()
{

    // regression_NB166841_02: Test that target acknowledgement fails when
    // target db is not present in the alert sent by remote device

    const QString sourceURI( "./storage" );
    const QString nextAnchor( "0" );
    const SyncDirection direction( DIRECTION_TWO_WAY );
    SyncAgentConfig config;
    ClientSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );
    ChangeLog* changeLog = new ChangeLog( "", sourceURI, direction );
    MockSyncTarget* target = new MockSyncTarget( changeLog, storage, direction, nextAnchor );

    sessionHandler.iStorages.append( storage );
    sessionHandler.iSyncTargets.append( target );

    SyncMode syncMode;
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.meta.anchor.next = nextAnchor;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.acknowledgeTarget( syncMode, alert );
    QCOMPARE( status, INCOMPLETE_COMMAND );

}
void ClientSessionHandlerTest::regression_NB166841_03()
{

    // regression_NB166841_03: Test that target acknowledgement fails when
    // next anchor is not present in the alert sent by remote device

    const QString sourceURI( "./storage" );
    const QString nextAnchor( "0" );
    const SyncDirection direction( DIRECTION_TWO_WAY );
    SyncAgentConfig config;
    ClientSessionHandler sessionHandler(&config);

    MockStorage* storage = new MockStorage( sourceURI );
    ChangeLog* changeLog = new ChangeLog( "", sourceURI, direction );
    MockSyncTarget* target = new MockSyncTarget( changeLog, storage, direction, nextAnchor );

    sessionHandler.iStorages.append( storage );
    sessionHandler.iSyncTargets.append( target );

    SyncMode syncMode;
    CommandParams alert( CommandParams::COMMAND_ALERT );
    ItemParams item;
    item.target = sourceURI;
    alert.items.append(item);
    ResponseStatusCode status = sessionHandler.acknowledgeTarget( syncMode, alert );
    QCOMPARE( status, INCOMPLETE_COMMAND );

}


QTEST_MAIN(ClientSessionHandlerTest)
