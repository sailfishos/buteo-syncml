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

#include "SyncMLMapTest.h"
#include "datatypes.h"
#include "SyncMLMap.h"
#include "SyncMLMapItem.h"
#include "QtEncoder.h"
#include <QDebug>

void SyncMLMapTest::testBasic()
{
    DataSync::SyncMLMapItem* mapItem = new DataSync::SyncMLMapItem("guid", "luid");
    DataSync::SyncMLMap* map = new DataSync::SyncMLMap( 1, "target", "source" );
    QVERIFY(mapItem != 0);
    QVERIFY(map != 0);
    map->addChild(mapItem);

    QByteArray expected = "<?xmlversion=\"1.0\"encoding=\"UTF-8\"?><Map><CmdID>1</CmdID><Target><LocURI>target</LocURI></Target><Source><LocURI>source</LocURI></Source><MapItem><Target><LocURI>guid</LocURI></Target><Source><LocURI>luid</LocURI></Source></MapItem></Map>";

    DataSync::QtEncoder encoder;
    QByteArray output;
    QVERIFY( encoder.encodeToXML( *map, output, true ) );

    output = output.replace(QByteArray("\r"), QByteArray(""));
    output = output.replace(QByteArray("\n"), QByteArray(""));
    output = output.replace(QByteArray("\t"), QByteArray(""));
    output = output.replace(QByteArray(" "), QByteArray(""));
    expected = expected.replace(QByteArray(" "), QByteArray(""));

    delete map;
    map = NULL;

    QCOMPARE(output, expected);
}
QTEST_MAIN(SyncMLMapTest)
