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

#include <QtTest>
#include <QObject>
#include "SyncMLHdrTest.h"
#include "SyncMLHdr.h"
#include "internals.h"
#include "TestLoader.h"
#include "SyncAgentConfig.h"
#include "QtEncoder.h"

#include <QDebug>
using namespace DataSync;

//initialise the header params
void SyncMLHdrTest::testStart() {
	iHdrParams = new DataSync::HeaderParams;
	QVERIFY(iHdrParams);
	iHdrParams->msgID = 100;
	iHdrParams->sessionID = "12345";
	iHdrParams->sourceDevice = "addressbook";
	iHdrParams->targetDevice = "card";
	iHdrParams->maxMsgSize = 20;
	iHdrParams->maxObjSize = 40;
}

//test the header params passed
void SyncMLHdrTest::testSyncMLHdr() {
	if(iHdrParams) {
		SyncMLHdr hdr(*iHdrParams, DS_1_2);
		QFile hdrFile("testfiles/SyncMLHdrTest.txt");
		if(!hdrFile.open(QIODevice::ReadOnly)) {
			QFAIL("failed to open the file testfiles/SyncMLHdrTest");
		} else {
		    QtEncoder encoder;
		    QByteArray output;
		    QVERIFY( encoder.encodeToXML( hdr, output, true ) );
			QCOMPARE( output,hdrFile.readAll());
			hdrFile.close();
		}
	} else {
		QFAIL("iHdrParams not initialised..Invalid Sequence of Testing.. ");
	}
}

//delete the memory held
void SyncMLHdrTest::testEnd() {
	if (iHdrParams) {
		delete iHdrParams;
		iHdrParams = NULL;
	}
}


void SyncMLHdrTest::testSyncMLHdr_11() {
    if(iHdrParams) {
            SyncMLHdr hdr(*iHdrParams, DS_1_1);
            QFile hdrFile("testfiles/SyncMLHdrTest_11.txt");
            if(!hdrFile.open(QIODevice::ReadOnly)) {
                QFAIL("failed to open the file testfiles/SyncMLHdrTest_11");
            } else {
                QtEncoder encoder;
                QByteArray output;
                QVERIFY( encoder.encodeToXML( hdr, output, true ) );
                QCOMPARE( output,hdrFile.readAll());
                hdrFile.close();
            }
        } else {
            QFAIL("iHdrParams not initialised..Invalid Sequence of Testing.. ");
        }
}

TESTLOADER_ADD_TEST(SyncMLHdrTest);
