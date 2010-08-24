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

#include "CommandHandlerTest.h"

#include <QSignalSpy>

#include "internals.h"
#include "CommandHandler.h"
#include "ResponseGenerator.h"
#include "ConflictResolver.h"
#include "QtEncoder.h"
#include "SyncMLMessage.h"
#include "DeviceInfo.h"
#include "Mock.h"
#include "TestUtils.h"
#include "LogMacros.h"

#include "TestLoader.h"

using namespace DataSync;


CommandHandlerTest::CommandHandlerTest()
{

}

CommandHandlerTest::~CommandHandlerTest()
{

}

// @todo:
/*
void CommandHandlerTest::testPut()
{
    SyncActionData actionData;
    actionData.cmdID = 1;
    actionData.action = SYNCML_PUT;
    actionData.meta.type = SYNCML_CONTTYPE_DEVINF_XML;

    ItemParams item;
    actionData.items.append( item );

    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    CommandHandler handler( ROLE_CLIENT );

    actionData.items[0].source = SYNCML_DEVINF_PATH_11;

    handler.handlePut( actionData, DS_1_1, generator );
    handler.handlePut( actionData, DS_1_2, generator );

    actionData.items[0].source = SYNCML_DEVINF_PATH_12;

    handler.handlePut( actionData, DS_1_1, generator );
    handler.handlePut( actionData, DS_1_2, generator );

    actionData.meta.type = "foo";

    handler.handlePut( actionData, DS_1_1, generator );
    handler.handlePut( actionData, DS_1_2, generator );

    actionData.meta.type = SYNCML_CONTTYPE_DEVINF_XML;
    actionData.items[0].source = "foo";

    handler.handlePut( actionData, DS_1_1, generator );
    handler.handlePut( actionData, DS_1_2, generator );

    QtEncoder encoder;
    QByteArray document;
    SyncMLMessage* message = generator.generateNextMessage( 65535, DS_1_2 );
    QVERIFY( encoder.encodeToXML( *message, document, false ) );
    delete message;
    message = NULL;

    QByteArray correct;
    QVERIFY( readFile( "testfiles/cmdhandler_put.txt", correct ) );

    QVERIFY( correct.simplified() == document.simplified() );

}

void CommandHandlerTest::testGet()
{
    SyncActionData actionData;
    actionData.cmdID = 1;
    actionData.action = SYNCML_GET;
    actionData.meta.type = SYNCML_CONTTYPE_DEVINF_XML;

    ItemParams item;
    actionData.items.append( item );

    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    CommandHandler handler( ROLE_CLIENT );
    QList<StoragePlugin*> dataStores;
    DeviceInfo devInfo;


    actionData.items[0].target = SYNCML_DEVINF_PATH_11;
    handler.handleGet( actionData, DS_1_2, dataStores, devInfo, generator );

    actionData.items[0].target = SYNCML_DEVINF_PATH_12;
    handler.handleGet( actionData, DS_1_1, dataStores, devInfo, generator );

    actionData.meta.type = "foo";
    handler.handleGet( actionData, DS_1_1, dataStores, devInfo, generator );

    actionData.meta.type = SYNCML_CONTTYPE_DEVINF_XML;
    actionData.items[0].target = "foo";
    handler.handleGet( actionData, DS_1_2, dataStores, devInfo, generator );

    actionData.items[0].target = SYNCML_DEVINF_PATH_12;
    handler.handleGet( actionData, DS_1_2, dataStores, devInfo, generator );

    QtEncoder encoder;
    QByteArray document;
    SyncMLMessage* message = generator.generateNextMessage( 65535, DS_1_2 );
    QVERIFY( encoder.encodeToXML( *message, document, false ) );
    delete message;
    message = NULL;

    QByteArray correct;
    QVERIFY( readFile( "testfiles/cmdhandler_get.txt", correct ) );

    QVERIFY( correct.simplified() == document.simplified() );

}
*/

void CommandHandlerTest::testAdd_Client01()
{

    // Test adding a new item from server without parent

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );
    QString mime( "mime/foo1" );
    QString itemId( "id0" );
    int cmdId = 1;

    // Setup sync target
    CommitTestStorage storage( localDb );

    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    // Setup conflict resolver

    LocalChanges changes;
    ConflictResolver conflictResolver( changes, PREFER_LOCAL_CHANGES );

    StorageHandler storageHandler;
    CommandHandler commandHandler( ROLE_CLIENT );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    // Create sync element

    SyncParams syncParams;

    syncParams.cmdID = cmdId++;
    syncParams.sourceDatabase = remoteDb;
    syncParams.targetDatabase = localDb;

    SyncActionData add;

    add.cmdID = cmdId++;
    add.action = SYNCML_ADD;

    ItemParams addItem;
    addItem.source = itemId;
    addItem.Data = "foodata";
    addItem.meta.type = mime;
    add.items.append( addItem );

    syncParams.actionList.append( add );

    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
    QSignalSpy processed_spy( &storageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    commandHandler.handleSync( syncParams, target, storageHandler, generator, conflictResolver, false );

    // Check that item was added to storage
    QCOMPARE( storage.iAddedItems.count(), 1 );
    QCOMPARE( storage.iReplacedItems.count(), 0 );
    QCOMPARE( storage.iDeletedItems.count(), 0 );
    QCOMPARE( storage.iIdCounter, 1 );

    // Check that item was added to mappings
    QCOMPARE( target.getUIDMappings().count(), 1 );
    QCOMPARE( target.mapToLocalUID( itemId ), QString( "1" ) );

    // Check that proper signals were emitted
    QCOMPARE( processed_spy.count(), 1 );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(0).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(0).at(3).toString(), mime );

}

void CommandHandlerTest::testAdd_Server01()
{

    // Test adding a new item from client without parent

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );
    QString mime( "mime/foo1" );
    QString itemId( "id0" );
    int cmdId = 1;

    // Setup sync target
    CommitTestStorage storage( localDb );

    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    // Setup conflict resolver

    LocalChanges changes;
    ConflictResolver conflictResolver( changes, PREFER_REMOTE_CHANGES );

    StorageHandler storageHandler;
    CommandHandler commandHandler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    // Create sync element

    SyncParams syncParams;

    syncParams.cmdID = cmdId++;
    syncParams.sourceDatabase = remoteDb;
    syncParams.targetDatabase = localDb;

    SyncActionData add;

    add.cmdID = cmdId++;
    add.action = SYNCML_ADD;

    ItemParams addItem;
    addItem.source = itemId;
    addItem.Data = "foodata";
    addItem.meta.type = mime;
    add.items.append( addItem );

    syncParams.actionList.append( add );

    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
    QSignalSpy processed_spy( &storageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    commandHandler.handleSync( syncParams, target, storageHandler, generator, conflictResolver, false );

    // Check that item was added to storage
    QCOMPARE( storage.iAddedItems.count(), 1 );
    QCOMPARE( storage.iReplacedItems.count(), 0 );
    QCOMPARE( storage.iDeletedItems.count(), 0 );
    QCOMPARE( storage.iIdCounter, 1 );

    // Check that item was added to mappings
    QCOMPARE( target.getUIDMappings().count(), 1 );
    QCOMPARE( target.mapToLocalUID( itemId ), QString( "1" ) );

    // Check that proper signals were emitted
    QCOMPARE( processed_spy.count(), 1 );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(0).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(0).at(3).toString(), mime );
}

void CommandHandlerTest::testSyncAdd()
{

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );

    StorageHandler storageHandler;
    MockStorage storage( localDb );
    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    LocalChanges changes;
    ConflictResolver conflictResolver( changes, PREFER_REMOTE_CHANGES );
    CommandHandler handler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdID = cmdId++;
    syncParams.sourceDatabase = localDb;
    syncParams.targetDatabase = remoteDb;

    SyncActionData add;

    add.cmdID = cmdId++;
    add.action = SYNCML_ADD;

    ItemParams addItem;
    addItem.source = "fooid";
    addItem.Data = "foodata";
    addItem.meta.type = "mime/foo1";
    add.items.append( addItem );

    syncParams.actionList.append( add );

    SyncActionData replace;

    replace.cmdID = cmdId++;
    replace.action = SYNCML_REPLACE;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.Data = "foodata";
    replaceItem1.meta.type = "mime/foo2";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.Data = "foodata";
    replaceItem2.meta.type = "mime/foo3";
    replace.items.append( replaceItem2 );

    syncParams.actionList.append( replace );

    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
    QSignalSpy processed_spy( &storageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    handler.handleSync( syncParams, target, storageHandler, generator, conflictResolver, false );

    QCOMPARE( processed_spy.count(), 3 );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(0).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(0).at(3).toString(), QString( "mime/foo1" ) );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(1).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(1).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(1).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(1).at(3).toString(), QString( "mime/foo2" ) );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(2).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(2).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(2).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(2).at(3).toString(), QString( "mime/foo3" ) );

    QVERIFY( !target.mapToLocalUID( "fooid" ).isEmpty() );
    QVERIFY( !target.mapToLocalUID( "key1" ).isEmpty() );
    QVERIFY( !target.mapToLocalUID( "key2" ).isEmpty() );

}

void CommandHandlerTest::testSyncReplace()
{

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );

    StorageHandler storageHandler;
    MockStorage storage( localDb );
    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    LocalChanges changes;
    ConflictResolver conflictResolver( changes, PREFER_REMOTE_CHANGES );
    CommandHandler handler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdID = cmdId++;
    syncParams.sourceDatabase = localDb;
    syncParams.targetDatabase = remoteDb;

    SyncActionData replace;

    replace.cmdID = cmdId++;
    replace.action = SYNCML_REPLACE;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.Data = "foodata";
    replaceItem1.meta.type = "mime/foo1";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.Data = "foodata";
    replaceItem2.meta.type = "mime/foo2";
    replace.items.append( replaceItem2 );

    ItemParams replaceItem3;
    replaceItem3.source = "secretkey";
    replaceItem3.Data = "foodata";
    replaceItem3.meta.type = "mime/foo3";
    replace.items.append( replaceItem3 );

    syncParams.actionList.append( replace );

    UIDMapping mapping;
    mapping.iRemoteUID= "key1";
    mapping.iLocalUID= "key1";
    target.addUIDMapping( mapping );

    mapping.iRemoteUID = "key2";
    mapping.iLocalUID = "key2";
    target.addUIDMapping( mapping );

    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
    QSignalSpy processed_spy( &storageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    handler.handleSync( syncParams, target, storageHandler, generator, conflictResolver, false );

    QCOMPARE( processed_spy.count(), 3 );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_ADDED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(0).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(0).at(3).toString(), QString( "mime/foo3" ) );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(1).at(0)), MOD_ITEM_MODIFIED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(1).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(1).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(1).at(3).toString(), QString( "mime/foo1" ) );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(2).at(0)), MOD_ITEM_MODIFIED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(2).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(2).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(2).at(3).toString(), QString( "mime/foo2" ) );

}

void CommandHandlerTest::testSyncDelete()
{
    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );

    StorageHandler storageHandler;
    MockStorage storage( localDb );
    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    LocalChanges changes;
    ConflictResolver conflictResolver( changes, PREFER_REMOTE_CHANGES );
    CommandHandler handler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdID = cmdId++;
    syncParams.sourceDatabase = localDb;
    syncParams.targetDatabase = remoteDb;

    SyncActionData del;

    del.cmdID = cmdId++;
    del.action = SYNCML_DELETE;

    ItemParams deleteItem1;
    deleteItem1.source = "key1";
    del.items.append( deleteItem1 );

    ItemParams deleteItem2;
    deleteItem2.source = "key2";
    del.items.append( deleteItem2 );

    syncParams.actionList.append( del );

    UIDMapping mapping;
    mapping.iRemoteUID = "key1";
    mapping.iLocalUID = "key1";
    target.addUIDMapping( mapping );

    mapping.iRemoteUID = "key2";
    mapping.iLocalUID = "key2";
    target.addUIDMapping( mapping );

    qRegisterMetaType<DataSync::ModificationType>("DataSync::ModificationType");
    qRegisterMetaType<DataSync::ModifiedDatabase>("DataSync::ModifiedDatabase");
    QSignalSpy processed_spy( &storageHandler, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase, QString ,QString, int ) ));

    handler.handleSync( syncParams, target, storageHandler, generator, conflictResolver, false );

    QCOMPARE( processed_spy.count(), 2 );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(0).at(0)), MOD_ITEM_DELETED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(0).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(0).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(0).at(3).toString(), QString( "text/x-vcard" ) );

    QCOMPARE( qvariant_cast<DataSync::ModificationType>(processed_spy.at(1).at(0)), MOD_ITEM_DELETED );
    QCOMPARE( qvariant_cast<DataSync::ModifiedDatabase>(processed_spy.at(1).at(1)), MOD_LOCAL_DATABASE );
    QCOMPARE( processed_spy.at(1).at(2).toString(), localDb );
    QCOMPARE( processed_spy.at(1).at(3).toString(), QString( "text/x-vcard" ) );

}

TESTLOADER_ADD_TEST(CommandHandlerTest);
