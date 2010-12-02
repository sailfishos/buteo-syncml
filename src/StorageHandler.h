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

#ifndef STORAGEHANDLER_H
#define STORAGEHANDLER_H

#include <QObject>
#include <QMap>
#include <QString>

#include "SyncAgentConsts.h"
#include "SyncItemKey.h"
#include "StoragePlugin.h"

namespace DataSync {

class SyncItem;
class ConflictResolver;


/*! \brief Item commit status
 *
 */
enum CommitStatus
{
    COMMIT_ADDED,              /*!<Successful, new item added*/
    COMMIT_REPLACED,           /*!<Successful, existing item replaced*/
    COMMIT_DELETED,            /*!<Successful, item deleted*/
    COMMIT_DUPLICATE,          /*!<Successful, but item was duplicate so no change made*/
    COMMIT_NOT_DELETED,        /*!<Failed, item to be deleted was not found*/
    COMMIT_UNSUPPORTED_FORMAT, /*!<Failed, unsupported format*/
    COMMIT_ITEM_TOO_BIG,       /*!<Failed, item too big*/
    COMMIT_NOT_ENOUGH_SPACE,   /*!<Failed, not enough space*/
    COMMIT_GENERAL_ERROR,      /*!<Failed, unspecified error*/
    COMMIT_INIT_ADD,            /*!<Successful, Initial state before add*/
    COMMIT_INIT_REPLACE,       /*!<Successful, Initial state before modifications*/
    COMMIT_INIT_DELETE         /*!<Successful, Initial state before deletions*/
};

/*! \brief Conflict status on item commit
 *
 */
enum ConflictStatus {
    CONFLICT_NO_CONFLICT,   /*!<No conflict was present*/
    CONFLICT_REMOTE_WIN,    /*!<Item conflicted and remote side won the conflict*/
    CONFLICT_LOCAL_WIN      /*!<Item conflicted and local side won the conflict*/
};

/*! \brief Item commit results
 *
 */
struct CommitResult
{
    SyncItemKey     iItemKey;   /*!<Item key associated with the commit*/
    CommitStatus    iStatus;    /*!<Status of the commit*/
    ConflictStatus  iConflict;  /*!<Conflict status*/
};

/*! \brief Item identification
 *
 */
struct ItemId
{
    int iCmdId;         /*!<Command id of the command this item is associated with*/
    int iItemIndex;     /*!<Index of this item in the command this item is associated with*/

    /*! \brief Less-than operator
     *
     * Needed by QMap
     *
     * @param aItemId Item identification to compare against
     * @return True if less, otherwise false
     */
    bool operator<( const ItemId& aItemId ) const
    {

        if( iCmdId < aItemId.iCmdId ) {
            return true;
        }
        else if( iCmdId == aItemId.iCmdId ) {
            if( iItemIndex < aItemId.iItemIndex ) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

};

/*! \brief Utility class for storing items into local database
 *
 */
class StorageHandler : public QObject
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     */
    StorageHandler();

    /*! \brief Destructor
     *
     */
    virtual ~StorageHandler();

    /*! \brief Adds a new item to local database
     *
     * @param aItemId Item identification
     * @param aPlugin Local storage plugin
     * @param aLocalKey Local key of the item or empty 
     * @param aParentKey Key of the parent of this item (local id)
     * @param aType MIME type of the item
     * @param aFormat Format of the item
     * @param aVersion Version of the item
     * @param aData Data of the item
     *
     */
    bool addItem( const ItemId& aItemId,
                  StoragePlugin& aPlugin,
		  const SyncItemKey& aLocalKey,
                  const SyncItemKey& aParentKey,
                  const QString& aType,
                  const QString& aFormat,
                  const QString& aVersion,
                  const QString& aData);

    /*! \brief Replaces an existing item in local database
     *
     * @param aItemId Item identification
     * @param aPlugin Local storage plugin
     * @param aLocalKey Local key of the item
     * @param aParentKey Key of the parent of this item (local id)
     * @param aType MIME type of the item
     * @param aFormat Format of the item
     * @param aVersion Version of the item
     * @param aData Data of the item
     */
    bool replaceItem( const ItemId& aItemId,
                      StoragePlugin& aPlugin,
                      const QString& aLocalKey,
                      const SyncItemKey& aParentKey,
                      const QString& aType,
                      const QString& aFormat,
                      const QString& aVersion,
                      const QString& aData);

    /*! \brief Deletes an existing item in local database
     *
     * @param aItemId Item identification
     * @param aLocalKey Local key of the item
     */
    bool deleteItem( const ItemId& aItemId,
                     const SyncItemKey& aLocalKey );

    /*! \brief Begin composing large object to add to local database
     *
     * @param aPlugin Local storage plugin
     * @param aRemoteKey Remote key of the item
     * @param aParentKey Key of the parent of this item (local id)
     * @param aType MIME type of the item
     * @param aFormat Format of the item
     * @param aVersion Version of the item
     * @param aSize Expected size of the item
     */
    bool startLargeObjectAdd( StoragePlugin& aPlugin,
                              const QString& aRemoteKey,
                              const SyncItemKey& aParentKey,
                              const QString& aType,
                              const QString& aFormat,
                              const QString& aVersion,
                              qint64 aSize );

    /*! \brief Begin composing large object to replace in local database
     *
     * @param aPlugin Local storage plugin
     * @param aLocalKey Local key of the item
     * @param aParentKey Key of the parent of this item (local id)
     * @param aType MIME type of the item
     * @param aFormat Format of the item
     * @param aVersion Version of the item
     * @param aSize Expected size of the item
     */
    bool startLargeObjectReplace( StoragePlugin& aPlugin,
                                  const QString& aLocalKey,
                                  const SyncItemKey& aParentKey,
                                  const QString& aType,
                                  const QString& aFormat,
                                  const QString& aVersion,
                                  qint64 aSize );

    /*! \brief Returns true if composing large object is in progress
     *
     * @return True if building large object, otherwise false
     */
    bool buildingLargeObject() const;

    /*! \brief Checks if the large object being composed matches given key
     *
     * Automatically aborts large object if false is returned
     *
     * @param aKey Item key to match against
     * @return True if key matches large object, otherwise false
     */
    bool matchesLargeObject( const QString& aKey );


    /*! \brief Appends data to large object being composed
     *
     * Automatically aborts large object if false is returned
     *
     * @param aData Data to append
     * @return True if append was successful, otherwise false
     */
    bool appendLargeObjectData( const QString& aData );

    /*! \brief Finishes the large object being composed
     *
     * Automatically aborts large object if false is returned
     *
     * @param aItemId Item identification to assign to large object
     * @return True on success, otherwise false
     */
    bool finishLargeObject( const ItemId& aItemId );
    
    /*! \brief Checks for conflicts in changes in local db with remote
     *
     * @param aConflictResolver conflict resolver.
     * @param aList Map of itemid-sync items to check
     * @param aStatus Status of o/p add/modify/delete
     * @return Commit results
     */
    QMap<ItemId, CommitResult> resolveConflicts( ConflictResolver* aConflictResolver,
                                                 QMap<ItemId, SyncItem*> &aList,
                                                 CommitStatus aStatus);
    /*! \brief Same as above but takes QMap with SyncItemKey as input
      *  called in case of delete o/p when we have only the item key.
      */
    QMap<ItemId, CommitResult> resolveConflicts( ConflictResolver* aConflictResolver,
                                                 QMap<ItemId, SyncItemKey> &aList,
                                                 CommitStatus aStatus);

    /*! \brief Commits added items to local database
     *
     * @param aPlugin Local storage plugin
     * @param aConflictResolver If conflict resolution is to be done, conflict resolver.
     *        Otherwise NULL
     * @return Commit results
     */
    QMap<ItemId, CommitResult> commitAddedItems( StoragePlugin& aPlugin, 
		    ConflictResolver* aConflictResolver );

    /*! \brief Commits replaced items to local database
     *
     * @param aPlugin Local storage plugin
     * @param aConflictResolver If conflict resolution is to be done, conflict resolver.
     *        Otherwise NULL
     * @return Commit results
     */
    QMap<ItemId, CommitResult> commitReplacedItems( StoragePlugin& aPlugin,
                                                    ConflictResolver* aConflictResolver );

    /*! \brief Commits deleted items to local database
     *
     * @param aPlugin Local storage plugin
     * @param aConflictResolver If conflict resolution is to be done, conflict resolver.
     *        Otherwise NULL
     * @return Commit results
     */
    QMap<ItemId, CommitResult> commitDeletedItems( StoragePlugin& aPlugin,
                                                   ConflictResolver* aConflictResolver );

signals:

    /*! \brief Signal indicating that an item has been processed
     *
     * @param aModificationType Type of modification made to the item (addition, modification or delete)
     * @param aModifiedDatabase Database that was modified (local or remote)
     * @param aDatabase Identifier of the database that was modified
     * @param aMimeType Mime type of the item being processed
     * @param aCommittedItems No. of items committed for this operation (addition, modification or delete)
     */
    void itemProcessed( DataSync::ModificationType aModificationType,
                        DataSync::ModifiedDatabase aModifiedDatabase,
                        const QString aDatabase,const QString aMimeType, int aCommittedItems);
private:

    CommitStatus generalStatus( StoragePlugin::StoragePluginStatus aStatus ) const;

    QMap<ItemId, SyncItem*>    iAddList;
    QMap<ItemId, SyncItem*>    iReplaceList;
    QMap<ItemId, SyncItemKey>  iDeleteList;

    SyncItem*                  iLargeObject;
    qint64                     iLargeObjectSize;
    QString                    iLargeObjectKey;

    friend class StorageHandlerTest;
};

}

#endif  //  STORAGEHANDLER_H
