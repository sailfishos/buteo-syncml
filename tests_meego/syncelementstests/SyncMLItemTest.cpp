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

#include "SyncMLItemTest.h"

#include "SyncMLItem.h"
#include "SyncItemKey.h"
#include "QtEncoder.h"
#include "LibWbXML2Encoder.h"
#include "TestUtils.h"
#include "TestLoader.h"
#include "datatypes.h"

#include <QtTest>

using namespace DataSync;

void SyncMLItemTest::testSyncMLItem()
{
    SyncItemKey source("contacts");
    SyncItemKey target("calendar");
    SyncItemKey sourceParent("contactsparent");
    SyncItemKey targetParent("calendarparent");

    SyncMLItem item;
    item.insertSource(source);
    item.insertTarget(target);
    item.insertSourceParent(sourceParent);
    item.insertTargetParent(targetParent);

    QByteArray data;

    QVERIFY( readFile("testfiles/testItem.txt", data ) );

    QtEncoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToXML( item, output, false ) );
    QCOMPARE( output.simplified(), data.simplified() );

}

void SyncMLItemTest::regressionNB188615_01()
{
    // Regression test for NB#188615: make sure that data encoded
    // in UTF-8 is written properly when constructing XML using
    // Qt
    const char utf[] = { 0xC5, 0x9F, 0 };

    QByteArray data( utf );

    SyncMLItem item;

    item.insertData( data );

    QtEncoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToXML( item, output, false ) );

    QVERIFY( output.contains( data ) );

}

void SyncMLItemTest::regressionNB188615_02()
{
    // Regression test for NB#188615: make sure that data encoded
    // in UTF-8 is written properly when constructing XML using
    // libwbxml2
    const char utf[] = { 0xC5, 0x9F, 0 };
    QByteArray data( utf );

    SyncMLItem item;
    // Add namespace attribute, needed by libwbxml2
    item.addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML12 );
    item.insertData( data );

    LibWbXML2Encoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToXML( item, SYNCML_1_2, output, false ) );

    QVERIFY( output.contains( data ) );

}

void SyncMLItemTest::regressionNB188615_03()
{
    // Regression test for NB#188615: make sure that data encoded
    // in UTF-8 is written properly when constructing WnXML using
    // libwbxml2
    const char utf[] = { 0xC5, 0x9F, 0 };

    QByteArray data( utf );

    SyncMLItem item;
    // Add namespace attribute, needed by libwbxml2
    item.addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML12 );
    item.insertData( data );

    LibWbXML2Encoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToWbXML( item, SYNCML_1_2, output ) );

    QVERIFY( output.contains( data ) );

}

TESTLOADER_ADD_TEST(SyncMLItemTest);
