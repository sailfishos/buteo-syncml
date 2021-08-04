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

#include "HTTPTransportTest.h"

#include "SyncMLMessage.h"
#include "HTTPTransport.h"
#include "SyncAgentConfigProperties.h"
#include <QNetworkProxy>

#include "TestUtils.h"
#include "Fragments.h"
#include "Mock.h"

#include "SyncMLLogging.h"

#include <QSignalSpy>
#include <QtTest>

Q_DECLARE_METATYPE(QIODevice*);

void HTTPTransportTest::initTestCase()
{

}

void HTTPTransportTest::cleanupTestCase()
{

}

void HTTPTransportTest::testBasicXMLSend()
{
    HTTPTransport transport;

    QSignalSpy sendEvent(&transport, SIGNAL(sendEvent(DataSync::TransportStatusEvent, const QString&)));
    QSignalSpy readData(&transport, SIGNAL(readXMLData(QIODevice*, bool)));

    transport.setWbXml(false);
    transport.init();

    HeaderParams params;

    params.verDTD = SYNCML_DTD_VERSION_1_2;
    params.verProto = DS_VERPROTO_1_2;
    params.msgID = 1;
    params.targetDevice = "targetDevice";
    params.sourceDevice = "sourceDevice";
    transport.addXheader("foo", "bar");

    SyncMLMessage* message = new SyncMLMessage(params, SYNCML_1_2);

    QVERIFY(transport.sendSyncML(NULL) == false);
    QVERIFY(sendEvent.count() == 0);
    QVERIFY(readData.count() == 0);

    QVERIFY(transport.sendSyncML(message) == true);
    QVERIFY(sendEvent.count() == 0);
    QVERIFY(readData.count() == 0);

    transport.close();
}

void HTTPTransportTest::testSetProperty()
{
    HTTPTransport transport;
    transport.setProperty("foo", "bar");
    QString proxyHost("http://foo.bar.net");
    transport.setProperty(HTTPNUMBEROFRESENDATTEMPTSPROP, "10");
    transport.setProperty(HTTPPROXYHOSTPROP, proxyHost);
    transport.setProperty(HTTPPROXYPORTPROP, "5555");

    QNetworkProxy proxy = transport.getProxyConfig();
    QCOMPARE(proxy.hostName(), proxyHost);
    QCOMPARE(proxy.port(), (quint16)5555);
}

void HTTPTransportTest::testSetProxy()
{
    HTTPTransport transport;
    QNetworkProxy proxy;
    QString proxyHost("http://foo.bar.net");
    quint16 port = 5555;
    proxy.setHostName(proxyHost);
    proxy.setPort(port);
    transport.setProxyConfig(proxy);

    proxy = transport.getProxyConfig();
    QCOMPARE(proxy.hostName(), proxyHost);
    QCOMPARE(proxy.port(), port);
}

QTEST_MAIN(HTTPTransportTest)
