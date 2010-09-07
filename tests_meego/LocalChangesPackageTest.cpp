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

#include "LocalChangesPackageTest.h"

#include <QTest>

#include "TestLoader.h"
#include "SyncItem.h"
#include "SyncTarget.h"
#include "LocalChangesPackage.h"
#include "SyncMLMessage.h"
#include "QtEncoder.h"
#include "Mock.h"
#include "LogMacros.h"


LocalChangesPackageStorage::LocalChangesPackageStorage( const QString& aSourceURI )
    : iSourceURI( aSourceURI )
{
    StoragePlugin::ContentFormat format;
    format.iType = "text/foo";
    format.iVersion = "1.0";
    iFormats.append( format );
}

LocalChangesPackageStorage::~LocalChangesPackageStorage()
{
    qDeleteAll( iSyncItems );
    iSyncItems.clear();
}

void LocalChangesPackageStorage::setItems( const QList<SyncItem*> aSyncItems )
{
    iSyncItems = aSyncItems;
}

const QString& LocalChangesPackageStorage::getSourceURI() const
{
    return iSourceURI;
}

qint64 LocalChangesPackageStorage::getMaxObjSize() const
{
    return 0;
}

const QList<StoragePlugin::ContentFormat>& LocalChangesPackageStorage::getSupportedFormats() const
{
    return iFormats;
}

const StoragePlugin::ContentFormat& LocalChangesPackageStorage::getPreferredFormat() const
{
    return iFormats[0];
}

QByteArray LocalChangesPackageStorage::getPluginCTCaps( ProtocolVersion aVersion ) const
{
    Q_UNUSED( aVersion );
    return QByteArray();
}

bool LocalChangesPackageStorage::getAll( QList<SyncItemKey>& aKeys )
{
    Q_UNUSED( aKeys );
    return true;
}

bool LocalChangesPackageStorage::getModifications( QList<SyncItemKey>& aNewKeys,
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

SyncItem* LocalChangesPackageStorage::newItem()
{
    return NULL;
}

SyncItem* LocalChangesPackageStorage::getSyncItem( const SyncItemKey& aKey )
{
    SyncItem* item = 0;

    for( int i = 0; i < iSyncItems.count(); ++i )
    {
        if( iSyncItems[i] && iSyncItems[i]->getKey() == aKey )
        {
            item = iSyncItems[i];
            iSyncItems.removeAt(i);
            break;
        }
    }

    return item;
}

QList<SyncItem*> LocalChangesPackageStorage::getSyncItems( const QList<SyncItemKey>& aKeyList )
{
    QList<SyncItem*> items;

    for( int i = 0; i < aKeyList.count(); ++i )
    {
        SyncItem* item = 0;

        for( int a = 0; a < iSyncItems.count(); ++a )
        {
            if( iSyncItems[a] && iSyncItems[a]->getKey() == aKeyList[i] )
            {
                item = iSyncItems[a];
                iSyncItems.removeAt(a);
                break;
            }
        }

        items.append( item );
    }

    return items;
}

QList<StoragePlugin::StoragePluginStatus> LocalChangesPackageStorage::addItems( const QList<SyncItem*>& aItems )
{
    Q_UNUSED( aItems );
    QList<StoragePluginStatus> status;
    return status;
}

QList<StoragePlugin::StoragePluginStatus> LocalChangesPackageStorage::replaceItems( const QList<SyncItem*>& aItems )
{
    Q_UNUSED( aItems );
    QList<StoragePluginStatus> status;
    return status;
}

QList<StoragePlugin::StoragePluginStatus> LocalChangesPackageStorage::deleteItems( const QList<SyncItemKey>& aKeys )
{
    Q_UNUSED( aKeys );
    QList<StoragePluginStatus> status;
    return status;
}

LocalChangesPackageTest::LocalChangesPackageTest()
{

}

LocalChangesPackageTest::~LocalChangesPackageTest()
{

}

void LocalChangesPackageTest::testSimpleClient()
{
    // Simple test for LocalChangesPackage for sending few
    // modifications in a single message in client mode

    const int msgSize = 65535;
    const int maxChanges = 50;

    LocalChangesPackageStorage storage( "./LocalContacts" );

    LocalChanges changes;
    QList<SyncItem*> items;
    const QString itemTypes( "text/foo" );

    const QString addedItemId( "addedItem" );
    const QByteArray addedItemData ( "addedData" );
    MockSyncItem* addedItem = new MockSyncItem( addedItemId );
    addedItem->setType( itemTypes );
    addedItem->write( 0, addedItemData );
    items.append( addedItem );
    changes.added.append( addedItemId );


    const QString replacedItemId( "replacedItem" );
    const QByteArray replacedItemData ( "replacedData" );
    MockSyncItem* replacedItem = new MockSyncItem( replacedItemId );
    replacedItem->setType( itemTypes );
    replacedItem->write( 0, replacedItemData );
    items.append( replacedItem );
    changes.modified.append( replacedItemId );

    const QString deletedItemId( "deletedItem" );
    changes.removed.append( deletedItemId );

    storage.setItems( items );

    SyncMode syncMode;
    SyncTarget target( NULL, &storage, syncMode, "localAnchor" );
    target.setTargetDatabase( "./RemoteContacts");

    LocalChangesPackage package( target, changes, msgSize, ROLE_CLIENT, maxChanges );

    SyncMLMessage msg( HeaderParams(), DS_1_2 );

    int remaining = msgSize;
    QVERIFY( package.write( msg, remaining ) );
    QVERIFY( remaining < msgSize );

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    // Check that the items were written
    QVERIFY( result_xml.contains( addedItemId.toAscii() ) );
    QVERIFY( result_xml.contains( addedItemData ) );
    QVERIFY( result_xml.contains( replacedItemId.toAscii() ) );
    QVERIFY( result_xml.contains( replacedItemData ) );
    QVERIFY( result_xml.contains( deletedItemId.toAscii() ) );

}

void LocalChangesPackageTest::testSimpleServer()
{
    // Simple test for LocalChangesPackage for sending few
    // modifications in a single message in server mode

    const int msgSize = 65535;
    const int maxChanges = 50;

    LocalChangesPackageStorage storage( "./LocalContacts" );

    LocalChanges changes;
    QList<SyncItem*> items;
    const QString itemTypes( "text/foo" );

    const QString addedItemId( "addedItem" );
    const QByteArray addedItemData ( "addedData" );
    MockSyncItem* addedItem = new MockSyncItem( addedItemId );
    addedItem->setType( itemTypes );
    addedItem->write( 0, addedItemData );
    items.append( addedItem );
    changes.added.append( addedItemId );


    const QString replacedItemId( "replacedItem" );
    const QByteArray replacedItemData ( "replacedData" );
    MockSyncItem* replacedItem = new MockSyncItem( replacedItemId );
    replacedItem->setType( itemTypes );
    replacedItem->write( 0, replacedItemData );
    items.append( replacedItem );
    changes.modified.append( replacedItemId );

    const QString deletedItemId( "deletedItem" );
    changes.removed.append( deletedItemId );

    storage.setItems( items );

    SyncMode syncMode;
    SyncTarget target( NULL, &storage, syncMode, "localAnchor" );
    target.setTargetDatabase( "./RemoteContacts");

    const QString mappedReplacedItemId( "replacedItemMapped" );
    UIDMapping replacedMap;
    replacedMap.iLocalUID = replacedItemId;
    replacedMap.iRemoteUID = mappedReplacedItemId;
    target.addUIDMapping( replacedMap );

    const QString mappedDeletedItemId( "deletedItemMapped" );
    UIDMapping deletedMap;
    deletedMap.iLocalUID = deletedItemId;
    deletedMap.iRemoteUID = mappedDeletedItemId;
    target.addUIDMapping( deletedMap );

    LocalChangesPackage package( target, changes, msgSize, ROLE_SERVER, maxChanges );

    SyncMLMessage msg( HeaderParams(), DS_1_2 );

    int remaining = msgSize;
    QVERIFY( package.write( msg, remaining ) );
    QVERIFY( remaining < msgSize );

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    // Check that the items were written

    QVERIFY( result_xml.contains( addedItemId.toAscii() ) );
    QVERIFY( result_xml.contains( addedItemData ) );
    QVERIFY( result_xml.contains( mappedReplacedItemId.toAscii() ) );
    QVERIFY( result_xml.contains( replacedItemData ) );
    QVERIFY( result_xml.contains( mappedDeletedItemId.toAscii() ) );


}

void LocalChangesPackageTest::testLargeObjects()
{
    // Test for LocalChangesPackage for checking if sending of large objects
    // functions properly

    const int msgSize = 1024;
    const int objSize = 1536;
    const int maxChanges = 50;

    LocalChangesPackageStorage storage( "./LocalContacts" );

    LocalChanges changes;
    QList<SyncItem*> items;
    const QString itemTypes( "text/foo" );

    const QString addedItemId( "addedItem" );
    QByteArray addedItemData;
    addedItemData.fill( '0', objSize );
    MockSyncItem* addedItem = new MockSyncItem( addedItemId );
    addedItem->setType( itemTypes );
    addedItem->write( 0, addedItemData );
    items.append( addedItem );
    changes.added.append( addedItemId );

    storage.setItems( items );

    SyncMode syncMode;
    SyncTarget target( NULL, &storage, syncMode, "localAnchor" );
    target.setTargetDatabase( "./RemoteContacts");

    LocalChangesPackage package( target, changes, msgSize, ROLE_CLIENT, maxChanges );

    int remaining = msgSize;
    SyncMLMessage msg1( HeaderParams(), DS_1_2 );
    QVERIFY( !package.write( msg1, remaining ) );
    QVERIFY( remaining < msgSize );

    QtEncoder encoder;
    QByteArray result_xml1;
    QVERIFY( encoder.encodeToXML( msg1, result_xml1, true ) );

    // Check that the item was written with MoreData

    QVERIFY( result_xml1.contains( addedItemId.toAscii() ) );
    QVERIFY( result_xml1.contains( "MoreData" ) );

    remaining = msgSize;
    SyncMLMessage msg2( HeaderParams(), DS_1_2 );
    QVERIFY( package.write( msg2, remaining ) );
    QVERIFY( remaining < msgSize );

    QByteArray result_xml2;
    QVERIFY( encoder.encodeToXML( msg2, result_xml2, true ) );

    // Check that the rest of the item was written

    QVERIFY( result_xml2.contains( addedItemId.toAscii() ) );
    QVERIFY( !result_xml2.contains( "MoreData" ) );

}
TESTLOADER_ADD_TEST(LocalChangesPackageTest);
