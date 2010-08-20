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

#include "DevInfPackage.h"

#include "SyncMLPut.h"
#include "SyncMLGet.h"
#include "SyncMLResults.h"
#include "SyncMLMessage.h"
#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

DevInfPackage::DevInfPackage( const QList<StoragePlugin*>& aDataStores,
                              const DeviceInfo& aDeviceInfo,
                              const ProtocolVersion& aVersion,
                              const Role& aRole )
: iDataStores( aDataStores ), iDeviceInfo( aDeviceInfo ),
  iVersion( aVersion ), iRole( aRole ), iType( PUTGET )
{
    FUNCTION_CALL_TRACE;
}

DevInfPackage::DevInfPackage( int aMsgRef, int aCmdRef,
                              const QList<StoragePlugin*>& aDataStores,
                              const DeviceInfo& aDeviceInfo,
                              const ProtocolVersion& aVersion,
                              const Role& aRole,
                              bool aRetrieveRemoteDevInf )
: iMsgRef( aMsgRef ), iCmdRef( aCmdRef ), iDataStores( aDataStores ),
  iDeviceInfo( aDeviceInfo ),iVersion( aVersion ), iRole( aRole )
{
    FUNCTION_CALL_TRACE;

    if( aRetrieveRemoteDevInf )
    {
        iType = RESULTSGET;
    }
    else
    {
        iType = RESULTS;
    }
}

DevInfPackage::~DevInfPackage()
{
    FUNCTION_CALL_TRACE;
}

bool DevInfPackage::write( SyncMLMessage& aMessage, int& aSizeThreshold )
{
    FUNCTION_CALL_TRACE;

    if( iType == PUTGET )
    {
        // Compose PUT
        SyncMLPut* put = new SyncMLPut( aMessage.getNextCmdId(), iDataStores,
                                        iDeviceInfo, iVersion, iRole );

        // Compose GET
        SyncMLGet* get = new SyncMLGet( aMessage.getNextCmdId(), SYNCML_CONTTYPE_DEVINF_XML,
                                        iVersion == DS_1_1 ? SYNCML_DEVINF_PATH_11 : SYNCML_DEVINF_PATH_12 );

        aSizeThreshold -= put->sizeAsXML();
        aMessage.addToBody( put );

        aSizeThreshold -= get->sizeAsXML();
        aMessage.addToBody( get );
    }
    else if( iType == RESULTS )
    {
        // Compose RESULTS
        SyncMLResults* results = new SyncMLResults( aMessage.getNextCmdId(), iMsgRef, iCmdRef,
                                                    iDataStores, iDeviceInfo, iVersion,
                                                    iRole );
        aSizeThreshold -= results->sizeAsXML();
        aMessage.addToBody( results );
    }
    else if( iType == RESULTSGET )
    {
        // Compose RESULTS
        SyncMLResults* results = new SyncMLResults( aMessage.getNextCmdId(), iMsgRef, iCmdRef,
                                                    iDataStores, iDeviceInfo, iVersion,
                                                    iRole );
        // Compose GET
        SyncMLGet* get = new SyncMLGet( aMessage.getNextCmdId(), SYNCML_CONTTYPE_DEVINF_XML,
                                        iVersion == DS_1_1 ? SYNCML_DEVINF_PATH_11 : SYNCML_DEVINF_PATH_12 );

        aSizeThreshold -= results->sizeAsXML();
        aMessage.addToBody( results );

        aSizeThreshold -= get->sizeAsXML();
        aMessage.addToBody( get );
    }
    else
    {
        Q_ASSERT(0);
    }

    return true;
}
