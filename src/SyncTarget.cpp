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

#include "SyncTarget.h"

#include "ChangeLog.h"
#include "StoragePlugin.h"
#include "SyncItem.h"
#include "DatabaseHandler.h"

#include "SyncMLLogging.h"

using namespace DataSync;

SyncTarget::SyncTarget( ChangeLog* aChangeLog, StoragePlugin* aPlugin,
                        const SyncMode& aSyncMode, const QString& aLocalNextAnchor ) :
    iChangeLog( aChangeLog ),
    iPlugin( aPlugin ),
    iSyncMode( aSyncMode ),
    iLocalNextAnchor( aLocalNextAnchor ),
    iReverted( false ),
    iLocalChangesDiscovered( false )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

SyncTarget::~SyncTarget()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    delete iChangeLog;
    iChangeLog = NULL;

}

QString SyncTarget::getSourceDatabase() const
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    QString sourceDb;

    if (iPlugin != NULL) {
        sourceDb = iPlugin->getSourceURI();
    }

    return sourceDb;
}

void SyncTarget::setTargetDatabase( const QString& aTargetDatabase )
{
    iTargetDatabase = aTargetDatabase;
}

QString SyncTarget::getTargetDatabase() const
{
    return iTargetDatabase;
}

QString SyncTarget::getLocalLastAnchor() const
{
    return iChangeLog->getLastLocalAnchor();
}

QString SyncTarget::getLocalNextAnchor() const
{
    return iLocalNextAnchor;
}

QString SyncTarget::getRemoteLastAnchor() const
{
    return iChangeLog->getLastRemoteAnchor();
}

QString SyncTarget::getRemoteNextAnchor() const
{
    return iRemoteNextAnchor;
}

void SyncTarget::setRemoteNextAnchor( const QString& aRemoteNextAnchor )
{
    iRemoteNextAnchor = aRemoteNextAnchor;
}

SyncMode* SyncTarget::getSyncMode()
{
    return &iSyncMode;
}

const SyncMode* SyncTarget::getSyncMode() const
{
    return &iSyncMode;
}

void SyncTarget::setSyncMode( const SyncMode& aSyncMode )
{
    iSyncMode = aSyncMode;
}

void SyncTarget::revertSyncMode()
{
	FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( iSyncMode.syncType() == TYPE_FAST ) {
        iSyncMode.toSlowSync();
        iReverted = true;
    }
}

bool SyncTarget::setRefreshFromClient()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
    
    bool refreshSet = false;

    if( iSyncMode.syncDirection() == DIRECTION_FROM_CLIENT &&
        iSyncMode.syncInitiator() == INIT_CLIENT )
    {
        iSyncMode.setRefresh();
        refreshSet = true;
    }
  
    return refreshSet;
}

bool SyncTarget::reverted() const
{
    return iReverted;
}

bool SyncTarget::discoverLocalChanges( const Role& aRole )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( iLocalChangesDiscovered ) {
        return true;
    }

    bool success = false;

    iLocalChanges.added.clear();
    iLocalChanges.modified.clear();
    iLocalChanges.removed.clear();

    qCDebug(lcSyncML) << "Analyzing local changes";
    qCDebug(lcSyncML) << "Sync Type getting Local Changes " << iSyncMode.toSyncMLCode();

    SyncDirection direction = iSyncMode.syncDirection();

    if( direction == DIRECTION_TWO_WAY ||
        ( aRole == ROLE_CLIENT && direction == DIRECTION_FROM_CLIENT ) ||
        ( aRole == ROLE_SERVER && direction == DIRECTION_FROM_SERVER ) ) {


        if( iSyncMode.syncType() == TYPE_SLOW ) {
            qCDebug(lcSyncML) << "Slow sync mode";

            if (iPlugin != NULL) {
                success = iPlugin->getAll( iLocalChanges.added );
            }
        }
	else if( iSyncMode.syncType() == TYPE_REFRESH ) {
            qCDebug(lcSyncML) << "Refresh sync mode";
            // As server, we don't initiate a refresh sync
            if( aRole == ROLE_CLIENT && direction == DIRECTION_FROM_CLIENT ) {
                qCDebug(lcSyncML) << "We need to send all changes as a client";
                if (iPlugin != NULL) {
                    success = iPlugin->getAll( iLocalChanges.added );
                }
            }
        }
        else {
            qCDebug(lcSyncML) << "Fast sync mode";

            QDateTime time = iChangeLog->getLastSyncTime();

            qCDebug(lcSyncML) << "Getting modifications after: " << time;

            if (iPlugin != NULL) {
                if( time.toString().isEmpty() )
                {
                    qCDebug(lcSyncML) << "Getting All modifications for a 1st time fast sync req";
                    success = iPlugin->getAll( iLocalChanges.added );
                }
                else
                {
                    success = iPlugin->getModifications( iLocalChanges.added,
                                                      iLocalChanges.modified,
                                                      iLocalChanges.removed,
                                                      time );
                }
            }

        }

    }
    else {
        qCDebug(lcSyncML) << "Local changes not needed in current sync mode";
        success = true;
    }

    qCDebug(lcSyncML) << "Number of items added: " << iLocalChanges.added.count();
    qCDebug(lcSyncML) << "Number of items modified: " << iLocalChanges.modified.count();
    qCDebug(lcSyncML) << "Number of items deleted: " << iLocalChanges.removed.count();

    iLocalChangesDiscovered = success;

    return success;

}

const LocalChanges* SyncTarget::getLocalChanges() const
{
    return &iLocalChanges;
}

LocalChanges* SyncTarget::getLocalChanges() 
{
    return &iLocalChanges;
}

StoragePlugin* SyncTarget::getPlugin() const
{
    return iPlugin;
}

void SyncTarget::addUIDMapping( const UIDMapping& aMapping )
{
    iUIDMappings.append( aMapping );
}


void SyncTarget::removeUIDMapping( const SyncItemKey& aLocalKey )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    for( int i = 0; i < iUIDMappings.count(); ++i ) {
        if( iUIDMappings[i].iLocalUID == aLocalKey ) {
            iUIDMappings.removeAt( i );
            break;
        }
    }
}

SyncItemKey SyncTarget::mapToLocalUID( const QString& aRemoteKey ) const
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    SyncItemKey localUID;

    for( int i = 0; i < iUIDMappings.count(); ++i ) {
        if( iUIDMappings[i].iRemoteUID == aRemoteKey ) {
            localUID = iUIDMappings[i].iLocalUID;
            break;
        }
    }

    if( localUID.isEmpty() ) {
        qCDebug(lcSyncML) << "Warning: no existing mapping found for remote key" << aRemoteKey;
    }

    return localUID;
}

QString SyncTarget::mapToRemoteUID( const SyncItemKey& aLocalUID ) const
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    QString remoteUID;

    for( int i = 0; i < iUIDMappings.count(); ++i ) {
        if( iUIDMappings[i].iLocalUID == aLocalUID ) {
            remoteUID = iUIDMappings[i].iRemoteUID;
            break;
        }
    }
    return remoteUID;
}

void SyncTarget::loadUIDMappings()
{
    iUIDMappings = iChangeLog->getMaps();
}

const QList<UIDMapping>& SyncTarget::getUIDMappings() const
{
    return iUIDMappings;
}

void SyncTarget::clearUIDMappings()
{
    iUIDMappings.clear();
}

void SyncTarget::saveSession( DatabaseHandler& aDbHandler, const QDateTime& aSyncEndTime )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iChangeLog->setLastLocalAnchor( iLocalNextAnchor );
    iChangeLog->setLastRemoteAnchor( iRemoteNextAnchor );
    iChangeLog->setLastSyncTime( aSyncEndTime );
    iChangeLog->setMaps( iUIDMappings );

    if( !iChangeLog->save( aDbHandler.getDbHandle() ) ) {
        qCWarning(lcSyncML) << "Could not save information to persistent storage!";
    }

}

