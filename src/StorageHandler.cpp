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

#include "StorageHandler.h"

#include <QMutableMapIterator>

#include "StoragePlugin.h"
#include "SyncItem.h"
#include "ConflictResolver.h"

#include "LogMacros.h"

using namespace DataSync;

StorageHandler::StorageHandler() :
    iLargeObject( NULL ),
    iLargeObjectSize(0)
{
    FUNCTION_CALL_TRACE;
}

StorageHandler::~StorageHandler()
{
    FUNCTION_CALL_TRACE;

    qDeleteAll(iAddList);
    qDeleteAll(iReplaceList);
    
    delete iLargeObject;
    iLargeObject = NULL;
}

bool StorageHandler::addItem( const ItemId& aItemId,
                              StoragePlugin& aPlugin,
                              const SyncItemKey& aLocalKey,
                              const SyncItemKey& aParentKey,
                              const QString& aType,
                              const QString& aFormat,
                              const QString& aVersion,
                              const QString& aData )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Processing item for add:" << aItemId.iCmdId <<"/" << aItemId.iItemIndex );

    if( iLargeObject ) {
        LOG_CRITICAL( "Already processing large object, aborting" );
        return false;
    }

    SyncItem* newItem = aPlugin.newItem();

    if( !newItem ) {
        LOG_CRITICAL( "Could not create new item" );
        return false;
    }

    //Setting empty string as we dont have any local key for it.
    newItem->setKey( aLocalKey );
    newItem->setParentKey( aParentKey );
    newItem->setType( aType );
    newItem->setFormat( aFormat );
    newItem->setVersion( aVersion );

    if( !newItem->write( 0, aData.toUtf8() ) ) {
        delete newItem;
        LOG_CRITICAL( "Could not write to item" );
        return false;
    }

    iAddList.insert( aItemId, newItem );
    LOG_DEBUG( "Item queued for addition" );

    return true;
}

bool StorageHandler::replaceItem( const ItemId& aItemId,
                                  StoragePlugin& aPlugin,
                                  const QString& aLocalKey,
                                  const SyncItemKey& aParentKey,
                                  const QString& aType,
                                  const QString& aFormat,
                                  const QString& aVersion,
                                  const QString& aData )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Processing item for replace:" << aItemId.iCmdId <<"/" << aItemId.iItemIndex );

    if( iLargeObject ) {
        LOG_CRITICAL( "Already processing large object, aborting" );
        return false;
    }

    SyncItem* item = NULL;

    // If local key is empty, this Replace should be handled as Add (this is allowed by the protocol)

    if( !aLocalKey.isEmpty() ) {
        item = aPlugin.getSyncItem( aLocalKey );
    }

    if( !item ) {
        LOG_DEBUG( "Could not find item, processing as Add" );
        return addItem( aItemId, aPlugin, aLocalKey, aParentKey, aType, aFormat, aVersion, aData );
    }

    item->setParentKey( aParentKey );
    item->setType( aType );
    item->setFormat( aFormat );
    item->setVersion( aVersion );

    if( !item->write( 0, aData.toUtf8() ) ) {
        delete item;
        LOG_CRITICAL( "Could not write to item" );
        return false;
    }

    iReplaceList.insert( aItemId, item );
    LOG_DEBUG( "Item queued for replace" );

    return true;
}

bool StorageHandler::deleteItem( const ItemId& aItemId,
                                 const SyncItemKey& aLocalKey )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Processing item for delete:" << aItemId.iCmdId <<"/" << aItemId.iItemIndex );

    if( iLargeObject ) {
        LOG_CRITICAL( "Already processing large object, aborting" );
        return false;
    }

    iDeleteList.insert( aItemId, aLocalKey );
    LOG_DEBUG( "Item queued for delete" );

    return true;
}

bool StorageHandler::startLargeObjectAdd( StoragePlugin& aPlugin,
                                          const QString& aRemoteKey,
                                          const SyncItemKey& aParentKey,
                                          const QString& aType,
                                          const QString& aFormat,
                                          const QString& aVersion,
                                          qint64 aSize )
{
    FUNCTION_CALL_TRACE;

    if( iLargeObject ) {
        LOG_CRITICAL( "Already processing large object, aborting" );
        return false;
    }

    SyncItem* newItem = aPlugin.newItem();

    if( !newItem ) {
        LOG_CRITICAL( "Could not create new item for large object" );
        return false;
    }

    //Setting empty string as we dont have any local key for it.
    newItem->setKey(QString());
    newItem->setParentKey( aParentKey );
    newItem->setType( aType );
    newItem->setFormat( aFormat );
    newItem->setVersion( aVersion );

    iLargeObject = newItem;
    iLargeObjectSize = aSize;
    iLargeObjectKey = aRemoteKey;

    LOG_DEBUG( "Large object created for addition" );

    return true;
}

bool StorageHandler::startLargeObjectReplace( StoragePlugin& aPlugin,
                                              const QString& aLocalKey,
                                              const SyncItemKey& aParentKey,
                                              const QString& aType,
                                              const QString& aFormat,
                                              const QString& aVersion,
                                              qint64 aSize )
{
    FUNCTION_CALL_TRACE;

    if( iLargeObject ) {
        LOG_CRITICAL( "Already processing large object, aborting" );
        return false;
    }

    SyncItem* item = NULL;

    // If local key is empty, this Replace should be handled as Add (this is allowed by the protocol)

    if( !aLocalKey.isEmpty() ) {
        item = aPlugin.getSyncItem( aLocalKey );
    }

    if( !item ) {
        LOG_CRITICAL( "Could not find item, processing as Add" );
        return startLargeObjectAdd( aPlugin, aLocalKey, aParentKey, aType, aFormat, aVersion, aSize );
    }

    item->setParentKey( aParentKey );
    item->setType( aType );
    item->setFormat( aFormat );
    item->setVersion( aVersion );

    iLargeObject = item;
    iLargeObjectSize = aSize;
    iLargeObjectKey = aLocalKey;
    if( !iLargeObject->resize(0) )
    {
        LOG_DEBUG( "Large object created for replace couldn't be resized" );
    }

    LOG_DEBUG( "Large object created for replace" );

    return true;
}

bool StorageHandler::buildingLargeObject() const
{
    FUNCTION_CALL_TRACE;

    if( iLargeObject ) {
        return true;
    }
    else {
        return false;
    }
}

bool StorageHandler::matchesLargeObject( const QString& aKey )
{
    FUNCTION_CALL_TRACE;

    if( aKey == iLargeObjectKey ) {
        return true;
    }
    else {
        delete iLargeObject;
        iLargeObject = NULL;
        iLargeObjectSize = 0;
        iLargeObjectKey.clear();
        return false;
    }

}

bool StorageHandler::appendLargeObjectData( const QString& aData )
{
    FUNCTION_CALL_TRACE;

    if( !iLargeObject ) {
        LOG_CRITICAL( "Not building large object, aborting" );
        return false;
    }

    if( iLargeObject->write( iLargeObject->getSize(), aData.toUtf8() ) ) {
        return true;
    }
    else {
        delete iLargeObject;
        iLargeObject = NULL;
        iLargeObjectSize = 0;
        iLargeObjectKey.clear();
        LOG_CRITICAL( "Could not write to large object" );
        return false;
    }

}

bool StorageHandler::finishLargeObject( const ItemId& aItemId )
{
    FUNCTION_CALL_TRACE;

    if( !iLargeObject ) {
        LOG_CRITICAL( "Not building large object, aborting" );
        return false;
    }

    if(iLargeObject->getKey()->isEmpty()) {
        LOG_DEBUG( "Queuing large object for addition" );
	iLargeObject->setKey(iLargeObjectKey);
        iAddList.insert( aItemId, iLargeObject );
    }
    else {
        LOG_DEBUG( "Queuing large object for replace" );
	iLargeObject->setKey(iLargeObjectKey);
        iReplaceList.insert( aItemId, iLargeObject );
    }

    iLargeObject = NULL;
    iLargeObjectSize = 0;
    iLargeObjectKey.clear();

    return true;

}

QMap<ItemId, CommitResult> StorageHandler::resolveConflicts( ConflictResolver* aConflictResolver,
                                                             QMap<ItemId, SyncItemKey> &aMap,
                                                             CommitStatus aStatus )
{
    FUNCTION_CALL_TRACE;
    QMutableMapIterator<ItemId, SyncItemKey> i(aMap);

    QMap<ItemId, CommitResult> results;
    ItemId iId;
    while( i.hasNext() ) {

        i.next();

        CommitResult result;

        result.iItemKey = i.value();

        result.iStatus = aStatus;

        iId.iCmdId = i.key().iCmdId;
        iId.iItemIndex = i.key().iItemIndex;

        LOG_DEBUG( "Checking item" << iId.iCmdId <<"/" << iId.iItemIndex << "for conflict" );

        if( aConflictResolver && aConflictResolver->isConflict( result.iItemKey, false ) ) {

            LOG_DEBUG( "Conflict detected" );

            if( aConflictResolver->localSideWins() ) {
                LOG_DEBUG( "Conflict resolved, local side wins" );
                result.iConflict = CONFLICT_LOCAL_WIN;
                aConflictResolver->revertLocalChange ( result.iItemKey, CR_MODIFY_TO_ADD );
                i.remove();
            }
            else {
                LOG_DEBUG( "Conflict resolved, remote side wins" );
                result.iConflict = CONFLICT_REMOTE_WIN;
                aConflictResolver->revertLocalChange ( result.iItemKey, CR_REMOVE_LOCAL );
            }
        }
        else {
            LOG_DEBUG( "No conflict detected" );
            result.iConflict = CONFLICT_NO_CONFLICT;
        }

        results.insert( iId, result );

    }
    return results;
}

QMap<ItemId, CommitResult> StorageHandler::resolveConflicts( ConflictResolver* aConflictResolver,
                                                             QMap<ItemId, SyncItem*> &aMap,
                                                             CommitStatus aStatus )
{
    FUNCTION_CALL_TRACE;
    QMap<ItemId, CommitResult> results;
    ItemId iId;

    QMutableMapIterator<ItemId, SyncItem*> i(aMap);
    while( i.hasNext() ) {

        i.next();

        CommitResult result;

        result.iItemKey = *i.value()->getKey();

        result.iStatus = aStatus;

        iId.iCmdId = i.key().iCmdId;
        iId.iItemIndex = i.key().iItemIndex;

        LOG_DEBUG( "Checking item" << iId.iCmdId <<"/" << iId.iItemIndex << "for conflict" );

        if( aConflictResolver && aConflictResolver->isConflict( result.iItemKey, false ) ) {

            LOG_DEBUG( "Conflict detected" );

            if( aConflictResolver->localSideWins() ) {
                LOG_DEBUG( "Conflict resolved, local side wins" );
                result.iConflict = CONFLICT_LOCAL_WIN;
                delete i.value();
                i.remove();
            }
            else {
                LOG_DEBUG( "Conflict resolved, remote side wins" );
                result.iConflict = CONFLICT_REMOTE_WIN;
                aConflictResolver->revertLocalChange ( result.iItemKey, CR_REMOVE_LOCAL );
            }
        }
        else {
            LOG_DEBUG( "No conflict detected" );
            result.iConflict = CONFLICT_NO_CONFLICT;
        }

        results.insert( iId, result );

    }
    return results;
}

QMap<ItemId, CommitResult> StorageHandler::commitAddedItems( StoragePlugin& aPlugin, 
		                               ConflictResolver* aConflictResolver )
{
    FUNCTION_CALL_TRACE;

    QMap<ItemId, CommitResult> results = resolveConflicts (aConflictResolver, iAddList, COMMIT_INIT_ADD);    
    QList<ItemId> addIds = iAddList.keys();
    QList<SyncItem*> addItems = iAddList.values();

    LOG_DEBUG( "Committing" << addItems.count() << "added items" );

    QList<StoragePlugin::StoragePluginStatus> addStatus = aPlugin.addItems( addItems );

    for( int i = 0; i < addStatus.count(); ++i ) {

        CommitResult& result = results[addIds[i]];
        result.iItemKey = *addItems[i]->getKey();
        
        LOG_DEBUG( "Item" << addIds[i].iCmdId << "/" << addIds[i].iItemIndex << "committed" );

        switch( addStatus[i] )
        {

            case StoragePlugin::STATUS_OK:
            {
                LOG_DEBUG( "Commit result: COMMIT_ADDED" );
                result.iStatus = COMMIT_ADDED;
                
		emit itemProcessed( MOD_ITEM_ADDED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , addItems[i]->getType(), addItems.count() );

                break;
            }
            case StoragePlugin::STATUS_DUPLICATE:
            {
                LOG_DEBUG( "Commit result: COMMIT_DUPLICATE" );
                result.iStatus = COMMIT_DUPLICATE;

                emit itemProcessed( MOD_ITEM_ADDED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , addItems[i]->getType(), addItems.count() );

                break;
            }
            default:
            {
                result.iStatus = generalStatus( addStatus[i] );

                emit itemProcessed( MOD_ITEM_ERROR, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , addItems[i]->getType(), addItems.count() );

                break;
            }

        }

        results.insert( addIds[i], result );

    }

    qDeleteAll( addItems );
    iAddList.clear();

    return results;
}

QMap<ItemId, CommitResult> StorageHandler::commitReplacedItems( StoragePlugin& aPlugin,
                                                                ConflictResolver* aConflictResolver )
{
    FUNCTION_CALL_TRACE;

    QMap<ItemId, CommitResult> results = resolveConflicts (aConflictResolver, iReplaceList, COMMIT_INIT_REPLACE);

    QList<ItemId> replaceIds = iReplaceList.keys();
    QList<SyncItem*> replaceItems = iReplaceList.values();

    LOG_DEBUG( "Committing" << replaceItems.count() << "replaced items" );

    QList<StoragePlugin::StoragePluginStatus> replaceStatus = aPlugin.replaceItems( replaceItems );

    for( int i = 0; i < replaceStatus.count(); ++i ) {

        CommitResult& result = results[replaceIds[i]];
        LOG_DEBUG( "Item" << replaceIds[i].iCmdId << "/" << replaceIds[i].iItemIndex << "committed" );

        switch( replaceStatus[i] )
        {

            case StoragePlugin::STATUS_OK:
            {
                LOG_DEBUG( "Commit result: COMMIT_REPLACED" );
                result.iStatus = COMMIT_REPLACED;

                emit itemProcessed( MOD_ITEM_MODIFIED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , replaceItems[i]->getType(), replaceItems.count() );

                break;
            }
            case StoragePlugin::STATUS_DUPLICATE:
            {
                LOG_DEBUG( "Commit result: COMMIT_DUPLICATE" );
                result.iStatus = COMMIT_DUPLICATE;

                emit itemProcessed( MOD_ITEM_MODIFIED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , replaceItems[i]->getType(), replaceItems.count() );

                break;
            }
            default:
            {
                result.iStatus = generalStatus( replaceStatus[i] );

                emit itemProcessed( MOD_ITEM_ERROR, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() , replaceItems[i]->getType(), replaceItems.count() );

                break;
            }

        }

    }

    qDeleteAll( iReplaceList );
    iReplaceList.clear();

    return results;

}

QMap<ItemId, CommitResult> StorageHandler::commitDeletedItems( StoragePlugin& aPlugin,
                                                               ConflictResolver* aConflictResolver )
{
    FUNCTION_CALL_TRACE;

    QMap<ItemId, CommitResult> results = resolveConflicts (aConflictResolver, iDeleteList, COMMIT_INIT_DELETE);
    QList<ItemId> deleteIds = iDeleteList.keys();
    QList<SyncItemKey> deleteItems = iDeleteList.values();

    LOG_DEBUG( "Committing" << deleteItems.count() << "deleted items" );

    QList<StoragePlugin::StoragePluginStatus> deleteStatus = aPlugin.deleteItems( deleteItems );

    for( int i = 0; i < deleteStatus.count(); ++i ) {

        CommitResult& result = results[deleteIds[i]];

        LOG_DEBUG( "Item" << deleteIds[i].iCmdId << "/" << deleteIds[i].iItemIndex << "committed" );

        switch( deleteStatus[i] )
        {

            case StoragePlugin::STATUS_OK:
            {
                LOG_DEBUG( "Commit result: COMMIT_DELETED" );
                result.iStatus = COMMIT_DELETED;

                emit itemProcessed( MOD_ITEM_DELETED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() ,aPlugin.getFormatInfo().getPreferredRx().iType, deleteItems.count() );

                break;
            }
            case StoragePlugin::STATUS_NOT_FOUND:
            {
                LOG_DEBUG( "Commit result: COMMIT_NOT_DELETED" );
                result.iStatus = COMMIT_NOT_DELETED;

                emit itemProcessed( MOD_ITEM_DELETED, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() ,aPlugin.getFormatInfo().getPreferredRx().iType, deleteItems.count() );

                break;
            }
            default:
            {
                result.iStatus = generalStatus( deleteStatus[i] );

                emit itemProcessed( MOD_ITEM_ERROR, MOD_LOCAL_DATABASE,
                                    aPlugin.getSourceURI() ,aPlugin.getFormatInfo().getPreferredRx().iType, deleteItems.count() );

                break;
            }

        }

    }

    iDeleteList.clear();

    return results;

}

CommitStatus StorageHandler::generalStatus( StoragePlugin::StoragePluginStatus aStatus ) const
{
    FUNCTION_CALL_TRACE;

    CommitStatus status;

    switch( aStatus )
    {

        case StoragePlugin::STATUS_OBJECT_TOO_BIG:
        {
            LOG_DEBUG( "Commit result: COMMIT_ITEM_TOO_BIG" );
            status = COMMIT_ITEM_TOO_BIG;
            break;
        }
        case StoragePlugin::STATUS_STORAGE_FULL:
        {
            LOG_DEBUG( "Commit result: COMMIT_NOT_ENOUGH_SPACE" );
            status = COMMIT_NOT_ENOUGH_SPACE;
            break;
        }
        case StoragePlugin::STATUS_INVALID_FORMAT:
        {
            LOG_DEBUG( "Commit result: COMMIT_UNSUPPORTED_FORMAT" );
            status = COMMIT_UNSUPPORTED_FORMAT;
            break;
        }
        default:
        {
            LOG_DEBUG( "Commit result: COMMIT_GENERAL_ERROR" );
            status = COMMIT_GENERAL_ERROR;
            break;
        }

    }

    return status;
}
