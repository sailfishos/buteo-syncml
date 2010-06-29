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

#include "ConflictResolver.h"
#include "LogMacros.h"
#include "SyncAgentConsts.h"
#include <QMap>

using namespace DataSync;

ConflictResolver::ConflictResolver( const LocalChanges& aLocalChanges,
                                    ConflictResolutionPolicy aPolicy )
 : iLocalChanges( aLocalChanges ), iPolicy( aPolicy )
{
    FUNCTION_CALL_TRACE;

}


ConflictResolver::~ConflictResolver()
{
    FUNCTION_CALL_TRACE;
}


bool ConflictResolver::isConflict( const SyncItemKey& aKey,
                                   bool aDelete ) const
{
    FUNCTION_CALL_TRACE;

    // If item can be found from modified list, it is always a conflict. If item can
    // be found from removed list, it is conflict only if command doesn't involve deleting the item

    bool removalConflict = iLocalChanges.removed.contains( aKey ) && !aDelete;
    bool modificationConflict = iLocalChanges.modified.contains( aKey );

    return ( removalConflict || modificationConflict );
}



bool ConflictResolver::localSideWins() const
{
    FUNCTION_CALL_TRACE;

    if( iPolicy == PREFER_LOCAL_CHANGES )
    {
        return true;
    }
    else
    {
        return false;
    }

}

