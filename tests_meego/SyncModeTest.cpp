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

#include "SyncMode.h"
#include "SyncModeTest.h"
#include "TestLoader.h"

#include "internals.h"


using namespace DataSync;

void SyncModeTest::initTestCase()
{
}


void SyncModeTest::cleanupTestCase()
{
}


void SyncModeTest::testIsValid()
{
    SyncMode validMode1(DIRECTION_TWO_WAY, INIT_CLIENT);
    QVERIFY(validMode1.isValid());
    SyncMode validMode2(SLOW_SYNC);
    QVERIFY(validMode2.isValid());
    SyncMode invalidMode(-1);
    QVERIFY(!invalidMode.isValid());
}

void SyncModeTest::testSyncDirection()
{
    SyncDirection direction = DIRECTION_TWO_WAY;
    SyncMode mode( direction );
    QCOMPARE( mode.syncDirection(), direction );
}

void SyncModeTest::testSyncInitiator()
{
    SyncInitiator initiator = INIT_CLIENT;
    SyncMode mode(DIRECTION_TWO_WAY, initiator );
    QCOMPARE( mode.syncInitiator(), initiator );
}

void SyncModeTest::testSyncType()
{
    SyncType type = TYPE_SLOW;
    SyncMode mode(DIRECTION_TWO_WAY, INIT_CLIENT, type );

    QCOMPARE( mode.syncType(), type );

}

TESTLOADER_ADD_TEST(SyncModeTest);
