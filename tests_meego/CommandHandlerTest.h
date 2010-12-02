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
#ifndef COMMANDHANDLERTEST_H
#define COMMANDHANDLERTEST_H

#include <QTest>

#include "StoragePlugin.h"
#include "Mock.h"

namespace DataSync {

class CommandHandlerTest;


class CommitTestStorage : public StoragePlugin
{
public:

    CommitTestStorage( const QString& aSourceURI ) : iSourceURI( aSourceURI ), iIdCounter( 0 )
    {
        ContentFormat format;
        format.iType = "foo";
        format.iVersion = "0.0";
        iFormats.setPreferredRx( format );
        iFormats.setPreferredTx( format );
        iFormats.rx().append( format );
        iFormats.tx().append( format );
    }

    virtual ~CommitTestStorage()
    {
    }

    virtual const QString& getSourceURI() const
    {
        return iSourceURI;
    }

    const StorageContentFormatInfo& getFormatInfo() const
    {
        return iFormats;
    }

    virtual qint64 getMaxObjSize() const
    {
        return 0;
    }

    virtual QByteArray getPluginCTCaps( ProtocolVersion /*aVersion*/ ) const
    {
        return "";
    }

    QByteArray getPluginExts( ) const
    {
        return "";
    }

    virtual bool getAll( QList<SyncItemKey>& /*aKeys*/ )
    {
        return true;
    }

    virtual bool getModifications( QList<SyncItemKey>& /*aNewKeys*/,
                                   QList<SyncItemKey>& /*aReplacedKeys*/,
                                   QList<SyncItemKey>& /*aDeletedKeys*/,
                                   const QDateTime& /*aTimeStamp*/ )
    {
        return true;
    }

    virtual SyncItem* newItem()
    {
        return new MockSyncItem( "" );
    }

    virtual SyncItem* getSyncItem( const SyncItemKey& /*aKey*/ )
    {
        Q_ASSERT( 0 );
        return NULL;
    }

    virtual QList<SyncItem*> getSyncItems( const QList<SyncItemKey>& aKeyList )
    {
        Q_UNUSED( aKeyList );
        QList<SyncItem*> items;
        Q_ASSERT( 0 );
        return items;
    }

    virtual QList<StoragePluginStatus> addItems( const QList<SyncItem*>& aItems )
    {
        QList<StoragePluginStatus> statuses;

        for( int i = 0; i < aItems.count(); ++i ) {
            aItems[i]->setKey( QString::number( ++iIdCounter ) );
            iAddedItems.append( *aItems[i]->getKey() );
            statuses.append( STATUS_OK );
        }

        return statuses;
    }

    virtual QList<StoragePluginStatus> replaceItems( const QList<SyncItem*>& aItems )
    {
        QList<StoragePluginStatus> statuses;

        for( int i = 0; i < aItems.count(); ++i ) {
            iReplacedItems.append( *aItems[i]->getKey() );
            statuses.append( STATUS_OK );
        }

        return statuses;
    }

    virtual QList<StoragePluginStatus> deleteItems( const QList<SyncItemKey>& aKeys )
    {
        QList<StoragePluginStatus> statuses;

        for( int i = 0; i < aKeys.count(); ++i ) {
            iDeletedItems.append( aKeys );
            statuses.append( STATUS_OK );
        }

        return statuses;
    }

#if 0
    virtual bool deleteAllItems()
    {
        return true;
    }
#endif

    friend class CommandHandlerTest;

private:
    QString                     iSourceURI;
    StorageContentFormatInfo    iFormats;
    QList<SyncItemKey>          iAddedItems;
    QList<SyncItemKey>          iReplacedItems;
    QList<SyncItemKey>          iDeletedItems;
    int                         iIdCounter;

};



class CommandHandlerTest : public QObject
{
    Q_OBJECT;
public:

    CommandHandlerTest();
    virtual ~CommandHandlerTest();

private slots:

    void testAdd_Client01();
    void testAdd_Server01();

    void testSyncAdd();
    void testSyncReplace();
    void testSyncDelete();
    void testSyncReplaceConflict();
    void testSyncReplaceConflict_01();

private:

};

}

#endif  //  COMMANDHANDLERTEST_H
