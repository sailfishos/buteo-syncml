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

#ifndef STORAGEPLUGIN_H
#define STORAGEPLUGIN_H

#include <QDateTime>
#include <QList>

#include "SyncItemKey.h"
#include "SyncAgentConsts.h"
#include "CTCap.h"

namespace DataSync {

class SyncItem;

/*! \brief StoragePlugin provides the content for the synchronization process.
 *
 * StoragePlugin is an entity which provides an interface for adding, removing
 * a SyncItem (like an email or bookmark), and read or write data to it.
 * The actual writing of data to an item is done via the SyncItem class methods.
 *
 * Every StoragePlugin has an identifier that can be retrieved using getInfo() method.
 * This identifier is used by server to identify to which data store (represented by
 * storage plugin) the client wishes to sync against. Therefore StoragePlugin identifiers
 * should be unique inside one device.
 */
class StoragePlugin
{
public:

    /*! \brief Status of operation performed by storage plugin
     *
     */
    enum StoragePluginStatus
    {
        STATUS_INVALID_FORMAT = -6,  /*!< Operation failed because data is in invalid format*/
        STATUS_STORAGE_FULL = -5,    /*!< Operation failed because storage is full*/
        STATUS_OBJECT_TOO_BIG = -4,  /*!< Operation failed because object is too big*/
        STATUS_ERROR = -3,           /*!< General error occurred during operation*/
        STATUS_DUPLICATE = -2,       /*!< Operation was not performed as object was duplicate*/
        STATUS_NOT_FOUND = -1,       /*!< Operation failed as object was not found*/
        STATUS_OK = 0                /*!< Operation was completed successfully*/
    };

    /*! \brief Destructor
     *
     */
    virtual ~StoragePlugin() {}

    /*! \brief Returns the URI of the storage plugin.
     *
     * @return
     */
    virtual const QString& getSourceURI() const = 0;

    /*! \brief Returns the maximum size of items supported by the storage plugin
     *
     * @return
     */
    virtual qint64 getMaxObjSize() const = 0;

    /*! \brief Returns a list of the format supported by the storage plugin
     *
     * List includes all supported formats, including preferred format
     *
     * @return
     */
    virtual const QList<ContentFormat>& getSupportedFormats() const = 0;

    /*! \brief Returns the preferred format of the storage plugin
     *
     * @return
     */
    virtual const ContentFormat& getPreferredFormat() const = 0;

    /*! \brief Returns SyncML Device Information CTCaps of this plugin
     *
     * MUST be in XML format with SyncML CTCap as root element
     *
     * @param aVersion Protocol version
     * @return
     */
    virtual QByteArray getPluginCTCaps( ProtocolVersion aVersion ) const = 0;

    /*! \brief Get the id's of all stored items
     *
     * @param aKeys Array to which store item id's
     * @return True on success, otherwise false
     */
    virtual bool getAll( QList<SyncItemKey>& aKeys ) = 0;

    /*! \brief Get the id's of all items that have been modified after timestamp
     *
     * @param aNewKeys Array to which store item id's of new items
     * @param aReplacedKeys Array to which stora item id's of replaced items
     * @param aDeletedKeys Array to which stora item id's of deleted
     * @param aTimeStamp Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getModifications( QList<SyncItemKey>& aNewKeys,
                                   QList<SyncItemKey>& aReplacedKeys,
                                   QList<SyncItemKey>& aDeletedKeys,
                                   const QDateTime& aTimeStamp ) = 0;

    /*! \brief Creates a new sync item
     *
     * This function generates a new sync item and returns it. The returned item
     * is temporary, which means it is not guaranteed that it has a key assigned,
     * and it is guaranteed that it has no data.
     *
     * @return New SyncItem instance on success, otherwise NULL
     */
    virtual SyncItem* newItem() = 0;

    /*! \brief Returns a specific sync item identified by SyncItemKey
     *
     * @param aKey Key of the sync item
     * @return Sync item. If matching sync item is not found, NULL is returned
     */
    virtual SyncItem* getSyncItem( const SyncItemKey& aKey ) = 0;

    /*! \brief Returns a list of sync items identified by sync item keys
     *
     * This function takes the advantage of batch fetches from storage plug-in's
     * that do support this.
     *
     * @param aKeyList Keys of the sync items
     * @return Sync item list.
     */
    virtual QList<SyncItem*> getSyncItems( const QList<SyncItemKey>& aKeyList ) = 0;

    /*! \brief Adds new items
     *
     * Items must NOT have their keys set before calling this function. After successful
     * addition, storage plugin sets the item id to it's allocated value
     *
     * @param aItems List of items to add
     * @return List of status codes corresponding to each item
     */
    virtual QList<StoragePluginStatus> addItems( const QList<SyncItem*>& aItems ) = 0;

    /*! \brief Replaces existing items
     *
     * Items must have their keys set before calling this function
     *
     * @param aItems List of items to replace
     * @return List of status codes corresponding to each item
     */
    virtual QList<StoragePluginStatus> replaceItems( const QList<SyncItem*>& aItems ) = 0;

    /*! \brief Deletes existing items
     *
     * @param aKeys List of items to delete
     * @return List of status codes corresponding to each key
     */
    virtual QList<StoragePluginStatus> deleteItems( const QList<SyncItemKey>& aKeys ) = 0;

#if 0
    /*! \brief Delete all existing items
     *
     * @return true if all items were deleted, false otherwise.
     */
    virtual bool deleteAllItems() = 0;
#endif

};


}
#endif
