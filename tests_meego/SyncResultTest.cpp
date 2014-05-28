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

#include "SyncResultTest.h"
#include "SyncResults.h"

using namespace DataSync;

void SyncResultsTest::initTestCase()
{
    iSyncResults = new SyncResults();
}

void SyncResultsTest::cleanupTestCase()
{
    delete iSyncResults;
    iSyncResults = NULL;
}


void SyncResultsTest::testGetLastState()
{
    iSyncResults->setState( SYNC_FINISHED );
    SyncState state = iSyncResults->getState();
    QVERIFY( state == SYNC_FINISHED );
}


void SyncResultsTest::testGetLastErrorString()
{
    QString errorString( "Guru meditation" );

    iSyncResults->setErrorString( errorString );
    QString error = iSyncResults->getErrorString();
    QVERIFY( error == errorString );
}


void SyncResultsTest::testAddProcessedItem()
{
    DataSync::ModifiedDatabase modBase = MOD_LOCAL_DATABASE;
    DataSync::ModificationType modType = MOD_ITEM_ADDED;
    QString database = "foo";
    
    iSyncResults->addProcessedItem(modType, modBase, database);
    
    modType = MOD_ITEM_MODIFIED;
    iSyncResults->addProcessedItem(modType, modBase, database);
    
    modType = MOD_ITEM_DELETED;
    iSyncResults->addProcessedItem(modType, modBase, database);
    
    modBase = MOD_REMOTE_DATABASE;
    modType = MOD_ITEM_ADDED;
    iSyncResults->addProcessedItem(modType, modBase, database);
    
    modType = MOD_ITEM_MODIFIED;
    iSyncResults->addProcessedItem(modType, modBase, database);
    
    modType = MOD_ITEM_DELETED;
    iSyncResults->addProcessedItem(modType, modBase, database);
}



QTEST_MAIN(DataSync::SyncResultsTest)
