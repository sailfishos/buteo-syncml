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


RemoteDeviceInfo::RemoteDeviceInfo()
: iUTC( false ), iSupportLargeObjs( false ), iSupportNumberOfChanges( false )
{
    FUNCTION_CALL_TRACE;
}

RemoteDeviceInfo::~RemoteDeviceInfo()
{
    FUNCTION_CALL_TRACE;
}

DeviceInfo& RemoteDeviceInfo::deviceInfo()
{
    return iDeviceInfo;
}

const DeviceInfo& RemoteDeviceInfo::deviceInfo() const
{
    return iDeviceInfo;
}

QList<Datastore>& RemoteDeviceInfo::datastores()
{
    return iDatastores;
}

const QList<Datastore>& RemoteDeviceInfo::datastores() const
{
    return iDatastores;
}

void RemoteDeviceInfo::setSupportsUTC( bool aUTC )
{
    iUTC = aUTC;
}

bool RemoteDeviceInfo::getSupportsUTC() const
{
    return iUTC;
}

void RemoteDeviceInfo::setSupportsLargeObjs( bool aSupportsLargeObjs )
{
    iSupportLargeObjs = aSupportsLargeObjs;
}

bool RemoteDeviceInfo::getSupportsLargeObjs() const
{
    return iSupportLargeObjs;
}

void RemoteDeviceInfo::setSupportsNumberOfChanges( bool aSupportsNumberOfChanges )
{
    iSupportNumberOfChanges = aSupportsNumberOfChanges;
}

bool RemoteDeviceInfo::getSupportsNumberOfChanges() const
{
    return iSupportNumberOfChanges;
}
