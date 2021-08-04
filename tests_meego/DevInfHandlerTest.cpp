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

#include "DevInfHandlerTest.h"

#include "DevInfHandler.h"
#include "ResponseGenerator.h"
#include "DevInfPackage.h"

#include "SyncMLLogging.h"

using namespace DataSync;

void DevInfHandlerTest::testComposeLocalInitiatedDevInfExchange1()
{
    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    QList<StoragePlugin*>storages;
    ProtocolVersion version = SYNCML_1_2;
    Role role = ROLE_CLIENT;
    ResponseGenerator generator;

    handler.iLocalDevInfSent = false;
    handler.composeLocalInitiatedDevInfExchange( storages, version, role,
                                                 generator );
    QCOMPARE( handler.iLocalDevInfSent, true );

    const QList<Package*>& packages = generator.getPackages();

    QCOMPARE( packages.count(), 1 );
    QCOMPARE( packages[0]->metaObject()->className(), "DataSync::DevInfPackage" );
}

void DevInfHandlerTest::testComposeLocalInitiatedDevInfExchange2()
{
    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    QList<StoragePlugin*>storages;
    ProtocolVersion version = SYNCML_1_2;
    Role role = ROLE_CLIENT;
    ResponseGenerator generator;

    handler.iLocalDevInfSent = true;
    handler.composeLocalInitiatedDevInfExchange( storages, version, role,
                                                 generator );
    QCOMPARE( handler.iLocalDevInfSent, true );

    const QList<Package*>& packages = generator.getPackages();

    QCOMPARE( packages.count(), 0 );

}

void DevInfHandlerTest::testHandleGet1()
{
    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    QList<StoragePlugin*>storages;
    ProtocolVersion version = SYNCML_1_2;
    Role role = ROLE_CLIENT;
    ResponseGenerator generator;
    CommandParams get;

    handler.iLocalDevInfSent = false;
    handler.iRemoteDevInfReceived = false;
    ResponseStatusCode response = handler.handleGet( get, version, storages,
                                                     role, generator );

    QCOMPARE( response, COMMAND_FAILED );
    QCOMPARE( handler.iLocalDevInfSent, false );
    QCOMPARE( handler.iRemoteDevInfReceived, false );

}
void DevInfHandlerTest::testHandleGet2()
{

    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    QList<StoragePlugin*>storages;
    ProtocolVersion version = SYNCML_1_2;
    Role role = ROLE_CLIENT;
    ResponseGenerator generator;
    CommandParams get;

    ItemParams item;
    item.target = SYNCML_DEVINF_PATH_12;
    get.items.append(item);

    handler.iLocalDevInfSent = false;
    handler.iRemoteDevInfReceived = false;
    ResponseStatusCode response = handler.handleGet( get, version, storages,
                                                     role, generator );

    QCOMPARE( response, SUCCESS );
    QCOMPARE( handler.iLocalDevInfSent, true );
    QCOMPARE( handler.iRemoteDevInfReceived, false );

    const QList<Package*>& packages = generator.getPackages();

    QCOMPARE( packages.count(), 1 );
    QCOMPARE( packages[0]->metaObject()->className(), "DataSync::DevInfPackage" );

}

void DevInfHandlerTest::testHandleGet3()
{

    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    QList<StoragePlugin*>storages;
    ProtocolVersion version = SYNCML_1_2;
    Role role = ROLE_CLIENT;
    ResponseGenerator generator;
    CommandParams get;

    ItemParams item;
    item.target = SYNCML_DEVINF_PATH_12;
    get.items.append(item);

    handler.iLocalDevInfSent = false;
    handler.iRemoteDevInfReceived = true;
    ResponseStatusCode response = handler.handleGet( get, version, storages,
                                                     role, generator );

    QCOMPARE( response, SUCCESS );
    QCOMPARE( handler.iLocalDevInfSent, true );
    QCOMPARE( handler.iRemoteDevInfReceived, true );

    const QList<Package*>& packages = generator.getPackages();

    QCOMPARE( packages.count(), 1 );
    QCOMPARE( packages[0]->metaObject()->className(), "DataSync::DevInfPackage" );

}

void DevInfHandlerTest::testHandlePut()
{
    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    ProtocolVersion version = SYNCML_1_2;
    PutParams put;

    ResponseStatusCode response = handler.handlePut( put, version );
    QCOMPARE( response, COMMAND_FAILED );

    put.devInf.source = SYNCML_DEVINF_PATH_12;
    response = handler.handlePut( put, version );
    QCOMPARE( response, SUCCESS );

}

void DevInfHandlerTest::testHandleResults()
{
    DeviceInfo deviceInfo;
    DevInfHandler handler( deviceInfo );

    ProtocolVersion version = SYNCML_1_2;
    ResultsParams results;

    ResponseStatusCode response = handler.handleResults( results, version );
    QCOMPARE( response, COMMAND_FAILED );

    results.devInf.source = SYNCML_DEVINF_PATH_12;
    response = handler.handleResults( results, version );
    QCOMPARE( response, SUCCESS );
}

QTEST_MAIN(DevInfHandlerTest)

