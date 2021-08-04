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


#include "ChangeLogTest.h"

#include "DatabaseHandler.h"
#include "SyncMode.h"

#include "SyncMLLogging.h"

using namespace DataSync;

const QString DB1( "/tmp/changelogtest1.db" );
const QString DB2( "/tmp/changelogtest2.db" );

void ChangeLogTest::initTestCase()
{
    iDbHandler = new DatabaseHandler( DB1 );
}
void ChangeLogTest::cleanupTestCase()
{
    delete iDbHandler;
    iDbHandler = 0;
}

void ChangeLogTest::testBorrowedGetSetLastSyncTime()
{

    ChangeLog changeLog( "testdevice1", "sourcedb1", DIRECTION_TWO_WAY );

    // Test in-memory modifications
    QDateTime syncTime = QDateTime::currentDateTime();

    changeLog.setLastSyncTime(syncTime);
    QCOMPARE(changeLog.getLastSyncTime(), syncTime );

    // Test database commit & load
    QVERIFY( changeLog.save( iDbHandler->getDbHandle() ) );
    QVERIFY( changeLog.load( iDbHandler->getDbHandle() ) );

    // Compare only in accuracy of 1 second as sql database stores times
    // with the accuracy of 1 second only
    QCOMPARE( changeLog.getLastSyncTime().toTime_t(), syncTime.toTime_t() );

    // Test database remove
    changeLog.remove( iDbHandler->getDbHandle() );
    QVERIFY( !changeLog.load( iDbHandler->getDbHandle() ) );

}

void ChangeLogTest::testBorrowedGetSetAnchor()
{

    ChangeLog changeLog( "testdevice2", "sourcedb2", DIRECTION_TWO_WAY );

    // Test in-memory modifications

    QString local("localanchor");
    QString remote("remoteanchor");

    changeLog.setLastLocalAnchor( local );
    changeLog.setLastRemoteAnchor( remote );
    QCOMPARE( changeLog.getLastLocalAnchor(), local );
    QCOMPARE( changeLog.getLastRemoteAnchor(), remote );

    // Test database commit & load
    QVERIFY( changeLog.save( iDbHandler->getDbHandle() ) );
    QVERIFY( changeLog.load( iDbHandler->getDbHandle() ) );

    QCOMPARE( changeLog.getLastLocalAnchor(), local );
    QCOMPARE( changeLog.getLastRemoteAnchor(), remote );

    // Test database remove
    changeLog.remove( iDbHandler->getDbHandle() );
    QVERIFY( !changeLog.load( iDbHandler->getDbHandle() ) );

}

void ChangeLogTest::testBorrowedGetSetMaps()
{
    ChangeLog changeLog( "testdevice3", "sourcedb3", DIRECTION_TWO_WAY );

    // Test in-memory modifications

    const QString localId1( "local1" );
    const QString remoteId1( "remote1" );
    const QString localId2( "local2");
    const QString remoteId2( "remote2" );

    UIDMapping map1;
    map1.iLocalUID  = localId1;
    map1.iRemoteUID = remoteId1;

    UIDMapping map2;
    map2.iLocalUID = localId2;
    map2.iRemoteUID = remoteId2;

    QList<UIDMapping> maps;
    maps.append(map1);
    maps.append(map2);

    changeLog.setMaps( maps );
    QCOMPARE( maps.count(), 2 );
    QCOMPARE( changeLog.getMaps().count(), 2 );
    QCOMPARE( changeLog.getMaps().at(0).iLocalUID, maps.at(0).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(0).iRemoteUID, maps.at(0).iRemoteUID );
    QCOMPARE( changeLog.getMaps().at(1).iLocalUID, maps.at(1).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(1).iRemoteUID, maps.at(1).iRemoteUID );

    // Test database commit & load
    QVERIFY( changeLog.save( iDbHandler->getDbHandle() ) );
    QVERIFY( changeLog.load( iDbHandler->getDbHandle() ) );

    QCOMPARE( changeLog.getMaps().count(), 2 );
    QCOMPARE( changeLog.getMaps().at(0).iLocalUID, maps.at(0).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(0).iRemoteUID, maps.at(0).iRemoteUID );
    QCOMPARE( changeLog.getMaps().at(1).iLocalUID, maps.at(1).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(1).iRemoteUID, maps.at(1).iRemoteUID );

    // Test database remove
    changeLog.remove( iDbHandler->getDbHandle() );
    QVERIFY( !changeLog.load( iDbHandler->getDbHandle() ) );

}

void ChangeLogTest::testOwnedGetSetLastSyncTime()
{

    ChangeLog changeLog( "testdevice4", "sourcedb4", DIRECTION_TWO_WAY );

    // Test in-memory modifications
    QDateTime syncTime = QDateTime::currentDateTime();

    changeLog.setLastSyncTime(syncTime);
    QCOMPARE(changeLog.getLastSyncTime(), syncTime );

    // Test database commit & load
    QVERIFY( changeLog.save( DB2 ) );
    QVERIFY( changeLog.load( DB2 ) );

    // Compare only in accuracy of 1 second as sql database stores times
    // with the accuracy of 1 second only
    QCOMPARE( changeLog.getLastSyncTime().toTime_t(), syncTime.toTime_t() );

    // Test database remove
    changeLog.remove( DB2 );
    QVERIFY( !changeLog.load( DB2 ) );

}
void ChangeLogTest::testOwnedGetSetLastAnchor()
{
    ChangeLog changeLog( "testdevice5", "sourcedb5", DIRECTION_TWO_WAY );

    // Test in-memory modifications

    QString local("localanchor");
    QString remote("remoteanchor");

    changeLog.setLastLocalAnchor( local );
    changeLog.setLastRemoteAnchor( remote );
    QCOMPARE( changeLog.getLastLocalAnchor(), local );
    QCOMPARE( changeLog.getLastRemoteAnchor(), remote );

    // Test database commit & load
    QVERIFY( changeLog.save( DB2 ) );
    QVERIFY( changeLog.load( DB2 ) );

    QCOMPARE( changeLog.getLastLocalAnchor(), local );
    QCOMPARE( changeLog.getLastRemoteAnchor(), remote );

    // Test database remove
    changeLog.remove( DB2 );
    QVERIFY( !changeLog.load( DB2 ) );
}

void ChangeLogTest::testOwnedGetSetMaps()
{
    ChangeLog changeLog( "testdevice6", "sourcedb6", DIRECTION_TWO_WAY );

    // Test in-memory modifications

    const QString localId1( "local1" );
    const QString remoteId1( "remote1" );
    const QString localId2( "local2");
    const QString remoteId2( "remote2" );

    UIDMapping map1;
    map1.iLocalUID  = localId1;
    map1.iRemoteUID = remoteId1;

    UIDMapping map2;
    map2.iLocalUID = localId2;
    map2.iRemoteUID = remoteId2;

    QList<UIDMapping> maps;
    maps.append(map1);
    maps.append(map2);

    changeLog.setMaps( maps );
    QCOMPARE( maps.count(), 2 );
    QCOMPARE( changeLog.getMaps().count(), 2 );
    QCOMPARE( changeLog.getMaps().at(0).iLocalUID, maps.at(0).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(0).iRemoteUID, maps.at(0).iRemoteUID );
    QCOMPARE( changeLog.getMaps().at(1).iLocalUID, maps.at(1).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(1).iRemoteUID, maps.at(1).iRemoteUID );

    // Test database commit & load
    QVERIFY( changeLog.save( DB2 ) );
    QVERIFY( changeLog.load( DB2 ) );

    QCOMPARE( changeLog.getMaps().count(), 2 );
    QCOMPARE( changeLog.getMaps().at(0).iLocalUID, maps.at(0).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(0).iRemoteUID, maps.at(0).iRemoteUID );
    QCOMPARE( changeLog.getMaps().at(1).iLocalUID, maps.at(1).iLocalUID );
    QCOMPARE( changeLog.getMaps().at(1).iRemoteUID, maps.at(1).iRemoteUID );

    // Test database remove
    changeLog.remove( DB2 );
    QVERIFY( !changeLog.load( DB2 ) );

}

QTEST_MAIN(ChangeLogTest)
