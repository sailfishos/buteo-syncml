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

#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>

#include "SyncAgentConsts.h"
#include "internals.h"
#include "Transport.h"

#include "CommandHandler.h"
#include "DatabaseHandler.h"
#include "StorageHandler.h"
#include "ResponseGenerator.h"
#include "SyncMLMessageParser.h"
#include "NonceStorage.h"
#include "DevInfHandler.h"

class ServerSessionHandlerTest;
class ClientSessionHandlerTest;
class SessionHandlerTest;

namespace DataSync
{

class SyncAgentConfig;
class SyncMode;
class SyncTarget;

/*! \brief Structure to hold reference to an item
 *
 */
struct ItemReference {
    int iMsgId;                         /*!<Message ID related to the item*/
    int iCmdId;                         /*!<Command ID related to the item*/
    SyncItemKey iKey;                   /*!<Key of the item*/
    ModificationType iModificationType; /*!<Type of modification related to the item*/
    QString iLocalDatabase;             /*!<Local database related to the item*/
    QString iRemoteDatabase;            /*!<Remote database related to the item*/
    QString iMimeType;                  /*!<MIME type of the item*/

};

/*! \brief Structure to hold reference to a Map
 *
 */
struct MapReference {
    int iMsgId;                 /*!<Message ID related to the map*/
    int iCmdId;                 /*!<Command ID related to the map*/
    QString iLocalDatabase;     /*!<Local database related to the map*/
    QString iRemoteDatabase;    /*!<Remote database related to the map*/
};

/*! \brief SessionHandler handles all control flow and session related tasks of SyncML protocol.
 * SessionHandler contains the base case for the execution flow for the syncml session. What
 * has happend, what will happen next etc.
 */
class SessionHandler : public QObject
{
    Q_OBJECT
public:
    /**
     * \brief Constructor
     * @param aConfig Pointer to configuration object
     * @param aRole Role in use
     * @param aParent A pointer to parent object
     */
    SessionHandler( const SyncAgentConfig* aConfig,
                    const Role& aRole,
                    QObject* aParent );

    /**
     * \brief Destructor
     */
    virtual ~SessionHandler();

    /*! \brief Perform various preparation for a sync session
     *
     */
    void prepareSync();

    /*! \brief Returns true if sync session has finished
     *
     * @return
     */
    bool syncFinished();

    /*! \brief Gets protocol version of this session
     *
     * @return Protocol version
     */
    ProtocolVersion getProtocolVersion() const;

public slots:

    /*! \brief Initiate a synchronization session with remote device
     *
     */
    virtual void initiateSync() = 0;

    /*! \brief Suspend an ongoing synchronization session
     *
     */
    virtual void suspendSync() = 0;

    /*! \brief Resume a suspended synchronization session
     *
     */
    virtual void resumeSync() = 0;

    /*! \brief Abort an ongoing synchronization session
     *
     * @param aSyncState State to set as final state upon abort
     * @param aDescription Developer-oriented, human-readable description
     */
    void abortSync( DataSync::SyncState aSyncState, const QString& aDescription );

signals:

    /*! \brief Inform that the state of the sync has changed
    *
    * @param aState The new state of the sync session
    */
    void syncStateChanged( DataSync::SyncState aState );

    /*! \brief Inform that the sync session has been finished
     *
     * @param aDeviceImei Remote device IMEI
     * @param aState The final state of the sync session
     * @param aErrorString In case of errors, contains textual debug description of error
     */
    void syncFinished( QString aDeviceImei, DataSync::SyncState aState, QString aErrorString );

    /*! \brief A signal that informs the sync progress
     *
     * @param aModificationType Type of modification made to the item (addition, modification or delete)
     * @param aModifiedDatabase Database that was modified (local or remote)
     * @param aDatabase Identifier of the database
     * @param aMimeType Mime type of the item being processed
     */
    void itemProcessed( DataSync::ModificationType aModificationType,
                        DataSync::ModifiedDatabase aModifiedDatabase,
                        QString aDatabase,
                        QString aMimeType);

    /*! \brief A signal that informs that a storage has been acquired
     *
     * @param aMimeType MIME type of the storage
     */
    void storageAccquired (QString aMimeType);

protected slots:

    /*! \brief A signal handler for changing transport status
     *
     * @param aEvent Occurred event
     * @param aErrorString Optional error string describing what has happened
     */
    void setTransportStatus( DataSync::TransportStatusEvent aEvent , QString aErrorString = "" );

    /*! \brief Slot for handling processing of parsed message
     *
     * @param aLastMessageInPackage True if parsed message contained Final element
     */
    void handleParsingComplete( bool aLastMessageInPackage );

    /*! \brief A slot handler for handling parser errors
     *
     *  @param aError Occurred error
     */
    void handleParserErrors( DataSync::ParserError aError );

    /*! \brief Slot that should be invoked if SAN package has been
     *         received
     *
     * @param aDevice IO device that can be used to read the package
     */
    virtual void SANPackageReceived( QIODevice* aDevice );

    /*! \brief Called when the parser finds a header element in the message being parsed
     *
     * @param aHeaderParams Parameters of the response message header
     */
    void handleHeaderElement( DataSync::HeaderParams* aHeaderParams);

    /*! \brief Called when the parser finds a status element in the message being parsed
     *
     * @param aStatusParams Parameters of the response message status element
     */
    virtual void handleStatusElement( DataSync::StatusParams* aStatusParams );

    /*! \brief Called when the parser finds a sync element in the message being parsed
     *
     * @param aSyncParams Parameters of the response message sync element
     */
    void handleSyncElement(DataSync::SyncParams* aSyncParams);

    /*! \brief Called when the parser finds an alert element in the message being parsed
     *
     * @param aAlertParams Parameters of the response message alert element
     */
    void handleAlertElement(DataSync::AlertParams* aAlertParams);

    /*! \brief Called when the parser finds a Get element in the message being parsed
     *
     * @param aGetParams Get parameters
     */
    void handleGetElement( DataSync::SyncActionData* aGetParams );

    /*! \brief Called when the parser finds a Put element in the message being parsed
     *
     * @param aPutParams Put parameters
     */
    void handlePutElement( DataSync::SyncActionData* aPutParams );

    /*! \brief Called when the parser finds a results element in the message being parsed
     *
     * @param aResults of the response message sync element
     */
    void handleResultsElement( DataSync::ResultsParams* aResults );

    /*! \brief Called when the parser finds a map element in the message being parsed
     *
     * @param aMapParams Parameters of a map element
     */
    void handleMapElement( DataSync::MapParams* aMapParams );

    /*! \brief Called when the parser finds a final element in the message being parsed
     *
     */
    void handleFinal();

    /*! \brief Called when the parser has finished parsing a message
     */
    void handleEndOfMessage();

    /*! \brief Should be called when new item reference is available
     *
     * @param aMsgId Message id of the item
     * @param aCmdId Command id of the item
     * @param aKey Key of the item
     * @param aModificationType Type of modification to the item
     * @param aLocalDatabase Local database where item exists
     * @param aRemoteDatabase Remote database to which item is being sent
     * @param aMimeType Mime type of the item
     */
    void newItemReference( int aMsgId, int aCmdId, SyncItemKey aKey,
                           ModificationType aModificationType,
                           QString aLocalDatabase, QString aRemoteDatabase,
                           QString aMimeType );

    /*! \brief Should be called when new map reference is available
     *
     * @param aMsgId Message id of the map
     * @param aCmdId Command id of the map
     * @param aLocalDatabase Local database to which map relates to
     * @param aRemoteDatabase Local database to which map relates to
     */
    void newMapReference( int aMsgId, int aCmdId,
                          const QString& aLocalDatabase,
                          const QString& aRemoteDatabase );

    /*! \brief Should be called when remote side has responded to item reference
     *
     * @param aMsgRef Message reference of the item
     * @param aCmdRef Command reference of the item
     * @param aKey Key of the item
     */
    void processItemStatus( int aMsgRef, int aCmdRef, SyncItemKey aKey );

    /*! \brief Should be called when remote side has responded to map reference
     *
     * @param aMsgRef Message reference of the item
     * @param aCmdRef Command reference of the item
     */
    void processMapStatus( int aMsgRef, int aCmdRef );

protected:

    /*! \brief Invoked when SyncML message has been received from remote side
     *
     * @param aHeaderParams Header parameters
     */
    virtual void messageReceived( HeaderParams& aHeaderParams ) = 0;

    /*! \brief Invoked when Alert related to initiating sync has been received from
     *         remote side
     *
     * @param aSyncMode Sync mode of Alert
     * @param aAlertParams Alert params
     */
    virtual ResponseStatusCode syncAlertReceived( const SyncMode& aSyncMode, AlertParams& aAlertParams ) = 0;

    /*! \brief Invoked when Sync related to receiving items has been received from
     *         remote side
     *
     *
     * @return True if command is allowed, false otherwise
     */
    virtual bool syncReceived() = 0;

    /*! \brief Invoked when Map related to receiving mappings has been received from
     *         remote side
     *
     * @return True if command is allowed, false otherwise
     */
    virtual bool mapReceived() = 0;

    /*! \brief Invoked when Final element has been received from remote side
     *
     */
    virtual void finalReceived() = 0;

    /*! \brief Invoked when SyncML message received from remote side has been completely
     *         parsed
     */
    virtual void messageParsed() = 0;

    /*! \brief Resend the last package by refilling package queue based on sync state
     *
     */
    virtual void resendPackage() = 0;

    /*! \brief Process a message sent by remote device
     *
     * @param aFragments Protocol fragments of the message. Ownership is transferred.
     * @param aLastMessageInPackage True if message contained Final element, otherwise
     *                              false
     */
    void processMessage( QList<Fragment*>& aFragments, bool aLastMessageInPackage );

    /*! \brief Sets current state of the sync
     *
     * @param aSyncState New status to set
     */
    void setSyncState( SyncState aSyncState );

    /*!
     * \brief Returns current state of the sync
     *
     * @return
     */
    SyncState getSyncState() const;

    /*! \brief Finish a successful sync session
     *
     * Sync state will be set to SYNC_FINISHED and no further messages
     * will be sent
     */
    void finishSync();

    /**
     * \brief Sends next pending message
     */
    void sendNextMessage();

    /*! \brief Adds local changes to the outgoing message
     *
     */
    void composeLocalChanges();

    /*! \brief Adds authentication to the outgoing message
     *
     */
    void composeAuthentication();

    /*! \brief Handles authentication info received from remote side
     *
     * @param aHeaderParams Header parameters
     */
    void authenticationInformationReceived( const HeaderParams& aHeaderParams );

    /*! \brief Set up session based on header received from remote party ( remote
     *         side is initiating sync )
     *
     * @param aHeaderParams Header parameters
     */
    void setupSession( HeaderParams& aHeaderParams );

    /*! \brief Set up session based on SyncAgentConfig( local side is initiating sync )
     *
     * @param aSessionId Session Id to use
     */
    void setupSession( const QString& aSessionId );

    /*! \brief Saves current session (anchors etc) to change log
     *
     */
    void saveSession();

    /*! \brief Return current session ID
     *
     * @return Current session ID
     */
    const QString& getSessionId() const;

    /*! \brief Sets session ID
     *
     * @param aSessionId Session ID
     */
    void setSessionId( const QString& aSessionId );

    /*! \brief Sets protocol version of this session
     *
     * @param aProtocolVersion Protocol version
     */
    void setProtocolVersion( const ProtocolVersion& aProtocolVersion );

    /*! \brief Gets protocol attribute of this session
     *
     * @see ProtocolAttributes for list of attributes
     *
     * @param aAttribute Protocol attribute to check
     * @return True if attribute is set, otherwise false
     */
    bool getProtocolAttribute( int aAttribute ) const;

    /*! \brief Sets protocol attribute for this session
     *
     * @see ProtocolAttributes for list of attributes
     *
     * @param aAttribute Protocol attribute to set
     */
    void setProtocolAttribute( int aAttribute );

    /*! \brief Clears protocol attribute for this session
     *
     * @see ProtocolAttributes for list of attributes
     *
     * @param aAttribute Protocol attribute to clear
     */
    void clearProtocolAttribute( int aAttribute );

    /*! \brief Returns local NEXT anchor
     *
     * @return Local NEXT anchor
     */
    const QString& getLocalNextAnchor() const;

    /*! \brief Sets local NEXT anchor
     *
     * @param aLocalNextAnchor Local NEXT anchor
     */
    void setLocalNextAnchor( const QString& aLocalNextAnchor );

    /*! \brief Checks for last anchor mismatch
     *
     * @param aSyncMode Proposed sync mode
     * @param aTarget Target to check
     * @param aRemoteLastAnchor Last anchor to check
     * @return True if anchor mismatch, otherwise false
     */
    bool anchorMismatch( const SyncMode& aSyncMode, const SyncTarget& aTarget, const QString& aRemoteLastAnchor ) const;

    /*! \brief Creates a new storage based on URI
     *
     * @param aURI URI of the storage
     * @return Storage plugin on success, otherwise NULL
     */
    StoragePlugin* createStorageByURI( const QString& aURI );

    /*! \brief Creates a new storage based on MIME
     *
     * @param aMIME MIME of the storage
     * @return Storage plugin on success, otherwise NULL
     */
    StoragePlugin* createStorageByMIME( const QString& aMIME );

    /*! \brief Get list of all storages
     *
     * @return
     */
    const QList<StoragePlugin*>& getStorages() const;

    /*! \brief Creates a new sync target
     *
     * Sync target is NOT added to the list of sync targets, addSyncTarget() should be called
     * separately
     *
     * @param aPlugin Storage plugin representing local database
     * @param aSyncMode Sync mode to use
     */
    SyncTarget* createSyncTarget( StoragePlugin& aPlugin, const SyncMode& aSyncMode );

    /*! \brief Adds a new sync target to the list of available sync targets
     *
     * @param aTarget Sync target to add
     */
    void addSyncTarget( SyncTarget* aTarget );

    /*! \brief Searches for existing sync target
     *
     * @param aSourceURI Source URI
     * @return Sync target if found, otherwise NULL
     */
    SyncTarget* getSyncTarget( const QString& aSourceURI ) const;

    /*! \brief Get list of all sync targets
     *
     * @return List containing sync targets
     */
    const QList<SyncTarget*>& getSyncTargets() const;

    /*! \brief Set SyncML header parameters for messages going out
     *
     * msgId field is ignored as this is filled by response generator
     *
     * @param aHeaderParams Header parameters
     */
    void setLocalHeaderParams( const HeaderParams& aHeaderParams );

    /*! \brief Retrieve SyncML header parameters for messages going out
     *
     * @return Header parameters
     */
    const HeaderParams& getLocalHeaderParams() const;

    /*! \brief Sets the maximum permitted size of SyncML message sent to remote device
     *
     * By default set to maximum transmit size of transport. Can be overridden by remote device
     *
     * @param aMaxMsgSize New maximum permitted size
     */
    void setRemoteMaxMsgSize( int aMaxMsgSize );

    /*! \brief Get the maximum permitted size of SyncML message sent to remote device
     *
     * @return Maximum permitted size
     */
    int getRemoteMaxMsgSize() const;

    /*! \brief Get the maximum permitted size of SyncML messages that we can receive
     *
     * @return
     */
    int getLocalMaxMsgSize();

    /*! \brief Sets the URI to use in next send operation
     *
     * @param aURI New URI to remote side
     */
    void setRemoteLocURI( const QString& aURI );

    /*! \brief Returns whether the session has been authenticated
     *
     * @return True if session has been authenticated, otherwise false
     */
    bool getSessionAuthenticated() const;

    /*! \brief Sets whether the session has been authenticated
     *
     * @param aAuthenticated True if session authenticated, otherwise false
     */
    void setSessionAuthenticated( bool aAuthenticated );

    /*! \brief Sets the authentication type to use
     *
     * @param aAuthenticationType Authentication type
     */
    void setAuthenticationType( const AuthenticationType& aAuthenticationType );

    /*! \brief Gets the authentication type to use
     *
     * @return
     */
    AuthenticationType getAuthenticationType() const;

    /*! \brief Returns pointer to configuration object
     *
     * @return A pointer to configuration object
     */
    const SyncAgentConfig* getConfig() const;

    /*! \brief Returns reference to transport
     *
     */
    Transport& getTransport();

    /*! \brief Returns reference to response generator
     *
     * @return
     */
    ResponseGenerator& getResponseGenerator();

    /*! \brief Returns reference to database handler
     *
     * @return
     */
    DatabaseHandler& getDatabaseHandler();

    /**
     * \brief Generates a new session ID
     * @return Formatted session ID string
     */
    QString generateSessionID();

    /*! \brief used to check the status sent by the server when we send local modifications
     *
     * @return Returns true if server reported Busy
     */
    bool isRemoteBusyStatusSet() const;

    /*! \brief resets the server busy status once it is handled
     *
     * @return
     */
    void resetRemoteBusyStatus();

    /*! \brief Sets the name of the local device in this session
     *
     * @param aLocalDeviceName Local device name
     */
    void setLocalDeviceName( const QString& aLocalDeviceName );

    /*! \brief Retrieves the name of the local device in this session
     *
     * @return
     */
    const QString& getLocalDeviceName() const;

    /*! \brief Sets the name of the remote device in this session
     *
     * @param aRemoteDeviceName Remote device name
     */
    void setRemoteDeviceName( const QString& aRemoteDeviceName );

    /*! \brief Retrieves the name of the remote device in this session
     *
     * @return
     */
    const QString& getRemoteDeviceName() const;

    /*! \brief Retrieves device info handler
     *
     */
    DevInfHandler& getDevInfHandler();

private: // functions

    void exitSync();

    void releaseStoragesAndTargets();

    /**
     * \brief A helper function used to conenect signal in constructor
     */
    void connectSignals();

    ResponseStatusCode handleInformativeAlert( const AlertParams& aAlertParams );

    void handleChallenge( const ChalParams& aChallenge );

private: // data
    CommandHandler                      iCommandHandler;            ///< A pointer to command handler object
    StorageHandler                      iStorageHandler;            ///< Handles sync item storage operations
    DevInfHandler                       iDevInfHandler;             ///< Handles device info related things
    ResponseGenerator                   iResponseGenerator;         ///< Response generator object
    const DataSync::SyncAgentConfig*    iConfig;                    ///< A pointer to configuration
    SyncMLMessageParser                 iParser;                    ///< XML parser
    QList<StoragePlugin*>               iStorages;                  ///< A list of reserved storages
    QList<SyncTarget*>                  iSyncTargets;               ///< A list of sync targets
    SyncState                           iSyncState;                 ///< State of the synchronization session
    QString                             iSessionId;                 ///< Session ID
    QString                             iLocalNextAnchor;           ///< Local NEXT anchor of this session
    QString                             iSyncError;                 ///< Human-readable description upon sync abort
    QBitArray                           iProtocolAttributes;        ///< Protocol attributes in use in current session
    int                                 iRemoteMaxMsgSize;          ///< Maximum size for messages sent to remote device
    DatabaseHandler                     iDatabaseHandler;           ///< Handler for database operations
    AuthenticationType                  iAuthenticationType;        ///< Type of authentication to use
    QList<ItemReference>                iItemReferences;            ///<Keeps track which status refers to which item in which database
    QList<MapReference>                 iMapReferences;             ///<Keeps track which status refers to which map in which database
    NonceStorage*                       iNonceStorage;              ///< Storage for MD5 nonces
    bool                                iSyncFinished;              ///< Set to true when sync has ended
    bool                                iSessionClosed;             ///< Set to true when Session tearing down started.
    bool                                iSessionAuthenticated;      ///< Set to true when session has been authenticated
    bool                                iAuthenticationPending;     ///< True if authentication is pending from remote side
    bool                                iParsing;                   ///< Set to true when we are parsing a message
    ProtocolVersion                     iProtocolVersion;           ///< Protocol version in use in current session
    bool 								iRemoteReportedBusy;        ///< indicates that server reported busy
    Role                                iRole;                      ///< Role in use
    QString                             iLocalDeviceName;           ///< Name of the local device in this session
    QString                             iRemoteDeviceName;          ///< Name of the remote device in this session

    friend class ::ServerSessionHandlerTest;
    friend class ::ClientSessionHandlerTest;
    friend class ::SessionHandlerTest;

};

}

#endif  //  SESSIONHANDLER_H
