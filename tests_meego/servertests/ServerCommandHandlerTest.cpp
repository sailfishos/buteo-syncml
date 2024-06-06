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

#include "ServerCommandHandlerTest.h"
#include "CommandHandler.h"
#include "ChangeLog.h"
#include "SyncAgentConfig.h"
#include "datatypes.h"
#include "Mock.h"
#include "SyncTarget.h"
#include "StorageHandler.h"
#include "ConflictResolver.h"
#include "DeviceInfo.h"
#include <QDebug>
#include "SyncMLLogging.h"

using namespace DataSync;

/*!
    \fn ServerCommandHandlerTest::initTestCase()
 */
void ServerCommandHandlerTest::initTestCase()
{
    iDbHandler = new DatabaseHandler( QProcessEnvironment::systemEnvironment().value("TMPDIR", "/tmp") + "/servercommandhandlertest.db");
    iStorageHandler = new StorageHandler();
    iCommandHandler = new CommandHandler( MODE_SERVER );
    iRemoteDevice = "remotedevice";
    iLocalDb = "localcontacts";
    iRemoteDb = "remotecontacts";
    iStorage = new MockStorage( iLocalDb );
    SyncMode syncMode;

    ChangeLog* changeLog = new ChangeLog( iDbHandler->getDbHandle(), iRemoteDevice, iLocalDb, DIRECTION_TWO_WAY );

    SyncTarget* target = new SyncTarget( changeLog, *iStorage, syncMode, "fooanchor" );

    iTargets.append(target);

}

/*!
    \fn ServerCommandHandlerTest::cleanupTestCase()
 */
void ServerCommandHandlerTest::cleanupTestCase()
{
    qDeleteAll(iTargets);
    iTargets.clear();

    delete iStorage;
    iStorage = 0;

    delete iCommandHandler;
    iCommandHandler = 0;

    delete iStorageHandler;
    iStorageHandler = 0;

    delete iDbHandler;
    iDbHandler = 0;

}



/*!
    \fn ServerCommandHandlerTest::testHandlerPut()
 */
void ServerCommandHandlerTest::testHandlerPut()
{
    MetaParams metaParams;
    metaParams.type = SYNCML_CONTTYPE_DEVINF_XML;

    ItemParams item;
    item.source = SYNCML_DEVINF_PATH_11;

    SyncActionData actionData;
    actionData.action = SYNCML_PUT;
    actionData.meta = metaParams;
    actionData.items.append(item);

    ProtocolVersion version = DS_1_1;

    iCommandHandler->handlePut(actionData, version, iResponseGenerator);

    version = DS_1_2;
    iCommandHandler->handlePut(actionData, version, iResponseGenerator);

    actionData.items[0].source = SYNCML_DEVINF_PATH_12;
    iCommandHandler->handlePut(actionData, version, iResponseGenerator);

    version = DS_1_1;
    iCommandHandler->handlePut(actionData, version, iResponseGenerator);
}

/*!
    \fn ServerCommandHandlerTest::testHandleGet()
 */
void ServerCommandHandlerTest::testHandleGet()
{
    MetaParams metaParams;
    metaParams.type = SYNCML_CONTTYPE_DEVINF_XML;

    ItemParams item;
    item.target = SYNCML_DEVINF_PATH_11;
    QList<ItemParams> itemList;
    itemList.append(item);

    SyncActionData actionData;
    actionData.action = SYNCML_GET;
    actionData.meta = metaParams;
    actionData.items = itemList;

    ProtocolVersion version = DS_1_1;
    OperationMode opMode = MODE_CLIENT;
    QList<StoragePlugin*> dataStores;
    DeviceInfo devInfo;

    iCommandHandler->handleGet( actionData, version, opMode, dataStores, devInfo, iResponseGenerator );

}


/*!
    \fn ServerCommandHandlerTest::testHandleStatus()
 */
void ServerCommandHandlerTest::testHandleStatus()
{
    StatusParams status;

    status.data = IN_PROGRESS;
    iCommandHandler->handleStatus(&status);

    status.data = SUCCESS;
    iCommandHandler->handleStatus(&status);

    status.data = MULTIPLE_CHOICES;
    iCommandHandler->handleStatus(&status);

    status.data = BAD_REQUEST;
    iCommandHandler->handleStatus(&status);
    /// @todo slot for catching abort-signal

    status.data = COMMAND_FAILED;
    iCommandHandler->handleStatus(&status);

    status.data = (ResponseStatusCode)600;
    iCommandHandler->handleStatus(&status);
}

/*!
    \fn DataSync::ServerCommandHandlerTest::testResolveConflicts()
 */
void DataSync::ServerCommandHandlerTest::testResolveConflicts()
{
    bool actual = iCommandHandler->resolveConflicts();
    bool expexted = true;
    QCOMPARE(actual, expexted);
}


/*!
    \fn DataSync::ServerCommandHandlerTest::testAddUIDMapping()
 */
void DataSync::ServerCommandHandlerTest::testAddUIDMapping()
{
    QString uid = "Foo";
    MockStorage storagePlugin(0);
    SyncMode syncMode;
    ChangeLog* changeLog = new ChangeLog( iDbHandler->getDbHandle(), "device1", "sourcedb", DIRECTION_TWO_WAY );
    MockSyncTarget syncTarget(changeLog, storagePlugin, syncMode, "");
    MapParams mapParams;
    MapItem mapItem;
    mapItem.source = "foo";
    mapItem.target = "bar";
    QList<MapItem> mapItemList;
    mapItemList.append(mapItem);
    mapParams.mapItemList = mapItemList;
    /*ResponseStatusCode result = */iCommandHandler->handleMap(mapParams, syncTarget);
//     delete changeLog;
//     changeLog = NULL;
//     QVERIFY(syncTarget.iMappingCount == mapItemList.size());

    iCommandHandler->removeUIDMapping(syncTarget, "foo");
}

void DataSync::ServerCommandHandlerTest::testMapUID()
{
    SyncTarget* target;

    if (!iTargets.isEmpty()) {
        target = iTargets.first();
    }

    iCommandHandler->mapUID(*target, "foo");
}

void DataSync::ServerCommandHandlerTest::testHandleSync()
{
    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdID = cmdId++;
    syncParams.targetDatabase = iLocalDb;
    syncParams.sourceDatabase = iRemoteDb;

    SyncActionData add;

    add.action = SYNCML_ADD;
    add.cmdID = cmdId++;

    ItemParams addItem;
    addItem.source = "fooid";
    addItem.Data = "foodata";
    add.items.append( addItem );

    syncParams.actionList.append( add );


    SyncActionData replace;

    replace.action = SYNCML_REPLACE;
    replace.cmdID = cmdId++;

    ItemParams replaceItem;
    replaceItem.source = "fooid2";
    replaceItem.Data = "foodata2";
    replace.items.append( replaceItem );

    syncParams.actionList.append( replace );


    SyncActionData del;

    del.action = SYNCML_DELETE;
    del.cmdID = cmdId++;

    ItemParams deleteItem;
    deleteItem.source = "fooid3";
    del.items.append( deleteItem );

    syncParams.actionList.append( del );


    SyncTarget& target = *iTargets.first();
    StorageHandler& storageHandler = *iStorageHandler;
    ResponseGenerator& responseGenerator = iResponseGenerator;

    LocalChanges changes;
    ConflictResolver ConflictResolver( changes, PREFER_CLIENT, MODE_SERVER );

    iCommandHandler->handleSync( syncParams, target, storageHandler,
                                 responseGenerator, ConflictResolver, MODE_SERVER );
}

void DataSync::ServerCommandHandlerTest::testHandleSyncLargeObject()
{


    SyncTarget& target = *iTargets.first();
    StorageHandler& storageHandler = *iStorageHandler;
    ResponseGenerator& responseGenerator = iResponseGenerator;

    LocalChanges changes;
    ConflictResolver ConflictResolver( changes, PREFER_CLIENT, MODE_SERVER );

    SyncParams syncParams1;

    syncParams1.cmdID = 1;
    syncParams1.targetDatabase = iLocalDb;
    syncParams1.sourceDatabase = iRemoteDb;

    SyncActionData add1;
    add1.action = SYNCML_ADD;
    add1.cmdID = 2;
    add1.meta.size = 2;

    ItemParams addItem1;
    addItem1.source = "fooid";
    addItem1.Data = QByteArray( "1" );
    addItem1.moreData = true;
    add1.items.append( addItem1 );

    syncParams1.actionList.append( add1 );

    iCommandHandler->handleSync( syncParams1, target, storageHandler,
                                 responseGenerator, ConflictResolver, MODE_SERVER );


    SyncParams syncParams2;

    syncParams2.cmdID = 1;
    syncParams2.targetDatabase = iLocalDb;
    syncParams2.sourceDatabase = iRemoteDb;

    SyncActionData add2;
    add2.action = SYNCML_ADD;
    add2.cmdID = 2;

    ItemParams addItem2;
    addItem2.source = "fooid";
    addItem2.Data = QByteArray( "2" );
    add2.items.append( addItem2 );

    syncParams2.actionList.append( add2 );

    iCommandHandler->handleSync( syncParams2, target, storageHandler,
                                 responseGenerator, ConflictResolver, MODE_SERVER );
}

QTEST_MAIN(DataSync::ServerCommandHandlerTest)



