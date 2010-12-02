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

#include "StorageHandlerTest.h"
#include "Mock.h"
#include "ConflictResolver.h"
#include "LogMacros.h"

#include "TestLoader.h"

using namespace DataSync;

void StorageHandlerTest::testAddItem()
{

    MockStorage storage( "id" );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString parent("");
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );
    LocalChanges changes;
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    QVERIFY( iStorageHandler.addItem( id, storage, QString(), parent, type, format, version, data ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iStatus == COMMIT_ADDED );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );

}

void StorageHandlerTest::testReplaceItem()
{

    MockStorage storage( "id" );

    LocalChanges changes;
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key( "fookey" );
    QString parent( "" );
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitReplacedItems( storage, &resolver );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iItemKey == key );
    QVERIFY( results[0].iStatus == COMMIT_REPLACED );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );

}

void StorageHandlerTest::testDeleteItem()
{

    MockStorage storage( "id" );

    LocalChanges changes;
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );


    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "fookey";

    QVERIFY( iStorageHandler.deleteItem( id, key ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitDeletedItems( storage, &resolver );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iItemKey == key );
    QVERIFY( results[0].iStatus == COMMIT_DELETED );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );

}

void StorageHandlerTest::testLargeObjectReplace()
{

    MockStorage storage( "id" );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "ab" );
    QString key = "fookey";
    qint64 size = 4;

    QVERIFY( iStorageHandler.startLargeObjectReplace( storage, key, parent, type, format, version, size ) );
    QVERIFY( iStorageHandler.buildingLargeObject() );
    QVERIFY( iStorageHandler.matchesLargeObject( key ) );
    QVERIFY( iStorageHandler.appendLargeObjectData( data ) );
    QVERIFY( iStorageHandler.appendLargeObjectData( data ) );
    QVERIFY( iStorageHandler.finishLargeObject( id ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitReplacedItems( storage, NULL );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iItemKey == key );
    QVERIFY( results[0].iStatus == COMMIT_REPLACED );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );

}

void StorageHandlerTest::regression_NB153991_01()
{
    // regression_NB153991_01:
    // Test that Replace with an empty local key is processed as an Add

    MockStorage storage( "id" );

    LocalChanges changes;
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitReplacedItems( storage, &resolver );
    QVERIFY( commits.count() == 0 );

    commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iStatus == COMMIT_ADDED );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );
}

void StorageHandlerTest::regression_NB203771_01()
{
    MockStorage storage( "id" );

    LocalChanges changes;
    changes.modified.append("key1");
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "key1";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitReplacedItems( storage, &resolver );
    QVERIFY( commits.count() == 1 );
    QList<CommitResult> results = commits.values();

    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iStatus == COMMIT_INIT_REPLACE );
    QVERIFY( results[0].iConflict == CONFLICT_LOCAL_WIN );
}

void StorageHandlerTest::regression_NB203771_02()
{
    MockStorage storage( "id" );

    LocalChanges changes;
    changes.modified.append("key1");
    ConflictResolver resolver( changes, PREFER_REMOTE_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "key1";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.addItem( id, storage, key, parent, type, format, version, data ) );

    QMap<ItemId, CommitResult> commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QVERIFY( commits.count() == 1 );

    QList<CommitResult> results = commits.values();
    QVERIFY( results.count() == 1 );
    qDebug() << results[0].iStatus;
    QVERIFY( results[0].iConflict == CONFLICT_REMOTE_WIN );
    QVERIFY( results[0].iStatus == COMMIT_ADDED );
    QVERIFY( changes.modified.size() == 0 ); //local change removed
}

void StorageHandlerTest::regression_NB203771_03()
{
    MockStorage storage( "id" );

    LocalChanges changes;
    changes.removed.append("key1");
    ConflictResolver resolver( changes, PREFER_REMOTE_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    // should be in add list
    SyncItem *item = iStorageHandler.iAddList.value(id);
    QVERIFY(item);
    item->setKey("key1");
    
    QMap<ItemId, CommitResult> commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QVERIFY( commits.count() == 1 );

    QList<CommitResult> results = commits.values();
    QVERIFY( results.count() == 1 );
    qDebug() << results[0].iStatus;
    QVERIFY( results[0].iConflict == CONFLICT_REMOTE_WIN );
    QVERIFY( results[0].iStatus == COMMIT_ADDED );
    QVERIFY( changes.removed.size() == 0 ); //local change removed
}

void StorageHandlerTest::regression_NB203771_04()
{
    MockStorage storage( "id" );

    LocalChanges changes;
    changes.removed.append("key1");
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    // should be in add list
    SyncItem *item = iStorageHandler.iAddList.value(id);
    QVERIFY(item);
    item->setKey("key1");
    
    QMap<ItemId, CommitResult> commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QVERIFY( commits.count() == 1 );

    QList<CommitResult> results = commits.values();
    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iConflict == CONFLICT_LOCAL_WIN );
    QVERIFY( results[0].iStatus == COMMIT_INIT_ADD );
    QVERIFY( changes.removed.size() == 1 ); 
}

void StorageHandlerTest::regression_NB203771_05()
{
    MockStorage storage( "id" );

    LocalChanges changes;
    changes.removed.append("key11");
    ConflictResolver resolver( changes, PREFER_LOCAL_CHANGES );

    ItemId id;
    id.iCmdId = 1;
    id.iItemIndex = 0;

    QString key = "";
    QString parent = "";
    QString type( "text/x-vcard" );
    QString format("");
    QString version("");
    QString data( "fasdaagadtadg" );

    QVERIFY( iStorageHandler.replaceItem( id, storage, key, parent, type, format, version, data ) );

    // should be in add list
    SyncItem *item = iStorageHandler.iAddList.value(id);
    QVERIFY(item);
    item->setKey("key1");
    
    QMap<ItemId, CommitResult> commits = iStorageHandler.commitAddedItems( storage, &resolver );
    QVERIFY( commits.count() == 1 );

    QList<CommitResult> results = commits.values();
    QVERIFY( results.count() == 1 );
    QVERIFY( results[0].iConflict == CONFLICT_NO_CONFLICT );
    QVERIFY( results[0].iStatus == COMMIT_ADDED );
    QVERIFY( changes.removed.size() == 1 ); 
}

TESTLOADER_ADD_TEST(StorageHandlerTest);
