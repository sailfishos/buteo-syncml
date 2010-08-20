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

#include "SuspendLog.h"

#include "LogMacros.h"

using namespace DataSync;

SuspendLog::SuspendLog( QSqlDatabase& aDbHandle, const QString& aRemoteDevice,
                        const QString& aSourceDbURI, const QString& aTargetDbURI )
 : iDbHandle( aDbHandle ), iRemoteDevice( aRemoteDevice ),
   iSourceDbURI( aSourceDbURI ), iTargetDbURI( aTargetDbURI )
{
    FUNCTION_CALL_TRACE;
}

SuspendLog::~SuspendLog()
{
    FUNCTION_CALL_TRACE;
}

/*
bool DataSync::ChangeLogHandler::createItemTrackingTable()
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "CREATE TABLE if not exists localchanges(id integer primary key autoincrement, remote_device varchar(512), local_database varchar(512), remote_database varchar(512), syncitemkey varchar(512), syncitemcmd integer)";

    QSqlQuery query(queryString, iDbHandle);
    query.exec();

    bool success = true;

    if (query.lastError().isValid()) {
        LOG_WARNING("Error found: " << query.lastError());
        success = false;
    }

    return success;
}

bool DataSync::ChangeLogHandler::createMappingTrackingTable()
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "CREATE TABLE if not exists localmappings(id integer primary key autoincrement, remote_device varchar(512), local_database varchar(512), remote_database varchar(512), localkey varchar(512), remotekey varchar(512), msgref integer, cmdref  interger)";

    QSqlQuery query(queryString, iDbHandle);
    query.exec();

    bool success = true;

    if (query.lastError().isValid()) {
        LOG_WARNING("Error found: " << query.lastError());
        success = false;
    }

    return success;
}
void DataSync::ChangeLog::addPendingSyncItem(QString aRemoteDevice,
                                             QString aLocalDatabase,
                                             QString aRemoteDatabase,
                                             SyncItemKey aSyncItemKey,
                                             SyncMLCommand aSyncCmd)
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "INSERT INTO localchanges(remote_device, local_database, remote_database, syncitemkey, syncitemcmd) VALUES (:remote_device, :local_database, :remote_database, :syncitemkey, :syncitemcmd)";
    QSqlQuery query(iDbHandle);
    query.prepare( queryString );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.bindValue(":syncitemkey", aSyncItemKey);
    query.bindValue(":syncitemcmd", aSyncCmd);
    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
    }
}

void DataSync::ChangeLog::removePendingSyncItem(QString aRemoteDevice,
                                                QString aLocalDatabase,
                                                QString aRemoteDatabase,
                                                SyncItemKey aSyncItemKey)
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "DELETE FROM localchanges WHERE remote_device = :remote_device AND local_database = :local_database AND remote_database = :remote_database AND syncitemkey = :syncitemkey";
    QSqlQuery query(iDbHandle);
    query.prepare( queryString );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.bindValue(":syncitemkey", aSyncItemKey);
    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
    }
}

QList<SyncItemKey> DataSync::ChangeLog::getPendingLocalChanges(QString aRemoteDevice,
                                                               QString aLocalDatabase,
                                                               QString aRemoteDatabase,
                                                               SyncMLCommand aSyncCmd)
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "SELECT syncitemkey FROM localchanges WHERE remote_device = :remote_device AND local_database = :local_database AND remote_database = :remote_database AND syncitemcmd = :syncitemcmd" );
    QSqlQuery query( iDbHandle );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.bindValue(":syncitemcmd", aSyncCmd);
    query.prepare( queryString );

    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG(query.lastError());
    }

    SyncItemKey syncItemKey = "";
    QList<SyncItemKey> changelist;

    while(query.next()) {
        syncItemKey = query.value(0).toString();
        changelist.append(syncItemKey);
    }

    return changelist;
}

int DataSync::ChangeLog::getLastSyncMode()
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "SELECT sync_mode FROM change_logs WHERE id=:id" );
    QSqlQuery query( iDbHandle );

    query.prepare( queryString );
    query.bindValue(":id", iChangeLogId);
    query.exec();

    int syncMode;

    if( query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
        syncMode = 0;
    }
    else if (query.next()) {
        syncMode = query.value(0).toInt();
    }
    else {
        LOG_DEBUG( "Could not find sync mode" );
        syncMode = 0;
    }

    return syncMode;
}

void DataSync::ChangeLog::addPendingMapping(QString aRemoteDevice,
                                            QString aLocalDatabase,
                                            QString aRemoteDatabase,
                                            SyncItemKey aLocalKey,
                                            SyncItemKey aRemoteKey,
                                            int aMsgRef,
                                            int aCmdRef)
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "INSERT INTO localmappings(remote_device, local_database, remote_database, localkey, remotekey, msgref, cmdref) VALUES (:remote_device, :local_database, :remote_database, :localkey, :remotekey, :msgref, :cmdref)";
    QSqlQuery query(iDbHandle);
    query.prepare( queryString );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.bindValue(":localkey", aLocalKey);
    query.bindValue(":remotekey", aRemoteKey);
    query.bindValue(":msgref", aMsgRef);
    query.bindValue(":cmdref", aCmdRef);
    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
    }
}

void DataSync::ChangeLog::removePendingMappings(QString aRemoteDevice,
                                                QString aLocalDatabase,
                                                QString aRemoteDatabase,
                                                int aMsgRef,
                                                int aCmdRef)
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "DELETE FROM localmappings WHERE remote_device = :remote_device AND local_database = :local_database AND remote_database = :remote_database AND msgref = :msgref AND cmdref = :cmdref";

    QSqlQuery query(iDbHandle);
    query.prepare( queryString );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.bindValue(":msgref", aMsgRef);
    query.bindValue(":cmdref", aCmdRef);
    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
    }
}

QList<UIDMapping> DataSync::ChangeLog::getPendingLocalMappings(QString aRemoteDevice,
                                                               QString aLocalDatabase,
                                                               QString aRemoteDatabase)
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "SELECT localkey, remotekey FROM localmappings WHERE remote_device = :remote_device AND local_database = :local_database AND remote_database = :remote_database" );
    QSqlQuery query( iDbHandle );
    query.bindValue(":remote_device", aRemoteDevice);
    query.bindValue(":local_database", aLocalDatabase);
    query.bindValue(":remote_database", aRemoteDatabase);
    query.prepare( queryString );

    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG(query.lastError());
    }

    SyncItemKey localKey;
    SyncItemKey remoteKey;
    QList<UIDMapping> mappingList;

    while(query.next()) {
        localKey = query.value(0).toString();
        remoteKey = query.value(1).toString();
        UIDMapping mapping = {remoteKey, localKey};
        mappingList.append( mapping );
    }

    return mappingList;
}

bool DataSync::ChangeLog::getIntentionallySuspended() const
{
    FUNCTION_CALL_TRACE;

    bool intentionalSuspension = false;

    const QString queryString( "SELECT intentional_suspension FROM change_logs WHERE id=:id" );
    QSqlQuery query( iDbHandle );

    query.prepare( queryString );
    query.bindValue(":id", iChangeLogId);
    query.exec();

    if (query.lastError().isValid()) {
        LOG_DEBUG("Query Error" << query.lastError());
    }
    else {
        if (query.next()) {
            intentionalSuspension = query.value(0).toInt();
        }
    }

    return intentionalSuspension;
}

void ChangeLog::setIntentionallySuspended( bool aIntentionallySuspended)
{
    FUNCTION_CALL_TRACE;

    const QString queryString( "UPDATE change_logs SET intentional_suspension=:intentional_suspension WHERE id =:id" );

    QSqlQuery query(iDbHandle);

    query.prepare( queryString );
    query.bindValue(":id", iChangeLogId);
    query.bindValue(":intentional_suspension", aIntentionallySuspended);
    query.exec();


    if (query.lastError().isValid()) {
        LOG_DEBUG(query.lastError());
    }

}
*/
