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

#include "SyncMLCTCap.h"
#include "SyncMLGlobals.h"

#include <QXmlStreamReader>

#include "datatypes.h"
#include "SyncMLLogging.h"

using namespace DataSync;

SyncMLCTCap::SyncMLCTCap() : SyncMLCmdObject( SYNCML_ELEMENT_CTCAP )
{
}

SyncMLCTCap::~SyncMLCTCap()
{
}

void SyncMLCTCap::addCTCap( const QByteArray& aDescription )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);


    QXmlStreamReader reader( aDescription );

    while( !reader.atEnd() ) {
        reader.readNext();

        // Ignore the enclosing <CTCap> start and end elements
        if( reader.name() == SYNCML_ELEMENT_CTCAP ) {
            continue;
        }

        // Convert the found XML elements to SyncMLCmdObjects.
        if( reader.isStartElement() ) {
            processElement( reader, this );
        }

    }

}

void SyncMLCTCap::processElement( QXmlStreamReader& aReader, SyncMLCmdObject* aParent )
{

    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    SyncMLCmdObject* obj = new SyncMLCmdObject( aReader.name().toString() );

    while( !aReader.atEnd() ) {

        aReader.readNext();

        if( aReader.isStartElement() ) {
            processElement( aReader, obj );
        }
        else if( aReader.isCharacters() ) {
            obj->setValue( aReader.text().toString().simplified() );
        }
        else if( aReader.isEndElement() ) {
            break;
        }

    }

    aParent->addChild( obj );

}
