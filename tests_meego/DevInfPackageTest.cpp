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

void DevInfPackageTest::testPackage()
{
    QList<StoragePlugin*> storage_plugins;
    DeviceInfo devInfo;
    MockStorage storage("storage");
    storage_plugins.append(&storage);

    const int SIZE_TRESHOLD = 10000;

    // One test section for each constructor type.
    {
        DevInfPackage pkg(storage_plugins, devInfo, DS_1_2, ROLE_CLIENT );
        SyncMLMessage msg(HeaderParams(), DS_1_2);
        int remaining = SIZE_TRESHOLD;
        QCOMPARE(pkg.write(msg, remaining), true);
        QVERIFY(remaining < SIZE_TRESHOLD);

        QtEncoder encoder;
        QByteArray result_xml;
        QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_MSGREF) == -1);
        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_CMDREF) == -1);
    }

    {
        DevInfPackage pkg(1, 2, storage_plugins, devInfo, DS_1_2, ROLE_CLIENT, false );
        SyncMLMessage msg(HeaderParams(), DS_1_2);
        int remaining = SIZE_TRESHOLD;
        QCOMPARE(pkg.write(msg, remaining), true);
        QVERIFY(remaining < SIZE_TRESHOLD);

        QtEncoder encoder;
        QByteArray result_xml;
        QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_MSGREF) != -1);
        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_CMDREF) != -1);

        QVERIFY(result_xml.contains("text/x-vBookmark"));
        QVERIFY(result_xml.contains("application/vnd.omads-folder+xml"));
    }

    {
        DevInfPackage pkg(1, 2, storage_plugins, devInfo, DS_1_1, ROLE_CLIENT, false );
        SyncMLMessage msg(HeaderParams(), DS_1_1);
        int remaining = SIZE_TRESHOLD;
        QCOMPARE(pkg.write(msg, remaining), true);
        QVERIFY(remaining < SIZE_TRESHOLD);

        QtEncoder encoder;
        QByteArray result_xml;
        QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_MSGREF) != -1);
        QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_CMDREF) != -1);

        QVERIFY(result_xml.contains("text/x-vBookmark"));
        QVERIFY(!result_xml.contains("application/vnd.omads-folder+xml"));

    }
}

TESTLOADER_ADD_TEST(DevInfPackageTest);
