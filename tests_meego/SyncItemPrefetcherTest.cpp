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

#include "SyncItemPrefetcherTest.h"

#include <QTest>

#include "SyncItemPrefetcher.h"
#include "Mock.h"
#include "TestLoader.h"

#include "LogMacros.h"

PrefetchStorage::PrefetchStorage( const QList<SyncItemKey>& aItemIds )
    : iItemIds( aItemIds ), iForceSyncItems( false )
{
    ContentFormat format;
    format.iType = "text/foo";
    format.iVersion = "1.0";
    iFormats.append( format );
}

PrefetchStorage::~PrefetchStorage()
{
    qDeleteAll( iSyncItems );
    iSyncItems.clear();
}

void PrefetchStorage::forceSyncItems( const QList<SyncItem*> aSyncItems )
{
    qDeleteAll( iSyncItems );
    iSyncItems.clear();

    iForceSyncItems = true;
    iSyncItems = aSyncItems;
}

const QString& PrefetchStorage::getSourceURI() const
{
    return iSourceURI;
}

qint64 PrefetchStorage::getMaxObjSize() const
{
    return 0;
}

const QList<ContentFormat>& PrefetchStorage::getSupportedFormats() const
{
    return iFormats;
}

const ContentFormat& PrefetchStorage::getPreferredFormat() const
{
    return iFormats[0];
}

QByteArray PrefetchStorage::getPluginCTCaps( ProtocolVersion aVersion ) const
{
    Q_UNUSED( aVersion );
    return QByteArray();
}

bool PrefetchStorage::getAll( QList<SyncItemKey>& aKeys )
{
    Q_UNUSED( aKeys );
    return true;
}

bool PrefetchStorage::getModifications( QList<SyncItemKey>& aNewKeys,
                                        QList<SyncItemKey>& aReplacedKeys,
                                        QList<SyncItemKey>& aDeletedKeys,
                                        const QDateTime& aTimeStamp )
{
    Q_UNUSED( aNewKeys );
    Q_UNUSED( aReplacedKeys );
    Q_UNUSED( aDeletedKeys );
    Q_UNUSED( aTimeStamp );
    return true;
}

SyncItem* PrefetchStorage::newItem()
{
    return NULL;
}

SyncItem* PrefetchStorage::getSyncItem( const SyncItemKey& aKey )
{
   Q_UNUSED( aKey );
   return NULL;
}

QList<SyncItem*> PrefetchStorage::getSyncItems( const QList<SyncItemKey>& aKeyList )
{
    if( iForceSyncItems )
    {
        QList<SyncItem*> items = iSyncItems;
        iSyncItems.clear();
        return items;
    }
    else
    {
        QList<SyncItem*> items;
        for( int i = 0; i < aKeyList.count(); ++i )
        {
            MockSyncItem* item = NULL;
            if( iItemIds.contains( aKeyList[i] ) )
            {
                item = new MockSyncItem( aKeyList[i] );
            }
            items.append( item );
        }

        return items;
    }
}

QList<StoragePlugin::StoragePluginStatus> PrefetchStorage::addItems( const QList<SyncItem*>& aItems )
{
    Q_UNUSED( aItems );
    QList<StoragePluginStatus> status;
    return status;
}

QList<StoragePlugin::StoragePluginStatus> PrefetchStorage::replaceItems( const QList<SyncItem*>& aItems )
{
    Q_UNUSED( aItems );
    QList<StoragePluginStatus> status;
    return status;
}

QList<StoragePlugin::StoragePluginStatus> PrefetchStorage::deleteItems( const QList<SyncItemKey>& aKeys )
{
    Q_UNUSED( aKeys );
    QList<StoragePluginStatus> status;
    return status;
}


void SyncItemPrefetcherTest::testNormalNoIdle()
{
    // Test item prefetcher using normal sequence without idle prefetching

    QList<SyncItemKey> items;
    items.append( "1" );
    items.append( "2" );
    items.append( "3" );
    items.append( "4" );
    items.append( "5" );
    const int batchSizeHint = 2;

    PrefetchStorage storage( items );

    SyncItemPrefetcher prefetcher( items, storage, batchSizeHint );

    QCOMPARE( prefetcher.iBatchSizeHint, batchSizeHint );
    QCOMPARE( &prefetcher.iStoragePlugin, &storage );
    QCOMPARE( prefetcher.iItemIdList, items );
    QVERIFY( prefetcher.iFetchedItems.isEmpty() );

    SyncItem* item = prefetcher.getItem( items.at(0) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    delete item;

    item = prefetcher.getItem( items.at(1) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), 0 );
    delete item;

    item = prefetcher.getItem( items.at(2) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - 2 - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    delete item;

    item = prefetcher.getItem( items.at(3) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - 2 - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), 0 );
    delete item;

    item = prefetcher.getItem( items.at(4) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), 0 );
    QCOMPARE( prefetcher.iFetchedItems.count(), 0 );
    delete item;

}

void SyncItemPrefetcherTest::testNormalIdle()
{

    // Test item prefetcher using normal sequence with idle prefetching

    QList<SyncItemKey> items;
    items.append( "1" );
    items.append( "2" );
    items.append( "3" );
    items.append( "4" );
    items.append( "5" );
    const int batchSizeHint = 2;

    PrefetchStorage storage( items );

    SyncItemPrefetcher prefetcher( items, storage, batchSizeHint );

    QCOMPARE( prefetcher.iBatchSizeHint, batchSizeHint );
    QCOMPARE( &prefetcher.iStoragePlugin, &storage );
    QCOMPARE( prefetcher.iItemIdList, items );
    QVERIFY( prefetcher.iFetchedItems.isEmpty() );

    // Items going to "first message"
    SyncItem* item = prefetcher.getItem( items.at(0) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    delete item;

    item = prefetcher.getItem( items.at(1) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), 0 );
    delete item;

    prefetcher.prefetch();

    // Item going to "second message"
    item = prefetcher.getItem( items.at(2) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - 2 - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    delete item;

    prefetcher.prefetch();

    // Item going to "third message"
    item = prefetcher.getItem( items.at(3) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), items.count() - 3 - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), 1 );
    delete item;

    prefetcher.prefetch();

    // Item going to "fourth message"

    item = prefetcher.getItem( items.at(4) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), 0 );
    QCOMPARE( prefetcher.iFetchedItems.count(), 0 );
    delete item;

    // Unnecessary prefetch call at the end of session

    prefetcher.prefetch();

}

void SyncItemPrefetcherTest::testAbnormalBadItems()
{
    // Test item prefetcher when some or all items returned by plugin are NULL

    QList<SyncItemKey> itemsIds;
    itemsIds.append( "1" );
    itemsIds.append( "2" );
    itemsIds.append( "3" );
    const int batchSizeHint = 2;

    PrefetchStorage storage( itemsIds );

    SyncItemPrefetcher prefetcher( itemsIds, storage, batchSizeHint );

    QCOMPARE( prefetcher.iBatchSizeHint, batchSizeHint );
    QCOMPARE( &prefetcher.iStoragePlugin, &storage );
    QCOMPARE( prefetcher.iItemIdList, itemsIds );
    QVERIFY( prefetcher.iFetchedItems.isEmpty() );

    QList<SyncItem*> items;
    items.append( new MockSyncItem( "1" ) );
    items.append( NULL );
    storage.forceSyncItems(items);

    SyncItem* item = prefetcher.getItem( itemsIds.at(0) );
    QVERIFY( item );
    QCOMPARE( prefetcher.iItemIdList.count(), itemsIds.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    // Items will be deleted by storage
    //delete item;

    item = prefetcher.getItem( itemsIds.at(1) );
    QVERIFY( !item );
    QCOMPARE( prefetcher.iItemIdList.count(), itemsIds.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 2 );
    // Items will be deleted by storage
    //delete item;

}

void SyncItemPrefetcherTest::testAbnormalBadItemCount()
{
    // Test item prefetcher when plugin returns invalid amount of items
    QList<SyncItemKey> itemsIds;
    itemsIds.append( "1" );
    itemsIds.append( "2" );
    itemsIds.append( "3" );
    const int batchSizeHint = 2;

    PrefetchStorage storage( itemsIds );

    SyncItemPrefetcher prefetcher( itemsIds, storage, batchSizeHint );

    QCOMPARE( prefetcher.iBatchSizeHint, batchSizeHint );
    QCOMPARE( &prefetcher.iStoragePlugin, &storage );
    QCOMPARE( prefetcher.iItemIdList, itemsIds );
    QVERIFY( prefetcher.iFetchedItems.isEmpty() );

    QList<SyncItem*> items;
    items.append( NULL );
    items.append( NULL );
    storage.forceSyncItems(items);

    SyncItem* item = prefetcher.getItem( itemsIds.at(0) );
    QVERIFY( !item );
    QCOMPARE( prefetcher.iItemIdList.count(), itemsIds.count() - batchSizeHint );
    QCOMPARE( prefetcher.iFetchedItems.count(), batchSizeHint - 1 );
    delete item;
}

TESTLOADER_ADD_TEST(SyncItemPrefetcherTest);
