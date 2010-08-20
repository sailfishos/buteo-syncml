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
