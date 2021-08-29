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

#include "OBEXDataHandler.h"

#include "SyncMLLogging.h"

#define SYNCMLTARGET "SYNCML-SYNC"

using namespace DataSync;

OBEXServerWorker::OBEXServerWorker( OBEXServerDataSource& aSource,
                                    int aFd, qint32 aMTU, int aTimeOut )
 : iSource( aSource ), iFd( aFd ), iMTU( aMTU ), iTimeOut( aTimeOut ),
   iConnectionId( 1 ), iProcessing( false ), iState( STATE_IDLE )
{
}

OBEXServerWorker::~OBEXServerWorker()
{

}

void OBEXServerWorker::waitForConnect()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( isConnected() )
    {
        qCDebug(lcSyncML) << "Already connected, ignoring connect attempt";
        return;
    }

    if( !setupOpenOBEX( iFd, iMTU, OBEXServerWorker::handleEvent ) )
    {
        qCCritical(lcSyncML) << "Could not set up OBEX link, aborting CONNECT";
        return;
    }

    OBEX_SetUserData( getHandle(), this );

    qCDebug(lcSyncML) << "Waiting for OBEX CONNECT";

    process( STATE_CONNECT );

}

void OBEXServerWorker::waitForDisconnect()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( isConnected() )
    {
        qCDebug(lcSyncML) << "Waiting for OBEX DISCONNECT";
        process( STATE_DISCONNECT );
    }
    else
    {
        qCDebug(lcSyncML) << "Not connected, ignoring disconnect attempt";
    }

    closeOpenOBEX();
}

void OBEXServerWorker::waitForPut()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( isConnected() )
    {
        qCDebug(lcSyncML) << "Waiting for OBEX PUT";
        process( STATE_PUT );
    }
    else
    {
        qCWarning(lcSyncML) << "Connection not established, cannot wait for PUT";
        emit connectionFailed();
    }

}

void OBEXServerWorker::waitForGet()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( isConnected() )
    {
        qCDebug(lcSyncML) << "Waiting for OBEX GET";
        process( STATE_GET );
    }
    else
    {
        qCWarning(lcSyncML) << "Connection not established, cannot wait for GET";
        emit connectionFailed();
    }

}

void OBEXServerWorker::process( State aNextState )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    int result = 0;

    iState = aNextState;
    iProcessing = true;

    while( iProcessing )
    {
        result = OBEX_HandleInput( getHandle(), iTimeOut );

        if( isLinkError() )
        {
            iState = STATE_IDLE;
            iProcessing = false;
            linkError();
            emit connectionError();
            break;
        }
        else if( result < 0 )
        {
            qCWarning(lcSyncML) << "OBEX operation failed";
            iState = STATE_IDLE;
            iProcessing = false;
            emit connectionError();
        }
        else if( result == 0 )
        {
            qCWarning(lcSyncML) << "OBEX timeout";
            iState = STATE_IDLE;
            iProcessing = false;
            emit connectionTimeout();
        }

    }

}

void OBEXServerWorker::handleEvent( obex_t *aHandle, obex_object_t *aObject, int aMode,
                                    int aEvent, int aObexCmd, int aObexRsp )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    OBEXServerWorker* worker= (OBEXServerWorker*)OBEX_GetUserData( aHandle );
    qCDebug(lcSyncML) << "OBEX Event: " << aEvent <<" Mode: " << aMode <<" Cmd: " << aObexCmd << " Resp: " << aObexRsp;

    switch( aEvent )
    {

        case OBEX_EV_REQHINT:
        {
            if( aObexCmd == OBEX_CMD_CONNECT || aObexCmd == OBEX_CMD_DISCONNECT ||
                aObexCmd == OBEX_CMD_PUT || aObexCmd == OBEX_CMD_GET )
            {
                OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_CONTINUE );
            }
            else
            {
                qCWarning(lcSyncML) << "Ignoring command related to unimplemented service";
                OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_IMPLEMENTED,
                                            OBEX_RSP_NOT_IMPLEMENTED );
            }
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
            worker->setLinkError( true );
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
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iState = STATE_IDLE;
    iProcessing = false;

    if( isConnected() )
    {
        setConnected( false );
        closeOpenOBEX();
        emit connectionError();
    }

    qCCritical(lcSyncML) << "Link error occurred";

    closeOpenOBEX();
    setConnected( false );

}

void OBEXServerWorker::requestReceived( obex_object_t *aObject, int aMode, int aObexCmd )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

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
            qCWarning(lcSyncML) << "Ignoring command related to unimplemented service";
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_IMPLEMENTED,
                                        OBEX_RSP_NOT_IMPLEMENTED );
            break;
        }
    }
}

void OBEXServerWorker::ConnectRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( isConnected() )
    {
        qCWarning(lcSyncML) << "Already connected, ignoring CONNECT";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::ConnectCmdData cmdData;

    if( !handler.parseConnectCmd( getHandle(), aObject, cmdData ) ) {
        qCWarning(lcSyncML) << "Could not parse CONNECT request, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( cmdData.iTarget != SYNCMLTARGET )
    {
        qCWarning(lcSyncML) << "CONNECT request not directed to SyncML, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }


    qCDebug(lcSyncML) << "Assigning connection id: " << iConnectionId;
    OBEXDataHandler::ConnectRspData rspData;
    rspData.iConnectionId = iConnectionId;
    rspData.iWho = cmdData.iTarget;

    if( !handler.createConnectRsp( getHandle(), aObject, rspData ) )
    {
        qCCritical(lcSyncML) << "Internal error when creating CONNECT response";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR, OBEX_RSP_INTERNAL_SERVER_ERROR );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    qCDebug(lcSyncML) << "OBEX session established as server";
    setConnected( true );

    iProcessing = false;
    iState = STATE_IDLE;

}

void OBEXServerWorker::DisconnectRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !isConnected() )
    {
        qCWarning(lcSyncML) << "Not connected, ignoring DISCONNECT";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::DisconnectCmdData data;

    if( !handler.parseDisconnectCmd( getHandle(), aObject, data ) )
    {
        qCWarning(lcSyncML) << "Could not parse DISCONNECT request, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( data.iConnectionId != iConnectionId )
    {
        qCWarning(lcSyncML) << "Received DISCONNECT request not matching the session, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    if( iState != STATE_DISCONNECT )
    {
        qCWarning(lcSyncML) << "Unexpected OBEX DISCONNECT received, presuming connection error";
        emit connectionError();
    }

    qCDebug(lcSyncML) << "OBEX session disconnected as server";
    setConnected( false );

    iProcessing = false;
    iState = STATE_IDLE;

}

void OBEXServerWorker::PutRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !isConnected() )
    {
        qCWarning(lcSyncML) << "Not connected, ignoring PUT";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::PutCmdData data;

    if( !handler.parsePutCmd( getHandle(), aObject, data ) ) {
        qCWarning(lcSyncML) << "Could not parse PUT request, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( data.iUnsupportedHeaders )
    {
        qCWarning(lcSyncML) << "PUT request included unsupported headers, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );
        return;
    }

    if( data.iConnectionId != iConnectionId )
    {
        qCWarning(lcSyncML) << "Received PUT request not matching the session, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );

    qCDebug(lcSyncML) << "Received PUT with body size of:" << data.iBody.size() <<", content type:" << data.iContentType;

    iProcessing = false;
    iState = STATE_IDLE;

    emit incomingData( data.iBody, data.iContentType );

}

void OBEXServerWorker::GetRequest( obex_object_t *aObject )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !isConnected() )
    {
        qCWarning(lcSyncML) << "Not connected, ignoring GET";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_SERVICE_UNAVAILABLE,
                                    OBEX_RSP_SERVICE_UNAVAILABLE );
        return;
    }

    OBEXDataHandler handler;
    OBEXDataHandler::GetCmdData cmdData;

    if( !handler.parseGetCmd( getHandle(), aObject, cmdData ) ) {
        qCWarning(lcSyncML) << "Could not parse GET request, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_BAD_REQUEST, OBEX_RSP_BAD_REQUEST );
        return;
    }

    if( cmdData.iConnectionId != iConnectionId )
    {
        qCWarning(lcSyncML) << "Received GET request not matching the session, ignoring";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND );
        return;
    }

    QByteArray data;

    if( iSource.getData( QString::fromLatin1( cmdData.iContentType ), data ) )
    {

        OBEXDataHandler::GetRspData rspData;
        rspData.iBody = data;
        rspData.iLength = data.length();

        if( handler.createGetRsp( getHandle(), aObject, rspData ) )
        {
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS );
            qCDebug(lcSyncML) << "Responded to GET request for content type" << cmdData.iContentType << "with response of" << data.length() << "bytes";
        }
        else
        {
            qCCritical(lcSyncML) << "Error when generating OBEX GET response";
            OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR,
                                        OBEX_RSP_INTERNAL_SERVER_ERROR );
        }

    }
    else
    {
        qCCritical(lcSyncML) << "Could not retrieve response data for GET!";
        OBEX_ObjectSetRsp( aObject, OBEX_RSP_INTERNAL_SERVER_ERROR, OBEX_RSP_INTERNAL_SERVER_ERROR );
    }

    iProcessing = false;
    iState = STATE_IDLE;

}
