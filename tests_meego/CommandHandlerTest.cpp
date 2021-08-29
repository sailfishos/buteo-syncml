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

#include "CommandHandler.h"
#include "ResponseGenerator.h"
#include "ChangeLog.h"
#include "ConflictResolver.h"
#include "QtEncoder.h"
#include "SyncMLMessage.h"
#include "DeviceInfo.h"
#include "Mock.h"
#include "TestUtils.h"
#include "SyncMLLogging.h"


using namespace DataSync;


CommandHandlerTest::CommandHandlerTest()
{

}

CommandHandlerTest::~CommandHandlerTest()
{

}

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

    syncParams.cmdId = cmdId++;
    syncParams.source = remoteDb;
    syncParams.target = localDb;

    CommandParams add( CommandParams::COMMAND_ADD );
    add.cmdId = cmdId++;

    ItemParams addItem;
    addItem.source = itemId;
    addItem.data = "foodata";
    addItem.meta.type = mime;
    add.items.append( addItem );

    syncParams.commands.append( add );

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

    syncParams.cmdId = cmdId++;
    syncParams.source = remoteDb;
    syncParams.target = localDb;

    CommandParams add( CommandParams::COMMAND_ADD );
    add.cmdId = cmdId++;

    ItemParams addItem;
    addItem.source = itemId;
    addItem.data = "foodata";
    addItem.meta.type = mime;
    add.items.append( addItem );

    syncParams.commands.append( add );

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

    syncParams.cmdId = cmdId++;
    syncParams.source = localDb;
    syncParams.target = remoteDb;

    CommandParams add( CommandParams::COMMAND_ADD );
    add.cmdId = cmdId++;

    ItemParams addItem;
    addItem.source = "fooid";
    addItem.data = "foodata";
    addItem.meta.type = "mime/foo1";
    add.items.append( addItem );

    syncParams.commands.append( add );

    CommandParams replace( CommandParams::COMMAND_REPLACE );
    replace.cmdId = cmdId++;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.data = "foodata";
    replaceItem1.meta.type = "mime/foo2";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.data = "foodata";
    replaceItem2.meta.type = "mime/foo3";
    replace.items.append( replaceItem2 );

    syncParams.commands.append( replace );

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

    syncParams.cmdId = cmdId++;
    syncParams.source = localDb;
    syncParams.target = remoteDb;

    CommandParams replace( CommandParams::COMMAND_REPLACE );
    replace.cmdId = cmdId++;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.data = "foodata";
    replaceItem1.meta.type = "mime/foo1";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.data = "foodata";
    replaceItem2.meta.type = "mime/foo2";
    replace.items.append( replaceItem2 );

    ItemParams replaceItem3;
    replaceItem3.source = "secretkey";
    replaceItem3.data = "foodata";
    replaceItem3.meta.type = "mime/foo3";
    replace.items.append( replaceItem3 );

    syncParams.commands.append( replace );

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

    syncParams.cmdId = cmdId++;
    syncParams.source = localDb;
    syncParams.target = remoteDb;

    CommandParams del( CommandParams::COMMAND_DELETE );
    del.cmdId = cmdId++;


    ItemParams deleteItem1;
    deleteItem1.source = "key1";
    del.items.append( deleteItem1 );

    ItemParams deleteItem2;
    deleteItem2.source = "key2";
    del.items.append( deleteItem2 );

    syncParams.commands.append( del );

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

void CommandHandlerTest::testSyncReplaceConflict()
{

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );

    StorageHandler storageHandler;
    MockStorage storage( localDb );
    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    LocalChanges changes;
    changes.modified.append(QString("key1"));
    changes.removed.append(QString("key2"));
    changes.removed.append(QString("secretkey"));
    ConflictResolver conflictResolver( changes, PREFER_REMOTE_CHANGES );
    CommandHandler handler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdId = cmdId++;
    syncParams.source = localDb;
    syncParams.target = remoteDb;

    CommandParams replace( CommandParams::COMMAND_REPLACE );
    replace.cmdId = cmdId++;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.data = "foodata";
    replaceItem1.meta.type = "mime/foo1";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.data = "foodata";
    replaceItem2.meta.type = "mime/foo2";
    replace.items.append( replaceItem2 );

    ItemParams replaceItem3;
    replaceItem3.source = "secretkey";
    replaceItem3.data = "foodata";
    replaceItem3.meta.type = "mime/foo3";
    replace.items.append( replaceItem3 );

    syncParams.commands.append( replace );

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

void CommandHandlerTest::testSyncReplaceConflict_01()
{

    QString localDb( "localdb" );
    QString remoteDb( "remotedb" );

    StorageHandler storageHandler;
    MockStorage storage( localDb );
    SyncMode mode;
    QString anchor;
    SyncTarget target( NULL, &storage, mode, anchor );

    LocalChanges changes;
    changes.modified.append(QString("key1"));
    changes.removed.append(QString("key2"));
    ConflictResolver conflictResolver( changes, PREFER_LOCAL_CHANGES );
    CommandHandler handler( ROLE_SERVER );
    ResponseGenerator generator;
    generator.setRemoteMsgId( 1 );

    SyncParams syncParams;

    int cmdId = 1;

    syncParams.cmdId = cmdId++;
    syncParams.source = localDb;
    syncParams.target = remoteDb;

    CommandParams replace( CommandParams::COMMAND_REPLACE );
    replace.cmdId = cmdId++;

    ItemParams replaceItem1;
    replaceItem1.source = "key1";
    replaceItem1.data = "foodata";
    replaceItem1.meta.type = "mime/foo1";
    replace.items.append( replaceItem1 );

    ItemParams replaceItem2;
    replaceItem2.source = "key2";
    replaceItem2.data = "foodata";
    replaceItem2.meta.type = "mime/foo2";
    replace.items.append( replaceItem2 );

    syncParams.commands.append( replace );

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

    QCOMPARE( processed_spy.count(), 0 );

}

void CommandHandlerTest::testGetStatusType()
{
    CommandHandler handler(ROLE_SERVER);

    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(-50)), UNKNOWN);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(0)), UNKNOWN);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(50)), UNKNOWN);

    QCOMPARE(handler.getStatusType(IN_PROGRESS), INFORMATIONAL);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(199)), UNKNOWN);

    QCOMPARE(handler.getStatusType(SUCCESS), SUCCESSFUL);
    QCOMPARE(handler.getStatusType(ATOMIC_ROLLBACK_OK), SUCCESSFUL);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(299)), UNKNOWN);

    QCOMPARE(handler.getStatusType(MULTIPLE_CHOICES), REDIRECTION);
    QCOMPARE(handler.getStatusType(USE_PROXY), REDIRECTION);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(399)), UNKNOWN);

    QCOMPARE(handler.getStatusType(BAD_REQUEST), ORIGINATOR_EXCEPTION);
    QCOMPARE(handler.getStatusType(MOVE_FAILED), ORIGINATOR_EXCEPTION);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(499)), UNKNOWN);


    QCOMPARE(handler.getStatusType(COMMAND_FAILED), RECIPIENT_EXCEPTION);
    QCOMPARE(handler.getStatusType(ATOMIC_RESPONSE_TOO_LARGE), RECIPIENT_EXCEPTION);
    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(599)), UNKNOWN);

    QCOMPARE(handler.getStatusType(static_cast<ResponseStatusCode>(1000)), UNKNOWN);
}

void CommandHandlerTest::testNotImplemented()
{
    CommandHandler handler(ROLE_SERVER);
    QCOMPARE(handler.handleRedirection(MULTIPLE_CHOICES), NOT_IMPLEMENTED);
}

void CommandHandlerTest::testHandleMap()
{
    CommandHandler handler(ROLE_SERVER);
    const QString src1("fooSource1");
    const QString src2("fooSource2");
    const QString trg1("barTarget1");
    const QString trg2("barTarget2");

    SyncTarget target(new ChangeLog("sourceDevice", "targetDevice", DIRECTION_TWO_WAY),
                      new MockStorage("storage"),
                      SyncMode(),
                      QString());
    MapParams map;

    MapItemParams param;
    param.source = src1;
    param.target = trg1;
    map.mapItems.append(param);

    param.source = src2;
    param.target = trg2;
    map.mapItems.append(param);

    QCOMPARE(handler.handleMap(map, target), SUCCESS);
    QCOMPARE(target.getUIDMappings().size(), 2);
    QCOMPARE(target.getUIDMappings().at(0).iRemoteUID, src1);
    QCOMPARE(target.getUIDMappings().at(0).iLocalUID, trg1);
    QCOMPARE(target.getUIDMappings().at(1).iRemoteUID, src2);
    QCOMPARE(target.getUIDMappings().at(1).iLocalUID, trg2);
}

QTEST_MAIN(DataSync::CommandHandlerTest)
