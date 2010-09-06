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
#ifndef BASETRANSPORTTEST_H
#define BASETRANSPORTTEST_H

#include <QTest>
#include <QObject>

#include "BaseTransport.h"

using namespace DataSync;

class BaseTransportTest : public QObject {
    Q_OBJECT;
public:

private slots:

    void initTestCase();
    void cleanupTestCase();

    void testBasicXMLSend();
    void testBasicXMLReceive();

    void testBasicWbXMLSend();
    void testBasicWbXMLReceive();

    void testSANReceive01();
    void testSANReceive02();

};

class TestTransport : public BaseTransport
{
    Q_OBJECT;

public:

    TestTransport( QObject* aParent = NULL );

    virtual ~TestTransport();

    virtual void setProperty( const QString& aProperty, const QString& aValue );

    virtual bool init();

    virtual void close();

protected:

    virtual bool prepareSend();

    virtual bool doSend( const QByteArray& aData, const QString& aContentType );

    virtual bool doReceive( const QString& aContentType );

private:

    QByteArray  iData;
    QString     iContentType;

    friend class BaseTransportTest;

};

#endif  //  BASETRANSPORTTEST_H
