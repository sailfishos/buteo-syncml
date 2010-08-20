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
#ifndef SYNCAGENTCONSTS_H
#define SYNCAGENTCONSTS_H

#include <QMetaType>

namespace DataSync {

/*! \brief SyncML protocol version to use
 *
 */
enum ProtocolVersion {
    DS_1_2,         /*!<SyncML 1.2*/
    DS_1_1,         /*!<SyncML 1.1*/
    DS_UNKNOWN      /*!<Protocol is unknown (can be used as initialization value)*/
};

/*! \brief Direction of sync
 *
 */
enum SyncDirection {
    DIRECTION_TWO_WAY,        /*!< Sync is to both directions*/
    DIRECTION_FROM_CLIENT,    /*!< Sync is from client only*/
    DIRECTION_FROM_SERVER     /*!< Sync is from server only*/
};

/*! \brief Side that initiates the sync
 *
 */
enum SyncInitiator {
    INIT_CLIENT,    /*!< Sync will be initiated by client*/
    INIT_SERVER,    /*!< Sync will be initiated by server*/
    INIT_UNKNOWN    /*!< Sync initiator is unknown (can be used as initialization value)*/
};

/*! \brief Type of sync
 *
 */
enum SyncType {
    TYPE_SLOW,      /*!< Slow sync*/
    TYPE_FAST,      /*!< Fast sync*/
    TYPE_REFRESH,   /*!< Refresh sync*/
};

/*! \brief SyncML authentication to use
 *
 */
enum AuthenticationType {
    AUTH_NONE,      /*!<No authentication should be attempted*/
    AUTH_BASIC,     /*!<Use SyncML Basic authentication*/
    AUTH_MD5        /*!<Use SyncML MD5 authentication*/
};

/*! \brief Conflict resolution policy to enforce
 *
 * It should be noted that conflict resolution is performed by local device only when it is
 * acting as server. If local device is acting as a client, remote device will be acting as
 * a server, therefore reinforcing it's own resolution policy.
 */
enum ConflictResolutionPolicy {
    PREFER_LOCAL_CHANGES    = 0,  /*!<Prefer modifications of this device over the remote device*/
    PREFER_REMOTE_CHANGES   = 1   /*!<Prefer modifications of remote device over this device*/
};

/*! \brief Indicates the overall synchronization process state
 */
enum SyncState {

    NOT_PREPARED = 101,
    /*
     * These status messages are returned as a parameter of statusChanged signal
     */

    // Progress 2xx
    FIRST_PROGRESS_STATUS = 200,
    PREPARED,
    LOCAL_INIT,
    REMOTE_INIT,
    SENDING_ITEMS,
    RECEIVING_ITEMS,
    SENDING_MAPPINGS,
    RECEIVING_MAPPINGS,
    FINALIZING,

    SUSPENDING,
    LAST_PROGRESS_STATUS = 299,

    // Successful 3xx
    SYNC_FINISHED = 301,

    // Client/Configuration errors 4xx
    INTERNAL_ERROR = 401,
    AUTHENTICATION_FAILURE,
    DATABASE_FAILURE,

    // Server/Network errors 5xx
    SUSPENDED = 501,
    ABORTED,
    CONNECTION_ERROR,
    INVALID_SYNCML_MESSAGE,
    UNSUPPORTED_SYNC_TYPE,
    UNSUPPORTED_STORAGE_TYPE

};

/*! \brief Type of modification made to database
 *
 */
enum ModificationType {
    MOD_ITEM_ADDED,     /*!<Item was added to database*/
    MOD_ITEM_MODIFIED,  /*!<Item in the database was modified*/
    MOD_ITEM_DELETED,   /*!<Item in the database was deleted*/
    MOD_ITEM_ERROR      /*!<Item was being processed, but error occurred*/
};

/*! \brief Database that has been modified
 *
 */
enum ModifiedDatabase {
    MOD_LOCAL_DATABASE, /*!<Modification was done to local database*/
    MOD_REMOTE_DATABASE /*!<Modification was done to remote database*/
};

}
Q_DECLARE_METATYPE( DataSync::SyncState );
Q_DECLARE_METATYPE( DataSync::ModificationType );
Q_DECLARE_METATYPE( DataSync::ModifiedDatabase );

#endif  //  SYNCAGENTCONSTS_H
