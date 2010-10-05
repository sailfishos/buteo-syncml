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

#ifndef SYNCTARGET_H
#define SYNCTARGET_H

#include "SyncMode.h"
#include "SyncAgentConsts.h"
#include "SyncMLGlobals.h"
#include "LocalChanges.h"
#include "datatypes.h"


namespace DataSync {

class StoragePlugin;
class ChangeLog;
class DatabaseHandler;
class SyncTargetTest;

/*! \brief Sync target represent sync of a single local database with a single
 *         remote database
 */
class SyncTarget {
public:

    /*! \brief Constructor
     *
     * @param aChangeLog Change log to associate with this sync target. SyncTarget takes ownership.
     * @param aPlugin Source storage plugin
     * @param aSyncMode Sync mode to use
     * @param aLocalNextAnchor Next anchor to use
     */
    SyncTarget( ChangeLog* aChangeLog,
                StoragePlugin* aPlugin,
                const SyncMode& aSyncMode,
                const QString& aLocalNextAnchor );

    /*! \brief Destructor
     *
     */
    ~SyncTarget();

    /*! \brief Returns the source database URI
     *
     * @return
     */
    QString getSourceDatabase() const;

    /*! \brief Sets the target database URI
     *
     * @param aTargetDatabase Target database URI
     */
    void setTargetDatabase( const QString& aTargetDatabase );

    /*! \brief Returns the target database URI
     *
     * @return
     */
    QString getTargetDatabase() const;

    /*! \brief Returns the last local anchor
     *
     * @return
     */
    QString getLocalLastAnchor() const;

    /*! \brief Returns the next local anchor
     *
     * @return
     */
    QString getLocalNextAnchor() const;

    /*! \brief Returns the last remote anchor
     *
     * @return
     */
    QString getRemoteLastAnchor() const;

    /*! \brief Returns the next remote anchor
     *
     * @return
     */
    QString getRemoteNextAnchor() const;

    /*! \brief Sets the next remote anchor
     *
     * @param aRemoteNextAnchor Next remote anchor
     */
    void setRemoteNextAnchor( const QString& aRemoteNextAnchor );

    /*! \brief Returns the sync mode in use
     *
     * @return
     */
    SyncMode* getSyncMode();

    /*! \brief Returns the sync mode in use
     *
     * @return
     */
    const SyncMode* getSyncMode() const;

    /*! \brief Set the sync mode in use
     *
     * @param aSyncMode Sync mode
     */
    void setSyncMode( const SyncMode& aSyncMode );

    /*! \brief Revert to slow sync mode
     *
     */
    void revertSyncMode();

    /*! \brief Set client only refresh sync type 
     *
     * @return True if sync mode has been set to client inited refresh from client (203)
     */
    bool setRefreshFromClient();

    /*! \brief Returns whether this target's sync mode has been reverted to slow sync
     *
     * @return True if sync mode has been reverted, otherwise false
     */
    bool reverted() const;

    /*! \brief Detects local changes since last sync
     *
     * To retrieve the changes, use getLocalChanges()
     *
     * @param aRole Role in use
     * @return True on success, otherwise false
     */
    bool discoverLocalChanges( const Role& aRole);

    /*! \brief Retrieve local changes since last sync
     *
     * Before calling this function, discoverLocalChanges() should be used to
     * find the changes
     *
     * @return Local changes
     */
    const LocalChanges* getLocalChanges() const;
    
    /*! \brief Retrieve local changes since last sync
     *
     * Before calling this function, discoverLocalChanges() should be used to
     * find the changes
     *
     * @return Local changes
     */
    LocalChanges* getLocalChanges() ;

    /*! \brief Return the plugin of the sync target
     *
     * @return
     */
    StoragePlugin* getPlugin() const;

    /*! \brief Adds a mapping from remote UID to local UID
     *
     * @param aMapping Mapping
     */
    void addUIDMapping( const UIDMapping& aMapping );

    /*! \brief Removes a mapping from remote UID to local UID
     *
     * @param aLocalKey Local UID of the mapping to remove
     */
    void removeUIDMapping( const SyncItemKey& aLocalKey );

    /*! \brief Map a remote UID to local UID
     *
     * @param aRemoteUID Remote UID
     * @return Local UID if found, otherwise ""
     */
    SyncItemKey mapToLocalUID( const QString& aRemoteUID ) const;

    /*! \brief Map a local UID to remote UID
     *
     * @param aLocalUID Local UID
     * @return Remote UID if found, otherwise ""
     */
    QString mapToRemoteUID( const SyncItemKey& aLocalUID ) const;

    /*! \brief Loads UID mappings from persistent storage
     *
     */
    void loadUIDMappings();

    /*! \brief Returns the list of all mappings from remote UID to local UID
     *
     * @return List of mappings
     */
    const QList<UIDMapping>& getUIDMappings() const;

    /*! \brief Clears the list of all mappings from remote UID to local UID
     *
     */
    void clearUIDMappings();

    /*! \brief Saves the sync session information after successful sync
     *
     * @param aDbHandler Database handler
     * @param aSyncEndTime Time of the end of sync
     */
    void saveSession( DatabaseHandler& aDbHandler, const QDateTime& aSyncEndTime );
    
protected:

private:

    ChangeLog*          iChangeLog;

    StoragePlugin*      iPlugin;
    QString             iTargetDatabase;

    SyncMode            iSyncMode;

    QString             iLocalNextAnchor;
    QString             iRemoteNextAnchor;

    LocalChanges        iLocalChanges;
    QList<UIDMapping>   iUIDMappings;

    bool                iReverted;
    bool                iLocalChangesDiscovered;

    friend class SyncTargetTest;

};

}
#endif  //  SYNCTARGET_H
