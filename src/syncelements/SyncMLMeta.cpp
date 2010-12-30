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

#include "SyncMLMeta.h"

#include "datatypes.h"


using namespace DataSync;

// @todo: The version information inside meta tag is not currently being written
// to the ougoing SyncML messages (Although the information is put in the
// SyncItem's for incoming messages). This may have to be changed in the future.

SyncMLMeta::SyncMLMeta()
 : SyncMLCmdObject( SYNCML_ELEMENT_META)
{
}


SyncMLMeta::~SyncMLMeta()
{
}

void SyncMLMeta::addType( const QString& aType )
{
    SyncMLCmdObject* typeObject = new SyncMLCmdObject( SYNCML_ELEMENT_TYPE, aType );
    addNamespace( *typeObject );
    addChild( typeObject );
}

void SyncMLMeta::addSize( qint64 aSize )
{
    SyncMLCmdObject* sizeObject = new SyncMLCmdObject( SYNCML_ELEMENT_SIZE, QString::number( aSize ));
    addNamespace( *sizeObject );
    addChild( sizeObject );
}

void SyncMLMeta::addFormat( const QString& aFormat )
{
    SyncMLCmdObject* formatObject = new SyncMLCmdObject( SYNCML_ELEMENT_FORMAT, aFormat );
    addNamespace( *formatObject );
    addChild( formatObject );
}

void SyncMLMeta::addAnchors( const QString& aLastAnchor, const QString& aNextAnchor )
{
    SyncMLCmdObject* anchorObject = new SyncMLCmdObject(SYNCML_ELEMENT_ANCHOR);
    addNamespace( *anchorObject );

    SyncMLCmdObject* lastAnchorObject = new SyncMLCmdObject( SYNCML_ELEMENT_LAST, aLastAnchor );
    addNamespace( *lastAnchorObject );
    anchorObject->addChild(lastAnchorObject);

    SyncMLCmdObject* nextAnchorObject = new SyncMLCmdObject( SYNCML_ELEMENT_NEXT, aNextAnchor );
    addNamespace( *nextAnchorObject );
    anchorObject->addChild(nextAnchorObject);

    addChild( anchorObject );
}

void SyncMLMeta::addMaxMsgSize( qint64 aMaxMsgSize )
{
    SyncMLCmdObject* maxMsgSizeObject = new SyncMLCmdObject( SYNCML_ELEMENT_MAXMSGSIZE, QString::number( aMaxMsgSize ) );
    addNamespace( *maxMsgSizeObject );

    addChild( maxMsgSizeObject );
}

void SyncMLMeta::addMaxObjSize( qint64 aMaxObjSize )
{
    SyncMLCmdObject* maxObjSizeObject = new SyncMLCmdObject( SYNCML_ELEMENT_MAXOBJSIZE, QString::number( aMaxObjSize ) );
    addNamespace( *maxObjSizeObject );

    addChild( maxObjSizeObject );
}

void SyncMLMeta::addEMI( const QString& aEMI )
{
    SyncMLCmdObject* emiObject = new SyncMLCmdObject( SYNCML_ELEMENT_EMI, aEMI );
    addNamespace( *emiObject );

    addChild( emiObject );
}

void SyncMLMeta::addNextNonce( const QString& aNextNonce )
{
    SyncMLCmdObject* nonceObject = new SyncMLCmdObject( SYNCML_ELEMENT_NEXTNONCE, aNextNonce );
    addNamespace( *nonceObject );

    addChild( nonceObject );
}

void SyncMLMeta::addMark( const QString& aMark )
{
    SyncMLCmdObject* markObject = new SyncMLCmdObject( SYNCML_ELEMENT_MARK, aMark );
    addNamespace( *markObject );

    addChild( markObject );
}

void SyncMLMeta::addNamespace( SyncMLCmdObject& aObject )
{
    aObject.addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_METINF );
}

void SyncMLMeta::addVersion( const QString& aVersion )
{
    SyncMLCmdObject* versionObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERSION, aVersion );
    addNamespace( *versionObject );
    addChild( versionObject );
}
