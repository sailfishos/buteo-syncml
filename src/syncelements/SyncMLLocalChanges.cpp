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

#include "SyncMLLocalChanges.h"

#include "internals.h"

using namespace DataSync;

SyncMLLocalChanges::SyncMLLocalChanges( const QString &aElementName, int aCmdID,
                                        const QString& aTarget, const QString& aSource )
 : SyncMLCmdObject( aElementName )
{
    SyncMLCmdObject* cmdObject = new SyncMLCmdObject( SYNCML_ELEMENT_CMDID,
                                                      QString::number( aCmdID ) );
    addChild( cmdObject );

    addChild( generateTargetElement( aTarget ) );
    addChild( generateSourceElement( aSource ) );
}

SyncMLLocalChanges::~SyncMLLocalChanges()
{
}


SyncMLCmdObject* SyncMLLocalChanges::generateTargetElement( const QString& aTargetDB )
{
    SyncMLCmdObject* targetLocURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI, aTargetDB );

    SyncMLCmdObject* targetObject = new SyncMLCmdObject( SYNCML_ELEMENT_TARGET );
    targetObject->addChild( targetLocURIObject );


    return targetObject;
}

SyncMLCmdObject* SyncMLLocalChanges::generateSourceElement( const QString& aSourceDB )
{
    SyncMLCmdObject* sourceLocURIObject = new SyncMLCmdObject( SYNCML_ELEMENT_LOCURI, aSourceDB );

    SyncMLCmdObject* sourceObject = new SyncMLCmdObject( SYNCML_ELEMENT_SOURCE );
    sourceObject->addChild( sourceLocURIObject );


    return sourceObject;
}
