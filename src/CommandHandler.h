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

#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <QMap>

#include "SyncMLGlobals.h"
#include "internals.h"
#include "SyncAgentConsts.h"
#include "StorageHandler.h"

namespace DataSync {

/*! \brief SyncML Status code groups
 *
 */
enum StatusCodeType {
    UNKNOWN,
    INFORMATIONAL,
    SUCCESSFUL,
    REDIRECTION,
    ORIGINATOR_EXCEPTION,
    RECIPIENT_EXCEPTION
};

class SyncTarget;
class StoragePlugin;
class StorageHandler;
class ResponseGenerator;
class ConflictResolver;
class DeviceInfo;
class CommandHandlerTest;

/*! \brief Responsible for handling and processing individual SyncML commands
 *
 */
class CommandHandler : public QObject {
    Q_OBJECT

public:
    /*! \brief Constructor
     *
     * @param aRole Role in use
     */
    explicit CommandHandler( const Role& aRole );

    /*! \brief Destructor
     *
     */
    virtual ~CommandHandler();

    /*! \brief Process SyncML MAP command
     *
     * @param aMapParams MAP element data
     * @param aTarget Target associated with the command
     * @return Status code
     */
    ResponseStatusCode handleMap( const MapParams& aMapParams, SyncTarget& aTarget );

    /*! \brief Process SyncML SYNC command
     *
     * @param aSyncParams SYNC element data
     * @param aTarget Target associated with the command
     * @param aStorageHandler Storage handler to use in manipulating local database
     * @param aResponseGenerator Response generator to use
     * @param aConflictResolver Conflict resolver to use
     * @param aFastMapsSend True if possible mappings should be sent immediately
     */
    void handleSync( const SyncParams& aSyncParams,
                     SyncTarget& aTarget,
                     StorageHandler& aStorageHandler,
                     ResponseGenerator& aResponseGenerator,
                     ConflictResolver& aConflictResolver,
                     bool aFastMapsSend);

    /*! \brief Reject SyncML SYNC command
     *
     * @param aSyncParams SYNC element data
     * @param aResponseGenerator Response generator to use
     * @param aResponseCode Response code to use
     */
    void rejectSync( const SyncParams& aSyncParams, ResponseGenerator& aResponseGenerator,
                     ResponseStatusCode aResponseCode );

    /*! \brief Handle incoming status element
     *
     * @param aStatusParams Status params of the response
     */
    void handleStatus(StatusParams* aStatusParams );

signals:

    /*! \brief Signal indicating that remote device has acknowledged an item we've sent
     *
     * @param aMsgRef Message reference to the item
     * @param aCmdRef Command reference to the item
     * @param aSyncItemKey Key of the item
     */
    void itemAcknowledged( int aMsgRef, int aCmdRef, SyncItemKey aSyncItemKey );

    /*! \brief Signal indicating that remote device has acknowledged a map we've sent
     *
     * @param aMsgRef Message reference to the item
     * @param aCmdRef Command reference to the item
     */
    void mappingAcknowledged( int aMsgRef, int aCmdRef );

    /*! \brief Signal indicating that the session should be aborted
     *
     * @param aAbortReason Abort code
     */
    void abortSync(ResponseStatusCode aAbortReason);


protected: // functions

    /*! \brief Returns whether conflict resolution is enabled
     *
     * @return True if conflict resolution should be done, otherwise false
     */
    bool resolveConflicts();

private: // functions

    /**
     * \brief Handles error situation
     * @param aErrorCode Error code
     */
    void handleError(ResponseStatusCode aErrorCode);


    /**
     * \brief Handles redirection information message
     * @param aRedirectionCode
     */
    virtual ResponseStatusCode handleRedirection( ResponseStatusCode aRedirectionCode );

    /**
     * \brief Return the type of the status code given as parameter
     * @param aStatus Status code
     * @return The type of the status code
     */
    StatusCodeType getStatusType(ResponseStatusCode aStatus);

    void composeBatches( const SyncParams& aSyncParams, SyncTarget& aTarget,
                         StorageHandler& aStorageHandler, ResponseGenerator& aResponseGenerator,
                         QMap<ItemId, ResponseStatusCode>& aResponses );

    void commitBatches( StorageHandler& aStorageHandler, ConflictResolver& aConflictResolver,
                        SyncTarget& aTarget, const SyncParams& aSyncParams,
                        QMap<ItemId, ResponseStatusCode>& aResponses,
                        QList<UIDMapping>& aNewMappings );

    void processResults( const SyncParams& aSyncParams, const QMap<ItemId, ResponseStatusCode>& aResponses,
                         ResponseGenerator& aResponseGenerator );

    void manageNewMappings( SyncTarget& aTarget, const QList<UIDMapping>& aNewMappings,
                            ResponseGenerator& aResponseGenerator, bool aFastMapsSend );


private: // data

    Role                        iRole;          ///< Role in use
    QMap<int, StatusCodeType>   iStatusTypes;   ///< A map that is used to find the type of a status code

    friend class CommandHandlerTest;

};

}

#endif
