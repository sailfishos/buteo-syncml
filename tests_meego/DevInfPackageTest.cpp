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

#include "DevInfPackageTest.h"

#include "DevInfPackage.h"
#include "SyncMLMessage.h"
#include "TestLoader.h"
#include "Mock.h"
#include "QtEncoder.h"
#include "DeviceInfo.h"
#include "Fragments.h"

void DevInfPackageTest::testPutGet()
{

    QList<StoragePlugin*> storage_plugins;
    DeviceInfo devInfo;
    MockStorage storage("storage");
    storage_plugins.append(&storage);
    const int SIZE_TRESHOLD = 10000;

    DevInfPackage pkg(storage_plugins, devInfo, SYNCML_1_2, ROLE_CLIENT );

    SyncMLMessage msg(HeaderParams(), SYNCML_1_2);
    int remaining = SIZE_TRESHOLD;
    QCOMPARE(pkg.write(msg, remaining, false, SYNCML_1_2), true);
    QVERIFY(remaining < SIZE_TRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QByteArray putData = extractElement( result_xml, "<Put>", "</Put>" );
    QVERIFY( !putData.isEmpty() );

    verifyDevInf(putData);

    QByteArray getData = extractElement( result_xml, "<Get>", "</Get>" );
    QVERIFY( !getData.isEmpty() );

    verifyGet(getData);

}

void DevInfPackageTest::testResults()
{

    QList<StoragePlugin*> storage_plugins;
    DeviceInfo devInfo;
    MockStorage storage("storage");
    storage_plugins.append(&storage);
    const int msgId = 99;
    const int cmdId = 109;
    const int SIZE_TRESHOLD = 10000;
    bool retrieveRemoteDevInf = false;

    DevInfPackage pkg(msgId, cmdId, storage_plugins, devInfo, SYNCML_1_2, ROLE_CLIENT, retrieveRemoteDevInf );

    SyncMLMessage msg(HeaderParams(), SYNCML_1_2);
    int remaining = SIZE_TRESHOLD;
    QCOMPARE(pkg.write(msg, remaining, false, SYNCML_1_2), true);
    QVERIFY(remaining < SIZE_TRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QByteArray resultsData  = extractElement( result_xml, "<Results>", "</Results>" );
    QVERIFY( !resultsData.isEmpty() );

    QByteArray msgRef = extractElement( resultsData, "<MsgRef>", "</MsgRef" );
    QVERIFY( !msgRef.isEmpty() );
    QCOMPARE( msgRef.toInt(), msgId );

    QByteArray cmdRef = extractElement( resultsData, "<CmdRef>", "</CmdRef" );
    QVERIFY( !cmdRef.isEmpty() );
    QCOMPARE( cmdRef.toInt(), cmdId );

    verifyDevInf(resultsData);

    QByteArray getData = extractElement( result_xml, "<Get>", "</Get>" );
    QVERIFY( getData.isEmpty() );

}

void DevInfPackageTest::testResultsGet()
{

    QList<StoragePlugin*> storage_plugins;
    DeviceInfo devInfo;
    MockStorage storage("storage");
    storage_plugins.append(&storage);
    const int msgId = 99;
    const int cmdId = 109;
    const int SIZE_TRESHOLD = 10000;
    bool retrieveRemoteDevInf = true;

    DevInfPackage pkg(msgId, cmdId, storage_plugins, devInfo, SYNCML_1_2, ROLE_CLIENT, retrieveRemoteDevInf );

    SyncMLMessage msg(HeaderParams(), SYNCML_1_2);
    int remaining = SIZE_TRESHOLD;
    QCOMPARE(pkg.write(msg, remaining, false, SYNCML_1_2), true);
    QVERIFY(remaining < SIZE_TRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QByteArray resultsData  = extractElement( result_xml, "<Results>", "</Results>" );
    QVERIFY( !resultsData.isEmpty() );

    QByteArray msgRef = extractElement( resultsData, "<MsgRef>", "</MsgRef" );
    QVERIFY( !msgRef.isEmpty() );
    QCOMPARE( msgRef.toInt(), msgId );

    QByteArray cmdRef = extractElement( resultsData, "<CmdRef>", "</CmdRef" );
    QVERIFY( !cmdRef.isEmpty() );
    QCOMPARE( cmdRef.toInt(), cmdId );

    verifyDevInf(resultsData);

    QByteArray getData = extractElement( result_xml, "<Get>", "</Get>" );
    QVERIFY( !getData.isEmpty() );

    verifyGet(getData);

}

QByteArray DevInfPackageTest::extractElement( const QByteArray& aData, const QByteArray& startElement,
                                              const QByteArray& endElement )
{
    QByteArray result;

    int start = aData.indexOf( startElement );

    if( start == -1 )
    {
        return result;
    }

    int end = aData.indexOf( endElement, start );

    if( end == -1 )
    {
        return result;
    }

    result = aData.mid( start + startElement.length(), end - start - startElement.length() );

    return result;
}

void DevInfPackageTest::verifyDevInf( const QByteArray& aData )
{
    QVERIFY( aData.contains( "<DevInf") );
    QVERIFY( aData.contains( "</DevInf>") );

    QVERIFY( aData.contains( "application/vnd.syncml-devinf+xml") );
    QVERIFY( aData.contains( "./devinf12") );

    QVERIFY( aData.contains( "syncml:devinf") );
}

void DevInfPackageTest::verifyGet( const QByteArray& aData )
{
    QVERIFY( aData.contains( "application/vnd.syncml-devinf+xml") );
    QVERIFY( aData.contains( "./devinf12") );
}

TESTLOADER_ADD_TEST(DevInfPackageTest);
