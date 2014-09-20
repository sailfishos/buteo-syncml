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

#include "CommandHandler.h"
#include "StorageHandler.h"
#include "SyncTarget.h"
#include "LogMacros.h"
#include "SyncAgentConfig.h"
#include "ResponseGenerator.h"
#include "ConflictResolver.h"
#include "DevInfPackage.h"
#include "AlertPackage.h"

#include "LocalMappingsPackage.h"


using namespace DataSync;

CommandHandler::CommandHandler( const Role& aRole )
 : iRole( aRole )
{
    FUNCTION_CALL_TRACE;
}

CommandHandler::~CommandHandler()
{
    FUNCTION_CALL_TRACE;
}

ResponseStatusCode CommandHandler::handleMap( const MapParams& aMapParams, SyncTarget& aTarget )
{
    FUNCTION_CALL_TRACE;

    UIDMapping mapping;
    for( int i = 0; i < aMapParams.mapItems.count(); ++i ) {
        mapping.iRemoteUID = aMapParams.mapItems[i].source;
        mapping.iLocalUID = aMapParams.mapItems[i].target;
        aTarget.addUIDMapping( mapping );
    }

    return SUCCESS;
}


void CommandHandler::handleSync( const SyncParams& aSyncParams,
                                 SyncTarget& aTarget,
                                 StorageHandler& aStorageHandler,
                                 ResponseGenerator& aResponseGenerator,
                                 ConflictResolver& aConflictResolver,
                                 bool aFastMapsSend )
{
    FUNCTION_CALL_TRACE;

    if( !aSyncParams.noResp ) {
        aResponseGenerator.addStatus( aSyncParams, SUCCESS );
    }

    QMap<ItemId, ResponseStatusCode> responses;
    composeBatches( aSyncParams, aTarget, aStorageHandler, aResponseGenerator, responses );

    QList<UIDMapping> newMappings;
    commitBatches( aStorageHandler, aConflictResolver, aTarget, aSyncParams, responses, newMappings );

    processResults( aSyncParams, responses, aResponseGenerator );

    manageNewMappings( aTarget, newMappings, aResponseGenerator, aFastMapsSend );

}

void CommandHandler::rejectSync( const SyncParams& aSyncParams, ResponseGenerator& aResponseGenerator,
                                 ResponseStatusCode aResponseCode )
{
    FUNCTION_CALL_TRACE;

    if( !aSyncParams.noResp )
    {
        aResponseGenerator.addStatus( aSyncParams, aResponseCode );
    }

    for( int i = 0; i < aSyncParams.commands.count(); ++i )
    {
        rejectCommand( aSyncParams.commands[i], aResponseGenerator, aResponseCode );
    }

}

void CommandHandler::rejectCommand( const CommandParams& aCommand, ResponseGenerator& aResponseGenerator,
                                    ResponseStatusCode aResponseCode )
{
    FUNCTION_CALL_TRACE;

    if( !aCommand.noResp )
    {
        aResponseGenerator.addStatus( aCommand, aResponseCode, true );
    }

    for( int i = 0; i < aCommand.subCommands.count(); ++i )
    {
        rejectCommand( aCommand.subCommands[i], aResponseGenerator, aResponseCode );
    }
}

void CommandHandler::handleStatus(StatusParams* aStatusParams )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode statusCode;
    StatusCodeType statusType;

    if (aStatusParams != NULL)
        statusCode = aStatusParams->data;
    else
        return;

    statusType = getStatusType(statusCode);

    switch (statusType) {
        case INFORMATIONAL:
        {
            // an informational message, no actions needed
            break;
        }

        case SUCCESSFUL:
        {
            // Success, no actions needed
            break;
        }

        case REDIRECTION:
        {
            handleRedirection( statusCode );
            break;
        }

        case ORIGINATOR_EXCEPTION:
        case RECIPIENT_EXCEPTION:
        {
            handleError(statusCode);
            break;
        }

        default:
        {
            // Unknown code
            LOG_DEBUG("Found unknown code: " << statusCode);
            break;
        }
    }

    if ( aStatusParams->cmd == SYNCML_ELEMENT_ADD ||
         aStatusParams->cmd == SYNCML_ELEMENT_REPLACE ||
         aStatusParams->cmd == SYNCML_ELEMENT_DELETE ) {
        emit itemAcknowledged( aStatusParams->msgRef, aStatusParams->cmdRef, aStatusParams->sourceRef );
    }

}

void CommandHandler::handleError( ResponseStatusCode aErrorCode )
{
    FUNCTION_CALL_TRACE;

    StatusCodeType statusType = getStatusType( aErrorCode );

    if (statusType == ORIGINATOR_EXCEPTION) {
        switch (aErrorCode) {
            case ALREADY_EXISTS:
            {
                // This merely an informational error that can happen e.g
                // during slow sync. No need to abort the session.
                break;
            }

            default:
            {
                emit abortSync(aErrorCode);
                break;
            }
        }

    }
    else if (statusType == RECIPIENT_EXCEPTION) {

        if (aErrorCode == REFRESH_REQUIRED) {
            /// @todo init resresh sync
        }
        else {
            emit abortSync(aErrorCode);
        }
    }
}

StatusCodeType CommandHandler::getStatusType(ResponseStatusCode aStatus)
{
    FUNCTION_CALL_TRACE;

    const int informationalLowBound        = 100;
    const int informationalHighBound       = 199;
    const int succesfullLowBound           = 200;
    const int successfullHighBound         = 299;
    const int redirectionLowBound          = 300;
    const int redirectionHighBound         = 399;
    const int originatorExceptionLowBound  = 400;
    const int originatorExceptionHighBound = 499;
    const int recipientExceptionLowBound   = 500;
    const int recipientExceptionHighBound  = 599;

    StatusCodeType statusType = UNKNOWN;

    if (aStatus >= informationalLowBound && aStatus < informationalHighBound) {
        statusType = INFORMATIONAL;
    }
    else if (aStatus >= succesfullLowBound && aStatus < successfullHighBound) {
        statusType = SUCCESSFUL;
    }
    else if (aStatus >= redirectionLowBound && aStatus < redirectionHighBound) {
        statusType = REDIRECTION;
    }
    else if (aStatus >= originatorExceptionLowBound && aStatus < originatorExceptionHighBound) {
        statusType = ORIGINATOR_EXCEPTION;
    }
    else if (aStatus >= recipientExceptionLowBound && aStatus < recipientExceptionHighBound) {
        statusType = RECIPIENT_EXCEPTION;
    }
    else {
        statusType = UNKNOWN;
    }

    return statusType;
}

bool CommandHandler::resolveConflicts()
{
    FUNCTION_CALL_TRACE;

    if( iRole == ROLE_CLIENT ) {

        // At the moment, do not try to resolve conflicts on the client side
        return false;

    }
    else {

        // Server should resolve conflicts
        return true;

    }

}

ResponseStatusCode CommandHandler::handleRedirection(ResponseStatusCode /*aRedirectionCode*/)
{
    FUNCTION_CALL_TRACE;

    return NOT_IMPLEMENTED;
}

void CommandHandler::composeBatches( const SyncParams& aSyncParams, SyncTarget& aTarget,
                                     StorageHandler& aStorageHandler, ResponseGenerator& aResponseGenerator,
                                     QMap<ItemId, ResponseStatusCode>& aResponses )
{
    FUNCTION_CALL_TRACE;

    // Batch updates
    for( int i = 0; i < aSyncParams.commands.count(); ++i ) {

        const CommandParams& data = aSyncParams.commands[i];

        QString defaultType = data.meta.type;
        QString defaultFormat = data.meta.format;

        // Process items associated with the command
        for( int a = 0; a < data.items.count(); ++a ) {

            const ItemParams& item = data.items[a];

            // Resolve id of the item
            ItemId id;
            id.iCmdId = data.cmdId;
            id.iItemIndex = a;

            // Resolve type of the item
            QString type;
            if( !item.meta.type.isEmpty() ) {
                type = item.meta.type;
            }
            else {
                type = defaultType;
            }

            // Resolve format of the item
            QString format;
            if( !item.meta.format.isEmpty() ) {
                format = item.meta.format;
            }
            else {
                format = defaultFormat;
            }

            // Get the version of the item
            QString version = item.meta.version;

            if( data.commandType == CommandParams::COMMAND_ADD ) {

                // Resolve item key
                QString remoteKey = item.source;

                // Resolve parent
                QString parentKey;

                if( iRole == ROLE_CLIENT ) {
                    // Client might receive SourceParent or TargetParent. SourceParent is used
                    // when server does not yet know the id of parent. TargetParent is used if
                    // server knows the id of the parent.

                    if( !item.sourceParent.isEmpty() ) {
                        parentKey = aTarget.mapToLocalUID( item.sourceParent );
                    }
                    else {
                        parentKey = item.targetParent;
                    }
                }
                else if( iRole == ROLE_SERVER && !item.sourceParent.isEmpty() ) {
                    // Server always receives SourceParent, which must be mapped to local id
                    parentKey = aTarget.mapToLocalUID( item.sourceParent );

                }
                // no else

                LOG_DEBUG( "Processing ADD with item URL:" << remoteKey );

                // Large object chunk
                if( item.moreData ) {

                    // First chunk of large object
                    if( !aStorageHandler.buildingLargeObject() ) {

                        // Size needs to be specified for first chunk
                        if( item.meta.size == 0 ) {
                            LOG_WARNING( "No size found for large object:" << id.iCmdId
                                          <<"/" << id.iItemIndex );
                        }
                        if( !aStorageHandler.startLargeObjectAdd( *aTarget.getPlugin(), remoteKey, 
                                                                       parentKey, type, format,
                                                                       version, item.meta.size ) ) {
                            aResponses.insert( id, COMMAND_FAILED );
                        }

                    }

                    if( aStorageHandler.buildingLargeObject() ) {

                        if( aStorageHandler.appendLargeObjectData( item.data ) ) {

                            aResponseGenerator.addPackage( new AlertPackage( NEXT_MESSAGE,
                                                                             aTarget.getSourceDatabase(),
                                                                             aTarget.getTargetDatabase() ) );
                            aResponses.insert( id, CHUNKED_ITEM_ACCEPTED );
                        }
                        else {
                            aResponses.insert( id, COMMAND_FAILED );
                        }

                    }


                }
                // Last chunk of large object
                else if( aStorageHandler.buildingLargeObject() ) {

                    if( !aStorageHandler.matchesLargeObject( remoteKey ) ) {
                        aResponseGenerator.addPackage( new AlertPackage( NO_END_OF_DATA,
                                                                         aTarget.getSourceDatabase(),
                                                                         aTarget.getTargetDatabase() ) );
                        aResponses.insert( id, COMMAND_NOT_ALLOWED );
                    }
                    else if( aStorageHandler.appendLargeObjectData( item.data ) ) {
                        if( !aStorageHandler.finishLargeObject( id ) ) {
                            aResponses.insert( id, COMMAND_FAILED );
                        }
                    }
                    else {
                        aResponses.insert( id, COMMAND_FAILED );
                    }

                }
                // Normal object
                else if( !aStorageHandler.addItem( id, *aTarget.getPlugin(), QString(), parentKey,
                                                   type, format, version, item.data ) ) {
                    aResponses.insert( id, COMMAND_FAILED );
                }
            }
            else if( data.commandType == CommandParams::COMMAND_REPLACE ) {

                // Resolve item key
                QString localKey;

                if( iRole == ROLE_CLIENT ) {
                    localKey = item.target;
                }
                else {
                    localKey = aTarget.mapToLocalUID( item.source );
                }

                // Resolve parent
                QString parentKey;

                if( iRole == ROLE_CLIENT ) {
                    // Client might receive SourceParent or TargetParent. SourceParent is used
                    // when server does not yet know the id of parent. TargetParent is used if
                    // server knows the id of the parent.

                    if( !item.sourceParent.isEmpty() ) {
                        parentKey = aTarget.mapToLocalUID( item.sourceParent );
                    }
                    else {
                        parentKey = item.targetParent;
                    }
                }
                else if( iRole == ROLE_SERVER ) {
                    // Server always receives SourceParent, which must be mapped to local id
                    parentKey = aTarget.mapToLocalUID( item.sourceParent );

                }
                // no else

                LOG_DEBUG( "Processing REPLACE with item URL:" << localKey );

                // Large object chunk
                if( item.moreData ) {

                    // First chunk of large object
                    if( !aStorageHandler.buildingLargeObject() ) {

                        // Size needs to be specified for first chunk
                        if( item.meta.size == 0 ) {
                            LOG_WARNING( "No size found for large object:" << id.iCmdId
                                           <<"/" << id.iItemIndex );
                        }
                        if( !aStorageHandler.startLargeObjectReplace( *aTarget.getPlugin(), localKey,
                                                                           parentKey, type, format,
                                                                           version, item.meta.size ) ) {
                            aResponses.insert( id, COMMAND_FAILED );
                        }

                    }

                    if( aStorageHandler.buildingLargeObject() ) {

                        if( aStorageHandler.appendLargeObjectData( item.data.toUtf8() ) ) {
                            aResponseGenerator.addPackage( new AlertPackage( NEXT_MESSAGE,
                                                                             aTarget.getSourceDatabase(),
                                                                             aTarget.getTargetDatabase() ) );
                            aResponses.insert( id, CHUNKED_ITEM_ACCEPTED );
                        }
                        else {
                            aResponses.insert( id, COMMAND_FAILED );
                        }

                    }


                }
                // Last chunk of large object
                else if( aStorageHandler.buildingLargeObject() ) {

                    if( !aStorageHandler.matchesLargeObject( localKey ) ) {
                        aResponseGenerator.addPackage( new AlertPackage( NO_END_OF_DATA,
                                                                         aTarget.getSourceDatabase(),
                                                                         aTarget.getTargetDatabase() ) );
                        aResponses.insert( id, COMMAND_NOT_ALLOWED );
                    }
                    else if( aStorageHandler.appendLargeObjectData( item.data ) ) {
                        if( !aStorageHandler.finishLargeObject( id ) ) {
                            aResponses.insert( id, COMMAND_FAILED );
                        }
                    }
                    else {
                        aResponses.insert( id, COMMAND_FAILED );
                    }

                }
                // Normal object
                else if( !aStorageHandler.replaceItem( id, *aTarget.getPlugin(), localKey,
                                                       parentKey, type, format, version, item.data ) ) {
                    aResponses.insert( id, COMMAND_FAILED );
                }


            }
            else if( data.commandType == CommandParams::COMMAND_DELETE ) {

                // Resolve item key
                QString localKey;

                if( iRole == ROLE_CLIENT ) {
                    localKey = item.target;
                }
                else {
                    localKey = aTarget.mapToLocalUID( item.source );
                }

                LOG_DEBUG( "Processing REPLACE with item URL:" << localKey );

                if( !aStorageHandler.deleteItem( id, localKey ) ) {
                    aResponses.insert( id, COMMAND_FAILED );
                }
            }
            else {
                // @todo: This is OK if we receive Copy/Move inside Sync, but what about
                //        Atomic/Sequence, should we also send status about the subcommands?
                aResponses.insert( id, NOT_IMPLEMENTED );

            }

        }

    }

}

void CommandHandler::commitBatches( StorageHandler& aStorageHandler, ConflictResolver& aConflictResolver,
                                    SyncTarget& aTarget, const SyncParams& aSyncParams,
                                    QMap<ItemId, ResponseStatusCode>& aResponses,
                                    QList<UIDMapping>& aNewMappings )
{
    FUNCTION_CALL_TRACE;

    // Commit batches

    QMap<ItemId, CommitResult> results;


    ConflictResolver* resolver = NULL;

    if( resolveConflicts() ) {
        resolver = &aConflictResolver;
    }
    else {
        resolver = NULL;
    }

    results.unite( aStorageHandler.commitAddedItems( *aTarget.getPlugin(), resolver ) );
    results.unite( aStorageHandler.commitReplacedItems( *aTarget.getPlugin(), resolver ) );
    results.unite( aStorageHandler.commitDeletedItems( *aTarget.getPlugin(), resolver ) );

    // Process commit results and convert them to result codes

    for( int i = 0; i < aSyncParams.commands.count(); ++i ) {

        const CommandParams& data = aSyncParams.commands[i];

        // Process items associated with the command
        for( int a = 0; a < data.items.count(); ++a ) {

            const ItemParams& item = data.items[a];
            ItemId id;

            id.iCmdId = data.cmdId;
            id.iItemIndex = a;

            if( !aResponses.contains( id ) ) {

                if( results.contains( id ) ) {

                    ResponseStatusCode statusCode = COMMAND_FAILED;

                    const CommitResult& result = results.value( id );

                    if( result.iStatus == COMMIT_ADDED || result.iStatus == COMMIT_INIT_ADD) {

                        if( result.iConflict == CONFLICT_LOCAL_WIN ) {

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                            else {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                        }
                        else if( result.iConflict == CONFLICT_REMOTE_WIN ) {

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                            else {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                        }
                        else {
                            statusCode = ITEM_ADDED;
                        }
                        UIDMapping map;
                        map.iRemoteUID = item.source;
                        map.iLocalUID = result.iItemKey;
                        aNewMappings.append( map );
                    }
                    else if( result.iStatus == COMMIT_REPLACED || result.iStatus == COMMIT_INIT_REPLACE ) {

                        if( result.iConflict == CONFLICT_LOCAL_WIN ) {

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                            else {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                        }
                        else if( result.iConflict == CONFLICT_REMOTE_WIN ) {

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                            else {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                        }
                        else {
                            statusCode = SUCCESS;
                        }

                    }
                    else if( result.iStatus == COMMIT_DELETED || result.iStatus == COMMIT_INIT_DELETE ) {

                        if( result.iConflict == CONFLICT_LOCAL_WIN ) {

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                            else {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                        }
                        else if( result.iConflict == CONFLICT_REMOTE_WIN ) {
                            aTarget.removeUIDMapping( result.iItemKey );

                            if( iRole == ROLE_CLIENT ) {
                                statusCode = RESOLVED_WITH_SERVER_DATA;
                            }
                            else {
                                statusCode = RESOLVED_CLIENT_WINNING;
                            }
                        }
                        else {
                            aTarget.removeUIDMapping( result.iItemKey );
                            statusCode = SUCCESS;
                        }

                    }
                    else if( result.iStatus == COMMIT_DUPLICATE ) {
                        statusCode = ALREADY_EXISTS;
                    }
                    else if( result.iStatus == COMMIT_NOT_DELETED ) {
                        statusCode = ITEM_NOT_DELETED;
                        aTarget.removeUIDMapping( result.iItemKey );
                    }
                    else if( result.iStatus == COMMIT_UNSUPPORTED_FORMAT ) {
                        statusCode = UNSUPPORTED_FORMAT;
                    }
                    else if( result.iStatus == COMMIT_ITEM_TOO_BIG ) {
                        statusCode = REQUEST_SIZE_TOO_BIG;
                    }
                    else if( result.iStatus == COMMIT_NOT_ENOUGH_SPACE ) {
                        statusCode = DEVICE_FULL;
                    }
                    else {
                        statusCode = COMMAND_FAILED;
                    }

                    aResponses.insert( id, statusCode );

                }
                else {
                    aResponses.insert( id, COMMAND_FAILED );
                }

            }
        }

    }
}

void CommandHandler::processResults( const SyncParams& aSyncParams, const QMap<ItemId, ResponseStatusCode>& aResponses,
                                     ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    // Process result codes and write corresponding status elements
    for( int i = 0; i < aSyncParams.commands.count(); ++i )
    {

        const CommandParams& command = aSyncParams.commands[i];

        if( command.noResp )
        {
            continue;
        }

        QMultiMap<ResponseStatusCode, int> responses;

        // Process items associated with the command
        for( int a = 0; a < command.items.count(); ++a )
        {

            ItemId id;

            id.iCmdId = command.cmdId;
            id.iItemIndex = a;

            responses.insert( aResponses.value( id ), a );
        }

        QList<ResponseStatusCode> codes = responses.uniqueKeys();

        for( int b = 0; b < codes.count(); ++b )
        {
            QList<int> itemIndexes = responses.values( codes[b] );

            // values() returns items sorted by most recently added, so we need to
            // reverse the list
            QList<int> reverseIndexes;
            QListIterator<int> iterator( itemIndexes );
            for( iterator.toBack(); iterator.hasPrevious(); )
            {
                reverseIndexes.append( iterator.previous() );
            }

            aResponseGenerator.addStatus( command, codes[b], reverseIndexes );
        }

    }

}

void CommandHandler::manageNewMappings( SyncTarget& aTarget, const QList<UIDMapping>& aNewMappings,
                                        ResponseGenerator& aResponseGenerator, bool aFastMapsSend )
{
    FUNCTION_CALL_TRACE;

    // Manage new mappings: Save them to persistent storage. Also if we are acting as a client
    // and we have been configured to fast-send mappings, compose LocalMappingsPackage

    for( int i = 0; i < aNewMappings.size(); ++i )
    {
        aTarget.addUIDMapping( aNewMappings[i] );
    }


    if ( (iRole == ROLE_CLIENT) && (aFastMapsSend) && (aNewMappings.size() > 0) )
    {
        LocalMappingsPackage* localMappingsPackage = new LocalMappingsPackage( aTarget.getSourceDatabase(),
                                                                               aTarget.getTargetDatabase(),
                                                                               aNewMappings );
        aResponseGenerator.addPackage( localMappingsPackage );
    }
}
