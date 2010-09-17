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
#ifndef DEVINFHANDLER_H
#define DEVINFHANDLER_H

#include "DeviceInfo.h"
#include "SyncAgentConsts.h"
#include "internals.h"

class DevInfHandlerTest;

namespace DataSync
{

class StoragePlugin;
class ResponseGenerator;

/*! \brief Class that governs the exchange of device information in
 *         synchronization session
 */
class DevInfHandler
{
public:

    /*! \brief Constructor
     *
     * @param aDeviceInfo Device information object
     */
    DevInfHandler( const DeviceInfo& aDeviceInfo );

    /*! \brief Destructor
     *
     */
    ~DevInfHandler();

    /*! \brief Retrieves the local device information
     *
     * @return
     */
    const DeviceInfo& getLocalDeviceInfo() const;

    /*! \brief Retrieves the remote device information
     *
     * @return
     */
    const RemoteDeviceInfo& getRemoteDeviceInfo() const;

    /*! \brief Initiate a device info exchange with remote device
     *
     * @param aDataStores Data stores to use
     * @param aVersion Protocol version in use
     * @param aRole Role in use
     * @param aResponseGenerator Response generator to use
     */
    void composeLocalInitiatedDevInfExchange( const QList<StoragePlugin*>& aDataStores,
                                              const ProtocolVersion& aVersion,
                                              const Role& aRole,
                                              ResponseGenerator& aResponseGenerator );

    /*! \brief Respond to device info requested by remote device
     *
     * @param aActionData GET element data
     * @param aDataStores Data stores to use
     * @param aVersion Protocol version in use
     * @param aRole Role in use
     * @param aResponseGenerator Response generator to use
     * @return
     */
    ResponseStatusCode handleGet( const SyncActionData& aActionData,
                                  const ProtocolVersion& aVersion,
                                  const QList<StoragePlugin*>& aDataStores,
                                  const Role& aRole,
                                  ResponseGenerator& aResponseGenerator );

    /*! \brief Respond to device info sent by remote device
     *
     * @param aPut PUT element data
     * @param aVersion Protocol version in use
     * @return
     */
    ResponseStatusCode handlePut( const PutParams& aPut,
                                  const ProtocolVersion& aVersion );

    /*! \brief Respond to device info received from remote device
     *
     * @param aResults RESULTS element data
     * @param aVersion Protocol version in use
     */
    ResponseStatusCode handleResults( const ResultsParams& aResults,
                                      const ProtocolVersion& aVersion );

protected:

private:

    friend class ::DevInfHandlerTest;

    DeviceInfo          iLocalDeviceInfo;
    RemoteDeviceInfo    iRemoteDevInfo;
    bool                iLocalDevInfSent;
    bool                iRemoteDevInfReceived;

};

}

#endif  //  DEVINFHANDLER_H
