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

#include "LocalChangesPackage.h"
#include "SyncTarget.h"
#include "StoragePlugin.h"
#include "SyncItem.h"
#include "SyncMLSync.h"
#include "SyncMLMessage.h"
#include "SyncMLAdd.h"
#include "SyncMLReplace.h"
#include "SyncMLDelete.h"
#include "SyncMLItem.h"
#include "SyncAgentConsts.h"
#include "LogMacros.h"

using namespace DataSync;



LocalChangesPackage::LocalChangesPackage( const SyncTarget& aSyncTarget,
                                          const LocalChanges& aLocalChanges,
                                          int aLargeObjectThreshold,
                                          const Role& aRole,
                                          int aMaxChangesPerMessage) :
    iLargeObjectThreshold( aLargeObjectThreshold ),
    iSyncTarget( aSyncTarget ),
    iLocalChanges( aLocalChanges ),
    iRole( aRole ),
    iMaxChangesPerMessage(aMaxChangesPerMessage)
{
    FUNCTION_CALL_TRACE;

    iNumberOfChanges = iLocalChanges.added.count() +
                       iLocalChanges.modified.count() +
                       iLocalChanges.removed.count();

}

LocalChangesPackage::~LocalChangesPackage()
{
    FUNCTION_CALL_TRACE;

}

bool LocalChangesPackage::write( SyncMLMessage& aMessage, int& aSizeThreshold )
{
    FUNCTION_CALL_TRACE
    bool allWritten = false;

    int remainingBytes = aSizeThreshold;

    SyncMLSync* sync = new SyncMLSync( aMessage.getNextCmdId(),
                                       iSyncTarget.getTargetDatabase(),
                                       iSyncTarget.getSourceDatabase() );


    sync->addNumberOfChanges( iNumberOfChanges );
    remainingBytes -= sync->sizeAsXML();

    int itemsThatCanBeSent = iMaxChangesPerMessage;

    if( iNumberOfChanges > 0 ) {

        if( processAddedItems(aMessage, *sync, remainingBytes ,itemsThatCanBeSent) &&
            processModifiedItems(aMessage, *sync, remainingBytes ,itemsThatCanBeSent) &&
            processRemovedItems(aMessage, *sync, remainingBytes , itemsThatCanBeSent) ) {
            allWritten = true;
        }

    }
    else {
        allWritten = true;
    }

    aMessage.addToBody( sync );
    aSizeThreshold = remainingBytes;

    return allWritten;

}

bool LocalChangesPackage::processAddedItems( SyncMLMessage& aMessage,
                                             SyncMLSync& aSyncElement,
                                             int& aSizeThreshold ,
                                             int& aItemsThatCanBeSent)
{
    FUNCTION_CALL_TRACE;

    int remainingBytes = aSizeThreshold;

    // Get all items corresponding to the keys from the plug-in's
    QList<SyncItem*> curritems;

    int count = iLocalChanges.added.count(); 
    int pos = 0;

    if( 0 < count && 0 == addeditems.count() )
    {
        while( count )
        {
            curritems = iSyncTarget.getPlugin()->getSyncItems( iLocalChanges.added.mid( pos, iMaxChangesPerMessage ) );
            if( !curritems.count() )
            {
                break;
            }
            count -= curritems.count();
            addeditems += curritems;
            pos = addeditems.count();
            LOG_DEBUG("Fetched " << curritems.count() << " items");
        }

        if( addeditems.count() != iLocalChanges.added.count() )
        {
            LOG_WARNING("We didn't get all the items we asked for from the storage plug-in");
        }
        iLocalChanges.added.clear();
    }

    // The order of the items returned to us may not correspond to iLocalChanges.added, so
    // we iterate over the returned items and get their keys.
    while( (addeditems.count() > 0) &&
    	   (aItemsThatCanBeSent  > 0) &&
    	   (remainingBytes > 0) ) {

        int cmdId = aMessage.getNextCmdId();
        SyncMLAdd* add = new SyncMLAdd(cmdId);
        QString mimeType;
        SyncItemKey key = *addeditems.first()->getKey();

        bool processed = processItem( key, addeditems.first(), *add, remainingBytes, SYNCML_ADD, mimeType );
        remainingBytes -= add->sizeAsXML();
        aSyncElement.addChild( add );

        if (processed) {
            emit newItemWritten( aMessage.getMsgId(), cmdId, key, MOD_ITEM_ADDED,
                                 iSyncTarget.getSourceDatabase(), iSyncTarget.getTargetDatabase(),
                                 mimeType );
            aItemsThatCanBeSent--;
        }
        else {
            LOG_WARNING("Sync item was not processed");
        }

        if( !addeditems.isEmpty() )
        {
            delete addeditems.takeFirst();
        }
    }

    aSizeThreshold = remainingBytes;
    bool processed = false;

    if( addeditems.count() == 0 ) {
        processed = true;
        LOG_DEBUG("Processed all added items");
    }

    return processed;

}

bool LocalChangesPackage::processModifiedItems( SyncMLMessage& aMessage,
                                                SyncMLSync& aSyncElement,
                                                int& aSizeThreshold,
                                                int& aItemsThatCanBeSent)
{
    FUNCTION_CALL_TRACE;

    int remainingBytes = aSizeThreshold;

    // Get all items corresponding to the keys from the plug-in's
    QList<SyncItem*> curritems;

    int count = iLocalChanges.modified.count(); 
    int pos = 0;

    if( 0 < count && 0 == modifieditems.count() )
    {
        while( count )
        {
            curritems = iSyncTarget.getPlugin()->getSyncItems( iLocalChanges.modified.mid( pos, iMaxChangesPerMessage ) );
            if( !curritems.count() )
            {
                break;
            }
            count -= curritems.count();
            modifieditems += curritems;
            pos = modifieditems.count();
            LOG_DEBUG("Fetched " << curritems.count() << " items");
        }

        if( modifieditems.count() != iLocalChanges.modified.count() )
        {
            LOG_WARNING("We didn't get all the items we asked for from the storage plug-in");
        }
        iLocalChanges.modified.clear();
    }

    while( (modifieditems.count() > 0)  &&
     	   (aItemsThatCanBeSent  > 0) &&
     	   (remainingBytes > 0) ) {
        int cmdId = aMessage.getNextCmdId();
        SyncMLReplace* replace = new SyncMLReplace( cmdId );
        QString mimeType;
        SyncItemKey key = *modifieditems.first()->getKey();

        bool processed = processItem( key, modifieditems.first(), *replace, remainingBytes, SYNCML_REPLACE, mimeType );
        remainingBytes -= replace->sizeAsXML();
        aSyncElement.addChild( replace );

        if (processed) {
            emit newItemWritten( aMessage.getMsgId(), cmdId, key, MOD_ITEM_MODIFIED,
                                 iSyncTarget.getSourceDatabase(), iSyncTarget.getTargetDatabase(),
                                 mimeType );
            aItemsThatCanBeSent--;
        } // no else

        if( !modifieditems.isEmpty() )
        {
            delete modifieditems.takeFirst();
        }
    }

    aSizeThreshold = remainingBytes;
    bool processed = false;

    if( modifieditems.count() == 0 ) {
        processed = true;
        LOG_DEBUG("Processed all modified items");
    }

    return processed;
}

bool LocalChangesPackage::processRemovedItems( SyncMLMessage& aMessage,
                                               SyncMLSync& aSyncElement,
                                               int& aSizeThreshold,
                                               int& aItemsThatCanBeSent)
{
    FUNCTION_CALL_TRACE;

    int remainingBytes = aSizeThreshold;

    while( iLocalChanges.removed.count() > 0 &&
      	   (aItemsThatCanBeSent > 0) &&
      	   (remainingBytes > 0) ) {
        int cmdId = aMessage.getNextCmdId();
        SyncMLDelete* del = new SyncMLDelete(cmdId);
        SyncItemKey syncItemKey = iLocalChanges.removed.first();

        // @todo: we cannot know the mime type in the case of deleted items. In overall it's bad
        // that we're using mimetype here, we should be able to handle identification of used
        // storage purely on the db uri's.
        QString mimeType;
        bool processed = processItem( syncItemKey, NULL, *del, remainingBytes, SYNCML_DELETE, mimeType );

        remainingBytes -= del->sizeAsXML();
        aSyncElement.addChild( del );

        if (processed) {
            emit newItemWritten( aMessage.getMsgId(), cmdId, syncItemKey, MOD_ITEM_DELETED,
                                 iSyncTarget.getSourceDatabase(), iSyncTarget.getTargetDatabase(), mimeType );
            iLocalChanges.removed.removeFirst();
            aItemsThatCanBeSent--;
        } // no else
    }

    aSizeThreshold = remainingBytes;
    bool processed = false;

    if( iLocalChanges.removed.count() == 0 ) {
        processed = true;
        LOG_DEBUG("Processed all deleted items");
    }

    return processed;
}

bool LocalChangesPackage::processItem( const SyncItemKey& aItemKey,
                                       SyncItem *aSyncItem,
                                       SyncMLLocalChange& aParent,
                                       int aSizeThreshold,
                                       SyncMLCommand aCommand,
                                       QString& aMimeType )
{
    FUNCTION_CALL_TRACE;

    bool processed = false;

    SyncMLItem* itemObject = new SyncMLItem();

    if( aCommand == SYNCML_ADD ) {

        itemObject->insertSource( aItemKey );

    }
    else {

        if( iRole == ROLE_SERVER ) {
            SyncItemKey remoteKey = iSyncTarget.mapToRemoteUID( aItemKey );

            if( !remoteKey.isEmpty() ) {
                itemObject->insertTarget( remoteKey );
            }
            else {
                LOG_WARNING("Could not find mapping to for local uid: " << aItemKey);
            }
        }
        else {
            itemObject->insertSource( aItemKey );
        }

    }

    if( aCommand == SYNCML_DELETE ) {
        // Delete command does not include item data
        processed = true;
    }
    else {

        if( aSyncItem ) {

            aMimeType = aSyncItem->getType();

            aParent.addMimeMetadata( aSyncItem->getType() );
            qint64 size = aSyncItem->getSize();

            if( !aSyncItem->getParentKey()->isEmpty() ) {

                const SyncItemKey* parentKey = aSyncItem->getParentKey();

                if( iRole == ROLE_SERVER ) {
                    SyncItemKey remoteKey = iSyncTarget.mapToRemoteUID( *parentKey );

                    if( !remoteKey.isEmpty() ) {
                        itemObject->insertTargetParent( remoteKey );
                    }
                    else {
                        itemObject->insertSourceParent( *parentKey );
                    }
                }
                else if( iRole == ROLE_CLIENT ) {
                    itemObject->insertSourceParent( *parentKey );
                }
                // no else

            }

            if( size > iLargeObjectThreshold ) {

                // Item is large

                if( iLargeObjectState.iItemKey != aItemKey ) {
                    // Start sending large object
                    iLargeObjectState.iItemKey = aItemKey;
                    iLargeObjectState.iSize = size;
                    iLargeObjectState.iOffset = 0;
                }

                QByteArray data;
                qint64 dataLeft = iLargeObjectState.iSize - iLargeObjectState.iOffset;

                if( aSizeThreshold < dataLeft ) {
                    aSyncItem->read( iLargeObjectState.iOffset, aSizeThreshold, data );
                    aParent.addSizeMetadata( size );
                    itemObject->insertData( data );
                    itemObject->insertMoreData();
                    iLargeObjectState.iOffset += aSizeThreshold;
                }
                else {
                    aSyncItem->read( iLargeObjectState.iOffset, dataLeft, data );
                    itemObject->insertData( data );
                    iLargeObjectState.iOffset += dataLeft;
                    processed = true;
                }

            }
            else
            {
                QByteArray data;
                aSyncItem->read( 0, size, data );
                itemObject->insertData( data );
                processed = true;
            }

        }
        else {
            LOG_WARNING( "Could not retrieve item data:" << aItemKey );
            processed = true;
        }
    }

    aParent.addChild( itemObject );

    return processed;
}
