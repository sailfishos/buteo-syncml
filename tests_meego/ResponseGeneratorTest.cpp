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

#include "ResponseGeneratorTest.h"
#include "ResponseGenerator.h"
#include "internals.h"
#include "LocalMappingsPackage.h"
#include "QtEncoder.h"
#include "SyncMLMessage.h"
#include "TestLoader.h"

#include <QDebug>

using namespace DataSync;

void ResponseGeneratorTest::testAddStatus()
{
    ResponseGenerator respGen;
    ResponseStatusCode responseCode = SUCCESS;
    MapParams mapParams;
    respGen.addStatus(mapParams, responseCode);
}


void ResponseGeneratorTest::testAddStatus2()
{
    ResponseGenerator respGen;
    ResponseStatusCode responseCode = SUCCESS;
    SyncActionData data;
    ItemParams item1;
    ItemParams item2;
    QList<ItemParams> itemList;
    itemList.append(item1);
    itemList.append(item2);
    data.items = itemList;
    
    data.action = SYNCML_ADD;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_DELETE;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_REPLACE;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_PUT;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_GET;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_ATOMIC;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_COPY;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_MOVE;
    respGen.addStatus(data, responseCode);
    
    data.action = SYNCML_SEQUENCE;
    respGen.addStatus(data, responseCode);
    
}

void ResponseGeneratorTest::testNB182304()
{

    // Regression test for NB#182304: make sure message size threshold is obeyed
    // when composing a message to device with very low max message size
    const int maxMsgSize = 3584;
    ResponseGenerator respGen;

    HeaderParams hdr;
    hdr.sessionID = 1;
    hdr.msgID = 8;
    hdr.targetDevice = "IMEI:356407011863641";
    hdr.sourceDevice = "IMEI:004402130345691";
    hdr.maxMsgSize = 65535;
    respGen.setHeaderParams( hdr );
    respGen.setRemoteMsgId(8);

    StatusParams* status1 = new StatusParams;
    status1->msgRef = 8;
    status1->cmdRef = 0;
    status1->cmd = "SyncHdr";
    status1->targetRef = "IMEI:004402130345691";
    status1->sourceRef = "IMEI:356407011863641";
    status1->data = SUCCESS;
    respGen.addStatus( status1 );

    StatusParams* status2 = new StatusParams;
    status2->msgRef = 8;
    status2->cmdRef = 7;
    status2->cmd = "Sync";
    status2->targetRef = "./contacts";
    status2->sourceRef = "/telecom/pb.vcf";
    status2->data = SUCCESS;
    respGen.addStatus( status2 );

    StatusParams* status3 = new StatusParams;
    status3->msgRef = 8;
    status3->cmdRef = 8;
    status3->cmd = "Sync";
    status3->targetRef = "./calendar";
    status3->sourceRef = "/telecom/cal.vcs";
    status3->data = SUCCESS;
    respGen.addStatus( status3 );

    StatusParams* status4 = new StatusParams;
    status4->msgRef = 8;
    status4->cmdRef = 9;
    status4->cmd = "Replace";
    status4->targetRef = "13";
    status4->sourceRef = "1317";
    status4->data = SUCCESS;
    respGen.addStatus( status4 );

    StatusParams* status5 = new StatusParams;
    status5->msgRef = 8;
    status5->cmdRef = 10;
    status5->cmd = "Replace";
    status5->targetRef = "12";
    status5->sourceRef = "1316";
    status5->data = SUCCESS;
    respGen.addStatus( status5 );

    StatusParams* status6 = new StatusParams;
    status6->msgRef = 8;
    status6->cmdRef = 11;
    status6->cmd = "Replace";
    status6->targetRef = "11";
    status6->sourceRef = "1315";
    status6->data = SUCCESS;
    respGen.addStatus( status6 );

    StatusParams* status7 = new StatusParams;
    status7->msgRef = 8;
    status7->cmdRef = 12;
    status7->cmd = "Replace";
    status7->targetRef = "10";
    status7->sourceRef = "1314";
    status7->data = SUCCESS;
    respGen.addStatus( status7 );

    StatusParams* status8 = new StatusParams;
    status8->msgRef = 8;
    status8->cmdRef = 13;
    status8->cmd = "Replace";
    status8->targetRef = "2";
    status8->sourceRef = "1306";
    status8->data = SUCCESS;
    respGen.addStatus( status8 );

    StatusParams* status9 = new StatusParams;
    status9->msgRef = 8;
    status9->cmdRef = 14;
    status9->cmd = "Add";
    status9->sourceRef = "1310";
    status9->data = ITEM_ADDED;
    respGen.addStatus( status9 );

    StatusParams* status10 = new StatusParams;
    status10->msgRef = 8;
    status10->cmdRef = 15;
    status10->cmd = "Add";
    status10->sourceRef = "1312";
    status10->data = ITEM_ADDED;
    respGen.addStatus( status10 );

    StatusParams* status11 = new StatusParams;
    status11->msgRef = 8;
    status11->cmdRef = 16;
    status11->cmd = "Add";
    status11->sourceRef = "1304";
    status11->data = ITEM_ADDED;
    respGen.addStatus( status11 );

    StatusParams* status12 = new StatusParams;
    status12->msgRef = 8;
    status12->cmdRef = 17;
    status12->cmd = "Add";
    status12->sourceRef = "1322";
    status12->data = ITEM_ADDED;
    respGen.addStatus( status12 );

    StatusParams* status13 = new StatusParams;
    status13->msgRef = 8;
    status13->cmdRef = 18;
    status13->cmd = "Add";
    status13->sourceRef = "1313";
    status13->data = ITEM_ADDED;
    respGen.addStatus( status13 );

    StatusParams* status14 = new StatusParams;
    status14->msgRef = 8;
    status14->cmdRef = 19;
    status14->cmd = "Add";
    status14->sourceRef = "1302";
    status14->data = ITEM_ADDED;
    respGen.addStatus( status14 );

    StatusParams* status15 = new StatusParams;
    status15->msgRef = 8;
    status15->cmdRef = 20;
    status15->cmd = "Add";
    status15->sourceRef = "1321";
    status15->data = ITEM_ADDED;
    respGen.addStatus( status15 );

    StatusParams* status16 = new StatusParams;
    status16->msgRef = 8;
    status16->cmdRef = 21;
    status16->cmd = "Sync";
    status16->targetRef = "./Notepad";
    status16->sourceRef = "/telecom/note.txt";
    status16->data = SUCCESS;
    respGen.addStatus( status16 );

    StatusParams* status17 = new StatusParams;
    status17->msgRef = 8;
    status17->cmdRef = 22;
    status17->cmd = "Add";
    status17->sourceRef = "1323";
    status17->data = ITEM_ADDED;
    respGen.addStatus( status17 );

    QList<UIDMapping> mappings;
    UIDMapping m1;
    m1.iLocalUID = "16";
    m1.iRemoteUID = "1310";
    mappings.append( m1 );

    UIDMapping m2;
    m2.iLocalUID = "17";
    m2.iRemoteUID = "1312";
    mappings.append( m2 );

    UIDMapping m3;
    m3.iLocalUID = "18";
    m3.iRemoteUID = "1304";
    mappings.append( m3 );

    LocalMappingsPackage* map = new LocalMappingsPackage( "/calendar",
                                                          "/telecom/cal.vcs",
                                                          mappings );

    respGen.addPackage( map );

    SyncMLMessage* msg = respGen.generateNextMessage( maxMsgSize, DS_1_1 );

    QVERIFY( msg );

    QtEncoder encoder;
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( *msg, result_xml, false ) );
    delete msg;
    msg = 0;

    // Add 157 bytes to account for XML header and DOCTYPE
    QVERIFY( result_xml.size() + 157 < maxMsgSize );

}
        
TESTLOADER_ADD_TEST(ResponseGeneratorTest);

