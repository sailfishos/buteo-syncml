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

#ifndef SYNCRESULTS_H
#define SYNCRESULTS_H

#include "SyncAgentConsts.h"
#include <QMap>
#include <QString>

namespace DataSync {

/*! \brief Describes details of items transferred between single databases
 *
 */
struct DatabaseResults {

    int     iLocalItemsAdded;       /*!<The number of items added to the local database*/
    int     iLocalItemsModified;    /*!<The number of items updated in the local database*/
    int     iLocalItemsDeleted;     /*!<The number of items deleted from the local database*/

    int     iRemoteItemsAdded;      /*!<The number of items added to the remote database*/
    int     iRemoteItemsModified;   /*!<The number of items updated in the remote database*/
    int     iRemoteItemsDeleted;    /*!<The number of items deleted from the remote database*/

    DatabaseResults() : iLocalItemsAdded( 0 ), iLocalItemsModified( 0 ), iLocalItemsDeleted( 0 ),
                        iRemoteItemsAdded( 0 ), iRemoteItemsModified( 0 ), iRemoteItemsDeleted( 0 ) { }

};

/*! \brief Class for retrieving results of sync session
 *
 */
class SyncResults {

public:

    /*! \brief Constructor
     *
     */
    SyncResults();

    /*! \brief Destructor
     *
     */
    ~SyncResults();

    /*! \brief Resets sync results
     *
     */
    void reset();

    /*! \brief Returns the resulting state of sync session
     *
     * @return Sync status
     */
    SyncState getState() const;

    /*! \brief Sets the resulting state of sync session
     *
     * @param aState Resulting state
     */
    void setState( const SyncState& aState );

    /*! \brief Return an error string associated to a possible error status
     *
     * @return Error string
     */
    QString getErrorString() const;

    /*! \brief Sets an error string associated to a possible error status
     *
     * @param aErrorString Error string
     */
    void setErrorString( const QString& aErrorString );

    /*! \brief Sets remote device IMEI
     *
     * @param aDeviceId Remote Device IMEI string
     */
    void setRemoteDeviceId(const QString& aDeviceId );

    /*! \brief Gets remote device IMEI
     *
     * @return Target Device Id string
     */
    QString getRemoteDeviceId( ) const;

    /*! \brief Returns database results of the sync
     *
     * @return Database results
     */
    const QMap<QString, DatabaseResults>* getDatabaseResults() const;

    /*! \brief Adds a processed item to database results
     *
     * @param aModificationType Type of modification made to the item (addition, modification or delete)
     * @param aModifiedDatabase Database that was modified (local or remote)
     * @param aDatabase Identifier of the database that was modified
     */
    void addProcessedItem( DataSync::ModificationType aModificationType,
                           DataSync::ModifiedDatabase aModifiedDatabase,
                           const QString& aDatabase );

private:

    SyncState                       iState;
    QString                         iErrorString;
    QString                         iRemoteId;
    QMap<QString, DatabaseResults>  iResults;

};

}

#endif // SYNCRESULTS_H
