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

#include "SyncMLMessage.h"

#include "internals.h"
#include "SyncMLHdr.h"
#include "SyncMLBody.h"

using namespace DataSync;

SyncMLMessage::SyncMLMessage( const HeaderParams& aHeaderParams,
                              ProtocolVersion aProtocolVersion)
 : SyncMLCmdObject(SYNCML_ELEMENT_SYNCML ), iMsgId( aHeaderParams.msgID ), iCmdId( 0 ),
   iProtocolVersion( aProtocolVersion )
{

    if( iProtocolVersion == DS_1_1 ) {
        addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML11 );
    }
    else if( iProtocolVersion == DS_1_2 ) {
        addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML12 );
    }

	iSyncHdr  = new SyncMLHdr( aHeaderParams, iProtocolVersion );
	addChild( iSyncHdr );

	iSyncBody = new SyncMLBody();
	addChild( iSyncBody );


}

SyncMLMessage::~SyncMLMessage()
{
}

void SyncMLMessage::addToHeader( SyncMLCmdObject* aObject )
{
    iSyncHdr->addChild( aObject );
}

void SyncMLMessage::addToBody( SyncMLCmdObject* aObject )
{
    iSyncBody->addChild( aObject );
}

int SyncMLMessage::getNextCmdId()
{
    return ++iCmdId;
}

int SyncMLMessage::getMsgId() const
{
    return iMsgId;
}

ProtocolVersion SyncMLMessage::getProtocolVersion() const
{
    return iProtocolVersion;
}
