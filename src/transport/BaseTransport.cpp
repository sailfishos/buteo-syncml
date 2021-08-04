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

#include "BaseTransport.h"

#include <QFile>
#include <QRegExp>

#include "SyncMLMessage.h"
#include "LibWbXML2Encoder.h"
#include "QtEncoder.h"
#include "datatypes.h"

#include "SyncMLLogging.h"

using namespace DataSync;

BaseTransport::BaseTransport( const ProtocolContext& aContext, QObject* aParent )
 : Transport( aParent ), iContext( aContext ), iHandleIncomingData( false ),
   iWbXml( false )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

BaseTransport::~BaseTransport()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iIODevice.close();
}

void BaseTransport::setRemoteLocURI( const QString& aURI )
{
    iRemoteLocURI = aURI;
}

bool BaseTransport::usesWbXML()
{
    return iWbXml;
}

bool BaseTransport::sendSyncML( SyncMLMessage* aMessage )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !aMessage ) {
        return false;
    }

    if( !prepareSend() ) {
        qCCritical(lcSyncML) << "prepareSend() failed, cannot send message";
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
        if( iContext== CONTEXT_DM )
        {
            contentType = SYNCML_CONTTYPE_DM_WBXML;
        }
        else
        {
            contentType = SYNCML_CONTTYPE_DS_WBXML;
        }
    }
    else
    {
        if( iContext == CONTEXT_DM )
        {
            contentType = SYNCML_CONTTYPE_DM_XML;
        }
        else
        {
            contentType = SYNCML_CONTTYPE_DS_XML;
        }
    }

    return doSend( data, contentType );

}

bool BaseTransport::sendSAN( const QByteArray& aMessage )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

#ifndef QT_NO_DEBUG
    qCDebug(lcSyncMLProtocol) << "\nSending SAN message:\n=========\n" << aMessage.toHex() << "\n=========";
#endif  //  QT_NO_DEBUG

    return doSend( aMessage, SYNCML_CONTTYPE_SAN_DS );
}

bool BaseTransport::receive()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( iIncomingData.size() > 0 ) {
        emitReadSignal();
        return true;
    }
    else {

        iHandleIncomingData = true;

        QString contentType;
        if( useWbXml() )
        {
            if( iContext == CONTEXT_DM )
            {
                contentType = SYNCML_CONTTYPE_DM_WBXML;
            }
            else
            {
                contentType = SYNCML_CONTTYPE_DS_WBXML;
            }
        }
        else
        {
            if( iContext == CONTEXT_DM )
            {
                contentType = SYNCML_CONTTYPE_DM_XML;
            }
            else
            {
                contentType = SYNCML_CONTTYPE_DS_XML;
            }
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
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iIODevice.close();

    if( aData.isEmpty() ) {
        emit sendEvent( TRANSPORT_DATA_INVALID_CONTENT, "" );
        return;
    }

    if( aContentType.contains( SYNCML_CONTTYPE_DS_WBXML ) ||
        aContentType.contains( SYNCML_CONTTYPE_DM_WBXML ) ) {
        receiveWbXMLData( aData );
    }
    else if( aContentType.contains( SYNCML_CONTTYPE_DS_XML ) ||
             aContentType.contains( SYNCML_CONTTYPE_DM_XML)) {
        receiveXMLData( aData );
    }
    else if( aContentType.contains( SYNCML_CONTTYPE_SAN_DS ) ) {
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
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    bool success = false;

    if( useWbXml() )
    {

        LibWbXML2Encoder encoder;

        if( encoder.encodeToWbXML( aMessage,
                                   aMessage.getProtocolVersion(),
                                   aData ) )
        {
            qCDebug(lcSyncML) << "WbXML encoding successful";

#ifndef QT_NO_DEBUG

            QByteArray xml;
            if( encoder.encodeToXML( aMessage,
                                     aMessage.getProtocolVersion(),
                                     xml, true ) )
            {
                qCDebug(lcSyncMLProtocol) << "\nSending message:\n=========\n" << xml << "\n=========size:"<<xml.size();
            }
            else
            {
                qCDebug(lcSyncMLProtocol) << "Failed to print request" ;
            }

#endif  //  QT_NO_DEBUG
            success = true;

        }
        else
        {
            qCCritical(lcSyncML) << "WbXML encoding failed!";
        }

    }
    else
    {

        QtEncoder encoder;

        if( encoder.encodeToXML( aMessage, aData, false ) )
        {
            qCDebug(lcSyncML) << "XML encoding successful";

#ifndef QT_NO_DEBUG

            QByteArray xml;
            if( encoder.encodeToXML( aMessage, xml, true ) ) {
                qCDebug(lcSyncMLProtocol) << "\nSending message:\n=========\n" << xml << "\n=========";
            } else {
                qCDebug(lcSyncMLProtocol) << "Failed to print request" ;
            }

#endif  //  QT_NO_DEBUG
            success = true;
        }
        else
        {
            qCCritical(lcSyncML) << "XML encoding failed!";
        }
    }

    return success;
}

void BaseTransport::emitReadSignal()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iIODevice.close();

    iIODeviceData = iIncomingData;
    iIncomingData.clear();

    iIODevice.setBuffer( &iIODeviceData );
    iIODevice.open( QIODevice::ReadOnly );

    if( iContentType == SYNCML_CONTTYPE_SAN_DS ) {
        emit readSANData( &iIODevice );
    }
    else if( iContentType == SYNCML_CONTTYPE_DM_XML ||
             iContentType == SYNCML_CONTTYPE_DS_XML ) {
        emit readXMLData( &iIODevice, true );
    }
    else {
        Q_ASSERT( 0 );
    }

}

void BaseTransport::setWbXml( bool aUse )
{
    iWbXml = aUse;
}

bool BaseTransport::useWbXml() const
{
    return iWbXml;
}

void BaseTransport::receiveWbXMLData( const QByteArray& aData )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    setWbXml( true );

    bool prettyPrint = false;

#ifndef QT_NO_DEBUG
    prettyPrint = true;
#endif  //  QT_NO_DEBUG

    LibWbXML2Encoder encoder;
    QByteArray xmlData;

    if( encoder.decodeFromWbXML( aData, xmlData, prettyPrint ) ) {
        receiveXMLData( xmlData );
    }
    else {

        qCWarning(lcSyncML) << "WbXML to XML conversion failed!";
        qCWarning(lcSyncML) << "Presuming SAN package sent with wrong content type...";
        receiveSANData( aData );
    }

}

void BaseTransport::receiveXMLData( const QByteArray& aData )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( iContext == CONTEXT_DM )
    {
        iContentType = SYNCML_CONTTYPE_DM_XML;
    }
    else
    {
        iContentType = SYNCML_CONTTYPE_DS_XML;
    }
    iIncomingData = aData;

#ifndef QT_NO_DEBUG
    qCDebug(lcSyncMLProtocol) << "\nReceived XML message:\n=========\n" << iIncomingData << "\n=========";
#endif  //  QT_NO_DEBUG

}

void BaseTransport::receiveSANData( const QByteArray& aData )
{

    setWbXml( true );

    iContentType = SYNCML_CONTTYPE_SAN_DS;
    iIncomingData = aData;

#ifndef QT_NO_DEBUG
    qCDebug(lcSyncMLProtocol) << "\nReceived SAN message:\n=========\n" << iIncomingData.toHex() << "\n=========";
#endif  //  QT_NO_DEBUG

}

void BaseTransport::purgeAndResendBuffer()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
    if(iIODeviceData.size() > 0)
    {
        QString dataString = QString::fromUtf8(iIODeviceData, iIODeviceData.size());
        // Strip illegal XML characters
        dataString.remove(QChar(0x00));
        dataString.remove(QRegExp("[\x0001-\x0008,\x000B-\x000C,\x000E-\x001F]"));

        iIODeviceData = dataString.toUtf8();

#ifndef QT_NO_DEBUG
        qCDebug(lcSyncMLProtocol) << "\nPurged XML message:\n=========\n" << iIODeviceData << "\n=========";
#endif  //  QT_NO_DEBUG

        // Put the new buffer into the IO device
        iIODevice.close();

        iIODevice.setBuffer( &iIODeviceData );
        iIODevice.open( QIODevice::ReadOnly );

        emit readXMLData( &iIODevice, false );
    }
}

