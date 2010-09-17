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
#ifndef SYNCITEMPREFETCHERTEST_H
#define SYNCITEMPREFETCHERTEST_H

#include <QObject>

#include "StoragePlugin.h"

using namespace DataSync;

class PrefetchStorage : public StoragePlugin
{
public:
    PrefetchStorage( const QList<SyncItemKey>& aItemIds );
    virtual ~PrefetchStorage();

    void forceSyncItems( const QList<SyncItem*> aSyncItems );

    virtual const QString& getSourceURI() const;

    virtual qint64 getMaxObjSize() const;

    virtual const StorageContentFormatInfo& getFormatInfo() const;

    virtual QByteArray getPluginCTCaps( ProtocolVersion aVersion ) const;

    virtual bool getAll( QList<SyncItemKey>& aKeys );

    virtual bool getModifications( QList<SyncItemKey>& aNewKeys,
                                   QList<SyncItemKey>& aReplacedKeys,
                                   QList<SyncItemKey>& aDeletedKeys,
                                   const QDateTime& aTimeStamp );

    virtual SyncItem* newItem();

    virtual SyncItem* getSyncItem( const SyncItemKey& aKey );

    virtual QList<SyncItem*> getSyncItems( const QList<SyncItemKey>& aKeyList );

    virtual QList<StoragePluginStatus> addItems( const QList<SyncItem*>& aItems );

    virtual QList<StoragePluginStatus> replaceItems( const QList<SyncItem*>& aItems );

    virtual QList<StoragePluginStatus> deleteItems( const QList<SyncItemKey>& aKeys );

private:
    QString                     iSourceURI;
    StorageContentFormatInfo    iFormats;

    QList<SyncItemKey>          iItemIds;

    bool                        iForceSyncItems;
    QList<SyncItem*>            iSyncItems;

};

class SyncItemPrefetcherTest : public QObject
{
    Q_OBJECT;
public:

private slots:
    void testNormalNoIdle();
    void testNormalIdle();

    void testAbnormalBadItems();
    void testAbnormalBadItemCount();

};

#endif // SYNCITEMPREFETCHERTEST_H
