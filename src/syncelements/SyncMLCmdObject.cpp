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

#include "SyncMLCmdObject.h"
#include "LogMacros.h"
#include "LibWbXML2Encoder.h"
#include "QtEncoder.h"
#include "datatypes.h"

// As this base class is extensively used in SyncML generation, please do not
// enable function tracing unless you have a very good reason to do that.
// Otherwise, prepare for severe debug spam.

using namespace DataSync;

SyncMLCmdObject::SyncMLCmdObject( const QString& aName, const QString& aValue )
: iName( aName ), iValue( aValue ), iIsCDATA( false )

{

}

SyncMLCmdObject::~SyncMLCmdObject() {

    qDeleteAll(iChildren);
    iChildren.clear();
}

const QString& SyncMLCmdObject::getName() const
{
    return iName;
}

void SyncMLCmdObject::setName( const QString& aName )
{
    iName = aName;
}

const QString& SyncMLCmdObject::getValue() const
{
    return iValue;
}

void SyncMLCmdObject::setValue( const QString& aValue )
{
    iValue = aValue;
}

bool SyncMLCmdObject::getCDATA() const
{
    return iIsCDATA;
}

void SyncMLCmdObject::setCDATA( bool aCDATA )
{
    iIsCDATA = aCDATA;
}

void SyncMLCmdObject::addAttribute( const QString& aName, const QString& aValue )
{
    iAttributes.insert( aName, aValue );
}

const QMap<QString, QString>& SyncMLCmdObject::getAttributes() const
{
    return iAttributes;
}

void SyncMLCmdObject::addChild( SyncMLCmdObject* aChild )
{

    Q_ASSERT( aChild );
    iChildren.append( aChild );

}

const QList<SyncMLCmdObject*>& SyncMLCmdObject::getChildren() const
{
    return iChildren;
}

int SyncMLCmdObject::calculateSize(bool aWBXML, const ProtocolVersion& aVersion)
{

    // It should be noted that this is just coarse estimation! Document size limit
    // is always set to 90% of the maximum available transport size, so this does
    // not need to be byte-accurate. We gain lots of performance when we don't have
    // to serialize to XML to check the current size

    if (aWBXML) {
        LibWbXML2Encoder encoder;

        QByteArray aData;

        if( aVersion == SYNCML_1_1 ) {
            addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML11 );
        }
        else if( aVersion == SYNCML_1_2 ) {
            addAttribute( XML_NAMESPACE, XML_NAMESPACE_VALUE_SYNCML12 );
        }


        if( encoder.encodeToWbXML( *this,
                                   aVersion,
                                   aData ) )
        {
            return aData.size();
        }
    }

    int size = 0;

    if( iValue.isEmpty() &&
        iChildren.isEmpty() )
    {
        // <element/>
        size += 3 + iName.length();
    }
    else
    {

        // <element> + </element>

        if( iName.length() > 0 )
        {
            size += 5 + 2* iName.length();
        }

        // value
        size += iValue.length();

        // CDATA
        if( iIsCDATA )
        {
            // <![CDATA[----]]>
            size += 12;
        }

        for( int i = 0; i < iChildren.count(); ++i ) {
            size += iChildren[i]->calculateSize(aWBXML, aVersion);
        }
    }

    // attributes ( attr="value" )
    QMapIterator<QString, QString> i( iAttributes );
    while( i.hasNext() )
    {
        i.next();
        size +=  1 + i.key().length() + 2 + i.value().length() + 1;
    }

    return size;
}
