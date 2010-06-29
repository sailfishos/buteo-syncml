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
#include "LocalChangesPackage.h"
#include "TestLoader.h"
#include "internals.h"
#include "Mock.h"
#include "SyncMLMessage.h"
#include "QtEncoder.h"

//#include <Logger.h>
//#include <LogMacros.h>

#include <iostream>

using namespace DataSync;

// Mock class for storage.
class LCPTStorage: public MockStorage
{
public:
    LCPTStorage(QString id) : MockStorage(id) { }

    virtual SyncItem* getSyncItem ( const SyncItemKey& aKey )
    {
        if (aKey == "invalid") {
            return NULL;
        }
        else {
            MockSyncItem* item = new MockSyncItem( aKey );
            item->write(0, QByteArray( "dataadataa" ) );
            return item;
        }
    }
};


void LocalChangesPackageTest::initTestCase()
{
//	Logger::createInstance("/tmp/sync-tests-log" , true);
}

void LocalChangesPackageTest::cleanupTestCase()
{
//    Logger::deleteInstance();
}


void LocalChangesPackageTest::testPackage()
{
    // Create a mock SyncTarget.
    const QString source_db = "source";
    const QString target_db = "target";

    LCPTStorage storage( source_db );
    SyncMode mode;
    SyncTarget target(NULL, &storage, mode, "" );

    LocalChanges changes;
    changes.added.append("added");
    changes.modified.append("modified");
    changes.removed.append("removed");

    LocalChangesPackage pkg(target, changes, 25, ROLE_CLIENT, 22);
    QCOMPARE(pkg.iNumberOfChanges, 3);

    SyncMLMessage msg(HeaderParams(), DS_1_2);
    const int SIZE_TRESHOLD = 1000;
    int remaining = SIZE_TRESHOLD;

    QCOMPARE(pkg.write(msg, remaining), true);
    QVERIFY(remaining < SIZE_TRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_ADD) != -1);
    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_REPLACE) != -1);
    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_DELETE) != -1);

    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_SIZE) == -1);

}

void LocalChangesPackageTest::testItemsProcessed()
{
    // Create a mock SyncTarget.
    const QString source_db = "source";
    const QString target_db = "target";

    LCPTStorage storage( source_db );
    SyncMode mode;
    SyncTarget target(NULL, &storage, mode, "" );

	LocalChanges moreChanges;
	for(int i = 1;i <= 25; i++) {
		moreChanges.added.append("added" + QString::number(i));
		moreChanges.modified.append("modified" + QString::number(i));
		moreChanges.removed.append("removed" + QString::number(i));
	}

	LocalChangesPackage pkg(target, moreChanges, 25, ROLE_CLIENT,22);
	QCOMPARE(pkg.iNumberOfChanges, 75);


	SyncMLMessage msg1(HeaderParams(), DS_1_2);
	const int SIZE_TRESHOLD = 65536;
	int remaining = SIZE_TRESHOLD;

	QCOMPARE(pkg.write(msg1, remaining), false);
	QVERIFY(remaining < SIZE_TRESHOLD);

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg1, result_xml, true ) );
//    LOG_DEBUG(result_xml);

    SyncMLMessage msg2(HeaderParams(), DS_1_2);

	QCOMPARE(pkg.write(msg2, remaining), false);
	QVERIFY(remaining < SIZE_TRESHOLD);

	result_xml.clear();
    QVERIFY( encoder.encodeToXML( msg2, result_xml, true ) );
//    LOG_DEBUG(result_xml);

    SyncMLMessage msg3(HeaderParams(), DS_1_2);

	QCOMPARE(pkg.write(msg3, remaining), false);
	QVERIFY(remaining < SIZE_TRESHOLD);

	result_xml.clear();
    QVERIFY( encoder.encodeToXML( msg3, result_xml, true ) );
//    LOG_DEBUG(result_xml);

    SyncMLMessage msg4(HeaderParams(), DS_1_2);

	QCOMPARE(pkg.write(msg4, remaining), true);
	QVERIFY(remaining < SIZE_TRESHOLD);


	result_xml.clear();
    QVERIFY( encoder.encodeToXML( msg4, result_xml, true ) );
//    LOG_DEBUG(result_xml);
}

TESTLOADER_ADD_TEST(LocalChangesPackageTest);
