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

#include "ResponseGenerator.h"

#include <QString>

#include "Package.h"
#include "SyncMLMessage.h"
#include "SyncMLStatus.h"
#include "SyncMLAlert.h"
#include "datatypes.h"
#include "LogMacros.h"

using namespace DataSync;

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
    iHeaderParams = aHeaderParams;
}

void ResponseGenerator::setRemoteMsgId( int aRemoteMsgId )
{
    iRemoteMsgId = aRemoteMsgId;
}

int ResponseGenerator::getRemoteMsgId() const
{
    return iRemoteMsgId;
}

SyncMLMessage* ResponseGenerator::generateNextMessage( int aMaxSize, const ProtocolVersion& aVersion,
                                                       bool aWbXML )
{
    FUNCTION_CALL_TRACE

    iHeaderParams.msgID = getNextMsgId();
    SyncMLMessage* message = new SyncMLMessage( iHeaderParams, aVersion );
    int messageSize = message->sizeAsXML();

    // Make sure we don't go over the maximum message size. Threshold-based solution is employed
    // with fast XML size estimator, so we don't need to serialize XML to find out how big the
    // message is every time we need to check it. Message size threshold is maximum message size
    // minus overhead. Overhead whatever's not included in the estimation, like XML header, DOCTYPE
    // and buffer for errors in estimator. Overhead is MAXMSGOVERHEADRATIO times max message size
    // (now at 10%), but always at least MINMSGOVERHEADBYTES (now at 256) bytes.
    // If wbXML is used, compression rate is taken into account to ensure best performance.
    // Safe estimate of XML->wbXML compression rate of 66% is currently presumed

    int maxSize = aMaxSize;

    if( aWbXML )
    {
        maxSize /= WBXMLCOMPRESSIONRATE;
    }

    int overhead = qMax( static_cast<int>( MAXMSGOVERHEADRATIO * maxSize), MINMSGOVERHEADBYTES );
    int messageSizeThreshold = maxSize - overhead;

    int remainingBytes = messageSizeThreshold - messageSize;

    while( iStatuses.count() > 0 ) {
        StatusParams* params = iStatuses.first();

        params->cmdId = message->getNextCmdId();
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
    LOG_DEBUG( "MessageSize:"<<message->sizeAsXML());
    LOG_DEBUG( "Message generated with following parameters:" );
    LOG_DEBUG( "Maximum size reported by remote device:" << aMaxSize );
    LOG_DEBUG( "Maximum size after compression efficiency estimation:" << maxSize );
    LOG_DEBUG( "Estimated overhead:" << overhead );
    LOG_DEBUG( "Message size threshold value was:" << messageSizeThreshold );
    LOG_DEBUG( "Remaining bytes was:" << remainingBytes );

    return message;

}

void ResponseGenerator::addPackage( Package* aPackage )
{
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
    return iPackages.isEmpty();
}

const QList<Package*>& ResponseGenerator::getPackages() const
{
    return iPackages;
}

void ResponseGenerator::ignoreStatuses( bool aIgnore )
{
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

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = aParams.msgID;
    statusParams->cmdRef = 0;
    statusParams->cmd = SYNCML_ELEMENT_SYNCHDR;
    statusParams->targetRef = aParams.targetDevice;
    statusParams->sourceRef = aParams.sourceDevice;
    statusParams->data = aStatusCode;

    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const HeaderParams& aParams, const ChalParams& aChalParams,
                                   ResponseStatusCode aStatusCode )
{

    FUNCTION_CALL_TRACE;

    // Bypass iIgnoreStatuses flag: Status for SyncHdr should always be written

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

void ResponseGenerator::addStatus( const CommandParams& aParams, ResponseStatusCode aStatusCode,
                                   bool aWriteItemRefs )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->data = aStatusCode;

    if( aParams.commandType == CommandParams::COMMAND_ALERT )
    {

        statusParams->cmd = SYNCML_ELEMENT_ALERT;

        // @todo: this could be made better
        if( !aParams.items.isEmpty() )
        {
            statusParams->nextAnchor = aParams.items.first().meta.anchor.next;
        }

    }
    else if( aParams.commandType == CommandParams::COMMAND_ADD )
    {
        statusParams->cmd = SYNCML_ELEMENT_ADD;
    }
    else if( aParams.commandType == CommandParams::COMMAND_REPLACE )
    {
        statusParams->cmd = SYNCML_ELEMENT_REPLACE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_DELETE )
    {
        statusParams->cmd = SYNCML_ELEMENT_DELETE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_GET )
    {
        statusParams->cmd = SYNCML_ELEMENT_GET;
    }
    else if( aParams.commandType == CommandParams::COMMAND_COPY )
    {
        statusParams->cmd = SYNCML_ELEMENT_COPY;
    }
    else if( aParams.commandType == CommandParams::COMMAND_MOVE )
    {
        statusParams->cmd = SYNCML_ELEMENT_MOVE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_EXEC )
    {
        statusParams->cmd = SYNCML_ELEMENT_EXEC;
    }
    else if( aParams.commandType == CommandParams::COMMAND_ATOMIC )
    {
        statusParams->cmd = SYNCML_ELEMENT_ATOMIC;
    }
    else if( aParams.commandType == CommandParams::COMMAND_SEQUENCE )
    {
        statusParams->cmd = SYNCML_ELEMENT_SEQUENCE;
    }
    else
    {
        Q_ASSERT(0);
    }

    if( aWriteItemRefs )
    {

        if( aParams.items.count() == 1 )
        {
            statusParams->sourceRef = aParams.items.first().source;
            statusParams->targetRef = aParams.items.first().target;
        }
        else
        {
            for( int i = 0; i < aParams.items.count(); ++i )
            {
                ItemParams item;
                item.source = aParams.items[i].source;
                item.target = aParams.items[i].target;
                statusParams->items.append(item);
            }
        }
    }

    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const CommandParams& aParams, ResponseStatusCode aStatusCode,
                                   const QList<int>& aItemIndexes )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->data = aStatusCode;

    if( aParams.commandType == CommandParams::COMMAND_ALERT )
    {

        statusParams->cmd = SYNCML_ELEMENT_ALERT;

        // @todo: this could be made better
        if( !aParams.items.isEmpty() )
        {
            statusParams->nextAnchor = aParams.items.first().meta.anchor.next;
        }

    }
    else if( aParams.commandType == CommandParams::COMMAND_ADD )
    {
        statusParams->cmd = SYNCML_ELEMENT_ADD;
    }
    else if( aParams.commandType == CommandParams::COMMAND_REPLACE )
    {
        statusParams->cmd = SYNCML_ELEMENT_REPLACE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_DELETE )
    {
        statusParams->cmd = SYNCML_ELEMENT_DELETE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_GET )
    {
        statusParams->cmd = SYNCML_ELEMENT_GET;
    }
    else if( aParams.commandType == CommandParams::COMMAND_COPY )
    {
        statusParams->cmd = SYNCML_ELEMENT_COPY;
    }
    else if( aParams.commandType == CommandParams::COMMAND_MOVE )
    {
        statusParams->cmd = SYNCML_ELEMENT_MOVE;
    }
    else if( aParams.commandType == CommandParams::COMMAND_EXEC )
    {
        statusParams->cmd = SYNCML_ELEMENT_EXEC;
    }
    else if( aParams.commandType == CommandParams::COMMAND_ATOMIC )
    {
        statusParams->cmd = SYNCML_ELEMENT_ATOMIC;
    }
    else if( aParams.commandType == CommandParams::COMMAND_SEQUENCE )
    {
        statusParams->cmd = SYNCML_ELEMENT_SEQUENCE;
    }
    else
    {
        Q_ASSERT(0);
    }

    if( aItemIndexes.count() == 1 )
    {
        statusParams->sourceRef = aParams.items[aItemIndexes.first()].source;
        statusParams->targetRef = aParams.items[aItemIndexes.first()].target;
    }
    else
    {
        for( int i = 0; i < aItemIndexes.count(); ++i )
        {
            ItemParams item;
            item.source = aParams.items[aItemIndexes.at(i)].source;
            item.target = aParams.items[aItemIndexes.at(i)].target;
            statusParams->items.append(item);
        }
    }

    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const SyncParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->cmd = SYNCML_ELEMENT_SYNC;
    statusParams->targetRef = aParams.target;
    statusParams->sourceRef = aParams.source;
    statusParams->data = aStatusCode;

    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const MapParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->cmd = SYNCML_ELEMENT_MAP;
    statusParams->targetRef = aParams.target;
    statusParams->sourceRef = aParams.source;
    statusParams->data = aStatusCode;
    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const ResultsParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->cmd = SYNCML_ELEMENT_RESULTS;
    statusParams->data = aStatusCode;
    addStatus( statusParams );

}

void ResponseGenerator::addStatus( const PutParams& aParams, ResponseStatusCode aStatusCode )
{
    FUNCTION_CALL_TRACE;

    if( iIgnoreStatuses )
    {
        return;
    }

    StatusParams* statusParams = new StatusParams;
    statusParams->msgRef = iRemoteMsgId;
    statusParams->cmdRef = aParams.cmdId;
    statusParams->cmd = SYNCML_ELEMENT_PUT;
    statusParams->data = aStatusCode;
    statusParams->sourceRef = aParams.devInf.source;
    addStatus( statusParams );
}

const QList<StatusParams*>& ResponseGenerator::getStatuses() const
{
    return iStatuses;
}

int ResponseGenerator::getNextMsgId()
{
    return ++iMsgId;
}
