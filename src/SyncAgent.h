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

/*! \mainpage libmeegosyncml
 *
 * \section intro_sec Library overview
 *
 * Libmeegosyncml implements Open Mobile Alliance Data Synchronization protocol
 * (OMA DS), formerly known as SyncML. This library supports OMA DS versions
 * 1.1.2 and 1.2.1, and implements both OMA DS Server and OMA DS Client
 * functionality. In addition, this library offers transports for synchronization
 * occurring over HTTP, and over OBEX using Bluetooth or USB.
 *
 * \section features_sec Supported features
 * \li \ref features11\n
 * \li \ref features12\n
 * \li \ref otherfeatures\n
 *
 * \section example_sec Examples
 * \li \ref example12httpsync\n
 * \li \ref example11btsync\n
 *
 * \page features12 Supported DS 1.2 features
 *
 * \section client12 OMA DS 1.2 Client features
 *
 * \subsection client12protocolsupported Supported conformance requirements
 * \li SCR-DS-CLIENT-001: Support of 'two-way sync' sync type
 * \li SCR-DS-CLIENT-002: Support of 'slow two-way sync' sync type
 * \li SCR-DS-CLIENT-003: Support of 'one-way sync from client only' sync type
 * \li SCR-DS-CLIENT-005: Support of 'one-way sync from server only' sync type
 * \li SCR-DS-CLIENT-007: Support of 'sync alert'
 * \li SCR-DS-CLIENT-008: Support of 'Sync Without Separate Initialization'
 * \li SCR-DS-CLIENT-009: Support of sending 'Large Objects'
 * \li SCR-DS-CLIENT-010: Support of receiving 'Large Objects'
 * \li SCR-DS-CLIENT-011: Support of 'Busy signaling'
 * \li SCR-DS-CLIENT-012: Support of suspend/resume
 * \li SCR-DS-CLIENT-014: Support for hierarchical synchronization
 * \li SCR-DS-CLIENT-LO-R-001: Indicate support for receiving Large Object in the DevInf
 * \li SCR-DS-CLIENT-LO-R-002: Sending MaxObjSize and MaxMsgSize
 * \li SCR-DS-CLIENT-LO-R-003: Sync Commands inside Large Object is handles as Atomic
 * \li SCR-DS-CLIENT-LO-S-004: Data chunks must be sent in continuous order without any new commands
 * \li SCR-DS-CLIENT-LO-S-005: Include Size in the first data chunk
 * \li SCR-DS-CLIENT-LO-S-006: All chunks except the last one must include "MoreData" tag
 * \li SCR-DS-CLIENT-LO-S-007: Repeat Meta and Item information in each chunk
 *
 *
 * \subsection client12protocolunsupported Unsupported conformance requirements
 * \li SCR-DS-CLIENT-004: Support of 'refresh sync from client only' sync type
 * \li SCR-DS-CLIENT-006: Support of 'refresh sync from server only' sync type
 * \li SCR-DS-CLIENT-013: Support for filtering
 * \li SCR-DS-CLIENT-015: Support WAP PUSH operation
 * \li SCR-DS-CLIENT-016: Support of 'sync alert' by WAP Push method
 *
 *
 * \subsection client12reprosupported Supported representation protocol requirements
 * \li SCR-DS-CUE-C-002: Support for Chal element (database-level authentication not supported)
 * \li SCR-DS-CUE-C-003: Support for Cmd element
 * \li SCR-DS-CUE-C-004: Support for CmdId element
 * \li SCR-DS-CUE-C-005: Support for CmdRef element
 * \li SCR-DS-CUE-C-006: Support for Cred element (database-level authentication not supported)
 * \li SCR-DS-CUE-C-010: Support for Final element
 * \li SCR-DS-CUE-C-013: Support for LocURI element
 * \li SCR-DS-CUE-C-014: Support for MoreData element
 * \li SCR-DS-CUE-C-015: Support for MsgID element
 * \li SCR-DS-CUE-C-016: Support for MsgRef element
 * \li SCR-DS-CUE-C-017: Support for NoResp element
 * \li SCR-DS-CUE-C-019: Support for NumberOfChanges element
 * \li SCR-DS-CUE-C-021: Support for RespURI element
 * \li SCR-DS-CUE-C-022: Support for SessionID element
 * \li SCR-DS-CUE-C-024: Support for Source element
 * \li SCR-DS-CUE-C-025: Support for SourceParent element
 * \li SCR-DS-CUE-C-026: Support for SourceRef element
 * \li SCR-DS-CUE-C-027: Support for Target element
 * \li SCR-DS-CUE-C-028: Support for TargetParent element
 * \li SCR-DS-CUE-C-029: Support for TargetRef element
 * \li SCR-DS-CUE-C-030: Support for VerDTD element
 * \li SCR-DS-CUE-C-031: Support for VerProto element
 * \li SCR-DS-MCE-C-001: Support for SyncML element
 * \li SCR-DS-MCE-C-002: Support for SyncHdr element
 * \li SCR-DS-MCE-C-003: Support for SyncBody element
 * \li SCR-DS-DDE-C-001: Support for Data element
 * \li SCR-DS-DDE-C-002: Support for Item element
 * \li SCR-DS-DDE-C-003: Support for Meta element
 * \li SCR-DS-PME-C-001: Support for Status element
 * \li SCR-DS-PCE-C-001: Support for Add element
 * \li SCR-DS-PCE-C-002: Support for Alert element
 * \li SCR-DS-PCE-C-005: Support for Delete element
 * \li SCR-DS-PCE-C-007: Support for Get element (Only for device info)
 * \li SCR-DS-PCE-C-008: Support for Map element
 * \li SCR-DS-PCE-C-009: Support for MapItem element
 * \li SCR-DS-PCE-C-011: Support for Put element (Only for device info)
 * \li SCR-DS-PCE-C-012: Support for Replace element
 * \li SCR-DS-PCE-C-013: Support for Result element (Only for device info)
 * \li SCR-DS-PCE-C-016: Support for Sync element
 *
 *
 * \subsection client12reprounsupported Unsupported representation protocol requirements
 * \li SCR-DS-CUE-C-001: Support for Archive element
 * \li SCR-DS-CUE-C-007: Support for Field element
 * \li SCR-DS-CUE-C-008: Support for Filter element
 * \li SCR-DS-CUE-C-009: Support for FilterType element
 * \li SCR-DS-CUE-C-011: Support for Lang element
 * \li SCR-DS-CUE-C-012: Support for LocName element
 * \li SCR-DS-CUE-C-018: Support for NoResults element
 * \li SCR-DS-CUE-C-020: Support for Record element
 * \li SCR-DS-CUE-C-023: Support for SftDel element
 * \li SCR-DS-PCE-C-003: Support for Atomic element
 * \li SCR-DS-PCE-C-004: Support for Copy element
 * \li SCR-DS-PCE-C-006: Support for Exec element
 * \li SCR-DS-PCE-C-010: Support for Move element
 * \li SCR-DS-PCE-C-014: Support for Search element
 * \li SCR-DS-PCE-C-015: Support for Sequence element
 *
 *
 * \section server12 OMA DS 1.2 Server features
 * \subsection server12protocolsupported Supported conformance requirements
 * \li SCR-DS-SERVER-001: Support of 'two-way sync' sync type
 * \li SCR-DS-SERVER-002: Support of 'slow two-way sync' sync type
 * \li SCR-DS-SERVER-003: Support of 'one-way sync from client only' sync type
 * \li SCR-DS-SERVER-005: Support of 'one-way sync from server only' sync type
 * \li SCR-DS-SERVER-007: Support of 'sync alert'
 * \li SCR-DS-SERVER-008: Support of 'Sync Without Separate Initialization'
 * \li SCR-DS-SERVER-009: Support of sending 'Large Objects'
 * \li SCR-DS-SERVER-010: Support of receiving 'Large Objects'
 * \li SCR-DS-SERVER-012: Support of suspend/resume
 * \li SCR-DS-SERVER-014: Support for hierarchical synchronization
 * \li SCR-DS-SERVER-LO-R-001: Indicate support for receiving Large Object in the DevInf
 * \li SCR-DS-SERVER-LO-R-002: Sending MaxObjSize and MaxMsgSize
 * \li SCR-DS-SERVER-LO-R-003: Sync Commands inside Large Object is handles as Atomic
 * \li SCR-DS-SERVER-LO-S-004: Data chunks must be sent in continuous order without any new commands
 * \li SCR-DS-SERVER-LO-S-005: Include Size in the first data chunk
 * \li SCR-DS-SERVER-LO-S-006: All chunks except the last one must include "MoreData" tag
 * \li SCR-DS-SERVER-LO-S-007: Repeat Meta and Item information in each chunk
 *
 *
 * \subsection server12protocolunsupported Unupported conformance requirements
 * \li SCR-DS-SERVER-004: Support of 'refresh sync from client only' sync type
 * \li SCR-DS-SERVER-006: Support of 'refresh sync from server only' sync type
 * \li SCR-DS-SERVER-011: Support of 'Busy signaling'
 * \li SCR-DS-SERVER-013: Support for filtering
 *
 *
 * \subsection server12reprosupported Supported representation protocol requirements
 * \li SCR-DS-CUE-S-002: Support for Chal element (database-level authentication not supported)
 * \li SCR-DS-CUE-S-003: Support for Cmd element
 * \li SCR-DS-CUE-S-004: Support for CmdId element
 * \li SCR-DS-CUE-S-005: Support for CmdRef element
 * \li SCR-DS-CUE-S-006: Support for Cred element (database-level authentication not supported)
 * \li SCR-DS-CUE-S-010: Support for Final element
 * \li SCR-DS-CUE-S-013: Support for LocURI element
 * \li SCR-DS-CUE-S-014: Support for MoreData element
 * \li SCR-DS-CUE-S-015: Support for MsgID element
 * \li SCR-DS-CUE-S-016: Support for MsgRef element
 * \li SCR-DS-CUE-S-017: Support for NoResp element
 * \li SCR-DS-CUE-S-019: Support for NumberOfChanges element
 * \li SCR-DS-CUE-S-021: Support for RespURI element
 * \li SCR-DS-CUE-S-022: Support for SessionID element
 * \li SCR-DS-CUE-S-024: Support for Source element
 * \li SCR-DS-CUE-S-025: Support for SourceParent element
 * \li SCR-DS-CUE-S-026: Support for SourceRef element
 * \li SCR-DS-CUE-S-027: Support for Target element
 * \li SCR-DS-CUE-S-028: Support for TargetParent element
 * \li SCR-DS-CUE-S-029: Support for TargetRef element
 * \li SCR-DS-CUE-S-030: Support for VerDTD element
 * \li SCR-DS-CUE-S-031: Support for VerProto element
 * \li SCR-DS-MCE-S-001: Support for SyncML element
 * \li SCR-DS-MCE-S-002: Support for SyncHdr element
 * \li SCR-DS-MCE-S-003: Support for SyncBody element
 * \li SCR-DS-DDE-S-001: Support for Data element
 * \li SCR-DS-DDE-S-002: Support for Item element
 * \li SCR-DS-DDE-S-003: Support for Meta element
 * \li SCR-DS-PME-S-001: Support for Status element
 * \li SCR-DS-PCE-S-001: Support for Add element
 * \li SCR-DS-PCE-S-002: Support for Alert element
 * \li SCR-DS-PCE-S-005: Support for Delete element
 * \li SCR-DS-PCE-S-007: Support for Get element (Only for device info)
 * \li SCR-DS-PCE-S-008: Support for Map element
 * \li SCR-DS-PCE-S-009: Support for MapItem element
 * \li SCR-DS-PCE-S-011: Support for Put element (Only for device info)
 * \li SCR-DS-PCE-S-012: Support for Replace element
 * \li SCR-DS-PCE-S-013: Support for Result element (Only for device info)
 * \li SCR-DS-PCE-S-016: Support for Sync element
 *
 *
 * \subsection server12reprounsupported Unsupported representation protocol requirements
 * \li SCR-DS-CUE-S-001: Support for Archive element
 * \li SCR-DS-CUE-S-007: Support for Field element
 * \li SCR-DS-CUE-S-008: Support for Filter element
 * \li SCR-DS-CUE-S-009: Support for FilterType element
 * \li SCR-DS-CUE-S-011: Support for Lang element
 * \li SCR-DS-CUE-S-012: Support for LocName element
 * \li SCR-DS-CUE-S-018: Support for NoResults element
 * \li SCR-DS-CUE-S-020: Support for Record element
 * \li SCR-DS-CUE-S-023: Support for SftDel element
 * \li SCR-DS-PCE-S-003: Support for Atomic element
 * \li SCR-DS-PCE-S-004: Support for Copy element
 * \li SCR-DS-PCE-S-006: Support for Exec element
 * \li SCR-DS-PCE-S-010: Support for Move element
 * \li SCR-DS-PCE-S-014: Support for Search element
 * \li SCR-DS-PCE-S-015: Support for Sequence element
 *
 *
 * \page features11 Supported DS 1.1 features
 *
 *
 * \section client11 OMA DS 1.1 Client features
 * \subsection client11protocolsupported Supported conformance requirements
 * \li Support of 'two-way sync'
 * \li Support of 'slow two-way sync'
 * \li Support of 'one-way sync from client only'
 * \li Support of 'one-way sync from server only'
 * \li Support of 'sync alert'
 *
 *
 * \subsection client11protocolunsupported Unupported conformance requirements
 * \li Support of 'refresh sync from client only'
 * \li Support of 'refresh sync from server only'
 *
 *
 * \subsection client11reprosupported Supported representation protocol requirements
 * \li Chal (database-level authentication not supported)
 * \li Cmd
 * \li CmdID
 * \li CmdRef
 * \li Cred (database-level authentication not supported)
 * \li Final
 * \li LocURI
 * \li MoreData
 * \li MsgID
 * \li MsgRef
 * \li NoResp
 * \li NumberOfChanges
 * \li RespURI
 * \li SessionID
 * \li Source
 * \li SourceRef
 * \li Target
 * \li TargetRef
 * \li VerDTD
 * \li VerProto
 * \li SyncML
 * \li SyncHdr
 * \li SyncBody
 * \li Data
 * \li Item
 * \li Meta
 * \li Add
 * \li Alert
 * \li Delete
 * \li Get (Only for device info)
 * \li Map
 * \li MapItem
 * \li Put (Only for device info)
 * \li Replace
 * \li Result (Only for device info)
 * \li Status
 * \li Sync
 *
 *
 * \subsection client11reprounsupported Unsupported representation protocol requirements
 * \li Archive
 * \li Lang
 * \li LocName
 * \li NoResults
 * \li SftDel
 * \li Atomic
 * \li Copy
 * \li Exec
 * \li Search
 * \li Sequence
 *
 *
 * \section server11 OMA DS 1.1 Server features
 * \subsection server11protocolsupported Supported conformance requirements
 * \li Support of 'two-way sync'
 * \li Support of 'slow two-way sync'
 * \li Support of 'one-way sync from client only'
 * \li Support of 'one-way sync from server only'
 * \li Support of 'sync alert'
 *
 *
 * \subsection server11protocolunsupported Unupported conformance requirements
 * \li Support of 'refresh sync from client only'
 * \li Support of 'refresh sync from server only'
 * \li Support of 'busy signaling'
 *
 *
 * \subsection server11reprosupported Supported representation protocol requirements
 * \li Chal (database-level authentication not supported)
 * \li Cmd
 * \li CmdID
 * \li CmdRef
 * \li Cred (database-level authentication not supported)
 * \li Final
 * \li LocURI
 * \li MoreData
 * \li MsgID
 * \li MsgRef
 * \li NoResp
 * \li NumberOfChanges
 * \li RespURI
 * \li SessionID
 * \li Source
 * \li SourceRef
 * \li Target
 * \li TargetRef
 * \li VerDTD
 * \li VerProto
 * \li SyncML
 * \li SyncHdr
 * \li SyncBody
 * \li Data
 * \li Item
 * \li Meta
 * \li Add
 * \li Alert
 * \li Delete
 * \li Get (Only for device info)
 * \li Map
 * \li MapItem
 * \li Put (Only for device info)
 * \li Replace
 * \li Result (Only for device info)
 * \li Status
 * \li Sync
 *
 *
 * \subsection server11reprounsupported Unsupported representation protocol requirements
 * \li Archive
 * \li Lang
 * \li LocName
 * \li NoResults
 * \li SftDel
 * \li Atomic
 * \li Copy
 * \li Exec
 * \li Search
 * \li Sequence
 *
 *
 * \page otherfeatures Other features
 *
 * \li Synchronization over HTTP or HTTPS
 * \li Synchronization over OBEX on Bluetooth and USB
 *
 * \page example12httpsync OMA DS 1.2 synchronization over HTTP
 * \page example11btsync OMA DS 1.1 synchronization over OBEX Bluetooth
 */

#ifndef SYNCAGENT_H
#define SYNCAGENT_H

#include <QObject>
#include "SyncAgentConsts.h"
#include "SyncResults.h"

class SyncAgentTest;

namespace DataSync {

class SyncAgentConfig;
class RequestListener;
class SessionHandler;

/*! \brief SyncAgent is the base API for using the synchronization library
 * An entity which provides the interface for synchronization to the application.
 *
 * SyncAgent entity which provides the interface for synchronization to the
 * application. It creates the synchronization thread for a specific connection
 * and fills up data the the protocol handler.
 *
 * As per Qt's Documentation on <a href="http://doc.trolltech.com/4.5/threads.html#qobject-reentrancy">Threads</a>, "The child of a QObject must always be
 * created in the thread where the parent was created. This implies, among other
 * things, that you should never pass the QThread object (this) as the parent of
 * an object created in the thread (since the QThread object itself was created
 * in another thread)." So, all actions pertaining to SyncAgent should be done
 * in one single thread. If the Agent is created in one thread and subsequent
 * commands (for example prepareSync() ) are issued in another thread,
 * Qt's parent child relationship between multiple threads prevents the
 * Signals to be emitted to parents in another thread.
 *
 * The behavior for "Issuing the same command twice" is unsupported and undefined.
 *
 */
class SyncAgent : public QObject
{
Q_OBJECT

public:
    /*! \brief Constructor
     *
     * @param aParent QObject parent
     *
     */
    explicit SyncAgent( QObject* aParent = 0 );

    /*! \brief Destructor
     *
     */
    virtual ~SyncAgent();

    /*! \brief Initiate a synchronization session
     *
     * Prepare and initiate a synchronization session using the supplied
     * configuration. During preparation the supplied configuration is validated.
     * If configuration is not valid, false is returned and no actions will be made.
     * If configuration is valid, start event will be posted to event queue and this
     * function returns true. This function does not block execution for the duration
     * of the synchronization session. Progress of the synchronization session can be
     * observed with stateChanged() signal. When synchronization session finishes due
     * to success or failure, syncFinished() signal is emitted. Synchronization session
     * can be aborted by calling abort() function.
     *
     * @param aConfig Configuration to use when initiating synchronization. Ownership is
     *                not transferred.
     * @return True if synchronization was successfully started and subsequent signals
     *         can be expected to be emitted. False if synchronization could not be
     *         started, and no subsequent signals will be emitted.
     */
    bool startSync( const SyncAgentConfig& aConfig );

    /*! \brief Returns whether synchronization is in progress
     *
     * @return True if synchronization is in progress, otherwise false
     */
    bool isSyncing() const;

    /*! \brief Listen for incoming synchronization requests
     *
     * Prepare and listen for incoming synchronization request using the supplied
     * configuration. During preparation the supplied configuration is validated.
     * If configuration is not valid, false is returned and no actions will be made.
     * If configuration is valid, listen event will be posted to event queue and this
     * function returns true. Upon first synchronization request, the request will be
     * automatically served and synchronization session will be established.
     * This function does not block execution for the duration of the listening, nor
     * during the possible synchronization session. Progress of the synchronization
     * session can be observed with stateChanged() signal. When synchronization session
     * finishes due to success or failure, syncFinished() signal is emitted. Listening
     * can be aborted by calling abort() function.
     *
     * @param aConfig Configuration to use when initiating synchronization. Ownership is
     *                not transferred.
     * @return True if listening was successfully started and subsequent signals
     *         can be expected to be emitted. False if listening was not started, and
     *         no subsequent signals will be emitted.
     */
    bool listen( const SyncAgentConfig& aConfig );

    /*! \brief Return whether listening is in progress
     *
     * @return True if listening is in progress, otherwise false
     */
    bool isListening() const;

    /*! \ brief Aborts listening for requests or an ongoing synchronization
     *
     *
     * @return True if there was some operation ongoing which was aborted, otherwise false
     */
    bool abort();

    /*! \brief Pause synchronization
    *
    * This method pauses an ongoing sync. The command will be processed and the
    * application will get a callback when the command has been executed successful
    * or with an error.
    *
    * @return
    */
   bool pauseSync();


   /*! \brief Resume synchronization
    *
    * This method resumes a suspended sync. The command will be
    * processed and the application will get a callback when the command
    * has been executed successful or with an error.
    *
    * @return
    */
   bool resumeSync();

    /*! \brief Cleans up any stored data of a profile which is deleted
     *
     * @return true on success.
     */
    bool cleanUp (const SyncAgentConfig* aConfig);

    /*! \brief Returns results of the latest sync session
     *
     * @return Results
     */
    const SyncResults& getResults() const;

signals:    //  public signals

    /*! \brief Signal indicating that the state of the synchronization process
     *         has changed
     *
     * @param aState New state of synchronization process
     */
    void stateChanged( DataSync::SyncState aState );

    /*! \brief Signal indicating that the synchronization process has finished
     *
     * @param aState Resulting state of the synchronization process
     */
    void syncFinished( DataSync::SyncState aState );

    /*! \brief Signal indicating that an item has been processed
     *
     * @param aModificationType Type of modification made to the item (addition, modification or delete)
     * @param aModifiedDatabase Database that was modified (local or remote)
     * @param aLocalDatabase Identifier of the local database used in the sync
     * @param aMimeType Mimetype of the item prcessed
     */
    void itemProcessed( DataSync::ModificationType aModificationType,
                        DataSync::ModifiedDatabase aModifiedDatabase,
                        QString aLocalDatabase,
                        QString aMimeType);

    /*! \brief Signal indicating that a storage has been acquired
     *
     * @param aMimeType MIME type of the storage
     */
    void storageAccquired (QString aMimeType);

private slots:

    void receiveStateChanged( DataSync::SyncState aState );

    void receiveSyncFinished( const QString& aRemoteDeviceId,
		    	      DataSync::SyncState aState,
                              const QString& aErrorString );

    void accquiredStorage( const QString& aStorageMimeType );

    void receiveItemProcessed( DataSync::ModificationType aModificationType,
                               DataSync::ModifiedDatabase aModifiedDatabase,
                               const QString aDatabase,
                               const QString aMimeType);


    void listenEvent();

    void listenError( DataSync::SyncState aState, QString aErrorString );

private:

    void finishSync( DataSync::SyncState aState, const QString& aErrorString );

    bool initiateSession( const SyncAgentConfig& aConfig );

    bool startClientInitiatedSession( const SyncAgentConfig& aConfig );
    bool startServerInitiatedSession( const SyncAgentConfig& aConfig );

    void abortSession();
    void cleanSession();

    bool initiateListen( const SyncAgentConfig& aConfig );

    void abortListen();
    void cleanListen();

    RequestListener*        iListener;
    SessionHandler*         iHandler;
    const SyncAgentConfig*  iConfig;
    SyncResults             iResults;

    friend class ::SyncAgentTest;
};

}

typedef DataSync::SyncAgent* (*createSyncAgent_t)(QObject* aParent);
typedef void destroySyncAgent_t(DataSync::SyncAgent*);

#endif
