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

#include "SyncResults.h"

#include "LogMacros.h"

using namespace DataSync;

SyncResults::SyncResults() : iState( NOT_PREPARED )
{
    FUNCTION_CALL_TRACE
}

DataSync::SyncResults::~SyncResults()
{
    FUNCTION_CALL_TRACE
}

void SyncResults::reset()
{
    FUNCTION_CALL_TRACE
    iResults.clear();
}

SyncState SyncResults::getState() const
{
    FUNCTION_CALL_TRACE
    return iState;
}

void SyncResults::setState( const SyncState& aState )
{
    FUNCTION_CALL_TRACE
    iState = aState;
}

QString SyncResults::getErrorString() const
{
    FUNCTION_CALL_TRACE
    return iErrorString;
}

void SyncResults::setErrorString( const QString& aErrorString )
{
    FUNCTION_CALL_TRACE
    iErrorString = aErrorString;
}

void SyncResults::setRemoteDeviceId( const QString& aDeviceId )
{
    FUNCTION_CALL_TRACE
    iRemoteId = aDeviceId;
}

QString SyncResults::getRemoteDeviceId( ) const
{
    FUNCTION_CALL_TRACE
    return iRemoteId;
}

const QMap<QString, DatabaseResults>* SyncResults::getDatabaseResults() const
{
    FUNCTION_CALL_TRACE
    return &iResults;
}

void SyncResults::addProcessedItem( DataSync::ModificationType aModificationType,
                                    DataSync::ModifiedDatabase aModifiedDatabase,
                                    const QString& aDatabase )
{
    FUNCTION_CALL_TRACE

    DatabaseResults& results = iResults[aDatabase];

    if( aModifiedDatabase == MOD_LOCAL_DATABASE ) {

        if( aModificationType == MOD_ITEM_ADDED ) {
            ++results.iLocalItemsAdded;
        }
        else if( aModificationType == MOD_ITEM_MODIFIED ) {
            ++results.iLocalItemsModified;
        }
        else if( aModificationType == MOD_ITEM_DELETED ) {
            ++results.iLocalItemsDeleted;
        }

    }
    else if( aModifiedDatabase == MOD_REMOTE_DATABASE ) {

        if( aModificationType == MOD_ITEM_ADDED ) {
            ++results.iRemoteItemsAdded;
        }
        else if( aModificationType == MOD_ITEM_MODIFIED ) {
            ++results.iRemoteItemsModified;
        }
        else if( aModificationType == MOD_ITEM_DELETED ) {
            ++results.iRemoteItemsDeleted;
        }

    }

}
