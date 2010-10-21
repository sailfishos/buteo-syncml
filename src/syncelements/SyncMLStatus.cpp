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

#include "SyncMLStatus.h"

#include "SyncMLMeta.h"
#include "SyncMLItem.h"
#include "Fragments.h"
#include "datatypes.h"

using namespace DataSync;

SyncMLStatus::SyncMLStatus(const StatusParams& aParams)

    : SyncMLCmdObject(SYNCML_ELEMENT_STATUS)
{

    SyncMLCmdObject* cmdIdObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMDID, QString::number(aParams.cmdId) );
    addChild(cmdIdObject);

    SyncMLCmdObject* msgRefObject = new SyncMLCmdObject( SYNCML_ELEMENT_MSGREF, QString::number(aParams.msgRef) );
    addChild(msgRefObject);

    SyncMLCmdObject* cmdRefObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMDREF, QString::number(aParams.cmdRef) );
    addChild(cmdRefObject);

    SyncMLCmdObject* cmdObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMD, aParams.cmd );
    addChild(cmdObject);

    if( !aParams.targetRef.isEmpty() )
    {
        SyncMLCmdObject* targetRefObject = new SyncMLCmdObject( SYNCML_ELEMENT_TARGETREF, aParams.targetRef );
        addChild(targetRefObject);
    }

    if( !aParams.sourceRef.isEmpty() )
    {
        SyncMLCmdObject* sourceRefObject = new SyncMLCmdObject( SYNCML_ELEMENT_SOURCEREF, aParams.sourceRef );
        addChild(sourceRefObject);
    }

    SyncMLCmdObject* dataObject = new SyncMLCmdObject( SYNCML_ELEMENT_DATA, QString::number(aParams.data) );
    dataObject->setCDATA( true );
    addChild(dataObject);

    if( !aParams.nextAnchor.isEmpty() )
    {
        SyncMLCmdObject* item = new SyncMLCmdObject(SYNCML_ELEMENT_ITEM);
        SyncMLCmdObject* data = new SyncMLCmdObject(SYNCML_ELEMENT_DATA);

        SyncMLCmdObject* anc = new SyncMLCmdObject(SYNCML_ELEMENT_ANCHOR);
        anc->addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_METINF );

        SyncMLCmdObject* next = new SyncMLCmdObject( SYNCML_ELEMENT_NEXT, aParams.nextAnchor );
        next->addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_METINF );
        anc->addChild(next);
        data->addChild(anc);
        item->addChild(data);
        addChild(item);
    }

    for( int i = 0; i < aParams.items.count(); ++i )
    {

        SyncMLItem* itemObject = new SyncMLItem();

        if( !aParams.items[i].source.isEmpty() )
        {
            itemObject->insertSource( aParams.items[i].source );
        }

        if( !aParams.items[i].target.isEmpty() )
        {
            itemObject->insertTarget( aParams.items[i].target );
        }

        if( !aParams.items[i].data.isEmpty() )
        {
            itemObject->insertData( aParams.items[i].data.toUtf8() );
        }

        addChild( itemObject );
    }

    if( !aParams.chal.meta.type.isEmpty() ) {
        SyncMLCmdObject* chalObject = new SyncMLCmdObject( SYNCML_ELEMENT_CHAL );
        SyncMLMeta* metaObject = new SyncMLMeta();
        metaObject->addType( aParams.chal.meta.type );
        metaObject->addFormat( aParams.chal.meta.format );

        if( !aParams.chal.meta.nextNonce.isEmpty() ) {
            metaObject->addNextNonce( aParams.chal.meta.nextNonce );
        }

        chalObject->addChild( metaObject );
        addChild( chalObject );

    }

}


SyncMLStatus::~SyncMLStatus()
{
}


