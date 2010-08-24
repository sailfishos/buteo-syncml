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

#include "SyncAgentConfig.h"
#include "SyncAgentConfigProperties.h"

#include "TestLoader.h"

using namespace DataSync;

void SyncAgentConfigTest::testConfParsing()
{

    const QString xsdFile( "testfiles/testconf.xsd" );
    const QString configFile( "testfiles/testconf1.xml" );
    const QString dbFile( "/fooland/syncml.db" );
    const QString localDeviceName( "FoolandDevice" );
    const QString proxyHost( "http://www.google.com" );
    const QString inq( "InqMe" );
    const QString ack( "AckMe" );

    SyncAgentConfig config;

    QVERIFY( config.fromFile( configFile, xsdFile ) );

    QCOMPARE( config.getDatabaseFilePath(), dbFile );

    QCOMPARE( config.getLocalDeviceName(), localDeviceName );

    QCOMPARE( config.getAgentProperty( MAXMESSAGESIZEPROP ).toInt(), 32768 );

    QCOMPARE( config.getAgentProperty( MAXCHANGESPERMESSAGEPROP ).toInt(), 10 );

    QCOMPARE( static_cast<ConflictResolutionPolicy>( config.getAgentProperty( CONFLICTRESOLUTIONPOLICYPROP ).toInt() ), PREFER_REMOTE_CHANGES );

    QCOMPARE( config.getAgentProperty( FASTMAPSSENDPROP ).toInt(), 1 );

    QCOMPARE( config.getTransportProperty( BTOBEXMTUPROP ).toInt(), 1024 );

    QCOMPARE( config.getTransportProperty( USBOBEXMTUPROP ).toInt(), 2048 );

    QCOMPARE( config.getTransportProperty( HTTPNUMBEROFRESENDATTEMPTSPROP ).toInt(), 3 );

    QCOMPARE( config.getTransportProperty( HTTPPROXYHOSTPROP ), proxyHost );

    QCOMPARE( config.getTransportProperty( HTTPPROXYPORTPROP ).toInt(), 666 );

    QVERIFY( config.extensionEnabled( EMITAGSEXTENSION ) );

    QVariant data = config.getExtensionData( EMITAGSEXTENSION );

    QVERIFY( data.isValid() );

    QStringList tags = data.toStringList();

    QCOMPARE( tags.count(), 2 );
    QCOMPARE( tags[0], inq );
    QCOMPARE( tags[1], ack );

    QVERIFY( config.extensionEnabled( SYNCWITHOUTINITPHASEEXTENSION ) );

}

TESTLOADER_ADD_TEST(SyncAgentConfigTest);
