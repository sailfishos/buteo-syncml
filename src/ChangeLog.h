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

#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <QString>
#include <QDateTime>

#include "SyncAgentConsts.h"
#include "SyncMLGlobals.h"

class QSqlDatabase;

namespace DataSync {

/*! \brief  Stores the sync anchors and changes of different sync profiles.
 *
 */
class ChangeLog
{

public:


    /*! \brief Constructor
     *
     * @param aRemoteDevice Remote device
     * @param aSourceDbURI URI of the local database
     * @param aSyncDirection Direction of the sync
     */
    ChangeLog( const QString& aRemoteDevice, const QString& aSourceDbURI,
               SyncDirection aSyncDirection );

    /*! \brief Destructor
     *
     */
    ~ChangeLog();

    /*! \brief Loads change log information from database
     *
     * @param aDbHandle Database handle to use
     * @return True on success, otherwise false
     */
    bool load( QSqlDatabase& aDbHandle );

    /*! \brief Loads change log information from database
     *
     * @param aDbName Name of the database file to open
     * @return True on success, otherwise false
     */
    bool load( const QString& aDbName );

    /*! \brief Saves change log information to database
     *
     * @param aDbHandle Database handle to use
     * @return True on success, otherwise false
     */
    bool save( QSqlDatabase& aDbHandle );

    /*! \brief Saves change log information to database
     *
     * @param aDbName Name of the database file to open
     * @return True on success, otherwise false
     */
    bool save( const QString& aDbName );

    /*! \brief Removes change log information from database
     *
     * @param aDbHandle Database handle to use
     * @return True on success, otherwise false
     */
    bool remove( QSqlDatabase& aDbHandle );

    /*! \brief Removes change log information from database
     *
     * @param aDbName Name of the database file to open
     * @return True on success, otherwise false
     */
    bool remove( const QString& aDbName );

    /*! \brief Gets the last local anchor of previous successful synchronization
     *
     * @return Last local anchor if found, otherwise empty
     */
    const QString& getLastLocalAnchor() const;

    /*! \brief Sets the last local anchor
     *
     * @param aLastLocalAnchor Last local anchor to set
     */
    void setLastLocalAnchor( const QString& aLastLocalAnchor );

    /*! \brief Gets the last remote anchor of previous successful synchronization
     *
     * @return Last remote anchor if found, otherwise empty
     */
    const QString& getLastRemoteAnchor() const;

    /*! \brief Sets the last remote anchor
     *
     * @param aLastRemoteAnchor Last remote anchor to set
     */
    void setLastRemoteAnchor( const QString& aLastRemoteAnchor );

    /*! \brief Returns the time when last sync was completed
     *
     * @return Last sync time
     */
    const QDateTime& getLastSyncTime() const;

    /*! \brief Stores the time when last sync was completed
     *
     * @param aLastSyncTime Last sync time
     * @return Operation status code
     */
    void setLastSyncTime( const QDateTime& aLastSyncTime );

    /*! \brief Returns the ID mappings associated with this ChangeLog
     *
     * @return
     */
    const QList<UIDMapping>& getMaps() const;

    /*! \brief Sets the ID mappings associated with this ChangeLog
     *
     * @return
     */
    void setMaps( const QList<UIDMapping>& aMaps );

private:

    QString generateConnectionName();

    bool ensureAnchorDatabase( QSqlDatabase& aDbHandle );
    bool ensureMapsDatabase( QSqlDatabase& aDbHandle );

    bool loadAnchors( QSqlDatabase& aDbHandle );
    bool saveAnchors( QSqlDatabase& aDbHandle );
    bool removeAnchors( QSqlDatabase& aDbHandle );

    bool loadMaps( QSqlDatabase& aDbHandle );
    bool saveMaps( QSqlDatabase& aDbHandle );
    bool removeMaps( QSqlDatabase& aDbHandle );

    QString             iRemoteDevice;
    QString             iSourceDbURI;
    SyncDirection       iSyncDirection;

    QString             iLastLocalAnchor;
    QString             iLastRemoteAnchor;
    QDateTime           iLastSyncTime;
    QList<UIDMapping>   iMaps;


};

}

#endif
