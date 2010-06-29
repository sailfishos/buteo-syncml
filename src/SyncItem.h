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
#ifndef SYNCITEM_H
#define SYNCITEM_H

#include <QtGlobal>
#include <QString>

#include "SyncItemKey.h"

namespace DataSync {

/*! \brief Base class for items of data that can be synchronized
 *
 * SyncItem describes one synchronizable item. Each SyncItem has a unique
 * key assigned to it. SyncItem can be temporary, in this case it has no key
 * assigned to it. getKey() will return "" in this case.
 */
class SyncItem {

public:

    /*! \brief Constructor
     *
     */
    SyncItem();

    /*! \brief Destructor
     *
     */
    virtual ~SyncItem();

    /*! \brief Returns the key of this item
     *
     * @return
     */
    const SyncItemKey* getKey() const;

    /*! \brief Sets the key of this item
     *
     * @param aKey Key for the item
     */
    void setKey( const SyncItemKey& aKey );

    /*! \brief Returns the key of the parent of this item
     *
     * @return
     */
    const SyncItemKey* getParentKey() const;

    /*! \brief Sets the key of the parent of this item
     *
     * @param aParentKey Key of the parent of this item
     */
    void setParentKey( const SyncItemKey& aParentKey );

    /*! \brief Returns the MIME type of this item
     *
     * @return
     */
    QString getType() const;

    /*! \brief Sets the MIME type of this item
     *
     * @param aType MIME type for the item
     */
    void setType( const QString& aType );

    /*! \brief Returns the format of this item
     *
     * @return
     */
    QString getFormat() const;

    /*! \brief Sets the format of this item
     *
     * @param aFormat Format of this item
     */
    void setFormat( const QString& aFormat );

    /*! \brief Returns the total size of the item data
     *
     * @return Size of the item data
     */
    virtual qint64 getSize() const = 0;

    /*! \brief This method reads bytes of data from sync item
     *
     * @param aOffset Offset to start reading from
     * @param aLength Number of bytes to read
     * @param aData Data buffer where to place read data
     * @return
     */
    virtual bool read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const = 0;

    /*! \brief This method resizes the sync item
     *
     * @param aLength New Size
     * @return True on success, otherwise false
     */
    virtual bool resize( qint64 aLength ) = 0;

    /*! \brief This method writes data to sync item.
     *
     * @param aOffset Offset to start writing from
     * @param aData Data buffer to write. All bytes from buffer are written
     * @return
     */
    virtual bool write( qint64 aOffset, const QByteArray& aData ) = 0;

private:
    SyncItemKey iKey;
    SyncItemKey iParentKey;
    QString     iType;
    QString     iFormat;

};


}

#endif /* SYNCITEM_H */
