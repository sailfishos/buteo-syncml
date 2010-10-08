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

#include "AuthenticationPackageTest.h"

#include "AuthenticationPackage.h"
#include "SyncMLMessage.h"
#include "QtEncoder.h"
#include "TestLoader.h"
#include "Fragments.h"
#include "datatypes.h"

// @todo: need better tests here

using namespace DataSync;

void AuthenticationPackageTest::testWrite()
{
    QString user_name("user");
    QString password("pw");

    AuthenticationPackage pkg(AUTH_BASIC, user_name, password);
    QCOMPARE(pkg.iUsername, user_name);
    QCOMPARE(pkg.iPassword, password);

    SyncMLMessage msg(HeaderParams(), SYNCML_1_2);

    QtEncoder encoder;
    QByteArray initial_xml;
    QVERIFY( encoder.encodeToXML( msg, initial_xml, true ) );

    const int SIZE_TRESHOLD = 10000;
    int remaining = SIZE_TRESHOLD;

    QCOMPARE(pkg.write(msg, remaining), true);
    QVERIFY(remaining < SIZE_TRESHOLD);
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );
    QVERIFY(result_xml.size() > initial_xml.size());
    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_CRED) != -1);


}

void AuthenticationPackageTest::testWrite2()
{
    QString user_name("user");
    QString password("pw");

    QtEncoder encoder;
    AuthenticationPackage pkg(AUTH_BASIC, user_name, password);
    QCOMPARE(pkg.iUsername, user_name);
    QCOMPARE(pkg.iPassword, password);

    pkg.iNonce = QString::number( QDateTime::currentDateTime().toTime_t() ).toAscii();

    SyncMLMessage msg(HeaderParams(), SYNCML_1_2);
    QByteArray initial_xml;

    QVERIFY( encoder.encodeToXML( msg, initial_xml, true ) );

    const int SIZE_TRESHOLD = 10000;
    int remaining = SIZE_TRESHOLD;

    QCOMPARE(pkg.write(msg, remaining), true);
    QVERIFY(remaining < SIZE_TRESHOLD);
    QByteArray result_xml;
    QVERIFY( encoder.encodeToXML( msg, result_xml, true ) );

    QVERIFY(result_xml.size() > initial_xml.size());
    QVERIFY(result_xml.indexOf(SYNCML_ELEMENT_CRED) != -1);


}

TESTLOADER_ADD_TEST(AuthenticationPackageTest);
