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

#include "ConflictResolver.h"
#include "LogMacros.h"
#include "SyncAgentConsts.h"
#include <QMap>

using namespace DataSync;

ConflictResolver::ConflictResolver( LocalChanges& aLocalChanges,
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
    
     if (aKey.isEmpty())
	    return false; 
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

void ConflictResolver::removeLocalChange( const SyncItemKey& aLocalKey ) 
{
    FUNCTION_CALL_TRACE

    bool retval = false;
    if (iLocalChanges.removed.contains( aLocalKey ))    
        retval = iLocalChanges.removed.removeOne( aLocalKey );   
    else
        retval = iLocalChanges.modified.removeOne( aLocalKey ) ;
}
    
void ConflictResolver::changeLocalModifyToLocalAdd( const SyncItemKey& aLocalKey )
{
    FUNCTION_CALL_TRACE

    /* Reason for this is that in the case of a conflict scenario if the remote is delete and local is
     * modify and if remote wins the mapping is lost in remote side so a replace returns with an error*/	    
    for (int i = 0; i < iLocalChanges.modified.size(); ++i) {
        LOG_DEBUG ("Key :" << aLocalKey << "Remote" << iLocalChanges.modified.at(i));  
        if (iLocalChanges.modified.at(i) == aLocalKey){
            LOG_DEBUG ("Change from replace to add ");  
	    iLocalChanges.modified.removeAt(i);
	    iLocalChanges.added.append(aLocalKey);
	    break;
        }
    }	  
}
    
void ConflictResolver::revertLocalChange( const SyncItemKey& aLocalKey, ConflictRevertPolicy policy ) 
{
    FUNCTION_CALL_TRACE
    switch (policy) {
        case CR_REMOVE_LOCAL:
	    removeLocalChange ( aLocalKey );	
        break;
        case CR_MODIFY_TO_ADD:
	    changeLocalModifyToLocalAdd ( aLocalKey );	
	break;
	default:
	    LOG_WARNING ("Wrong ConflictRevertPolicy");
	break;
    }
}
