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

#include "OBEXTransportTest.h"

#include "SyncMLMessage.h"
#include "OBEXTransport.h"
#include "SyncAgentConfigProperties.h"

#include "TestLoader.h"
#include "TestUtils.h"
#include "Fragments.h"
#include "Mock.h"

#include "LogMacros.h"
#include <QtTest>

Q_DECLARE_METATYPE(QIODevice*);

using namespace DataSync;

void OBEXTransportTest::initTestCase()
{
}

void OBEXTransportTest::cleanupTestCase()
{

}

void OBEXTransportTest::testServerSend()
{
    OBEXConnectionTest conn(true);
    OBEXTransport transport(conn, OBEXTransport::MODE_OBEX_SERVER);
    transport.init();

    HeaderParams params;

    params.verDTD = SYNCML_DTD_VERSION_1_2;
    params.verProto = DS_VERPROTO_1_2;
    params.msgID = 1;
    params.targetDevice = "targetDevice";
    params.sourceDevice = "sourceDevice";

    SyncMLMessage* message = new SyncMLMessage(params, SYNCML_1_2);

    QByteArray data;
    QCOMPARE(transport.getData(SYNCML_CONTTYPE_DS_XML, data), false);

    QCOMPARE(transport.sendSyncML(message), true);
    QCOMPARE(transport.getData(SYNCML_CONTTYPE_DS_XML, data), true);
    QCOMPARE(transport.getData("InexistentDataType", data), false);

    transport.close();
}

void OBEXTransportTest::testClientSend()
{
    OBEXConnectionTest conn(true);
    OBEXTransport transport(conn, OBEXTransport::MODE_OBEX_CLIENT);
    transport.setProperty(OBEXTIMEOUTPROP, "5");
    transport.init();

    HeaderParams params;

    params.verDTD = SYNCML_DTD_VERSION_1_2;
    params.verProto = DS_VERPROTO_1_2;
    params.msgID = 1;
    params.targetDevice = "targetDevice";
    params.sourceDevice = "sourceDevice";

    SyncMLMessage* message = new SyncMLMessage(params, SYNCML_1_2);
    QCOMPARE(transport.sendSyncML(NULL), false);
    transport.sendSyncML(message);

    transport.close();
}


TESTLOADER_ADD_TEST(OBEXTransportTest);
