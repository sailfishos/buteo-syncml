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

#include "SyncMLHdr.h"

#include "SyncMLMeta.h"
#include "datatypes.h"
#include "Fragments.h"

using namespace DataSync;

SyncMLHdr::SyncMLHdr( const HeaderParams& aHeaderParams )
 : SyncMLCmdObject(SYNCML_ELEMENT_SYNCHDR)
{
    SyncMLCmdObject* verDTDObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERDTD,
                                                         aHeaderParams.verDTD );
    addChild(verDTDObject);

    SyncMLCmdObject* verProtoObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERPROTO,
                                                           aHeaderParams.verProto );
    addChild(verProtoObject);

    SyncMLCmdObject* sessionIDObject = new SyncMLCmdObject( SYNCML_ELEMENT_SESSIONID,
                                                            aHeaderParams.sessionID );
    addChild(sessionIDObject);

    SyncMLCmdObject* msgIDObject = new SyncMLCmdObject( SYNCML_ELEMENT_MSGID,
                                                        QString::number(aHeaderParams.msgID) );
    addChild(msgIDObject);

    SyncMLCmdObject* targetObject = new SyncMLCmdObject(SYNCML_ELEMENT_TARGET);
    SyncMLCmdObject* targetURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI,
                                                            aHeaderParams.targetDevice );
    targetObject->addChild(targetURIObject);
    addChild(targetObject);

    SyncMLCmdObject* sourceObject = new SyncMLCmdObject(SYNCML_ELEMENT_SOURCE);

    SyncMLCmdObject* sourceURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI,
                                                            aHeaderParams.sourceDevice);
    sourceObject->addChild(sourceURIObject);

    addChild(sourceObject);

    if( !aHeaderParams.respURI.isEmpty() ) {
        SyncMLCmdObject *respURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_RESPURI,
                                                              aHeaderParams.respURI );
        addChild( respURIObject );
    }

    if( aHeaderParams.meta.maxMsgSize > 0 || aHeaderParams.meta.maxObjSize > 0 ||
        aHeaderParams.meta.EMI.size() > 0 )
    {

        SyncMLMeta* metaObject = new SyncMLMeta;

        if( aHeaderParams.meta.maxMsgSize > 0 )
        {
            metaObject->addMaxMsgSize( aHeaderParams.meta.maxMsgSize );

        }

        if( aHeaderParams.meta.maxObjSize > 0 )
        {
            metaObject->addMaxObjSize( aHeaderParams.meta.maxObjSize );
        }

        for( int i = 0; i < aHeaderParams.meta.EMI.size(); ++i )
        {
            metaObject->addEMI( aHeaderParams.meta.EMI[i] );
        }

        addChild(metaObject);
    }


}

SyncMLHdr::~SyncMLHdr()
{
}
