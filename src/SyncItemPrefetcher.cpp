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

#include "SyncItemPrefetcher.h"

#include "SyncItem.h"
#include "StoragePlugin.h"

#include "SyncMLLogging.h"

using namespace DataSync;

SyncItemPrefetcher::SyncItemPrefetcher( const QList<SyncItemKey>& aItemIds,
                                        StoragePlugin& aStoragePlugin,
                                        int aInitialBatchSizeHint )
 : iStoragePlugin( aStoragePlugin ), iItemIdList( aItemIds )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
    iDefaultBatchSizeHint = aInitialBatchSizeHint;
    setBatchSizeHint( aInitialBatchSizeHint );
}

SyncItemPrefetcher::~SyncItemPrefetcher()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    qDeleteAll( iFetchedItems.values() );
    iFetchedItems.clear();
}

void SyncItemPrefetcher::setBatchSizeHint( int aBatchSizeHint )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
    iBatchSizeHint = aBatchSizeHint;
}

SyncItem* SyncItemPrefetcher::getItem( const SyncItemKey& aItemId )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if(!iBatchSizeHint)
    {
        iBatchSizeHint = iDefaultBatchSizeHint - iFetchedItems.count();
    }

    if( iFetchedItems.contains( aItemId ) )
    {
        // Prefetch hit: return item immediately
        qCDebug(lcSyncML) << "Item" << aItemId << "found from prefetched items";
        return iFetchedItems.take( aItemId );
    }
    else
    {
        // Prefetch miss: fetch more items
        qCDebug(lcSyncML) << "Item" << aItemId << "not found from prefetched items";
        prefetch();
        return iFetchedItems.take( aItemId );
    }
}

void SyncItemPrefetcher::prefetch()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    qCDebug(lcSyncML) << "Item prefetcher waking...";

    if( iFetchedItems.count() < iBatchSizeHint )
    {

        qCDebug(lcSyncML) << "Prefetch cache not full";
        int batchSize = qMin( iBatchSizeHint, iItemIdList.size() );

        if( batchSize > 0 )
        {
            qCDebug(lcSyncML) << "Requesting" << batchSize << "items";
            QList<SyncItemKey> nextItemIds = iItemIdList.mid( 0, batchSize );
            QList<SyncItem*> nextItems = iStoragePlugin.getSyncItems( nextItemIds );

            if( nextItems.count() != nextItemIds.count() )
            {
                // We cannot trust the ordering nor the integrity of the items returned by the backend, so just
                // free them
                qCWarning(lcSyncML) << "Asked for" << nextItemIds.count() << "items, got" << nextItems.count() << "items";
                qDeleteAll( nextItems );
                nextItems.clear();
            }

            for ( int i = 0; i < nextItemIds.count(); ++i )
            {
                SyncItem* item = 0;

                for( int a = 0; a < nextItems.count(); ++a )
                {
                    if( nextItems[a] && nextItems[a]->getKey() == nextItemIds[i] )
                    {
                        item = nextItems[a];
                        nextItems.removeAt( a );
                    }
                }

                iFetchedItems.insert( nextItemIds[i], item );

            }

            iItemIdList = iItemIdList.mid( batchSize );
        }
        else
        {
            qCDebug(lcSyncML) << "No more items remaining, skipping item request";
        }
    }
    else
    {
        qCDebug(lcSyncML) << "Prefetch cache is already full";
    }

    qCDebug(lcSyncML) << iFetchedItems.count() << "items in prefetch cache," << iItemIdList.count() << "items still to fetch";

    qCDebug(lcSyncML) << "Item prefetcher going to sleep...";
}
