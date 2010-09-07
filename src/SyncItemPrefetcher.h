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
#ifndef SYNCITEMPREFETCHER_H
#define SYNCITEMPREFETCHER_H

#include <QObject>
#include <QList>
#include <QHash>

#include "SyncItemKey.h"

class SyncItemPrefetcherTest;

namespace DataSync
{

class StoragePlugin;
class SyncItem;

/*! \brief Class that prefetches items from storage plugin based on
 *         batch size hint to increase performance when sending items
 *
 * This class takes advantage on that the order of items requested from
 * storage plugin is known (aItemIds). Items are fetched in advance based
 * on current batch size hint.
 */
class SyncItemPrefetcher : public QObject
{
    Q_OBJECT;
public:
    SyncItemPrefetcher( const QList<SyncItemKey>& aItemIds,
                        StoragePlugin& aStoragePlugin,
                        int aInitialBatchSizeHint );

    virtual ~SyncItemPrefetcher();

    void setBatchSizeHint( int aBatchSizeHint );

    SyncItem* getItem( const SyncItemKey& aItemId );

public slots:

    void prefetch();

private:

    StoragePlugin&                  iStoragePlugin;
    int                             iBatchSizeHint;
    QList<SyncItemKey>              iItemIdList;
    QHash<SyncItemKey, SyncItem*>   iFetchedItems;


    friend class ::SyncItemPrefetcherTest;

};

}

#endif // SYNCITEMPREFETCHER_H
