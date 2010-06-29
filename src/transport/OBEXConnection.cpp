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

#include "OBEXConnection.h"

#include <fcntl.h>
#include <termios.h>

#include "LogMacros.h"

using namespace DataSync;

OBEXConnection::OBEXConnection( int aMTUSize )
: iTransportHandle( NULL ), iMTUSize( aMTUSize )
{
    FUNCTION_CALL_TRACE;
}

OBEXConnection::~OBEXConnection()
{
    FUNCTION_CALL_TRACE;
}

obex_t* OBEXConnection::connect( obex_event_t aEventHandler )
{
    FUNCTION_CALL_TRACE;

    if( iTransportHandle ) {
        return iTransportHandle;
    }

    int fd = connectLink();

    if( fd == -1 ) {
        LOG_DEBUG( "Could not establish link layer for OBEX, aborting" );
        return NULL;
    }

    if( setupOpenOBEX( fd, aEventHandler ) ) {
        emit connected();
    }

    return iTransportHandle;

}


void OBEXConnection::disconnect()
{
    FUNCTION_CALL_TRACE;

    if( iTransportHandle ) {
        emit disconnected();
        closeOpenOBEX();
    }

    disconnectLink();

}

bool OBEXConnection::setupOpenOBEX( int aFd, obex_event_t aEventHandler )
{
    FUNCTION_CALL_TRACE;

    if( !iTransportHandle ) {

        obex_t* handle = OBEX_Init( OBEX_TRANS_FD, aEventHandler, 0);

        if( handle ) {

            OBEX_SetTransportMTU( handle, iMTUSize, iMTUSize );
            if( FdOBEX_TransportSetup(handle, aFd, aFd, iMTUSize ) >= 0 ) {
                LOG_DEBUG("OpenOBEX initialized");
                iTransportHandle = handle;
                return true;
            }
            else {
                LOG_DEBUG("OpenOBEX transport setup failed");
                return false;
            }

        }
        else {
            LOG_DEBUG("OpenOBEX initialization failed");
            return false;
        }
    }
    else {
        return false;
    }
}

void OBEXConnection::closeOpenOBEX()
{
    FUNCTION_CALL_TRACE;

    if( iTransportHandle ) {
        OBEX_TransportDisconnect( iTransportHandle );
        OBEX_Cleanup( iTransportHandle );
        iTransportHandle = NULL;

    }

}
