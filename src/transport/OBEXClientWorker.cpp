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

#include "OBEXClientWorker.h"

#include "OBEXConnection.h"
#include "OBEXDataHandler.h"

#include "LogMacros.h"

#define SYNCMLTARGET "SYNCML-SYNC"

using namespace DataSync;

OBEXClientWorker::OBEXClientWorker( OBEXConnection* aConnection, int aTimeOut )
 : iConnection( aConnection ), iTimeOut( aTimeOut ), iConnected( false ),
   iConnectionId( -1 ), iTransportHandle( 0 ), iProcessing( false )
{
}

OBEXClientWorker::~OBEXClientWorker()
{

}

bool OBEXClientWorker::isConnected()
{
    return iConnected;
}

void OBEXClientWorker::connect()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        LOG_DEBUG( "Already connected, ignoring connect attempt" );
        return;
    }

    iTransportHandle = iConnection->connect( OBEXClientWorker::handleEvent );

    if( !iTransportHandle )
    {
        LOG_CRITICAL( "Could not set up OBEX link, aborting CONNECT" );
        return;
    }

    OBEX_SetUserData( iTransportHandle, this );

    LOG_DEBUG("Sending OBEX CONNECT");

    OBEXDataHandler handler;
    OBEXDataHandler::ConnectCmdData data;
    data.iTarget = SYNCMLTARGET;

    obex_object_t* object = handler.createConnectCmd( iTransportHandle, data );

    if( !object || !OBEX_Request( iTransportHandle, object ) < 0 )
    {
        LOG_CRITICAL( "Failed in OBEX_Request while doing CONNECT" );
        return;
    }

    process();

}

void OBEXClientWorker::disconnect()
{
    FUNCTION_CALL_TRACE;

    if( isConnected() )
    {
        OBEXDataHandler handler;
        OBEXDataHandler::DisconnectCmdData data;
        data.iConnectionId = iConnectionId;

        obex_object_t* object = handler.createDisconnectCmd( iTransportHandle, data );

        if( !object || !OBEX_Request( iTransportHandle, object ) < 0 )
        {
            // Cannot send disconnect, then we have no choice but to force transport
            // disconnection
            LOG_CRITICAL( "Failed in OBEX_Request while doing DISCONNECT" );
            LOG_CRITICAL( "Forcing link disconnect" );
        }
        else
        {
            process();
        }
    }
    else
    {
        LOG_DEBUG( "Not connected, ignoring sending OBEX DISCONNECT" );
    }

    iConnection->disconnect();

}

void OBEXClientWorker::send( const QByteArray& aBuffer, const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    if( !isConnected() )
    {
        LOG_WARNING( "Connection not established, cannot send" );
        emit connectionFailed();
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::PutCmdData data;
    data.iConnectionId = iConnectionId;
    data.iContentType = aContentType.toAscii();
    data.iLength = aBuffer.size();
    data.iBody = aBuffer;

    obex_object_t* object = handler.createPutCmd( iTransportHandle, data );

    if( !object || OBEX_Request( iTransportHandle, object ) < 0 )
    {
        LOG_WARNING( "Failed in OBEX_Request while doing PUT" );
        emit connectionError();
        return;
    }

    int result = process();

    if( result < 0 )
    {
        LOG_WARNING( "OBEX PUT failed" );
        emit connectionError();
    }
    else if( result == 0 )
    {
        LOG_WARNING( "OBEX PUT timed out" );
        emit connectionTimeout();
    }

}

void OBEXClientWorker::receive( const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    if( !isConnected() )
    {
        LOG_WARNING( "Connection not established, cannot receive" );
        emit connectionFailed();
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::GetCmdData data;
    data.iConnectionId = iConnectionId;
    data.iContentType = aContentType.toAscii();

    obex_object_t* object = handler.createGetCmd( iTransportHandle, data );

    if( !object || OBEX_Request( iTransportHandle, object ) < 0 ) {
        LOG_WARNING( "Failed in OBEX_Request while doing GET" );
        emit connectionError();
        return;
    }

    iGetContentType = aContentType;

    int result = process();

    if( result < 0 )
    {
        LOG_WARNING( "OBEX GET failed" );
        emit connectionError();
    }
    else if( result == 0 )
    {
        LOG_WARNING( "OBEX GET timed out" );
        emit connectionTimeout();
    }

}

int OBEXClientWorker::process()
{
    FUNCTION_CALL_TRACE;

    iProcessing = true;

    int result = 0;

    while( iProcessing )
    {
        result = OBEX_HandleInput( iTransportHandle, iTimeOut );

        if( result <= 0 )
        {
            iProcessing = false;
            break;
        }

    }

    return result;

}

void OBEXClientWorker::handleEvent( obex_t *aHandle, obex_object_t *aObject, int aMode,
                                    int aEvent, int aObexCmd, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    OBEXClientWorker* worker= (OBEXClientWorker*)OBEX_GetUserData( aHandle );
    LOG_DEBUG("OBEX Event: " << aEvent <<" Mode: " << aMode <<" Cmd: " << aObexCmd << " Resp: " << aObexRsp);

    switch( aEvent )
    {

        // Request is done
        case OBEX_EV_REQDONE:
        {
            worker->RequestCompleted(aObject, aMode, aObexCmd, aObexRsp);
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

void OBEXClientWorker::linkError()
{
    FUNCTION_CALL_TRACE;

    iProcessing = false;

    if( iConnected )
    {
        iConnected = false;
        // Close the OBEX link
        iConnection->disconnect();
        emit connectionError();
    }

}

void OBEXClientWorker::RequestCompleted( obex_object_t *aObject, int aMode, int aObexCmd, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aMode);
    Q_UNUSED(aObject);

    switch( aObexCmd )
    {
        case OBEX_CMD_CONNECT:
        {
            ConnectResponse( aObject, aObexRsp );
            break;
        }
        case OBEX_CMD_DISCONNECT:
        {
            DisconnectResponse( aObject, aObexRsp );
            break;
        }
        case OBEX_CMD_PUT:
        {
            PutResponse( aObject, aObexRsp );
            break;
        }
        case OBEX_CMD_GET:
        {
            GetResponse( aObject, aObexRsp );
            break;
        }
        default:
        {
            break;
        }

    }
}

void OBEXClientWorker::ConnectResponse( obex_object_t *aObject, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    if( aObexRsp == OBEX_RSP_SUCCESS )
    {

        OBEXDataHandler handler;
        OBEXDataHandler::ConnectRspData data;

        if( handler.parseConnectRsp( iTransportHandle, aObject, data ) ) {
            iConnectionId = data.iConnectionId;
            LOG_DEBUG("OBEX session established as client");
            iConnected = true;
        }
        else
        {
            LOG_WARNING( "OBEX Connect: failed, remote device sent invalid response" );
        }
    }
    else
    {
        LOG_WARNING( "OBEX Connect: failed, remote device sent " << aObexRsp );
    }

    iProcessing = false;
}

void OBEXClientWorker::DisconnectResponse( obex_object_t */*aObject*/, int /*aObexRsp*/ )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("OBEX session disconnected as client");
    iConnectionId = -1;
    iConnected = false;

    iProcessing = false;
}

void OBEXClientWorker::PutResponse( obex_object_t *aObject, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aObject);

    if( aObexRsp == OBEX_RSP_SUCCESS )
    {
        LOG_DEBUG( "OBEX PUT succeeded" );
    }
    else
    {
        LOG_WARNING( "OBEX PUT failed, remote device sent: " << aObexRsp );
        emit connectionError();
    }

    iProcessing = false;
}

void OBEXClientWorker::GetResponse( obex_object_t *aObject, int aObexRsp )
{
    FUNCTION_CALL_TRACE;

    if( aObexRsp == OBEX_RSP_SUCCESS )
    {
        LOG_DEBUG( "OBEX GET succeeded" );

        OBEXDataHandler handler;
        OBEXDataHandler::GetRspData rspData;

        if( handler.parseGetRsp( iTransportHandle, aObject, rspData ) )
        {
            emit incomingData( rspData.iBody, iGetContentType );
        }
        else
        {
            LOG_WARNING( "Unexpected data in OBEX GET" );
            emit connectionError();
        }
    }
    else
    {
        LOG_WARNING( "OBEX GET failed, remote device sent: " << aObexRsp );

        if( aObexRsp == OBEX_RSP_NOT_FOUND )
        {
            LOG_WARNING( "Treating failure as session rejection" );
            emit sessionRejected();
        }
        else
        {
            LOG_WARNING( "Treating failure as generic connection error");
            emit connectionError();
        }
    }

    iProcessing = false;

}
