/*
 * This file is part of buteo-syncml package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
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
    const DeviceInfo& getDeviceInfo() const;

    /*! \brief Initiate a device info exchange with remote device
     *
     * @param aDataStores Data stores to use
     * @param aVersion Protocol version to use
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
     * @param aVersion Protocol version to use
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
     * @param aActionData GET element data
     * @param aVersion Protocol version to use
     * @return
     */
    ResponseStatusCode handlePut( const SyncActionData& aActionData,
                                  const ProtocolVersion& aVersion );

    /*! \brief Respond to device info received from remote device
     *
     * @param aActionData RESULTS element data
     * @param aVersion Protocol version to use
     */
    ResponseStatusCode handleResults( const ResultsParams& aActionData,
                                      const ProtocolVersion& aVersion );

protected:

private:

    friend class ::DevInfHandlerTest;

    DeviceInfo  iDeviceInfo;
    bool        iLocalDevInfSent;
    bool        iRemoteDevInfReceived;

};

}

#endif  //  DEVINFHANDLER_H
