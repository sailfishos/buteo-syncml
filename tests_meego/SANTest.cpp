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

#include "SANTest.h"

#include <QDebug>

#include "TestUtils.h"
#include "TestLoader.h"
#include "ServerAlertedNotification.h"

using namespace DataSync;

void SANTest::testParser01()
{

    const QString serverIdentifier( "PC Suite Data Sync" );
    const QString password( "" );
    const QString nonce( "" );

    // testParser01: Test parsing of OVI Suite SAN for Contacts sync
    SANHandler parser;

    QByteArray message;
    SANData data;

    QVERIFY( readFile( "testfiles/SAN01.bin", message ) );

    QVERIFY( parser.checkDigest( message, serverIdentifier, password, nonce ) );
    QVERIFY( parser.parseSANMessage( message, data ) );

    QVERIFY( data.iVersion == DS_1_2 );
    QVERIFY( data.iUIMode == SANUIMODE_BACKGROUND );
    QVERIFY( data.iInitiator == SANINITIATOR_SERVER );
    QVERIFY( data.iSessionId == 0 );
    QVERIFY( data.iServerIdentifier == serverIdentifier );

    QVERIFY( data.iSyncInfo.count() == 1 );
    QVERIFY( data.iSyncInfo[0].iSyncType == 206 );
    QVERIFY( data.iSyncInfo[0].iContentType == "" );
    QVERIFY( data.iSyncInfo[0].iServerURI == "Contacts" );

}

void SANTest::testParser02()
{
    const QString serverIdentifier( "PC Suite Data Sync" );
    const QString password( "" );
    const QString nonce( "" );

    // testParser01: Test parsing of OVI Suite SAN for Contacts and
    // calendar sync
    SANHandler parser;

    QByteArray message;
    SANData data;

    QVERIFY( readFile( "testfiles/SAN02.bin", message ) );

    QVERIFY( parser.checkDigest( message, serverIdentifier, password, nonce ) );
    QVERIFY( parser.parseSANMessage( message, data ) );

    QVERIFY( data.iVersion == DS_1_2 );
    QVERIFY( data.iUIMode == SANUIMODE_BACKGROUND );
    QVERIFY( data.iInitiator == SANINITIATOR_SERVER );
    QVERIFY( data.iSessionId == 0 );
    QVERIFY( data.iServerIdentifier == serverIdentifier );

    QVERIFY( data.iSyncInfo.count() == 1 );

    QVERIFY( data.iSyncInfo[0].iSyncType == 206 );
    QVERIFY( data.iSyncInfo[0].iContentType == "" );
    QVERIFY( data.iSyncInfo[0].iServerURI == "Calendar" );
}

void SANTest::testGenerator01()
{
    // testGenerator01: Test generation of SAN package for OVI Suite for
    // for Contacts sync

    SANHandler generator;
    QByteArray message;

    SANData data;
    data.iVersion = DS_1_2;
    data.iUIMode = SANUIMODE_BACKGROUND;
    data.iInitiator = SANINITIATOR_SERVER;
    data.iSessionId = 0;
    data.iServerIdentifier = "PC Suite Data Sync";

    SANSyncInfo syncInfo;
    syncInfo.iSyncType = 206;
    syncInfo.iContentType == "";
    syncInfo.iServerURI = "Contacts";
    data.iSyncInfo.append( syncInfo );

    QVERIFY( generator.generateSANMessage( data, "", "", message ) );

    QByteArray expected;
    QVERIFY( readFile( "testfiles/SAN01.bin", expected ) );
    QCOMPARE( message, expected );

}

TESTLOADER_ADD_TEST( SANTest );
