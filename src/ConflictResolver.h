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

#ifndef CONFLICTRESOLVER_H
#define CONFLICTRESOLVER_H

#include "SyncAgentConsts.h"
#include "internals.h"

namespace DataSync {

class ConflictResolverTest;

enum ConflictRevertPolicy {
    CR_REMOVE_LOCAL,   /*!<Remove the local conflict change from local changes*/
    CR_MODIFY_TO_ADD   /*!<Modify local change (send add instead of replace) */
};


/*! \brief Class for conflict resolution
 *
 *   This class resolves the conflicts of items based on the current conflict
 *   resolution policy.
 */
class ConflictResolver {
public:

    /*! \brief Constructor
     *
     * @param aLocalChanges Local changes to check conflicts against	
     * @param aPolicy Conflict resolution policy
     */
    ConflictResolver( LocalChanges& aLocalChanges,
                      ConflictResolutionPolicy aPolicy );

    /*! \brief Destructor
     *
     */
    ~ConflictResolver();

    /*! \brief Check if executing the command would result in conflict
     *
     * @param aLocalKey Key of the item that command operates on
     * @param aDelete True if command involves deleting the item
     * @return True if command is in conflict, otherwise false
     */
    bool isConflict( const SyncItemKey& aLocalKey,
                     bool aDelete ) const;

    /*! \brief Returns true if in current resolution policy and operation
     *         mode local side wins conflicts
     *
     * @return True if the policy is set to prefer local side
     */
    bool localSideWins() const;
    
    /*! \brief Updates a local change from the list (added/modified/deleted)
     *
     * @param aLocalKey Local UID of the item to remove
     * @param policy The conflict revert policy
     */
    void revertLocalChange( const SyncItemKey& aLocalKey, ConflictRevertPolicy policy ) ;
    
private:

    /**
     * \brief Set up conflict resolution rules
     */
    void setResolutionRules();
    
    /*! \brief Removes a local change from the list (added/modified/deleted)
     *
     * @param aLocalKey Local UID of the item to remove
     */
    void removeLocalChange( const SyncItemKey& aLocalKey ) ;
    
    /*! \brief Updates a modified local change to added item
     *
     * @param aLocalKey Local UID of the item to update
     */
    void changeLocalModifyToLocalAdd( const SyncItemKey& aLocalKey );


private:

    LocalChanges&    iLocalChanges;
    ConflictResolutionPolicy    iPolicy;

    friend class ConflictResolverTest;
};

}

#endif
