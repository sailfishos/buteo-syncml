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

#include "ChangeLogHandlerTest.h"
#include "ChangeLog.h"
#include "ChangeLogHandler.h"
#include "DatabaseHandler.h"
#include "datatypes.h"
using namespace DataSync;

#include <QTest>
#include <QDebug>

void ChangeLogHandlerTest::initTestCase()
{
    iDbHandler = new DatabaseHandler( QProcessEnvironment::systemEnvironment().value("TMPDIR", "/tmp") + "/changeloghandlertest.db" );

}
void ChangeLogHandlerTest::cleanupTestCase()
{
    if( iDbHandler ) {
        delete iDbHandler;
        iDbHandler = NULL;
    }
}

void ChangeLogHandlerTest::testCreateChangeLogTable()
{


    ChangeLogHandler handler( iDbHandler->getDbHandle() );

    QVERIFY(handler.createChangeLogTable());

    ChangeLogInfo info("device1", "targetdb", "sourcedb", TWO_WAY_SYNC );

    ChangeLog* log = handler.create(info);
    uint id = log->getChangeLogId();
    //qDebug() << "log Number of Items " << log->getNrOfItems();

    // Try to get it again, same changelog should be returned.
    ChangeLog* log2 = handler.create(info);
    //qDebug() << id << " and " << log2->getChangeLogId();
    QVERIFY ( id == log2->getChangeLogId());

    handler.remove(info);

    // New remote device, the changelog should be different
    info.iRemoteDevice = "device2";

    ChangeLog* log3 = handler.create(info);

    QVERIFY( id != log3->getChangeLogId());

    delete log;
    delete log2;
    delete log3;

}

QTEST_MAIN(ChangeLogHandlerTest)
