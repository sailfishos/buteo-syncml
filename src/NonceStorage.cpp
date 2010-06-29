/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "NonceStorage.h"

#include <QtSql>

#include "LogMacros.h"

using namespace DataSync;

NonceStorage::NonceStorage( QSqlDatabase& aDbHandle )
 : iDbHandle( aDbHandle )
{

}

NonceStorage::~NonceStorage()
{

}

bool NonceStorage::createNonceTable()
{
    FUNCTION_CALL_TRACE;

    const QString queryString = "CREATE TABLE IF NOT EXISTS nonces(id integer primary key autoincrement, local_device varchar(512), remote_device varchar(512), nonce varchar(512))";
    QSqlQuery query( iDbHandle );

    query.prepare( queryString );
    query.exec();

    bool success = true;

    if (query.lastError().isValid()) {
        success = false;
        LOG_WARNING("Query failed: " << query.lastError());
    }

    return success;
}

QByteArray NonceStorage::generateNonce() const
{
    return QByteArray::number( QDateTime::currentDateTime().toTime_t() );
}

void NonceStorage::addNonce( const QString& aLocalDevice, const QString& aRemoteDevice, const QByteArray& aNonce )
{
    FUNCTION_CALL_TRACE;

    clearNonce( aLocalDevice, aRemoteDevice );

    const QString insertQuery( "INSERT INTO nonces(local_device, remote_device, nonce) values(:local_device, :remove_device, :nonce)" );

    QSqlQuery query( iDbHandle );

    query.prepare( insertQuery );
    query.bindValue( ":local_device", aLocalDevice );
    query.bindValue( ":remote_device", aRemoteDevice );
    query.bindValue( ":nonce", aNonce );
    query.exec();

    if( query.lastError().isValid() ) {
        LOG_WARNING("Query failed: " << query.lastError());
    }

}

QByteArray NonceStorage::retrieveNonce( const QString& aLocalDevice, const QString& aRemoteDevice )
{
    FUNCTION_CALL_TRACE;

    QByteArray nonce;

    const QString queryString( "SELECT nonce FROM nonces WHERE local_device = :local_device AND remote_device = :remote_device" );
    QSqlQuery query( iDbHandle );

    query.prepare( queryString );
    query.bindValue( ":local_device", aLocalDevice );
    query.bindValue( ":remote_device", aRemoteDevice );
    query.exec();


    if (query.lastError().isValid()) {
        LOG_WARNING("Query Error" << query.lastError());
    }
    else {
        if (query.next()) {
            LOG_DEBUG("Query Successful");
            nonce = query.value(0).toByteArray();
        }

    }

    return nonce;
}

void NonceStorage::clearNonce( const QString& aLocalDevice, const QString& aRemoteDevice )
{
    FUNCTION_CALL_TRACE;

    // Clear existing mappings
    const QString deleteQuery( "DELETE FROM nonces WHERE local_device = :local_device AND remote_device = :remote_device" );

    QSqlQuery query( iDbHandle );

    query.prepare( deleteQuery );
    query.bindValue( ":local_device", aLocalDevice );
    query.bindValue( ":remote_device", aRemoteDevice );
    query.exec();

    if( query.lastError().isValid() ) {
        LOG_WARNING("Query failed: " << query.lastError());
    }
}
