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

#include "RemoteDeviceInfo.h"
#include "LogMacros.h"

using namespace DataSync;

#define INFO_UNKNOWN "UNKNOWN"

RemoteDeviceInfo* RemoteDeviceInfo::iInstance = 0;

RemoteDeviceInfo* RemoteDeviceInfo::instance()
{
    FUNCTION_CALL_TRACE;
    if( 0 == iInstance )
    {
        iInstance = new RemoteDeviceInfo();
    }
    return iInstance;
}

void RemoteDeviceInfo::destroyInstance()
{
    FUNCTION_CALL_TRACE;
    if( iInstance )
    {
        delete iInstance;
        iInstance = 0;
    }
}

RemoteDeviceInfo::RemoteDeviceInfo(): 
    iSupportsLargeObjects(false), iSource(INFO_UNKNOWN), 
    iTarget(INFO_UNKNOWN), iManufacturer(INFO_UNKNOWN),
    iSwVersion(INFO_UNKNOWN), iModel(INFO_UNKNOWN)
{
    FUNCTION_CALL_TRACE;
}

RemoteDeviceInfo::~RemoteDeviceInfo()
{
    FUNCTION_CALL_TRACE;
}

QList<DataSync::SyncTypes> RemoteDeviceInfo::supportedSyncTypes() const
{
    FUNCTION_CALL_TRACE;
    return iSupportedSyncTypes;
}

void RemoteDeviceInfo::populateSupportedSyncType( DataSync::SyncTypes aType )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, add sync type supported : " << aType);
    iSupportedSyncTypes.append( aType );
}

QString RemoteDeviceInfo::sourceURI() const
{
    FUNCTION_CALL_TRACE;
    return iSource;
}

void RemoteDeviceInfo::populateSourceURI( const QString& aSource )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set source : " << aSource);
    iSource = aSource;
}

QString RemoteDeviceInfo::targetURI() const
{
    FUNCTION_CALL_TRACE;
    return iTarget;
}

void RemoteDeviceInfo::populateTargetURI( const QString& aTarget )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set target : " << aTarget);
    iTarget = aTarget;
}

const MetaParams& RemoteDeviceInfo::meta() const
{
    FUNCTION_CALL_TRACE;
    return iMeta;
}

void RemoteDeviceInfo::populateMeta( const MetaParams& aMeta)
{
    FUNCTION_CALL_TRACE;
    iMeta = aMeta;
}

bool RemoteDeviceInfo::isLargeObjectSupported() const
{
    FUNCTION_CALL_TRACE;
    return iSupportsLargeObjects;
}

void RemoteDeviceInfo::setLargeObjectSupported()
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set large object supported");
    iSupportsLargeObjects = true;
}

QString RemoteDeviceInfo::manufacturer() const
{
    FUNCTION_CALL_TRACE;
    return iManufacturer;
}

void RemoteDeviceInfo::populateManufacturer( const QString& aManufacturer )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set manufacturer : " << aManufacturer);
    iManufacturer = aManufacturer;
}

QString RemoteDeviceInfo::swVersion() const
{
    FUNCTION_CALL_TRACE;
    return iSwVersion;
}

void RemoteDeviceInfo::populateSwVersion( const QString& aSwVersion )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set sw version : " << aSwVersion);
    iSwVersion = aSwVersion;
}

QString RemoteDeviceInfo::model() const
{
    FUNCTION_CALL_TRACE;
    return iModel;
}

void RemoteDeviceInfo::populateModel( const QString& aModel )
{
    FUNCTION_CALL_TRACE;
    LOG_DEBUG("RemoteDeviceInfo, set model : " << aModel);
    iModel = aModel;
}
