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

#include "LocalMappingsPackageTest.h"
#include "LocalMappingsPackage.h"
#include "SyncMLMessage.h"
#include "TestLoader.h"
#include "internals.h"
#include "Mock.h"
#include "QtEncoder.h"

using namespace DataSync;

void LocalMappingsPackageTest::testPackage()
{
    // Create a mock SyncTarget and add two mappings to it.
    const QString source_db = "source";
    const QString target_db = "target";

    QList<UIDMapping> mappings;
    UIDMapping mapping1 = { "remoteUID1", "localUID1" };
    UIDMapping mapping2 = { "remoteUID2", "localUID2" };

    mappings.append( mapping1 );
    mappings.append( mapping2 );

    // Create a mappings package and verify that mapping is stored.
    LocalMappingsPackage pkg( source_db, target_db, mappings );
    QCOMPARE(pkg.iSourceDatabase, source_db);
    QCOMPARE(pkg.iTargetDatabase, target_db);
    QCOMPARE(pkg.iMappings.size(), 2);
    QCOMPARE(pkg.iMappings.at(0).iLocalUID, mapping1.iLocalUID);
    QCOMPARE(pkg.iMappings.at(0).iRemoteUID, mapping1.iRemoteUID);
    QCOMPARE(pkg.iMappings.at(1).iLocalUID, mapping2.iLocalUID);
    QCOMPARE(pkg.iMappings.at(1).iRemoteUID, mapping2.iRemoteUID);

    // Write, only first mapping should fit.
    SyncMLMessage msg(HeaderParams(), DS_1_2);
    const int SIZE_THRESHOLD = 160;
    int remaining = SIZE_THRESHOLD;
    QCOMPARE(pkg.write(msg, remaining), false);
    QVERIFY(remaining < SIZE_THRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_MAP) != -1);
    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_MAPITEM) != -1);

    // Write again, should complete.
    remaining += 200;
    QCOMPARE(pkg.write(msg, remaining), true);

}



TESTLOADER_ADD_TEST(LocalMappingsPackageTest);
