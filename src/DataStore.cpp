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

#include "DataStore.h"

using namespace DataSync;

Datastore::Datastore() : iSupportsHierarchicalSync( false )
{
}

Datastore::~Datastore()
{
}

void Datastore::setSourceURI( const QString& aSourceURI )
{
    iSourceURI = aSourceURI;
}

const QString& Datastore::getSourceURI() const
{
    return iSourceURI;
}

void Datastore::setPreferredRx( const ContentFormat& aFormat )
{
    iRxPref = aFormat;
}

const ContentFormat& Datastore::getPreferredRx() const
{
    return iRxPref;
}

void Datastore::setPreferredTx( const ContentFormat& aFormat )
{
    iTxPref = aFormat;
}

const ContentFormat& Datastore::getPreferredTx() const
{
    return iTxPref;
}

void Datastore::setSupportsHierarchicalSync( bool aSupports )
{
    iSupportsHierarchicalSync = aSupports;
}

bool Datastore::getSupportsHierarchicalSync() const
{
    return iSupportsHierarchicalSync;
}

const QList<ContentFormat>& Datastore::rx() const
{
    return iRx;
}

QList<ContentFormat>& Datastore::rx()
{
    return iRx;
}

const QList<ContentFormat>& Datastore::tx() const
{
    return iTx;
}

QList<ContentFormat>& Datastore::tx()
{
    return iTx;
}

const QList<SyncTypes>& Datastore::syncCaps() const
{
    return iSyncCaps;
}

QList<SyncTypes>& Datastore::syncCaps()
{
    return iSyncCaps;
}

const QList<CTCap>& Datastore::ctCaps() const
{
    return iCTCaps;
}

QList<CTCap>& Datastore::ctCaps()
{
    return iCTCaps;
}
