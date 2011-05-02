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

#include "SyncAgent.h"
#include "SyncAgentConfig.h"
#include "SyncAgentTest.h"
#include "TestLoader.h"
#include "Mock.h"
#include "StoragePlugin.h"
#include <QSignalSpy>

using namespace DataSync;

bool SyncAgentTest::getStorageContentFormatInfo( const QString& aURI,
                                                 StorageContentFormatInfo& aInfo )
{
    Q_UNUSED( aURI );
    MockStorage temp( "storage" );
    aInfo = temp.getFormatInfo();
    return true;
}

StoragePlugin* SyncAgentTest::acquireStorageByURI( const QString& /*aURI*/ )
{
    return new MockStorage( "storage" );
}

StoragePlugin* SyncAgentTest::acquireStorageByMIME( const QString& /*aMIME*/ )
{
    return new MockStorage( "storage" );
}

void SyncAgentTest::releaseStorage( StoragePlugin* aStorage )
{
    delete aStorage;
}

void SyncAgentTest::testSync()
{
    createSyncAgent_t createSyncAgent = (createSyncAgent_t) QLibrary::resolve(
            "libmeegosyncml.so", "createSyncAgent");
    QVERIFY(NULL != createSyncAgent);
    SyncAgent* agent = createSyncAgent(NULL);
    QVERIFY(NULL != agent);

    MockTransport transport("transport");
    const QString DB = "target";
    const QString ERROR = "no error";
    const QString MIMETYPE = "text/x-vcard";

    // Create signal spies.
    QSignalSpy status_spy(agent, SIGNAL(stateChanged(DataSync::SyncState)));
    QSignalSpy processed_spy(agent, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    // Initialize SyncAgent in client mode.
    SyncAgentConfig config;
    config.setTransport(&transport);
    config.setStorageProvider( this );
    config.addSyncTarget( "storage", DB );

    // Try operations when not prepared.
    QCOMPARE(agent->pauseSync(), false);
    QCOMPARE(agent->abort(), false);

    // Test changing status.
    status_spy.clear();
    agent->receiveStateChanged( PREPARED );
    QCOMPARE(status_spy.count(), 1);
    QCOMPARE(qvariant_cast<SyncState>(status_spy.at(0).at(0)), PREPARED);

    // Start.
    QCOMPARE(agent->startSync(config), true);
    QVERIFY(agent->isSyncing());
    agent->receiveStateChanged( LOCAL_INIT );
    QCOMPARE(agent->startSync(config), false);

    // Processed.
    agent->receiveItemProcessed( MOD_ITEM_ADDED, MOD_LOCAL_DATABASE, DB , MIMETYPE, 1 );
    const SyncResults& results = agent->getResults();
    const QMap<QString, DatabaseResults>* dbResults_p = results.getDatabaseResults();
    const QMap<QString, DatabaseResults>& dbResults = *dbResults_p;

    QCOMPARE( dbResults.count(), 1 );
    QCOMPARE( dbResults[DB].iLocalItemsAdded, 1 );
    QCOMPARE( dbResults[DB].iLocalItemsModified, 0 );
    QCOMPARE( dbResults[DB].iLocalItemsDeleted, 0 );
    QCOMPARE( dbResults[DB].iRemoteItemsAdded, 0 );
    QCOMPARE( dbResults[DB].iRemoteItemsModified, 0 );
    QCOMPARE( dbResults[DB].iRemoteItemsDeleted, 0 );

    QCOMPARE(processed_spy.count(), 1);
    QCOMPARE(qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_ADDED );
    QCOMPARE(qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE(processed_spy.at(0).at(2).toString(), DB);

    // Pause, abort, resume when finished.
    agent->receiveSyncFinished( QString("IMEI"),SYNC_FINISHED, ERROR );
    QCOMPARE(agent->isSyncing(), false);
    QCOMPARE(agent->abort(), false);
    QCOMPARE(agent->pauseSync(), false);
    QCOMPARE(agent->resumeSync(), false);

    // Test in server mode.
    config.setSyncParams(config.getRemoteDeviceName(),
                         config.getProtocolVersion(),
                         SyncMode(DIRECTION_FROM_SERVER, INIT_SERVER));
    QCOMPARE(agent->startSync(config), true);
    QCOMPARE(agent->pauseSync(), true);
    QCOMPARE(agent->abort(), true);

    // Listening
    agent->cleanSession();
    QCOMPARE(agent->isListening(), false);
    QCOMPARE(agent->listen(config), true);
    QCOMPARE(agent->isListening(), true);
    agent->listenEvent();
    QCOMPARE(agent->isListening(), false);

    agent->cleanSession();
    QCOMPARE(agent->listen(config), true);
    agent->listenError(ABORTED, "Testing error");
    QCOMPARE(agent->isListening(), false);

    agent->cleanSession();
    QCOMPARE(agent->listen(config), true);
    QCOMPARE(agent->abort(), true);
    QCOMPARE(agent->isListening(), false);

    QCOMPARE(agent->cleanUp(&config), false);

    destroySyncAgent_t* destroySyncAgent =
        (destroySyncAgent_t*) QLibrary::resolve("libmeegosyncml.so",
                "destroySyncAgent");
    QVERIFY(NULL != destroySyncAgent);
    destroySyncAgent(agent);
    agent = NULL;
}


TESTLOADER_ADD_TEST(SyncAgentTest);
