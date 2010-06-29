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

#include "SyncAgentConfigTest.h"
#include "TestLoader.h"
#include "SyncAgentConfig.h"

using namespace DataSync;

void SyncAgentConfigTest::initTestCase()
{
    iSyncAgentConfig = new SyncAgentConfig();
}

void SyncAgentConfigTest::cleanupTestCase()
{
    delete iSyncAgentConfig;
    iSyncAgentConfig = NULL;
}

void SyncAgentConfigTest::testSetLocalDevice()
{
    QString localDevice = "foo";
    iSyncAgentConfig->setLocalDevice(localDevice);
    QVERIFY(iSyncAgentConfig->getLocalDevice() == localDevice);
}

void SyncAgentConfigTest::testSetRemoteDevice()
{
    QString remoteDevice = "foo";
    iSyncAgentConfig->setRemoteDevice(remoteDevice);
    QVERIFY(iSyncAgentConfig->getRemoteDevice() == remoteDevice);

}

void SyncAgentConfigTest::testSetAuthenticationType()
{
    AuthenticationType authType = AUTH_NONE;
    iSyncAgentConfig->setAuthenticationType(authType);
    QVERIFY(iSyncAgentConfig->iAuthenticationType == authType);
}


void SyncAgentConfigTest::testSetUsername()
{
    QString username = "foo";
    iSyncAgentConfig->setUsername(username);
    QVERIFY(iSyncAgentConfig->iUsername == username);

}

void SyncAgentConfigTest::testGetUsername()
{
    QString username = iSyncAgentConfig->getUsername();
    QVERIFY(iSyncAgentConfig->iUsername == username);
}

void SyncAgentConfigTest::testSetPassword()
{
    QString pw = "foo";
    iSyncAgentConfig->setPassword(pw);
    QVERIFY(iSyncAgentConfig->iPassword == pw);

}

void SyncAgentConfigTest::testGetPassword()
{
    QString pw = iSyncAgentConfig->getPassword();
    QVERIFY(iSyncAgentConfig->iPassword == pw);
}

void SyncAgentConfigTest::testSetConflictResolutionPolicy()
{
    ConflictResolutionPolicy policy = PREFER_LOCAL_CHANGES;
    iSyncAgentConfig->setConflictResolutionPolicy(policy);
    QVERIFY(iSyncAgentConfig->iConflictResolutionPolicy == policy);

}

void SyncAgentConfigTest::testGetConflictResolutionPolicy()
{
    ConflictResolutionPolicy policy = iSyncAgentConfig->getConflictResolutionPolicy();
    QVERIFY(iSyncAgentConfig->iConflictResolutionPolicy == policy);
}

void SyncAgentConfigTest::testClearProtocolAttribute()
{
    iSyncAgentConfig->clearProtocolAttribute(0);
}

void SyncAgentConfigTest::testGetTargets()
{
    const QMap<QString, QString>* target = iSyncAgentConfig->getTargets();
    Q_UNUSED(target);
}

void SyncAgentConfigTest::testGetMaxChangesToSend()
{
    QVERIFY(iSyncAgentConfig->getMaxChangesToSend() == DEFAULT_MAX_CHANGES_TO_SEND);
}

void SyncAgentConfigTest::testSetMaxChangesToSend()
{

	iSyncAgentConfig->setMaxChangesToSend(-20);

    QVERIFY(iSyncAgentConfig->getMaxChangesToSend() == DEFAULT_MAX_CHANGES_TO_SEND);

    iSyncAgentConfig->setMaxChangesToSend(20);

    QVERIFY(iSyncAgentConfig->getMaxChangesToSend() == 20);

}



TESTLOADER_ADD_TEST(SyncAgentConfigTest);
