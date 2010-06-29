/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "SyncAgentConfig.h"

#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

SyncAgentConfig::SyncAgentConfig()
 : iTransport( NULL ),
   iStorageProvider( NULL ),
   iConflictResolutionPolicy( PREFER_LOCAL_CHANGES ),
   iDatabaseFilePath( "/tmp/syncml.db" ),
   iProtocolVersion( DS_1_2 ),
   iAuthenticationType( AUTH_NONE ),
   iProtocolAttributes( LAST_PROTOCOL_ATTRIBUTE ),
   iMaxChangesToSend(DEFAULT_MAX_CHANGES_TO_SEND),
   iRemoteDeviceName( SYNCML_UNKNOWN_DEVICE )
{
    FUNCTION_CALL_TRACE
}


SyncAgentConfig::~SyncAgentConfig()
{
    FUNCTION_CALL_TRACE
}


void SyncAgentConfig::setTransport( Transport* aTransport )
{
     iTransport = aTransport;
}

Transport* SyncAgentConfig::getTransport() const
{
    return iTransport;
}

void SyncAgentConfig::setStorageProvider( StorageProvider* aProvider )
{
    iStorageProvider = aProvider;
}

StorageProvider* SyncAgentConfig::getStorageProvider() const
{
    return iStorageProvider;
}

void SyncAgentConfig::addSyncTarget( const QString& aSourceDb, const QString& aTargetDb )
{
    FUNCTION_CALL_TRACE

    iTargets[aSourceDb] = aTargetDb;
    iTargetDbs.append(aSourceDb);
}

void SyncAgentConfig::addDisabledSyncTarget( const QString& aSourceDb)
{
    FUNCTION_CALL_TRACE
    iDTargetDbs.append(aSourceDb);
}

QList<QString> SyncAgentConfig::getDisabledSourceDbs() const
{
    FUNCTION_CALL_TRACE
    return iDTargetDbs;
}

QList<QString> SyncAgentConfig::getSourceDbs() const
{
    return iTargetDbs;
}

QString SyncAgentConfig::getTarget( const QString& aSourceDb ) const
{
    return iTargets.value( aSourceDb );
}

const QMap<QString, QString>* SyncAgentConfig::getTargets() const
{
    return &iTargets;
}

void SyncAgentConfig::setSyncMode( const SyncMode& aSyncMode )
{
    iSyncMode = aSyncMode;
}

const SyncMode& SyncAgentConfig::getSyncMode() const
{
    return iSyncMode;
}

void SyncAgentConfig::setLocalDevice( const QString& aLocalDevice )
{
    iLocalDeviceName = aLocalDevice;
}

QString SyncAgentConfig::getLocalDevice() const
{
    return iLocalDeviceName;
}

void SyncAgentConfig::setRemoteDevice( const QString& aRemoteDevice )
{
    iRemoteDeviceName = aRemoteDevice;
}

QString SyncAgentConfig::getRemoteDevice() const
{
    return iRemoteDeviceName;
}

void SyncAgentConfig::setAuthenticationType( AuthenticationType aAuthenticationType )
{
    iAuthenticationType = aAuthenticationType;
}

AuthenticationType SyncAgentConfig::getAuthenticationType() const
{
    return iAuthenticationType;
}

void SyncAgentConfig::setUsername( const QString& aUsername )
{
    iUsername = aUsername;
}

QString SyncAgentConfig::getUsername() const
{
    return iUsername;
}

void SyncAgentConfig::setPassword( const QString& aPassword )
{
    iPassword = aPassword;

}

QString SyncAgentConfig::getPassword() const
{
    return iPassword;
}

void SyncAgentConfig::setConflictResolutionPolicy( ConflictResolutionPolicy aPolicy )
{
    iConflictResolutionPolicy = aPolicy;
}

ConflictResolutionPolicy SyncAgentConfig::getConflictResolutionPolicy() const
{
    return iConflictResolutionPolicy;
}

void SyncAgentConfig::setDatabaseFilePath( const QString& aPath )
{
    iDatabaseFilePath = aPath;
}

QString SyncAgentConfig::getDatabaseFilePath() const
{
    FUNCTION_CALL_TRACE
    return iDatabaseFilePath;
}

void SyncAgentConfig::setProtocolVersion(ProtocolVersion aVersion)
{
    iProtocolVersion = aVersion;
}

ProtocolVersion SyncAgentConfig::getProtocolVersion() const
{
    return iProtocolVersion;

}

void SyncAgentConfig::setProtocolAttribute( int aAttribute )
{
    iProtocolAttributes.setBit( aAttribute );
}

void SyncAgentConfig::clearProtocolAttribute( int aAttribute )
{
    iProtocolAttributes.clearBit( aAttribute );
}

bool SyncAgentConfig::getProtocolAttribute( int aAttribute ) const
{
    return iProtocolAttributes.at( aAttribute );
}


void SyncAgentConfig::setMaxChangesToSend(int aChanges)
{
	if(aChanges > 0) {
		iMaxChangesToSend = aChanges;
	} // no else
}

int SyncAgentConfig::getMaxChangesToSend() const
{
	return iMaxChangesToSend;
}


void SyncAgentConfig::setDeviceInfo( const DeviceInfo &aDeviceInfo )
{
    iDeviceInfo = aDeviceInfo;
}

DeviceInfo& SyncAgentConfig::getDeviceInfo()
{
    return iDeviceInfo;
}

const DeviceInfo& SyncAgentConfig::getDeviceInfo() const
{
    return iDeviceInfo;
}
