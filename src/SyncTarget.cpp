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

#include "LogMacros.h"

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
    FUNCTION_CALL_TRACE;
}

SyncTarget::~SyncTarget()
{
    FUNCTION_CALL_TRACE

    delete iChangeLog;
    iChangeLog = NULL;

}

QString SyncTarget::getSourceDatabase() const
{
    FUNCTION_CALL_TRACE

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
	FUNCTION_CALL_TRACE

    if( iSyncMode.syncType() == TYPE_FAST ) {
        iSyncMode.toSlowSync();
        iReverted = true;
    }
}

bool SyncTarget::setRefreshFromClient()
{
    FUNCTION_CALL_TRACE
    
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
    FUNCTION_CALL_TRACE

    if( iLocalChangesDiscovered ) {
        return true;
    }

    bool success = false;

    iLocalChanges.added.clear();
    iLocalChanges.modified.clear();
    iLocalChanges.removed.clear();

    LOG_DEBUG("Analyzing local changes");
    LOG_DEBUG("Sync Type getting Local Changes " << iSyncMode.toSyncMLCode());

    SyncDirection direction = iSyncMode.syncDirection();

    if( direction == DIRECTION_TWO_WAY ||
        ( aRole == ROLE_CLIENT && direction == DIRECTION_FROM_CLIENT ) ||
        ( aRole == ROLE_SERVER && direction == DIRECTION_FROM_SERVER ) ) {


        if( iSyncMode.syncType() == TYPE_SLOW ) {
            LOG_DEBUG("Slow sync mode");

            if (iPlugin != NULL) {
                success = iPlugin->getAll( iLocalChanges.added );
            }
        }
	else if( iSyncMode.syncType() == TYPE_REFRESH ) {
            LOG_DEBUG("Refresh sync mode");
            // As server, we don't initiate a refresh sync
            if( aRole == ROLE_CLIENT && direction == DIRECTION_FROM_CLIENT ) {
                LOG_DEBUG("We need to send all changes as a client");
                if (iPlugin != NULL) {
                    success = iPlugin->getAll( iLocalChanges.added );
                }
            }
        }
        else {
            LOG_DEBUG("Fast sync mode");

            QDateTime time = iChangeLog->getLastSyncTime();

            LOG_DEBUG("Getting modifications after: " << time);

            if (iPlugin != NULL) {
                if( time.toString().isEmpty() )
                {
                    LOG_DEBUG("Getting All modifications for a 1st time fast sync req");
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
        LOG_DEBUG("Local changes not needed in current sync mode");
        success = true;
    }

    LOG_DEBUG("Number of items added: " << iLocalChanges.added.count());
    LOG_DEBUG("Number of items modified: " << iLocalChanges.modified.count());
    LOG_DEBUG("Number of items deleted: " << iLocalChanges.removed.count());

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
    FUNCTION_CALL_TRACE

    for( int i = 0; i < iUIDMappings.count(); ++i ) {
        if( iUIDMappings[i].iLocalUID == aLocalKey ) {
            iUIDMappings.removeAt( i );
            break;
        }
    }
}

SyncItemKey SyncTarget::mapToLocalUID( const QString& aRemoteKey ) const
{
    FUNCTION_CALL_TRACE

    SyncItemKey localUID;

    for( int i = 0; i < iUIDMappings.count(); ++i ) {
        if( iUIDMappings[i].iRemoteUID == aRemoteKey ) {
            localUID = iUIDMappings[i].iLocalUID;
            break;
        }
    }

    if( localUID.isEmpty() ) {
        LOG_DEBUG( "Warning: no existing mapping found for remote key" << aRemoteKey );
    }

    return localUID;
}

QString SyncTarget::mapToRemoteUID( const SyncItemKey& aLocalUID ) const
{
    FUNCTION_CALL_TRACE

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
    FUNCTION_CALL_TRACE

    iChangeLog->setLastLocalAnchor( iLocalNextAnchor );
    iChangeLog->setLastRemoteAnchor( iRemoteNextAnchor );
    iChangeLog->setLastSyncTime( aSyncEndTime );
    iChangeLog->setMaps( iUIDMappings );

    if( !iChangeLog->save( aDbHandler.getDbHandle() ) ) {
        LOG_WARNING( "Could not save information to persistent storage!");
    }

}

