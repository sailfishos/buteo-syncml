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

#include "ServerAlertedNotification.h"

#include <QCryptographicHash>

#include "LogMacros.h"

using namespace DataSync;

// MD5 digest is 128 bits, which is 16 bytes
#define DIGEST_SIZE             16

// SAN Header is 8 bytes (excluding server identified)
#define HEADER_SIZE             8

// SyncML version 1.1 (0000001011)
#define SYNCMLVERSION_1_1       0x0B

// SyncML version 1.2 (0000001100)
#define SYNCMLVERSION_1_2       0x0C

#define UIMODE_NOT_SPECIFIED    0x00
#define UIMODE_BACKGROUND       0x10
#define UIMODE_INFORMATIVE      0x20
#define UIMODE_USER_INTERACTION 0x30

#define INITIATOR_USER          0x00
#define INITIATOR_SERVER        0x08

#define MAX_SERVERURI_LENGTH    255
#define SYNCALERT_BASE          200

// Supported WSP content types
// From: http://www.wapforum.org/wina/wsp-content-type.htm
#define WSP_CONTACTS_ID         0x07
#define WSP_CONTACTS_MIME       "text/x-vcard"
#define WSP_CALENDAR_ID         0x06
#define WSP_CALENDAR_MIME       "text/x-vcalendar"
#define WSP_NOTES_ID            0x03
#define WSP_NOTES_MIME          "text/plain"

SANHandler::SANHandler()
{

}

SANHandler::~SANHandler()
{

}

bool SANHandler::checkDigest( const QByteArray& aMessage,
                              const QString& aServerIdentifier,
                              const QString& aPassword,
                              const QString& aNonce )
{
    FUNCTION_CALL_TRACE;

    QByteArray messageDigest = aMessage.left( DIGEST_SIZE );
    QByteArray messageNotification = aMessage.mid( DIGEST_SIZE );

    QByteArray digest = generateDigest( aServerIdentifier, aPassword, aNonce, messageNotification );

    if( digest == messageDigest )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SANHandler::parseSANMessage( const QByteArray& aMessage,
                                  SANData& aData )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Parsing SAN message of" << aMessage.size() << "bytes" );

    QByteArray digest = aMessage.left( DIGEST_SIZE );
    QByteArray header = aMessage.mid( DIGEST_SIZE, HEADER_SIZE );

    if( digest.size() != DIGEST_SIZE ) {
        LOG_WARNING( "Invalid digest" );
        return false;
    }

    if( header.size() != HEADER_SIZE ) {
        LOG_WARNING( "Invalid header" );
        return false;
    }

    LOG_DEBUG( "SAN digest:" << digest.toHex() );
    LOG_DEBUG( "SAN header:" << header.toHex() );

    int version = ( header[0] << 2 ) | ( header[1] >> 6 );
    char uimode = ( header[1] >> 4 ) & 0x03;
    bool initiator = ( header[1] >> 3 ) & 0x01;
    qint16 sessionId = ( header[5] << 8 ) | header[6];

    if( version == SYNCMLVERSION_1_1 ) {
        aData.iVersion = SYNCML_1_1;
    }
    else if( version == SYNCMLVERSION_1_2 ) {
        aData.iVersion = SYNCML_1_2;
    }
    else {
        LOG_WARNING( "Unsupported SyncML version" );
        return false;
    }

    aData.iUIMode = static_cast<SANUIMode>( uimode );
    aData.iInitiator = static_cast<SANInitiator>( initiator );
    aData.iSessionId = sessionId;

    int serverIdentifierLength = header[7];
    QString serverIdentifier = aMessage.mid( DIGEST_SIZE + HEADER_SIZE, serverIdentifierLength );

    if( serverIdentifier.length() != serverIdentifierLength ) {
        LOG_WARNING( "Invalid server identifier" );
        return false;
    }

    aData.iServerIdentifier = serverIdentifier;

    QByteArray notification = aMessage.mid( DIGEST_SIZE + HEADER_SIZE + serverIdentifierLength );

    if( notification.isEmpty() ) {
        LOG_WARNING( "Invalid notification body" );
        return false;
    }

    int numberOfSyncs = notification[0] >> 4;

    if( numberOfSyncs == 0 ) {
        LOG_WARNING( "Syncing of all data stores not supported" );
        return false;
    }

    int pos = 1;

    for( int i = 0; i< numberOfSyncs; ++i ) {

        // Check that SyncInfo is at least 5 bytes long
        if( notification.size() - pos < 5 ) {
            LOG_WARNING( "Invalid sync info" );
            return false;
        }

        SANSyncInfo syncInfo;

        syncInfo.iSyncType = SYNCALERT_BASE + ( notification[pos++] >> 4 );

        long contentType = notification[pos] << 16 | notification[pos+1] << 8 | notification[pos+2];
        pos += 3;

        // WSP Content type mapping
        if( contentType == WSP_CONTACTS_ID ){
            syncInfo.iContentType = WSP_CONTACTS_MIME;
        }
        else if( contentType == WSP_CALENDAR_ID ) {
            syncInfo.iContentType = WSP_CALENDAR_MIME;
        }
        else if( contentType == WSP_NOTES_ID ) {
            syncInfo.iContentType = WSP_NOTES_MIME;
        }
        else if( contentType != 0 ) {
            LOG_WARNING( "Unsupported WSP Content type:" << contentType );
        }

        char serverURILength = notification[pos++];

        QByteArray serverURI = notification.mid( pos, serverURILength );

        if( serverURI.length() != serverURILength ) {
            LOG_WARNING( "Invalid server URI");
            return false;
        }

        syncInfo.iServerURI = serverURI;
        pos += serverURILength;

        aData.iSyncInfo.append( syncInfo );

    }

    return true;

}

bool SANHandler::generateSANMessage( const SANData& aData,
                                     const QString& aPassword,
                                     const QString& aNonce,
                                     QByteArray& aMessage )
{
    FUNCTION_CALL_TRACE;

    QByteArray notification;

    // Write version, UI-mode, initiator
    unsigned char highByte = 0;
    unsigned char lowByte = 0;

    if( aData.iVersion == SYNCML_1_1 )
    {
        highByte = SYNCMLVERSION_1_1 >> 2;
        lowByte = (unsigned char)(SYNCMLVERSION_1_1 << 8);
    }
    else if( aData.iVersion == SYNCML_1_2 )
    {
        highByte = SYNCMLVERSION_1_2 >> 2;
        lowByte = (unsigned char)(SYNCMLVERSION_1_2 << 8);
    }
    else
    {
        LOG_WARNING( "Unsupported version: " << aData.iVersion );
        return false;
    }

    if( aData.iUIMode == SANUIMODE_NOT_SPECIFIED )
    {
        lowByte |= UIMODE_NOT_SPECIFIED;
    }
    else if( aData.iUIMode == SANUIMODE_BACKGROUND )
    {
        lowByte |= UIMODE_BACKGROUND;
    }
    else if( aData.iUIMode == SANUIMODE_INFORMATIVE )
    {
        lowByte |= UIMODE_INFORMATIVE;
    }
    else if( aData.iUIMode == SANUIMODE_USER_INTERACTION )
    {
        lowByte |= UIMODE_USER_INTERACTION;
    }
    else
    {
        LOG_WARNING( "Unsupported user interaction mode:" << aData.iUIMode );
        return false;
    }

    if( aData.iInitiator == SANINITIATOR_USER )
    {
        lowByte |= INITIATOR_USER;
    }
    else if( aData.iInitiator == SANINITIATOR_SERVER )
    {
        lowByte |= INITIATOR_SERVER;
    }
    else
    {
        LOG_WARNING( "Unsupported initiator of the notification:" << aData.iInitiator );
        return false;
    }

    notification.fill( 0, HEADER_SIZE );
    notification[0] = highByte;
    notification[1] = lowByte;

    int serverIdentifierLength = aData.iServerIdentifier.size();

    if( serverIdentifierLength > MAX_SERVERURI_LENGTH )
    {
        LOG_WARNING( "Server identifier lenght more than 255 characters" );
        return false;
    }

    notification[7] = (unsigned char)serverIdentifierLength;
    notification.append( aData.iServerIdentifier.toAscii() );

    // Special case for syncing all data storages if no sync infos were specified
    unsigned char numSync = 0;

    if( aData.iSyncInfo.count() > 0 )
    {
        numSync = aData.iSyncInfo.count() << 4;
    }

    notification.append( numSync );

    for( int i = 0; i < aData.iSyncInfo.count(); ++i )
    {

        const SANSyncInfo& info = aData.iSyncInfo[i];

        unsigned char syncType = (info.iSyncType - SYNCALERT_BASE) << 4;
        qint32 contentType = 0;

        if( info.iContentType == WSP_CONTACTS_MIME )
        {
            contentType = WSP_CONTACTS_ID;
        }
        else if( info.iContentType == WSP_CALENDAR_MIME )
        {
            contentType = WSP_CALENDAR_ID;
        }
        else if( info.iContentType == WSP_NOTES_MIME )
        {
            contentType = WSP_NOTES_ID;
        }
        else if( !info.iContentType.isEmpty() )
        {
            LOG_WARNING( "Unsupported WSP Content type:" << info.iContentType );
        }

        notification.append( syncType );
        notification.append( ( contentType & 0x00FF0000 ) >> 16 );
        notification.append( ( contentType & 0x0000FF00 ) >> 8 );
        notification.append( contentType & 0x000000FF );
        notification.append( info.iServerURI.length() );
        notification.append( info.iServerURI.toAscii() );

    }

    aMessage = generateDigest( aData.iServerIdentifier, aPassword, aNonce, notification );
    aMessage.append( notification );

    return true;
}

// Digest = H(B64(H(server-identifier:password)):nonce:B64(H(notification)))
// See SyncML Server Alarted Notification and DS Protocol specifications for more information
QByteArray SANHandler::generateDigest( const QString& aServerIdentifier, const QString& aPassword,
                                       const QString& aNonce, const QByteArray& aNotification )
{

    FUNCTION_CALL_TRACE;

    const QString colon( ":" );

    QByteArray digest;

    QByteArray first;
    first.append( aServerIdentifier.toAscii() );
    first.append( colon );
    first.append( aPassword.toAscii() );

    QByteArray second = QCryptographicHash::hash( first, QCryptographicHash::Md5 ).toBase64();
    second.append( colon );
    second.append( aNonce.toAscii() );
    second.append( colon );
    second.append( QCryptographicHash::hash( aNotification, QCryptographicHash::Md5 ).toBase64() );

    digest = QCryptographicHash::hash( second, QCryptographicHash::Md5 );

    return digest;
}
