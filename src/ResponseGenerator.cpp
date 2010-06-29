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

#include "ResponseGenerator.h"

#include <QString>

#include "Package.h"
#include "SyncMLMessage.h"
#include "SyncMLStatus.h"
#include "SyncMLAlert.h"
#include "LogMacros.h"

using namespace DataSync;

const float MAXMSGSIZETHRESHOLD = 0.9f;

ResponseGenerator::ResponseGenerator()
 : iMaxMsgSize( 0 ),
   iMsgId( 0 ),
   iRemoteMsgId( 0 ),
   iIgnoreStatuses( false )
{
    FUNCTION_CALL_TRACE;
}

ResponseGenerator::~ResponseGenerator()
{
    FUNCTION_CALL_TRACE;

    qDeleteAll( iStatuses );
    iStatuses.clear();
    clearPackageQueue();
}

const HeaderParams& ResponseGenerator::getHeaderParams() const
{
    return iHeaderParams;
}

void ResponseGenerator::setHeaderParams( const HeaderParams& aHeaderParams )
{
    FUNCTION_CALL_TRACE
    iHeaderParams = aHeaderParams;
}

void ResponseGenerator::setRemoteMsgId( int aRemoteMsgId )
{
    FUNCTION_CALL_TRACE
    iRemoteMsgId = aRemoteMsgId;
}

int ResponseGenerator::getRemoteMsgId() const
{
    return iRemoteMsgId;
}

SyncMLMessage* ResponseGenerator::generateNextMessage( int aMaxSize, const ProtocolVersion& aVersion )
{
    FUNCTION_CALL_TRACE

    LOG_DEBUG("Session: Preparing to send next message");

    iHeaderParams.msgID = getNextMsgId();
    SyncMLMessage* message = new SyncMLMessage( iHeaderParams, aVersion );
    int messageSize = message->sizeAsXML();
    int messageSizeThreshold = int( (float)aMaxSize * MAXMSGSIZETHRESHOLD );
    int remainingBytes = messageSizeThreshold - messageSize;

    while( iStatuses.count() > 0 ) {
        StatusParams* params = iStatuses.first();

        params->cmdID = message->getNextCmdId();
        SyncMLStatus* statusObject = new SyncMLStatus( *params );

        delete params;
        iStatuses.removeFirst();

        message->addToBody( statusObject );

        remainingBytes -= statusObject->sizeAsXML();

        if( remainingBytes < 0 ) {
            break;
        }

    }

    while( iPackages.count() > 0 ) {

        Package* package = iPackages.first();

        if( package->write( *message, remainingBytes ) ) {
            delete package;
            iPackages.removeFirst();
        }
        else {
            break;
        }
    }

    return message;

}

void ResponseGenerator::addPackage( Package* aPackage )
{
    FUNCTION_CALL_TRACE

    iPackages.append( aPackage );
}

void ResponseGenerator::clearPackageQueue()
{
    FUNCTION_CALL_TRACE

    qDeleteAll( iPackages );
    iPackages.clear();
}

bool ResponseGenerator::packageQueueEmpty() const
{
    FUNCTION_CALL_TRACE;

    return iPackages.isEmpty();
}

const QList<Package*>& ResponseGenerator::getPackages() const
{
    return iPackages;
}

void ResponseGenerator::ignoreStatuses( bool aIgnore )
{
    FUNCTION_CALL_TRACE;

    iIgnoreStatuses = aIgnore;
}

void ResponseGenerator::addStatus( StatusParams* aParams )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        iStatuses.append( aParams );
    }
}

void ResponseGenerator::addStatus( const HeaderParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = aParams.msgID;
        statusParams->cmdRef = 0;
        statusParams->cmd = SYNCML_ELEMENT_SYNCHDR;
        statusParams->targetRef = aParams.targetDevice;
        statusParams->sourceRef = aParams.sourceDevice;
        statusParams->data = aStatusCode;

        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const HeaderParams& aParams, const ChalParams& aChalParams,
                                   ResponseStatusCode aStatusCode )
{

    FUNCTION_CALL_TRACE;

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = aParams.msgID;
    statusParams->cmdRef = 0;
    statusParams->cmd = SYNCML_ELEMENT_SYNCHDR;
    statusParams->targetRef = aParams.targetDevice;
    statusParams->sourceRef = aParams.sourceDevice;
    statusParams->data = aStatusCode;
    statusParams->chal = aChalParams;

    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const AlertParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;
        statusParams->cmd = SYNCML_ELEMENT_ALERT;
        statusParams->targetRef = aParams.targetDatabase;
        statusParams->sourceRef = aParams.sourceDatabase;
        statusParams->data = aStatusCode;
        statusParams->nextAnchor = aParams.nextAnchor;

        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const SyncParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;
        statusParams->cmd = SYNCML_ELEMENT_SYNC;
        statusParams->targetRef = aParams.targetDatabase;
        statusParams->sourceRef = aParams.sourceDatabase;
        statusParams->data = aStatusCode;

        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const MapParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;
        statusParams->cmd = SYNCML_ELEMENT_MAP;
        statusParams->targetRef = aParams.target;
        statusParams->sourceRef = aParams.source;
        statusParams->data = aStatusCode;
        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const SyncActionData& aParams , ResponseStatusCode aStatusCode )

{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;

        switch( aParams.action )
        {
            case SYNCML_ADD:
            {
                statusParams->cmd = SYNCML_ELEMENT_ADD;
                break;
            }
            case SYNCML_DELETE:
            {
                statusParams->cmd = SYNCML_ELEMENT_DELETE;
                break;
            }
            case SYNCML_REPLACE:
            {
                statusParams->cmd = SYNCML_ELEMENT_REPLACE;
                break;
            }
            case SYNCML_PUT:
            {
                statusParams->cmd = SYNCML_ELEMENT_PUT;
                break;
            }
            case SYNCML_GET:
            {
                statusParams->cmd = SYNCML_ELEMENT_GET;
                break;
            }
            case SYNCML_ATOMIC:
            {
                statusParams->cmd = SYNCML_ELEMENT_ATOMIC;
                break;
            }
            case SYNCML_COPY:
            {
                statusParams->cmd = SYNCML_ELEMENT_COPY;
                break;
            }
            case SYNCML_MOVE:
            {
                statusParams->cmd = SYNCML_ELEMENT_MOVE;
                break;
            }
            case SYNCML_SEQUENCE:
            {
                statusParams->cmd = SYNCML_ELEMENT_SEQUENCE;
                break;
            }
            default:
            {
                Q_ASSERT(0);
                break;
            }
        }

        if( aParams.items.count() > 1 ) {
            for (int i = 0; i < aParams.items.count(); ++i) {
                ItemParams item;
                item.source = aParams.items[i].source;
                item.target = aParams.items[i].target;
                statusParams->itemList.append(item);
            }
        }
        else if( aParams.items.count() > 0 ){
            statusParams->sourceRef = aParams.items[0].source;
            statusParams->targetRef = aParams.items[0].target;
        }

        statusParams->data = aStatusCode;
        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const SyncActionData& aParams, const ItemParams& aItem,
                                   ResponseStatusCode aStatusCode )

{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;

        if (aParams.action == SYNCML_ADD) {
            statusParams->cmd = SYNCML_ELEMENT_ADD;
        }
        else if (aParams.action == SYNCML_DELETE) {
            statusParams->cmd = SYNCML_ELEMENT_DELETE;
        }
        else if (aParams.action == SYNCML_REPLACE) {
            statusParams->cmd = SYNCML_ELEMENT_REPLACE;
        }
        else if( aParams.action == SYNCML_PUT ) {
            statusParams->cmd = SYNCML_ELEMENT_PUT;
        }
        else if( aParams.action == SYNCML_GET ) {
            statusParams->cmd = SYNCML_ELEMENT_GET;
        }
        else if( aParams.action == SYNCML_ATOMIC ) {
            statusParams->cmd = SYNCML_ELEMENT_ATOMIC;
        }
        else if( aParams.action == SYNCML_COPY ) {
            statusParams->cmd = SYNCML_ELEMENT_COPY;
        }
        else if( aParams.action == SYNCML_MOVE ) {
            statusParams->cmd = SYNCML_ELEMENT_MOVE;
        }
        else if( aParams.action == SYNCML_SEQUENCE ) {
            statusParams->cmd = SYNCML_ELEMENT_SEQUENCE;
        }

        statusParams->sourceRef = aItem.source;
        statusParams->targetRef = aItem.target;
        statusParams->data = aStatusCode;
        addStatus( statusParams );
    }

}

void ResponseGenerator::addStatus( const ResultsParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( !iIgnoreStatuses ) {
        StatusParams* statusParams = new StatusParams;
        statusParams->msgRef = iRemoteMsgId;
        statusParams->cmdRef = aParams.cmdID;
        statusParams->cmd = SYNCML_ELEMENT_RESULTS;
        statusParams->data = aStatusCode;
        addStatus( statusParams );
    }
}

const QList<StatusParams*>& ResponseGenerator::getStatuses() const
{
    return iStatuses;
}

int ResponseGenerator::getNextMsgId()
{
    return ++iMsgId;
}
