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

#include "ChangeLog.h"

#include <QtSql>

#include "LogMacros.h"

using namespace DataSync;

ChangeLog::ChangeLog( const QString& aRemoteDevice, const QString& aSourceDbURI,
                      SyncDirection aSyncDirection )
: iRemoteDevice( aRemoteDevice ), iSourceDbURI( aSourceDbURI ), iSyncDirection( aSyncDirection )

{
    FUNCTION_CALL_TRACE;
}

ChangeLog::~ChangeLog()
{
    FUNCTION_CALL_TRACE;
}

bool ChangeLog::load( QSqlDatabase& aDbHandle )
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Loading changelog information:");
    LOG_DEBUG( "Remote device:" << iRemoteDevice );
    LOG_DEBUG( "Database URI:" << iSourceDbURI );
    LOG_DEBUG( "Sync direction:" << iSyncDirection );

    return ( loadAnchors( aDbHandle ) && loadMaps( aDbHandle ) );
}

bool ChangeLog::load( const QString& aDbName )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    QString connectionName = generateConnectionName();
    QSqlDatabase database = QSqlDatabase::addDatabase( "QSQLITE", connectionName );
    database.setDatabaseName( aDbName );

    if( database.open() )
    {
        success = load( database );
        database.close();
    }
    else
    {
        LOG_CRITICAL( "Could not open database!" );
    }

    database = QSqlDatabase();
    QSqlDatabase::removeDatabase( connectionName );

    return success;
}

bool ChangeLog::save( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Saving changelog information:");
    LOG_DEBUG( "Remote device:" << iRemoteDevice );
    LOG_DEBUG( "Database URI:" << iSourceDbURI );
    LOG_DEBUG( "Sync direction:" << iSyncDirection );

    if( !ensureAnchorDatabase( aDbHandle ) || !ensureMapsDatabase( aDbHandle ) )
    {
        return false;
    }

    bool transaction = aDbHandle.transaction();

    bool success = ( saveAnchors( aDbHandle ) && saveMaps( aDbHandle ) );

    if( transaction && ( !success || !aDbHandle.commit() ) ) {
        success = false;
        aDbHandle.rollback();
    }

    return success;

}

bool ChangeLog::save( const QString& aDbName )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    QString connectionName = generateConnectionName();
    QSqlDatabase database = QSqlDatabase::addDatabase( "QSQLITE", connectionName );
    database.setDatabaseName( aDbName );

    if( database.open() )
    {
        success = save( database );
        database.close();
    }
    else
    {
        LOG_CRITICAL( "Could not open database!" );
    }

    database = QSqlDatabase();
    QSqlDatabase::removeDatabase( connectionName );

    return success;
}

bool ChangeLog::remove( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Removing changelog information:");
    LOG_DEBUG( "Remote device:" << iRemoteDevice );
    LOG_DEBUG( "Database URI:" << iSourceDbURI );
    LOG_DEBUG( "Sync direction:" << iSyncDirection );

    return ( removeAnchors( aDbHandle ) && removeMaps( aDbHandle ) );
}

bool ChangeLog::remove( const QString& aDbName )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    QString connectionName = generateConnectionName();
    QSqlDatabase database = QSqlDatabase::addDatabase( "QSQLITE", connectionName );
    database.setDatabaseName( aDbName );

    if( database.open() )
    {
        success = remove( database );
        database.close();
    }
    else
    {
        LOG_CRITICAL( "Could not open database!" );
    }

    database = QSqlDatabase();
    QSqlDatabase::removeDatabase( connectionName );

    return success;
}

const QString& ChangeLog::getLastLocalAnchor() const
{
    return iLastLocalAnchor;
}

void ChangeLog::setLastLocalAnchor( const QString& aLastLocalAnchor )
{
    iLastLocalAnchor = aLastLocalAnchor;
}

const QString& ChangeLog::getLastRemoteAnchor() const
{
    return iLastRemoteAnchor;
}

void ChangeLog::setLastRemoteAnchor( const QString& aLastRemoteAnchor )
{
    iLastRemoteAnchor = aLastRemoteAnchor;
}

const QDateTime& ChangeLog::getLastSyncTime() const
{
    return iLastSyncTime;
}

void ChangeLog::setLastSyncTime( const QDateTime& aLastSyncTime )
{
    iLastSyncTime = aLastSyncTime;
}

const QList<UIDMapping>& ChangeLog::getMaps() const
{
    return iMaps;
}

void ChangeLog::setMaps( const QList<UIDMapping>& aMaps )
{
    iMaps = aMaps;
}

QString ChangeLog::generateConnectionName()
{
    FUNCTION_CALL_TRACE;

    const QString CONNECTIONNAME( "changelog" );

    static unsigned connectionNumber = 0;

    return CONNECTIONNAME + QString::number( connectionNumber++ );

}

bool ChangeLog::ensureAnchorDatabase( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "CREATE TABLE if not exists change_logs(id integer primary key autoincrement, remote_device varchar(512), source_db_uri varchar(512), sync_direction INTEGER, local_sync_anchor varchar(128), remote_sync_anchor varchar(128),  last_sync_time timestamp)" );

    QSqlQuery query( queryString, aDbHandle );

    if( query.exec() ) {
        return true;
    }
    else {
        LOG_CRITICAL("Could not ensure anchor database table:" << query.lastError() );
        return false;
    }


}

bool ChangeLog::ensureMapsDatabase( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "CREATE TABLE IF NOT EXISTS id_maps(id integer primary key autoincrement, remote_device varchar(512), source_db_uri varchar(512), sync_direction INTEGER, local_id varchar(128), remote_id varchar(128))" );

    QSqlQuery query( queryString, aDbHandle );

    if( query.exec() ) {
        return true;
    }
    else {
        LOG_CRITICAL("Could not ensure ID maps database table:" << query.lastError() );
        return false;
    }

}

bool ChangeLog::loadAnchors( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    bool loaded = false;

    const QString queryString( "SELECT local_sync_anchor, remote_sync_anchor, last_sync_time FROM change_logs WHERE remote_device = :remote_device AND source_db_uri = :source_db_uri AND sync_direction = :sync_direction" );

    QSqlQuery query( queryString, aDbHandle );
    query.bindValue( ":remote_device", iRemoteDevice );
    query.bindValue( ":source_db_uri", iSourceDbURI );
    query.bindValue( ":sync_direction", iSyncDirection );

    if( query.exec() ) {

        if( query.next() ) {
            iLastLocalAnchor = query.value(0).toString();
            iLastRemoteAnchor = query.value(1).toString();
            iLastSyncTime = query.value(2).toDateTime();

            LOG_DEBUG( "Found anchor information:" );
            LOG_DEBUG( "Last local anchor:" << iLastLocalAnchor );
            LOG_DEBUG( "Last remote anchor:" << iLastRemoteAnchor );
            LOG_DEBUG( "Sync session end time:" << iLastSyncTime );

            loaded = true;

        }
        else {
            LOG_DEBUG( "No existing anchor entry found from database, creating new" );
        }

    }
    else {
        LOG_WARNING( "Could not load anchors:" << query.lastError() );
    }

    return loaded;
}

bool ChangeLog::saveAnchors( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    if( removeAnchors( aDbHandle ) ) {

        const QString queryString( "INSERT INTO change_logs(remote_device, source_db_uri, sync_direction, local_sync_anchor, remote_sync_anchor, last_sync_time) VALUES (:remote_device, :source_db_uri, :sync_direction, :local_sync_anchor, :remote_sync_anchor, :last_sync_time)" );

        QSqlQuery query( queryString, aDbHandle );
        query.bindValue( ":remote_device", iRemoteDevice );
        query.bindValue( ":source_db_uri", iSourceDbURI );
        query.bindValue( ":sync_direction", iSyncDirection );
        query.bindValue( ":local_sync_anchor", iLastLocalAnchor );
        query.bindValue( ":remote_sync_anchor", iLastRemoteAnchor );
        query.bindValue( ":last_sync_time", iLastSyncTime );

        if( query.exec() ) {
            LOG_DEBUG( "Anchor information saved:" );
            LOG_DEBUG( "Last local anchor:" << iLastLocalAnchor );
            LOG_DEBUG( "Last remote anchor:" << iLastRemoteAnchor );
            LOG_DEBUG( "Sync session end time:" << iLastSyncTime );

            success = true;
        }
        else {
            LOG_CRITICAL( "Could not save anchors:" << query.lastError() );
        }

    }
    else {
        LOG_CRITICAL( "Could not save anchors as database cleaning failed" );
    }

    return success;

}

bool ChangeLog::removeAnchors( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    const QString queryString( "DELETE FROM change_logs WHERE remote_device = :remote_device AND source_db_uri = :source_db_uri AND sync_direction = :sync_direction" );

    QSqlQuery query( queryString, aDbHandle );
    query.bindValue( ":remote_device", iRemoteDevice );
    query.bindValue( ":source_db_uri", iSourceDbURI );
    query.bindValue( ":sync_direction", iSyncDirection );


    if( query.exec() ) {
        success = true;
    }
    else {
        LOG_WARNING( "Could not remove anchors:" << query.lastError() );
    }

    return success;
}

bool ChangeLog::loadMaps( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    bool loaded = false;

    const QString queryString("SELECT local_id, remote_id FROM id_maps WHERE remote_device = :remote_device AND source_db_uri = :source_db_uri AND sync_direction = :sync_direction" );

    QSqlQuery query( queryString, aDbHandle );
    query.bindValue( ":remote_device", iRemoteDevice );
    query.bindValue( ":source_db_uri", iSourceDbURI );
    query.bindValue( ":sync_direction", iSyncDirection );

    if( query.exec() )
    {
        iMaps.clear();

        while( query.next() )
        {
            UIDMapping mapping;
            mapping.iLocalUID = query.value(0).toString();
            mapping.iRemoteUID = query.value(1).toString();
            iMaps.append( mapping );
        }

        loaded = true;
    }
    else
    {
        LOG_CRITICAL( "Could not load ID maps:" << query.lastError() );
    }

    return loaded;
}

bool ChangeLog::saveMaps( QSqlDatabase& aDbHandle )
{

    FUNCTION_CALL_TRACE;

    bool success = false;

    if( removeMaps( aDbHandle ) )
    {
        const QString queryString( "INSERT INTO id_maps(remote_device, source_db_uri, sync_direction, local_id, remote_id) values(:remote_device, :source_db_uri, :sync_direction, :local_id, :remote_id)" );

        QSqlQuery query( queryString, aDbHandle );

        QVariantList device;
        QVariantList sourceDbURI;
        QVariantList syncDirection;
        QVariantList localId;
        QVariantList remoteId;

        for( int i = 0; i < iMaps.count(); ++i ) {
            device << iRemoteDevice;
            sourceDbURI << iSourceDbURI;
            syncDirection << iSyncDirection;
            localId << iMaps[i].iLocalUID;
            remoteId << iMaps[i].iRemoteUID;
        }

        query.addBindValue( device );
        query.addBindValue( sourceDbURI );
        query.addBindValue( syncDirection );
        query.addBindValue( localId );
        query.addBindValue( remoteId );

        if( query.execBatch() ) {
            LOG_DEBUG( "ID maps information saved" );
            success = true;
        }
        else {
            LOG_WARNING( "Query failed: " << query.lastError() );
        }
    }
    else
    {
        LOG_CRITICAL( "Could not save ID maps as database cleaning failed" );
    }

    return success;
}

bool ChangeLog::removeMaps( QSqlDatabase& aDbHandle )
{
    FUNCTION_CALL_TRACE;

    bool success = false;

    const QString queryString( "DELETE FROM id_maps WHERE remote_device = :remote_device AND source_db_uri = :source_db_uri AND sync_direction = :sync_direction" );

    QSqlQuery query( queryString, aDbHandle );
    query.bindValue( ":remote_device", iRemoteDevice );
    query.bindValue( ":source_db_uri", iSourceDbURI );
    query.bindValue( ":sync_direction", iSyncDirection );

    if( query.exec() ) {
        success = true;
    }
    else {
        LOG_WARNING( "Could not remove ID maps:" << query.lastError() );
    }

    return success;
}
