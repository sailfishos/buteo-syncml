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

#include "OBEXDataHandler.h"

#include "LogMacros.h"

using namespace DataSync;

OBEXDataHandler::OBEXDataHandler()
{
}

OBEXDataHandler:: ~OBEXDataHandler()
{
}

obex_object_t* OBEXDataHandler::createConnectCmd( obex_t* aHandle, const ConnectCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;

    obex_object_t* object = OBEX_ObjectNew( aHandle, OBEX_CMD_CONNECT );

    if( object ) {

        // Target header
        QByteArray target = aData.iTarget;
        header.bs = (uint8_t*)target.constData();
        int err = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_TARGET,
                                        header, target.size(), OBEX_FL_FIT_ONE_PACKET );

        if( err == -1 ) {
            OBEX_ObjectDelete( aHandle, object );
            object = NULL;
        }

    }

    return object;
}

bool OBEXDataHandler::parseConnectCmd( obex_t* aHandle, obex_object_t* aObject, ConnectCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    bool targetFound = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_TARGET:
            {
                aData.iTarget = QByteArray( (char *)header.bs, len );
                targetFound = true;
                LOG_DEBUG("Found target: " << aData.iTarget);
                break;
            }
            default:
            {
                LOG_DEBUG("Unknown header: " << headertype);
                break;
            }
        }
    }

    if( targetFound ) {
        return true;
    }
    else {
        return false;
    }

}

bool OBEXDataHandler::createConnectRsp( obex_t* aHandle, obex_object_t* aObject, const ConnectRspData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;
    header.bq4 = aData.iConnectionId;
    int err1 = OBEX_ObjectAddHeader( aHandle, aObject, OBEX_HDR_CONNECTION,
                             header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

    QByteArray who = aData.iWho;
    header.bs = (uint8_t*)who.constData();
    int err2 = OBEX_ObjectAddHeader( aHandle, aObject, OBEX_HDR_WHO,
                                  header, who.size(), OBEX_FL_FIT_ONE_PACKET );

    if( err1 == -1 || err2 == -1 ) {
     return false;
    }
    else {
     return true;
    }

}

bool OBEXDataHandler::parseConnectRsp( obex_t* aHandle, obex_object_t* aObject, ConnectRspData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    bool connIdFound = false;
    bool whoFound = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_CONNECTION:
            {
                aData.iConnectionId = header.bq4;
                connIdFound = true;
                LOG_DEBUG("Found connection id: " << aData.iConnectionId);

                break;
            }
            case OBEX_HDR_WHO:
            {
                aData.iWho = QByteArray( (char *)header.bs, len );
                whoFound = true;
                LOG_DEBUG("Found who: " << aData.iWho);
                break;
            }
            default:
            {
                LOG_DEBUG("Unknown header: " << headertype);
                break;
            }
        }
    }

    if( connIdFound && whoFound ) {
        return true;
    }
    else {
        return false;
    }
}

obex_object_t* OBEXDataHandler::createDisconnectCmd( obex_t* aHandle, const DisconnectCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;
    obex_object_t* object = OBEX_ObjectNew( aHandle, OBEX_CMD_DISCONNECT );

    if( object ) {

        // Connection ID header
        header.bq4 = aData.iConnectionId;
        int err = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_CONNECTION,
                                        header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

        if( err == -1 ) {
            OBEX_ObjectDelete( aHandle, object );
            object = NULL;
        }

    }

    return object;

}

bool OBEXDataHandler::parseDisconnectCmd( obex_t* aHandle, obex_object_t* aObject, DisconnectCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    bool connIdFound = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_CONNECTION:
            {
                aData.iConnectionId = header.bq4;
                connIdFound = true;
                LOG_DEBUG("Found connection id: " << aData.iConnectionId);

                break;
            }
            default:
            {
                LOG_DEBUG("Unknown header: " << headertype);
                break;
            }
        }
    }

    if( connIdFound ){
        return true;
    }
    else {
        return false;
    }

}

obex_object_t* OBEXDataHandler::createPutCmd( obex_t* aHandle, const PutCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;

    obex_object_t* object = OBEX_ObjectNew( aHandle, OBEX_CMD_PUT );

    if( object ) {

        // Connection ID header
        header.bq4 = aData.iConnectionId;
        int err1 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_CONNECTION,
                                         header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

        // Content type header
        QByteArray contentType = aData.iContentType;
        header.bs = (uint8_t*)contentType.constData();
        int err2 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_TYPE,
                                         header, contentType.size()+1, OBEX_FL_FIT_ONE_PACKET );

        // Length header
        header.bq4 = aData.iLength;
        int err3 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_LENGTH,
                                         header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

        // Add body
        header.bs = (uint8_t*)aData.iBody.constData();
        int err4 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_BODY,
                                         header, aData.iBody.size(), 0 );

        if( err1 == -1 || err2 == -1 || err3 == -1 || err4 == -1 ) {
            OBEX_ObjectDelete( aHandle, object );
            object = NULL;
        }

    }

    return object;

}

bool OBEXDataHandler::parsePutCmd( obex_t* aHandle, obex_object_t* aObject, PutCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    bool connIdFound = false;
    bool typeFound = false;
    bool lengthFound = false;

    aData.iLength = -1;
    aData.iUnsupportedHeaders = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_CONNECTION:
            {
                aData.iConnectionId = header.bq4;;
                connIdFound = true;
                LOG_DEBUG("Found connection id: " << aData.iConnectionId);

                break;
            }
            case OBEX_HDR_TYPE:
            {
                aData.iContentType = QByteArray( (char *)header.bs );
                typeFound = true;
                LOG_DEBUG("Found type: " << aData.iContentType);
                break;
            }
            case OBEX_HDR_LENGTH:
            {
                aData.iLength = header.bq4;
                lengthFound = true;
                LOG_DEBUG("Found length: " << aData.iLength);
                break;
            }
            case OBEX_HDR_BODY:
            {
                QByteArray body( (char*)header.bs, len );
                aData.iBody.append( body );
                LOG_DEBUG("Found body with length of " << len << " bytes");
                break;
            }
            case OBEX_HDR_BODY_END:
            {
                QByteArray body( (char*)header.bs, len );
                aData.iBody.append( body );
                LOG_DEBUG("Found end of body");
                break;
            }
            default:
            {
                LOG_DEBUG("Unknown header: " << headertype);
                aData.iUnsupportedHeaders = true;
                break;
            }
        }
    }

    if( !connIdFound ) {
        LOG_DEBUG("Warning: no connection id header found");
    }

    if( typeFound ) {
        return true;
    }
    else {
        LOG_WARNING( "Request did not include TYPE header as expected" );
        return false;
    }

}

obex_object_t* OBEXDataHandler::createGetCmd( obex_t* aHandle, const GetCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;

    obex_object_t* object = OBEX_ObjectNew( aHandle, OBEX_CMD_GET );

    if( object ) {

        // Connection ID header
        header.bq4 = aData.iConnectionId;
        int err1 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_CONNECTION,
                                         header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

        // Content type header
        QByteArray contentType = aData.iContentType;
        header.bs = (uint8_t*)contentType.constData();
        int err2 = OBEX_ObjectAddHeader( aHandle, object, OBEX_HDR_TYPE,
                                         header, contentType.size()+1, OBEX_FL_FIT_ONE_PACKET );

        if( err1 == -1 || err2 == -1 ) {
            OBEX_ObjectDelete( aHandle, object );
            object = NULL;
        }
    }

    return object;
}

bool OBEXDataHandler::parseGetCmd( obex_t* aHandle, obex_object_t* aObject, GetCmdData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    bool connIdFound = false;
    bool typeFound = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_CONNECTION:
            {
                aData.iConnectionId = header.bq4;;
                connIdFound = true;
                LOG_DEBUG("Found connection id: " << aData.iConnectionId);

                break;
            }
            case OBEX_HDR_TYPE:
            {
                aData.iContentType = QByteArray( (char *)header.bs );
                typeFound = true;
                LOG_DEBUG("Found type: " << aData.iContentType);
                break;
            }
        }
    }

    if( !connIdFound ) {
        LOG_DEBUG("Warning: no connection id header found");
    }

    if( typeFound ) {
        return true;
    }
    else {
        return false;
    }

}

bool OBEXDataHandler::createGetRsp( obex_t* aHandle, obex_object_t* aObject, const GetRspData& aData )
{
    FUNCTION_CALL_TRACE;

    obex_headerdata_t header;

    // Length header
    header.bq4 = aData.iLength;
    int err1 = OBEX_ObjectAddHeader( aHandle, aObject, OBEX_HDR_LENGTH,
                                     header, sizeof( header.bq4 ), OBEX_FL_FIT_ONE_PACKET );

    // Add body
    header.bs = (uint8_t*)aData.iBody.constData();
    int err2 = OBEX_ObjectAddHeader( aHandle, aObject, OBEX_HDR_BODY,
                                     header, aData.iBody.size(), 0 );

    if( err1 == -1 || err2 == -1 ) {
        return false;
    }
    else {
        return true;
    }

}

bool OBEXDataHandler::parseGetRsp( obex_t* aHandle, obex_object_t* aObject, GetRspData& aData )
{
    FUNCTION_CALL_TRACE;

    uint8_t headertype;
    obex_headerdata_t header;
    uint32_t len;

    aData.iLength = -1;
    bool lengthFound = false;

    while( OBEX_ObjectGetNextHeader( aHandle, aObject, &headertype,
                                     &header, &len ) ) {
        switch( headertype )
        {
            case OBEX_HDR_LENGTH:
            {
                aData.iLength = header.bq4;
                lengthFound = true;
                LOG_DEBUG("Found length: " << aData.iLength);
                break;
            }
            case OBEX_HDR_BODY:
            {
                QByteArray body( (char*)header.bs, len );
                aData.iBody.append( body );
                LOG_DEBUG("Found body with length of " << len << " bytes");
                break;
            }
            case OBEX_HDR_BODY_END:
            {
                QByteArray body( (char*)header.bs, len );
                aData.iBody.append( body );
                LOG_DEBUG("Found end of body");
                break;
            }
            default:
            {
                LOG_DEBUG("Unknown header: " << headertype);
                break;
            }
        }

    }

    return true;

}
