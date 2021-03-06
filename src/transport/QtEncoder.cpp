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

#include "QtEncoder.h"

#include <QXmlStreamWriter>

#include "SyncMLMessage.h"

#include "SyncMLLogging.h"

using namespace DataSync;

QtEncoder::QtEncoder()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

QtEncoder::~QtEncoder()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}


bool QtEncoder::encodeToXML( const SyncMLCmdObject& aRootObject,
                             QByteArray& aXMLDocument,
                             bool aPrettyPrint ) const
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    QXmlStreamWriter writer( &aXMLDocument );

    writer.setAutoFormatting( aPrettyPrint );

    writer.writeStartDocument();
    generateElement(aRootObject, writer );
    writer.writeEndDocument();

    return true;
}

void QtEncoder::generateElement( const SyncMLCmdObject& aObject,
                                 QXmlStreamWriter& aWriter ) const
{

    const QList<SyncMLCmdObject*>& children = aObject.getChildren();

    if( aObject.getValue().isEmpty() &&
        children.isEmpty() ) {

        aWriter.writeEmptyElement( aObject.getName() );

    }
    else {
        aWriter.writeStartElement( aObject.getName() );

        const QMap<QString, QString>& attributes = aObject.getAttributes();
        QXmlStreamAttributes attr;
        QMapIterator<QString, QString> i( attributes );
        while( i.hasNext() ) {
            i.next();

            attr.append( i.key(), i.value() );
        }

        aWriter.writeAttributes( attr );

        if( aObject.getCDATA() ) {
            aWriter.writeCDATA( aObject.getValue() );
        }
        else {
            aWriter.writeCharacters( aObject.getValue() );
        }

        for( int i = 0; i < children.count(); ++i ) {
            generateElement( *children[i], aWriter );
        }

        aWriter.writeEndElement();

    }

}
