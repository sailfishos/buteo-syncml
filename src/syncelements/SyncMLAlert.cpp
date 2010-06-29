/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "SyncMLAlert.h"

#include "SyncMLMeta.h"
#include "internals.h"

#include "LogMacros.h"

using namespace DataSync;

SyncMLAlert::SyncMLAlert(AlertParams& aAlertParams)
    : SyncMLCmdObject( SYNCML_ELEMENT_ALERT )
{

    SyncMLCmdObject* cmdIdObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMDID, QString::number(aAlertParams.cmdID) );
    addChild(cmdIdObject);

    SyncMLCmdObject* dataObject = new SyncMLCmdObject( SYNCML_ELEMENT_DATA, QString::number(aAlertParams.data) );
    dataObject->setCDATA( true );
    addChild(dataObject);

    SyncMLCmdObject* itemObject = new SyncMLCmdObject(SYNCML_ELEMENT_ITEM);

    if( !aAlertParams.targetDatabase.isEmpty() ) {
        SyncMLCmdObject* targetURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI, aAlertParams.targetDatabase );
        SyncMLCmdObject* targetObject = new SyncMLCmdObject(SYNCML_ELEMENT_TARGET);
        targetObject->addChild(targetURIObject);
        itemObject->addChild(targetObject);
    }

    if( !aAlertParams.sourceDatabase.isEmpty() ) {
        SyncMLCmdObject* sourceURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI, aAlertParams.sourceDatabase );
        SyncMLCmdObject* sourceObject = new SyncMLCmdObject(SYNCML_ELEMENT_SOURCE);
        sourceObject->addChild(sourceURIObject);
        itemObject->addChild(sourceObject);
    }

    if( !aAlertParams.lastAnchor.isEmpty() || !aAlertParams.nextAnchor.isEmpty() ||
        !aAlertParams.type.isEmpty() ) {

        SyncMLMeta* metaObject = new SyncMLMeta;

        if( !aAlertParams.lastAnchor.isEmpty() || !aAlertParams.nextAnchor.isEmpty() ) {
            metaObject->addAnchors( aAlertParams.lastAnchor, aAlertParams.nextAnchor );
        }

        if( !aAlertParams.type.isEmpty() ) {
            metaObject->addType( aAlertParams.type );
        }

        itemObject->addChild(metaObject);

    }

    addChild(itemObject);

}


SyncMLAlert::~SyncMLAlert()
{
}

