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

#include "DeviceInfo.h"
#include "DataStore.h"

namespace DataSync
{

/*! \brief Describes capabilities of remote device as received from protocol
 *         device info
 */
class RemoteDeviceInfo
{

public:

    /*! \brief Constructor
     *
     */
    RemoteDeviceInfo();

    /*! \brief Destructor
     *
     */
    ~RemoteDeviceInfo();

    /*! \brief Access device info
     *
     * @return
     */
    DeviceInfo& deviceInfo();

    /*! \brief Access device info
     *
     * @return
     */
    const DeviceInfo& deviceInfo() const;

    /*! \brief Find index of datastore based on source URI
     *
     * @param aURI Source URI
     * @return
     */
    int findDatastore( const QString& aURI );

    /*! \brief Access list of datastores
     *
     * @return
     */
    QList<Datastore>& datastores();

    /*! \brief Access list of datastores
     *
     * @return
     */
    const QList<Datastore>& datastores() const;

    /*! \brief Sets flag for UTC support
     *
     * @param aUTC Value to set
     */
    void setSupportsUTC( bool aUTC );

    /*! \brief Retrieve flag for UTC support
     *
     * @return
     */
    bool getSupportsUTC() const;

    /*! \brief Sets flag for large object support
     *
     * @param aSupportsLargeObjs Value to set
     */
    void setSupportsLargeObjs( bool aSupportsLargeObjs );

    /*! \brief Retrieves flag for large object support
     *
     * @return
     */
    bool getSupportsLargeObjs() const;

    /*! \brief Sets flag for sending number of changes support
     *
     * @param aSupportsNumberOfChanges
     */
    void setSupportsNumberOfChanges( bool aSupportsNumberOfChanges );

    /*! \brief Retrieves flag for sending number of changes support
     *
     * @return
     */
    bool getSupportsNumberOfChanges() const;

private:

    DeviceInfo          iDeviceInfo;
    QList<Datastore>    iDatastores;
    bool                iUTC;
    bool                iSupportLargeObjs;
    bool                iSupportNumberOfChanges;

};

}

#endif  //  REMOTEDEVICEINFO_H
