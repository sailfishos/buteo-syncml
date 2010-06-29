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

#include "SyncMLCmdObjectTest.h"

#include <QtTest>

#include "TestLoader.h"
#include "SyncMLCmdObject.h"

using namespace DataSync;

void SyncMLCmdObjectTest::testSetGetNameValue()
{
    QString name1("objname");
    QString value1("objvalue");

    // Test constructor
    SyncMLCmdObject obj( name1, value1 );

    QVERIFY( obj.getName() == name1 );
    QVERIFY( obj.getValue() == value1 );

    QString name2("anotherobjname");
    QString value2("anotherobjvalue");

    obj.setName( name2 );
    QVERIFY( obj.getName() == name2 );

    obj.setValue( value2 );
    QVERIFY( obj.getValue() == value2 );

}

void SyncMLCmdObjectTest::testSetGetCData()
{
    SyncMLCmdObject obj;

    obj.setCDATA( true );
    QVERIFY( obj.getCDATA() == true );

    obj.setCDATA( false );
    QVERIFY( obj.getCDATA() == false );
}

void SyncMLCmdObjectTest::testAddGetAttribute()
{

    SyncMLCmdObject obj;
    QString attrName( "attrName" );
    QString attrValue( "attrValue" );

    QVERIFY( obj.getAttributes().count() == 0 );

    obj.addAttribute( attrName, attrValue );

    QVERIFY( obj.getAttributes().count() == 1 );
    QVERIFY( obj.getAttributes().value( attrName) == attrValue );

}

void SyncMLCmdObjectTest::testAddGetChildren()
{

    SyncMLCmdObject obj;

    QVERIFY( obj.getChildren().count() == 0 );
    SyncMLCmdObject* child = new SyncMLCmdObject; //CID 23347 

    obj.addChild( child ); // Ownership is transferred.

    QVERIFY( obj.getChildren().count() == 1 );
    QVERIFY( obj.getChildren()[0] == child );

    child = 0;

}

TESTLOADER_ADD_TEST(SyncMLCmdObjectTest);
