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
#ifndef LOCALCHANGESPACKAGE_H
#define LOCALCHANGESPACKAGE_H

#include "Package.h"
#include "internals.h"
#include "SyncMLCommand.h"
#include "SyncItemKey.h"
#include "SyncAgentConsts.h"

class LocalChangesPackageTest;

namespace DataSync {

class SyncMLSync;
class SyncMLLocalChange;
class SyncTarget;

/*! \brief LocalChangesPackage handles sending local modifications phase for
 *         a single sync target
 *
 * LocalChangesPackage maintains a queue of modifications to be sent to
 * server.
 */
class LocalChangesPackage : public Package
{
    Q_OBJECT
public:

    /*! \brief Constructor
     *
     * @param aSyncTarget Sync target
     * @param aLocalChanges List of local changes in aSyncTarget
     * @param aLargeObjectThreshold If object size exceeds this threshold, it should be considered a large object
     * @param aRole The role of the session (client or server)
     * @param aMaxChangesPerMessage Maximum number of changes to write per one SyncML message
     */
    LocalChangesPackage( const SyncTarget& aSyncTarget,
                         const LocalChanges& aLocalChanges,
                         int aLargeObjectThreshold,
                         const Role& aRole,
                         int aMaxChangesPerMessage);

    /*! \brief Destructor
     *
     */
    virtual ~LocalChangesPackage();

    virtual bool write( SyncMLMessage& aMessage, int& aSizeThreshold );

signals:

    /*! \brief Signal that has been emitted when item has been added to an outgoing message
     *
     * Information supplied in this signal can be used to track the item.
     *
     * @param aMsgId Message Id of the message where the item was written
     * @param aCmdId Command Id of the command where the item was written
     * @param aKey Key of the written item
     * @param aModificationType Type of modification to the item
     * @param aLocalDatabase Local database where item exists
     * @param aRemoteDatabase Remote database to which item is being sent
     * @param aMimeType Mime type of the item
     */
    void newItemWritten( int aMsgId, int aCmdId, SyncItemKey aKey,
                         ModificationType aModificationType,
                         QString aLocalDatabase, QString aRemoteDatabase,
                         QString aMimeType );

protected:

private:

    struct LargeObjectState
    {
        SyncItemKey iItemKey;
        qint64      iSize;
        qint64      iOffset;
        LargeObjectState() : iSize(0), iOffset(0) {}
    };

    bool processAddedItems( SyncMLMessage& aMessage,
                            SyncMLSync& aSyncElement,
                            int& aSizeThreshold,
                            int& aItemsThatCanBeSent);

    bool processModifiedItems( SyncMLMessage& aMessage,
                               SyncMLSync& aSyncElement,
                               int& aSizeThreshold,
                               int& aItemsThatCanBeSent);

    bool processRemovedItems( SyncMLMessage& aMessage,
                              SyncMLSync& aSyncElement,
                              int& aSizeThreshold,
                              int& aItemsThatCanBeSent);

    bool processItem( const SyncItemKey& aItemKey,
                      SyncMLLocalChange& aParent,
                      int aSizeThreshold,
                      SyncMLCommand aCommand,
                      QString& aMimeType );

    int                     iLargeObjectThreshold;
    int                     iNumberOfChanges;
    const SyncTarget&       iSyncTarget;
    LocalChanges            iLocalChanges;
    LargeObjectState        iLargeObjectState;
    Role                    iRole;
    int 					iMaxChangesPerMessage;

    friend class ::LocalChangesPackageTest;

};

}
#endif  //  LOCALCHANGESPACKAGE_H
