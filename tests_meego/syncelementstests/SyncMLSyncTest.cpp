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

#include "SyncMLSyncTest.h"

#include "SyncMLSync.h"
#include "SyncMLAdd.h"
#include "SyncMLDelete.h"
#include "SyncMLReplace.h"
#include "QtEncoder.h"

#include <QDebug>

void SyncMLSyncTest::testAddNumberOfChanges()
{
    int cmdID = 1;
    QString source("foo");
    QString target("bar");
    DataSync::SyncMLSync sync(cmdID, source, target);
    int numOfChanges = 5;

    sync.addNumberOfChanges(numOfChanges);

    DataSync::QtEncoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToXML( sync, output, true ) );

    // remove any whitespace that might disturb the comparison
    output = output.replace(QByteArray("\r"), QByteArray(""));
    output = output.replace(QByteArray("\n"), QByteArray(""));
    output = output.replace(QByteArray("\t"), QByteArray(""));
    output = output.replace(QByteArray(" "), QByteArray(""));
    QByteArray expected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Sync><CmdID>1</CmdID><Target><LocURI>foo</LocURI></Target><Source><LocURI>bar</LocURI></Source><NumberOfChanges>5</NumberOfChanges></Sync>";
    expected = expected.replace(QByteArray(" "), QByteArray(""));

    QCOMPARE(output, expected);
}
QTEST_MAIN(SyncMLSyncTest)
