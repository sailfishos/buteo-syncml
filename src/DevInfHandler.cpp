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

#include "DevInfHandler.h"

#include "DevInfPackage.h"
#include "ResponseGenerator.h"

#include "LogMacros.h"

using namespace DataSync;

DevInfHandler::DevInfHandler( const DeviceInfo& aDeviceInfo )
 : iDeviceInfo( aDeviceInfo ), iLocalDevInfSent( false ),
   iRemoteDevInfReceived( false )
{
    FUNCTION_CALL_TRACE;
}

DevInfHandler::~DevInfHandler()
{
    FUNCTION_CALL_TRACE;
}

const DeviceInfo& DevInfHandler::getDeviceInfo() const
{
    return iDeviceInfo;
}

void DevInfHandler::composeLocalInitiatedDevInfExchange(
    const QList<StoragePlugin*>& aDataStores, const ProtocolVersion& aVersion,
    const Role& aRole, ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    if( !iLocalDevInfSent )
    {
        DevInfPackage* devInf = new DevInfPackage( aDataStores, iDeviceInfo,
                                                   aVersion, aRole );

        aResponseGenerator.addPackage( devInf );

        iLocalDevInfSent = true;
    }

}

ResponseStatusCode DevInfHandler::handleGet( const SyncActionData& aActionData,
                                             const ProtocolVersion& aVersion,
                                             const QList<StoragePlugin*>& aDataStores,
                                             const Role& aRole,
                                             ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status = NOT_IMPLEMENTED;

    bool valid = false;

    if( aActionData.items.count() == 1 ) {

        if( aVersion == DS_1_1 && aActionData.items[0].target == SYNCML_DEVINF_PATH_11 ) {
            valid = true;
        }
        else if( aVersion == DS_1_2 && aActionData.items[0].target == SYNCML_DEVINF_PATH_12 ) {
            valid = true;
        }

    }

    if( valid )
    {

        // If we haven't yet received remote device info, send GET in addition to the
        // RESULTS
        DevInfPackage* devInf = new DevInfPackage( aResponseGenerator.getRemoteMsgId(),
                                                   aActionData.cmdID,
                                                   aDataStores,
                                                   iDeviceInfo,
                                                   aVersion,
                                                   aRole,
                                                   !iRemoteDevInfReceived );

        aResponseGenerator.addPackage( devInf );

        iLocalDevInfSent = true;

        status = SUCCESS;
    }
    else
    {
        status = COMMAND_FAILED;
    }

    return status;
}

ResponseStatusCode DevInfHandler::handlePut( const SyncActionData& aActionData,
                                             const ProtocolVersion& aVersion )
{
    FUNCTION_CALL_TRACE;

    ResponseStatusCode status = NOT_IMPLEMENTED;

    bool valid = false;

    if( aActionData.items.count() == 1 ) {

        if( aVersion == DS_1_1 && aActionData.items[0].source == SYNCML_DEVINF_PATH_11 ) {
            valid = true;
        }
        else if( aVersion == DS_1_2 && aActionData.items[0].source == SYNCML_DEVINF_PATH_12 ) {
            valid = true;
        }

    }

    if( valid )
    {

        iRemoteDevInfReceived = true;

        status = SUCCESS;
    }
    else
    {
        status = COMMAND_FAILED;
    }

    return status;
}

ResponseStatusCode DevInfHandler::handleResults( const ResultsParams& aActionData,
                                                 const ProtocolVersion& aVersion )
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aActionData);
    Q_UNUSED(aVersion);

    ResponseStatusCode status = NOT_IMPLEMENTED;

    // @todo: currently cannot really validate RESULTS, because parser parses it differently
    // from other elements containing device information
    bool valid = true;

    if( valid )
    {

        iRemoteDevInfReceived = true;

        status = SUCCESS;
    }
    else
    {
        status = COMMAND_FAILED;
    }

    return status;
}
