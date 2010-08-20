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

#include "OBEXServerWorker.h"

#include "OBEXConnection.h"
#include "OBEXDataHandler.h"

#include "LogMacros.h"

#define SYNCMLTARGET "SYNCML-SYNC"

using namespace DataSync;

OBEXServerWorker::OBEXServerWorker( OBEXConnection* aConnection,
                                    OBEXServerDataSource& aSource,
                                    int aTimeOut )
 : iConnection( aConnection ), iSource( aSource ), iTimeOut( aTimeOut ),
   iConnected( false ), iConnectionId( 1 ), iTransportHandle( 0 ),
   iProcessing( false ), iState( STATE_IDLE )
{
}

OBEXServerWorker::~OBEXServerWorker()
{

}

bool OBEXServerWorker::isConnected()
{
    return iConnected;
}

void OBEXServerWorker::waitForConnect()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        LOG_DEBUG( "Already connected, ignoring connect attempt" );
        return;
    }

    iTransportHandle = iConnection->connect( OBEXServerWorker::handleEvent );

    if( !iTransportHandle )
    {
        LOG_CRITICAL( "Could not set up OBEX link, aborting CONNECT" );
        return;
    }

    OBEX_SetUserData( iTransportHandle, this );

    LOG_DEBUG("Waiting for OBEX CONNECT");

    process( STATE_CONNECT );
}

void OBEXServerWorker::waitForDisconnect()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        LOG_DEBUG( "Waiting for OBEX DISCONNECT" );
        process( STATE_DISCONNECT );
    }
    else
    {
        LOG_DEBUG( "Not connected, ignoring disconnect attempt" );
    }

    iConnection->disconnect();
}

void OBEXServerWorker::waitForPut()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        LOG_DEBUG( "Waiting for OBEX PUT" );
        process( STATE_PUT );
    }
    else
    {
        LOG_WARNING( "Connection not established, cannot wait for PUT" );
        emit connectionFailed();
    }

}

void OBEXServerWorker::waitForGet()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        LOG_DEBUG( "Waiting for OBEX GET" );
        process( STATE_GET );
    }
    else
    {
        LOG_WARNING( "Connection not established, cannot wait for GET" );
        emit connectionFailed();
    }

}

int OBEXServerWorker::process( State aNextState )
{
    FUNCTION_CALL_TRACE;

    int result = 0;

    iState = aNextState;
    iProcessing = true;

    while( iProcessing )
    {
        result = OBEX_HandleInput( iTransportHandle, iTimeOut );

        if( result <= 0 )
        {
            iState = STATE_IDLE;
            iProcessing = false;
            break;
        }

    }

    return result;

}

void OBEXServerWorker::handleEvent( obex_t *aHandle, obex_object_t *aObject, int aMode,
                                    int aEvent, int aObexCmd, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    OBEXServerWorker* worker= (OBEXServerWorker*)OBEX_GetUserData( aHandle );
    LOG_DEBUG("OBEX Event: " << aEvent <<" Mode: " << aMode <<" Cmd: " << aObexCmd << " Resp: " << aObexRsp);

    switch( aEvent )
    {

        case OBEX_EV_REQHINT:
        {
            // @todo: we should immediately reject commands that we don't support
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_CONTINUE );
            break;
        }
        case OBEX_EV_REQ:
        {
            worker->requestReceived(aObject,aMode,aObexCmd);
            break;
        }
        case OBEX_EV_LINKERR:
        case OBEX_EV_PARSEERR:
        case OBEX_EV_ABORT:
        {
            worker->linkError();
            break;
        }
        default:
        {
            break;
        }

    }
}

void OBEXServerWorker::linkError()
{
    FUNCTION_CALL_TRACE;

    iState = STATE_IDLE;
    iProcessing = false;

    if( iConnected )
    {
        iConnected = false;

        emit connectionError();
    }
}

void OBEXServerWorker::requestReceived( obex_object_t *aObject, int aMode, int aObexCmd )
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aMode);

    switch( aObexCmd )
    {
        // Accept only supported commands
        case OBEX_CMD_CONNECT:
        {
            ConnectRequest( aObject );
            break;
        }
        case OBEX_CMD_DISCONNECT:
        {
            DisconnectRequest( aObject );
            break;
        }
        case OBEX_CMD_PUT:
        {
            if( iState == STATE_PUT )
            {
                PutRequest( aObject );
            }
            else
            {
                OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                            OBEX_RSP_SERVICE_UNAVAILABLE );
            }

            break;
        }
        case OBEX_CMD_GET:
        {
            if( iState == STATE_GET )
            {
                GetRequest( aObject );
            }
            else
            {
                OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                            OBEX_RSP_SERVICE_UNAVAILABLE );
            }


            break;
        }
        default:
        {
            LOG_WARNING( "Ignoring command related to unimplemented service" );
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_IMPLEMENTED,
                                        OBEX_RSP_NOT_IMPLEMENTED );
            break;
        }
    }
}

void OBEXServerWorker::ConnectRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE;

    if( iConnected )
    {
        LOG_WARNING( "Already connected, ignoring CONNECT");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::ConnectCmdData cmdData;

    if( !handler.parseConnectCmd( iTransportHandle, aObject, cmdData ) ) {
        LOG_WARNING( "Could not parse CONNECT request, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( cmdData.iTarget != SYNCMLTARGET )
    {
        LOG_WARNING( "CONNECT request not directed to SyncML, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }


    LOG_DEBUG("Assigning connection id: " << iConnectionId);
    OBEXDataHandler::ConnectRspData rspData;
    rspData.iConnectionId = iConnectionId;
    rspData.iWho = cmdData.iTarget;

    if( !handler.createConnectRsp( iTransportHandle, aObject, rspData ) )
    {
        LOG_CRITICAL( "Internal error when creating CONNECT response" );
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR, OBEX_RSP_INTERNAL_SERVER_ERROR );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    LOG_DEBUG("OBEX session established as server");
    iConnected = true;

    iProcessing = false;
    iState = STATE_IDLE;

}

void OBEXServerWorker::DisconnectRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE;

    if( !iConnected )
    {
        LOG_WARNING( "Not connected, ignoring DISCONNECT");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::DisconnectCmdData data;

    if( !handler.parseDisconnectCmd( iTransportHandle, aObject, data ) )
    {
        LOG_WARNING( "Could not parse DISCONNECT request, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( data.iConnectionId != iConnectionId )
    {
        LOG_WARNING( "Received DISCONNECT request not matching the session, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    if( iState != STATE_DISCONNECT )
    {
        LOG_WARNING( "Unexpected OBEX DISCONNECT received, presuming connection error");
        emit connectionError();
    }

    LOG_DEBUG("OBEX session disconnected as server");
    iConnected = false;

    iProcessing = false;
    iState = STATE_IDLE;

}

void OBEXServerWorker::PutRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE;

    if( !iConnected )
    {
        LOG_WARNING( "Not connected, ignoring PUT");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::PutCmdData data;

    if( !handler.parsePutCmd( iTransportHandle, aObject, data ) ) {
        LOG_WARNING( "Could not parse PUT request, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( data.iUnsupportedHeaders )
    {
        LOG_WARNING( "PUT request included unsupported headers, ignoring" );
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );
        return;
    }

    if( data.iConnectionId != iConnectionId )
    {
        LOG_WARNING( "Received PUT request not matching the session, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    LOG_DEBUG( "Received PUT with body size of:" << data.iBody.size() <<", content type:" << data.iContentType );

    iProcessing = false;
    iState = STATE_IDLE;

    emit incomingData( data.iBody, data.iContentType );

}

void OBEXServerWorker::GetRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE;

    if( !iConnected )
    {
        LOG_WARNING( "Not connected, ignoring GET");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::GetCmdData cmdData;

    if( !handler.parseGetCmd( iTransportHandle, aObject, cmdData ) ) {
        LOG_WARNING( "Could not parse GET request, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( cmdData.iConnectionId != iConnectionId )
    {
        LOG_WARNING( "Received GET request not matching the session, ignoring");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    QByteArray data;

    if( iSource.getData( QString::fromAscii( cmdData.iContentType ), data ) )
    {

        OBEXDataHandler::GetRspData rspData;
        rspData.iBody = data;
        rspData.iLength = data.length();

        if( handler.createGetRsp( iTransportHandle, aObject, rspData ) )
        {
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );
            LOG_DEBUG( "Responded to GET request for content type" << cmdData.iContentType << "with response of" << data.length() << "bytes" );
        }
        else
        {
            LOG_CRITICAL( "Error when generating OBEX GET response" );
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR,
                                        OBEX_RSP_INTERNAL_SERVER_ERROR );
        }

    }
    else
    {
        LOG_CRITICAL( "Could not retrieve response data for GET!");
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR, OBEX_RSP_INTERNAL_SERVER_ERROR );
    }

    iProcessing = false;
    iState = STATE_IDLE;

}
