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
#ifndef SYNCAGENTCONFIG_H
#define SYNCAGENTCONFIG_H

#include <QBitArray>
#include <QMap>
#include <QString>
#include <QDebug>

#include "SyncAgentConsts.h"
#include "SyncMode.h"
#include "DeviceInfo.h"

namespace DataSync {

class StorageProvider;
class Transport;
class SyncAgentConfigTest;


/*! \brief Configuration for the Sync Agent
 *
 * It specifies the used transportation layer
 * and necessary information for performing sync action.
 */
class SyncAgentConfig {

public:

    /*! \brief Constructor
     *
     */
    SyncAgentConfig();

    /*! \brief Destructor
     *
     */
    ~SyncAgentConfig();

    /*! \brief Sets the transport to use in sync
     *
     * Ownership is NOT transferred. It is MANDATORY to set transport when
     * attempting sync. Otherwise, sync preparation will fail.
     *
     * @param aTransport Transport instance to use
     */
    void setTransport( Transport* aTransport );

    /*! \brief Returns the transport to use in sync
     *
     * Ownership is NOT transferred
     *
     * @return
     */
    Transport* getTransport() const;

    /*! \brief Sets the storage provider to use in sync
     *
     * Ownership is NOT transferred. It is MANDATORY to set transport when
     * attempting sync. Otherwise, sync preparation will fail.
     */
    void setStorageProvider( StorageProvider* aProvider );

    /*! \brief Returns the storage provider to use in sync
     *
     * Ownership is NOT transferred
     *
     * @return
     */
    StorageProvider* getStorageProvider() const;

    /*! \brief Adds a target for sync
     *
     * @param aSourceDb Source database
     * @param aTargetDb Target database
     */
    void addSyncTarget( const QString& aSourceDb, const QString& aTargetDb = "");

    /*! \brief Returns the source database targets
     *
     * @return List of source database targets
     */
    QList<QString> getSourceDbs() const;

    /*! \brief Adds a disabled sync target(required for deleting)
     *
     * @param aSourceDb Source database
     */
    void addDisabledSyncTarget( const QString& aSourceDb);

    /*! \brief Returns the disabled(UI) source database targets
     *
     * @return List of disabled  source database targets
     */
    QList<QString> getDisabledSourceDbs() const;

    /*! \brief Returns the set target database for given source database
     *
     * @return Target database if found, otherwise ""
     */
    QString getTarget( const QString& aSourceDb ) const;

    /*! \brief Returns a map containing all source -> target databases
     *
     * @return Map of source -> target databases
     */
    const QMap<QString, QString>* getTargets() const;

    /*! \brief Sets the type of sync to be used
     *
     * If not set, defaults to two-way fast sync initiated by client
     *
     * @param aSyncMode Sync mode to use
     *
     */
    void setSyncMode( const SyncMode& aSyncMode );

    /*! \brief Returns the type of sync to be used
     *
     * @return
     */
    const SyncMode& getSyncMode() const;

    /*! \brief Sets the local device identification to be used in sync
     *
     * If not set, defaults to device ID specified in device info
     *
     * @param aLocalDevice Local Device identification
     */
    void setLocalDevice( const QString& aLocalDevice );

    /*! \brief Returns the local device identification to be used in sync
     *
     * @return
     */
    QString getLocalDevice() const;

    /*! \brief Sets the remote device identification to be used in sync
     *
     * If not set, defaults to "/"
     *
     * @param aRemoteDevice Remote device identification
     */
    void setRemoteDevice( const QString& aRemoteDevice );

    /*! \brief Returns the remote device identification to be used in sync
     *
     * @return
     */
    QString getRemoteDevice() const;

    /*! \brief Sets the authentication type to be used
     *
     * If not set, defaults to AUTH_NONE
     *
     * @param aAuthenticationType Authentication type to be used
     */
    void setAuthenticationType( AuthenticationType aAuthenticationType );

    /*! \brief Gets the authentication type to be used
     *
     * @return
     */
    AuthenticationType getAuthenticationType() const;

    /*! \brief Sets the user name to be used in SyncML authentication
     *
     * If not set, SyncML authentication is not attempted
     *
     * @param aUsername Username to use
     */
    void setUsername( const QString& aUsername );

    /*! \brief Returns the user name to be used in SyncML authentication
     *
     * @return
     */
    QString getUsername() const;

    /*! \brief Sets the password to be used in SyncML authentication
     *
     * If not set, SyncML authentication is not attempted
     *
     * @param aPassword Password to use
     */
    void setPassword( const QString& aPassword );

    /*! \brief Returns the password to be used in SyncML authentication
     *
     * @return
     */
    QString getPassword() const;

    /*! \brief Sets conflict resolution policy to use
     *
     * If not set, defaults to PREFER_LOCAL_CHANGES
     *
     * @param aPolicy Conflict resolution policy
     */
    void setConflictResolutionPolicy( ConflictResolutionPolicy aPolicy );

    /*! \brief Returns the conflict resolution policy to use
     *
     * @return
     */
    ConflictResolutionPolicy getConflictResolutionPolicy() const;

    /*! \brief Set the database file location (full path)
     *
     * If not set, defaults to /tmp/syncml.db
     *
     * @param aPath Full database file path
     */
    void setDatabaseFilePath( const QString& aPath );

    /*! \brief Returns the database file location (full path)
     *
     * @return Full path to database file
     */
    QString getDatabaseFilePath() const;

    /**
     * \brief Get the protocol version for the synchronization.
     *
     * @return
     */
    ProtocolVersion getProtocolVersion() const;


    /*! \brief Set the protocol version for the synchronization.
     *
     * If not set, defaults to DS_1_2
     *
     * @param aVersion Protocol version
     */
    void setProtocolVersion( ProtocolVersion aVersion );

    /*! \brief Sets a protocol attribute for the synchronization.
     *
     * @see ProtocolAttributes for list of attributes
     *
     * @param aAttribute Attribute to set
     */
    void setProtocolAttribute( int aAttribute );

    /*! \brief Clears a protocol attribute for the synchronization.
     *
     * @see ProtocolAttributes for the list of attributes
     *
     * @param aAttribute Attribute to clear
     */
    void clearProtocolAttribute( int aAttribute );

    /*! \brief Checks if protocol attribute is set
     *
     * @see ProtocolAttributes for list of attributes
     *
     * @param aAttribute to check
     * @return True if attribute is set, otherwise false
     */
    bool getProtocolAttribute( int aAttribute ) const;

    /*! \brief Sets No. of changes to be sent per syncml message
     *
     *
     * @param aChanges number of changes
     */
    void setMaxChangesToSend(int aChanges);

    /*! \brief get the No. of changes that can be sent
     *
     * @return number of changes
     */
    int getMaxChangesToSend() const;

    /*! \brief Sets the Device Info to be used
     *
     * @param aDeviceInfo Device info to use
     */
    void setDeviceInfo(const DeviceInfo &aDeviceInfo);

    /*! \brief gets the Device Info of the Config
     *
     * @return DeviceInfo
     */
    DeviceInfo& getDeviceInfo();

    /*! \brief gets the Device Info of the Config
     *
     * @return DeviceInfo
     */
    const DeviceInfo& getDeviceInfo() const;

private:

    Transport*                      iTransport;
    StorageProvider*                iStorageProvider;
    QMap<QString, QString>          iTargets;
    QList<QString>                  iTargetDbs;
    QList<QString>                  iDTargetDbs;
    SyncMode                        iSyncMode;
    QString                         iUsername;
    QString                         iPassword;
    ConflictResolutionPolicy        iConflictResolutionPolicy;
    QString                         iDatabaseFilePath;
    ProtocolVersion                 iProtocolVersion;
    AuthenticationType              iAuthenticationType;
    QBitArray                       iProtocolAttributes;
    int 							iMaxChangesToSend;
    DeviceInfo                      iDeviceInfo;
    QString                         iLocalDeviceName;
    QString                         iRemoteDeviceName;

    friend class SyncAgentConfigTest;
};

}

Q_DECLARE_METATYPE(DataSync::SyncAgentConfig);

#endif  //  SYNCAGENTCONFIG_H
