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

#include "SyncMLPut.h"

#include "SyncMLMeta.h"
#include "SyncMLDevInf.h"
#include "internals.h"

#include "LogMacros.h"

using namespace DataSync;

SyncMLPut::SyncMLPut( int aCmdID, const QList<StoragePlugin*> &aDataStores,
                      const DeviceInfo& aDeviceInfo, ProtocolVersion aVersion,
                      Role aRole )
 : SyncMLCmdObject( SYNCML_ELEMENT_PUT )
{
    SyncMLCmdObject* cmdObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMDID,
                                                      QString::number( aCmdID ) );
    addChild( cmdObject );

    SyncMLMeta* metaObject = new SyncMLMeta;
    metaObject->addType( SYNCML_CONTTYPE_DEVINF_XML );
    addChild( metaObject );

    SyncMLDevInf* devInf = new SyncMLDevInf( aDataStores, aDeviceInfo,
                                             aVersion, aRole );
    addChild( devInf );


}
SyncMLPut::~SyncMLPut()
{
}