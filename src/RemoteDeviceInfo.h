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
#ifndef REMOTEDEVICEINFO_H
#define REMOTEDEVICEINFO_H

#include "datatypes.h"
#include "internals.h"

namespace DataSync
{

// @todo: How about simulataneous sync sessions, won't a singleton be problematic then?
/*! \brief DeviceInfo is a singleton that includes
 *  some information about the remote device
 *
 */
class RemoteDeviceInfo
{

public:


    /*! \brief Gets a pointer to the single instance
     *
     * @return pointer to RemoteDeviceInfo object
     */
    static RemoteDeviceInfo* instance();

    /*! \brief Destroys the single instance of RemoteDeviceInfo
     *
     */
    static void destroyInstance();

    /*! \brief Get the sync types supported by the remote device
     *
     * @return list of supported sync types
     */
    QList<DataSync::SyncTypes> supportedSyncTypes() const;

    /*! \brief Adds a supported sync type
     *
     * @param aType Supported sync type
     */
    void populateSupportedSyncType( DataSync::SyncTypes aType );

    /*! \brief Get the source URI
     *
     * @return Source URI
     */
    QString sourceURI() const;

    /*! \brief Set the source URI
     *
     * @param aSource Source URI
     */
    void populateSourceURI( const QString& aSource );

    /*! \brief Get the target URI
     *
     * @return target URI
     */
    QString targetURI() const;

    /*! \brief Set the target URI
     *
     * @param aTarget Target URI
     */
    void populateTargetURI( const QString& aTarget );

    /*! \brief Get the meta params
     *
     * @return Meta params
     */
    const MetaParams& meta() const;

    /*! \brief Set the meta params
     *
     * @param aMeta Meta params
     */
    void populateMeta( const MetaParams& aMeta);

    /*! \brief Checks support for large objects
     *
     * @return True if large objects are supported, false otherwise
     */
    bool isLargeObjectSupported() const;

    /*! \brief Populates support for large objects
     *
     */
    void setLargeObjectSupported();

    /*! \brief Gets the manufacturer
     *
     * @return Manufacturer name
     */
    QString manufacturer() const;

    /*! \brief Populates the manufacturer
     *
     * @param aManufacturer Manufacturer name
     */
    void populateManufacturer( const QString& aManufacturer );

    /*! \brief Gets the SW version
     *
     * @return SW Version
     */
    QString swVersion() const;

    /*! \brief Populates the sw version
     *
     * @param aSwVersion SW version
     */
    void populateSwVersion( const QString& aSwVersion);

    /*! \brief Gets the model
     *
     * @return Model
     */
    QString model() const;

    /*! \brief Populates the model
     *
     * @return aModel Model
     */
    void populateModel( const QString& aModel);

private:

    /*! \brief Constructor
     *
     */
    RemoteDeviceInfo();

    /*! \brief Destructor
     *
     */
    ~RemoteDeviceInfo();

    static RemoteDeviceInfo* iInstance;
    QList<DataSync::SyncTypes> iSupportedSyncTypes;
    QString iSource;
    QString iTarget;
    MetaParams iMeta;
    bool iSupportsLargeObjects;
    QString iManufacturer;
    QString iSwVersion;
    QString iModel;

    // @todo: There's more in device info

};
}

#endif  //  REMOTEDEVICEINFO_H
