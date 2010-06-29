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

#include "BaseTransport.h"

#include <QFile>

#include "SyncMLMessage.h"
#include "LibWbXML2Encoder.h"
#include "QtEncoder.h"
#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

BaseTransport::BaseTransport( QObject* aParent )
 : Transport( aParent ), iHandleIncomingData( false ), iWbXml( false )
{
    FUNCTION_CALL_TRACE;
}

BaseTransport::~BaseTransport()
{
    FUNCTION_CALL_TRACE;

    iIODevice.close();
}

void BaseTransport::setRemoteLocURI( const QString& aURI )
{
    FUNCTION_CALL_TRACE;

    iRemoteLocURI = aURI;
}

bool BaseTransport::sendSyncML( SyncMLMessage* aMessage )
{
    FUNCTION_CALL_TRACE;

    if( !aMessage ) {
        return false;
    }

    if( !prepareSend() ) {
        LOG_CRITICAL( "prepareSend() failed, cannot send message" );
        return false;
    }

    QByteArray data;

    if( !encodeMessage(*aMessage, data ) ) {
        return false;
    }

    delete aMessage;
    aMessage = NULL;

    QString contentType;
    if( useWbXml() )
    {
        contentType = SYNCML_CONTTYPE_WBXML;
    }
    else
    {
        contentType = SYNCML_CONTTYPE_XML;
    }

    return doSend( data, contentType );

}

bool BaseTransport::sendSAN( const QByteArray& aMessage )
{
    FUNCTION_CALL_TRACE;

#ifndef QT_NO_DEBUG
    LOG_PROTOCOL( "\nSending SAN message:\n=========\n" << aMessage.toHex() << "\n=========");
#endif  //  QT_NO_DEBUG

    return doSend( aMessage, SYNCML_CONTTYPE_SAN );
}

bool BaseTransport::receive()
{
    FUNCTION_CALL_TRACE;

    if( iIncomingData.size() > 0 ) {
        emitReadSignal();
        return true;
    }
    else {

        iHandleIncomingData = true;

        QString contentType;
        if( useWbXml() )
        {
            contentType = SYNCML_CONTTYPE_WBXML;
        }
        else
        {
            contentType = SYNCML_CONTTYPE_XML;
        }

        if( doReceive(contentType) ) {
            return true;
        }
        else {
            iHandleIncomingData = false;
            return false;
        }

    }
}

void BaseTransport::receive( const QByteArray& aData, const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    iIODevice.close();

    if( aData.isEmpty() ) {
        emit sendEvent( TRANSPORT_DATA_INVALID_CONTENT, "" );
        return;
    }

    if( aContentType.contains( SYNCML_CONTTYPE_WBXML ) ) {
        receiveWbXMLData( aData );
    }
    else if( aContentType.contains( SYNCML_CONTTYPE_XML ) ) {
        receiveXMLData( aData );
    }
    else if( aContentType.contains( SYNCML_CONTTYPE_SAN ) ) {
        receiveSANData( aData );
    }
    else {
        iIncomingData.clear();
        emit sendEvent( TRANSPORT_DATA_INVALID_CONTENT_TYPE, "" );
        return;
    }

    if( iHandleIncomingData ) {

        iHandleIncomingData = false;
        emitReadSignal();

    }
}

const QString& BaseTransport::getRemoteLocURI() const
{
    return iRemoteLocURI;
}

bool BaseTransport::encodeMessage( const SyncMLMessage& aMessage, QByteArray& aData )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    if( useWbXml() )
    {

        LibWbXML2Encoder encoder;

        if( encoder.encodeToWbXML( aMessage,
                                   aMessage.getProtocolVersion(),
                                   aData ) )
        {
            LOG_DEBUG( "WbXML encoding successful" );

#ifndef QT_NO_DEBUG

            QByteArray xml;
            if( encoder.encodeToXML( aMessage,
                                     aMessage.getProtocolVersion(),
                                     xml, true ) )
            {
                LOG_PROTOCOL( "\nSending message:\n=========\n" << xml << "\n=========");
            }
            else
            {
                LOG_PROTOCOL( "Failed to print request" );
            }

#endif  //  QT_NO_DEBUG
            success = true;

        }
        else
        {
            LOG_CRITICAL( "WbXML encoding failed!" );
        }

    }
    else
    {

        QtEncoder encoder;

        if( encoder.encodeToXML( aMessage, aData, false ) )
        {
            LOG_DEBUG( "XML encoding successful" );

#ifndef QT_NO_DEBUG

            QByteArray xml;
            if( encoder.encodeToXML( aMessage, xml, true ) ) {
                LOG_PROTOCOL( "\nSending message:\n=========\n" << xml << "\n=========");
            } else {
                LOG_PROTOCOL( "Failed to print request" );
            }

#endif  //  QT_NO_DEBUG
            success = true;
        }
        else
        {
            LOG_CRITICAL( "XML encoding failed!" );
        }
    }

    return success;
}

void BaseTransport::emitReadSignal()
{
    FUNCTION_CALL_TRACE;

    iIODevice.close();

    iIODeviceData = iIncomingData;
    iIncomingData.clear();

    iIODevice.setBuffer( &iIODeviceData );
    iIODevice.open( QIODevice::ReadOnly );

    if( iContentType == SYNCML_CONTTYPE_SAN ) {
        emit readSANData( &iIODevice );
    }
    else if( iContentType == SYNCML_CONTTYPE_XML ) {
        emit readXMLData( &iIODevice );
    }
    else {
        Q_ASSERT( 0 );
    }

}

void BaseTransport::setWbXml( bool aUse )
{
    FUNCTION_CALL_TRACE;
    iWbXml = aUse;
}

bool BaseTransport::useWbXml() const
{
    FUNCTION_CALL_TRACE;
    return iWbXml;
}

void BaseTransport::receiveWbXMLData( const QByteArray& aData )
{
    FUNCTION_CALL_TRACE;

    setWbXml( true );

    bool prettyPrint = false;

#ifndef QT_NO_DEBUG
    prettyPrint = true;
#endif  //  QT_NO_DEBUG

    LibWbXML2Encoder encoder;
    QByteArray xmlData;

    if( encoder.decodeFromWbxML( aData, xmlData, prettyPrint ) ) {
        receiveXMLData( xmlData );
    }
    else {

        LOG_WARNING( "WbXML to XML conversion failed!" );
        LOG_WARNING( "Presuming SAN package sent with wrong content type..." );
        receiveSANData( aData );
    }

}

void BaseTransport::receiveXMLData( const QByteArray& aData )
{
    FUNCTION_CALL_TRACE;

    iContentType = SYNCML_CONTTYPE_XML;
    iIncomingData = aData;

#ifndef QT_NO_DEBUG
    LOG_PROTOCOL( "\nReceived XML message:\n=========\n" << iIncomingData << "\n=========");
#endif  //  QT_NO_DEBUG

}

void BaseTransport::receiveSANData( const QByteArray& aData )
{

    setWbXml( true );

    iContentType = SYNCML_CONTTYPE_SAN;
    iIncomingData = aData;

#ifndef QT_NO_DEBUG
    LOG_PROTOCOL( "\nReceived SAN message:\n=========\n" << iIncomingData.toHex() << "\n=========");
#endif  //  QT_NO_DEBUG

}