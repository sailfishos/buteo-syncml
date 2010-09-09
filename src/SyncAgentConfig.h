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
#include <QVariant>

#include "SyncAgentConsts.h"
#include "SyncMode.h"
#include "DeviceInfo.h"

class QXmlStreamReader;

namespace DataSync {

class StorageProvider;
class Transport;
class SyncAgentConfigTest;


/*! \brief Configuration object for the SyncAgent
 *
 * This class specifies all necessary configuration parameters for
 * SyncAgent. Configuration parameters are divided between critical
 * parameters, non-critical properties and extensions. Critical
 * parameters can be accessed and manipulated using get/set-functions
 * related to a particular parameter. Non-critical properties are
 * divided between agent-related properties and transport-related
 * properties. Extensions can be used to enable or disable features
 * that SyncAgent supports that are not directly related to OMA DS
 * protocol.
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

    /*! \brief Internalizes configuration from a file
     *
     * It should be noted that only common configuration can be internalized
     * from a file. Configuration related to callback interfaces
     * ( setTransport() and setStorageProvider() ) and initiating a
     * synchronization session cannot be internalized from a file, and must
     * always be explicitly set.
     *
     * @param aFile Configuration file
     * @param aSchemaFile XML Schema file used when validating configuration file
     * @return True on success, otherwise false
     */
    bool fromFile( const QString& aFile,
                   const QString& aSchemaFile = "/etc/sync/meego-syncml-conf.xsd" );

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

    /*! \brief Sets the device information to be used
     *
     * @param aDeviceInfo Device info to use
     */
    void setDeviceInfo(const DeviceInfo &aDeviceInfo);

    /*! \brief Returns the device information to be user
     *
     * @return
     */
    const DeviceInfo& getDeviceInfo() const;

    /*! \brief Set the database file location (full path)
     *
     * If not set, defaults to /etc/syncml/syncml.db
     *
     * @param aPath Full database file path
     */
    void setDatabaseFilePath( const QString& aPath );

    /*! \brief Returns the database file location (full path)
     *
     * @return Full path to database file
     */
    QString getDatabaseFilePath() const;

    /*! \brief Sets the local device name to be used in sync
     *
     * If not set, defaults to device ID specified in device info
     *
     * @param aLocalDevice Local Device identification
     */
    void setLocalDeviceName( const QString& aLocalDevice );

    /*! \brief Returns the local device name to be used in sync
     *
     * @return
     */
    const QString& getLocalDeviceName() const;

    /*! \brief Sets a property value related to the agent
     *
     * If property already exists, old value is replaced. See
     * SyncAgentConfigProperties.h for a list of supported properties.
     *
     * @param aProperty Property to set
     * @param aValue Value to set
     */
    void setAgentProperty( const QString& aProperty, const QString& aValue );

    /*! \brief Gets a property value related to the agent
     *
     * @param aProperty Property to get
     * @return Value of property if found, otherwise empty
     */
    QString getAgentProperty( const QString& aProperty ) const;

    /*! \brief Sets a property value related to the transport
     *
     * If property already exists, old value is replaced. See
     * SyncAgentConfigProperties.h for a list of supported properties.
     *
     * @param aProperty Property to set
     * @param aValue Value to set
     */
    void setTransportProperty( const QString& aProperty, const QString& aValue );

    /*! \brief Gets a property value related to the transport
     *
     * @param aProperty Property to get
     * @return Value of property if found, otherwise empty
     */
    QString getTransportProperty( const QString& aProperty ) const;

    /*! \brief Set parameters for initiating synchronization
     *
     * It should be noted that these parameters are relevant only for
     * initiating synchronization, they are not used when serving. By default,
     * remote device name is set to unknown device identifier ("/"), protocol
     * version 1.2 is used, and two-way synchronization is performed.
     *
     * @param aRemoteDeviceName Name of the remote device to synchronize with.
     *                          If not set, defaults to unknown device "/"
     * @param aVersion Protocol version to use. If not set, defaults to DS_1_2
     * @param aSyncMode Type of sync to be used. If not set, defaults to
     *                  two-way fast sync initiated by client
     */
    void setSyncParams( const QString& aRemoteDeviceName,
                        ProtocolVersion aVersion,
                        const SyncMode& aSyncMode );

    /*! \brief Returns the remote device name to be used in sync
     *
     * @return
     */
    const QString& getRemoteDeviceName() const;

    /*! \brief Returns the protocol version to be used in sync
     *
     * @return
     */
    const ProtocolVersion& getProtocolVersion() const;

    /*! \brief Returns the type of sync to be used in sync
     *
     * @return
     */
    const SyncMode& getSyncMode() const;

    /*! \brief Sets the parameters to authenticate a synchronization session
     *
     * It should be noted that these parameters are relevant only for
     * initiating synchronization, they are not used when serving. By default,
     * no authentication is used.
     *
     * @param aAuthType Authentication type to use
     * @param aUsername Username to use (if required, otherwise empty)
     * @param aPassword Password to use (if required, otherwise empty)
     */
    void setAuthParams( const AuthenticationType& aAuthType,
                        const QString& aUsername,
                        const QString& aPassword );

    /*! \brief Returns the authentication type to be used
     *
     * @return
     */
    const AuthenticationType& getAuthenticationType() const;

    /*! \brief Returns the user name to use
     *
     * @return
     */
    const QString& getUsername() const;

    /*! \brief Returns the password to be used in SyncML authentication
     *
     * @return
     */
    const QString& getPassword() const;

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

    /*! \brief Enables a protocol extension
     *
     * @param aName Name of the extension
     * @param aData Data of the extension
     */
    void setExtension( const QString& aName, const QVariant& aData = QVariant() );

    /*! \brief Returns whether protocol extension has been enabled
     *
     * @param aName Name of the extension
     * @return
     */
    bool extensionEnabled( const QString& aName ) const;

    /*! \brief Returns data of enaled extension
     *
     * @param aName
     * @return
     */
    QVariant getExtensionData( const QString& aName ) const;

    /*! \brief Disables a protocol extension
     *
     * @param aName Name of the extension
     */
    void clearExtension( const QString& aName );

private:

    /*! \brief Read a file
     *
     * @param aFileName Filename of the file to read
     * @param aData If reading succeeded, contains file data
     * @return True if reading succeeded, false otherwise
     */
    bool readFile( const QString& aFileName, QByteArray& aData ) const;

    /*! \brief Parse contents of configuration file
     *
     * @param aData Data of conf file
     * @return True on success, otherwise false
     */
    bool parseConfFile( const QByteArray& aData );

    /*! \brief Parse agent properties
     *
     * @param aReader XML stream reader to use
     * @return True on success, otherwise false
     */
    bool parseAgentProps( QXmlStreamReader& aReader );

    /*! \brief Parse transport properties
     *
     * @param aReader XML stream reader to use
     * @return True on success, otherwise false
     */
    bool parseTransportProps( QXmlStreamReader& aReader );

    /*! \brief Parse protocol extensions
     *
     * @param aReader XML stream reader to use
     * @return True on success, otherwise false
     */
    bool parseSyncExtensions( QXmlStreamReader& aReader );

    /*! \brief Parse EMI tags extension data
     *
     * @param aReader XML stream reader to use
     * @return True on success, otherwise false
     */
    bool parseEMITagsExtension( QXmlStreamReader& aReader );

    /*! \brief Parse SAN Mappings extension data
     *
     * @param aReader XML stream reader to use
     * @return True on success, otherwise false
     */
    bool parseSANMappingsExtension( QXmlStreamReader& aReader );

    /*! \brief Parse data of single SAN Mapping
     *
     * @param aReader XML stream reader to use
     * @param aMappings Mappings table to insert data to
     * @return True on success, otherwise false
     */
    bool parseSANMappingData( QXmlStreamReader& aReader,
                              QStringList& aMappings );

    Transport*                      iTransport;
    StorageProvider*                iStorageProvider;

    QString                         iDatabaseFilePath;
    QString                         iLocalDeviceName;
    DeviceInfo                      iDeviceInfo;

    QMap<QString, QString>          iAgentProperties;
    QMap<QString, QString>          iTransportProperties;

    QString                         iRemoteDeviceName;
    ProtocolVersion                 iProtocolVersion;
    SyncMode                        iSyncMode;
    AuthenticationType              iAuthenticationType;
    QString                         iUsername;
    QString                         iPassword;

    QMap<QString, QVariant>         iExtensions;

    QMap<QString, QString>          iTargets;
    QList<QString>                  iTargetDbs;
    QList<QString>                  iDTargetDbs;

    friend class SyncAgentConfigTest;
};

}

Q_DECLARE_METATYPE(DataSync::SyncAgentConfig);

#endif  //  SYNCAGENTCONFIG_H
