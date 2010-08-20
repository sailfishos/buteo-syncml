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

#include "SyncMode.h"

#include "datatypes.h"

using namespace DataSync;

SyncMode::SyncMode( SyncDirection aDirection,
                    SyncInitiator aInitiator,
                    SyncType aType )
 : iDirection( aDirection ), iInitiator( aInitiator ), iType( aType ),
   iValid( true )
{
}


SyncMode::SyncMode( int aSyncMLCode )
:   iDirection(DIRECTION_TWO_WAY),
    iInitiator(INIT_CLIENT),
    iType(TYPE_FAST)
{
    if( aSyncMLCode < TWO_WAY_SYNC ||
        aSyncMLCode > REFRESH_FROM_SERVER_BY_SERVER ) {
        iValid = false;
    }
    else {
        iValid = true;
        switch( aSyncMLCode )
        {
            case TWO_WAY_SYNC:
            {
                iDirection = DIRECTION_TWO_WAY;
                iInitiator = INIT_CLIENT;
                iType = TYPE_FAST;
                break;
            }
            case SLOW_SYNC:
            {
                iDirection = DIRECTION_TWO_WAY;
                iInitiator = INIT_CLIENT;
                iType = TYPE_SLOW;
                break;
            }
            case ONE_WAY_FROM_CLIENT_SYNC:
            {
                iDirection = DIRECTION_FROM_CLIENT;
                iInitiator = INIT_CLIENT;
                iType = TYPE_FAST;
                break;
            }
            case REFRESH_FROM_CLIENT_SYNC:
            {
                iDirection = DIRECTION_FROM_CLIENT;
                iInitiator = INIT_CLIENT;
                iType = TYPE_REFRESH;
                break;
            }
            case ONE_WAY_FROM_SERVER_SYNC:
            {
                iDirection = DIRECTION_FROM_SERVER;
                iInitiator = INIT_CLIENT;
                iType = TYPE_FAST;
                break;
            }
            case REFRESH_FROM_SERVER_SYNC:
            {
                iDirection = DIRECTION_FROM_SERVER;
                iInitiator = INIT_CLIENT;
                iType = TYPE_REFRESH;
                break;
            }
            case TWO_WAY_BY_SERVER:
            {
                iDirection = DIRECTION_TWO_WAY;
                iInitiator = INIT_SERVER;
                iType = TYPE_FAST;
                break;
            }
            case ONE_WAY_FROM_CLIENT_BY_SERVER:
            {
                iDirection = DIRECTION_FROM_CLIENT;
                iInitiator = INIT_SERVER;
                iType = TYPE_FAST;
                break;
            }
            case REFRESH_FROM_CLIENT_BY_SERVER:
            {
                iDirection = DIRECTION_FROM_CLIENT;
                iInitiator = INIT_SERVER;
                iType = TYPE_REFRESH;
                break;
            }
            case ONE_WAY_FROM_SERVER_BY_SERVER:
            {
                iDirection = DIRECTION_FROM_SERVER;
                iInitiator = INIT_SERVER;
                iType = TYPE_FAST;
                break;
            }
            case REFRESH_FROM_SERVER_BY_SERVER:
            {
                iDirection = DIRECTION_FROM_SERVER;
                iInitiator = INIT_SERVER;
                iType = TYPE_REFRESH;
                break;
            }
            default:
            {
                iValid = false;
                break;
            }
        }
    }
}


SyncMode::~SyncMode()
{
}


bool SyncMode::isValid() const
{
    return iValid;
}

SyncDirection SyncMode::syncDirection() const
{
    return iDirection;
}

SyncInitiator SyncMode::syncInitiator() const
{
    return iInitiator;
}

SyncType SyncMode::syncType() const
{
    return iType;
}

void SyncMode::toSlowSync()
{
    iType = TYPE_SLOW;
}

void SyncMode::setRefresh()
{
    iType = TYPE_REFRESH;
}

void SyncMode::toClientInitiated()
{
    iInitiator = INIT_CLIENT;
}


int SyncMode::toSyncMLCode() const
{

    int code = -1;

    if( iDirection == DIRECTION_TWO_WAY ) {

        if( iInitiator == INIT_CLIENT ) {

            if( iType == TYPE_FAST ) {
                code = TWO_WAY_SYNC;
            }
            else if( iType == TYPE_SLOW ) {
                code = SLOW_SYNC;
            }

        }
        else if( iInitiator == INIT_SERVER ) {
            code = TWO_WAY_BY_SERVER;
        }

    }
    else if( iDirection == DIRECTION_FROM_CLIENT ) {

        if( iInitiator == INIT_CLIENT ) {

            if( iType == TYPE_REFRESH ) {
                code = REFRESH_FROM_CLIENT_SYNC;
            }
            else {
                code = ONE_WAY_FROM_CLIENT_SYNC;
            }
        }
        else if( iInitiator == INIT_SERVER ) {

            if( iType == TYPE_REFRESH ) {
                code = REFRESH_FROM_CLIENT_BY_SERVER;
            }
            else {
                code = ONE_WAY_FROM_CLIENT_BY_SERVER;
            }

        }

    }
    else if( iDirection == DIRECTION_FROM_SERVER ) {

        if( iInitiator == INIT_CLIENT ) {

            if( iType == TYPE_REFRESH ) {
                code = REFRESH_FROM_SERVER_SYNC;
            }
            else {
                code = ONE_WAY_FROM_SERVER_SYNC;
            }

        }
        else if( iInitiator == INIT_SERVER ) {

            if( iType == TYPE_REFRESH ) {
                code = REFRESH_FROM_SERVER_BY_SERVER;
            }
            else {
                code = ONE_WAY_FROM_SERVER_BY_SERVER;
            }

        }

    }

    return code;
}
