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

#ifndef SYNCMLSYNC_H
#define SYNCMLSYNC_H

#include "SyncMLLocalChanges.h"

namespace DataSync {

/*! \brief SyncMLSync is class for generating SyncML Sync XML object
 *
 * Does not support generation of FreeMem and FreeId elements
 */
class SyncMLSync : public SyncMLLocalChanges
{
public:

    /*! \brief Constructor
     *
     * @param aCmdID Command id for this element
     * @param aTarget Target database URI
     * @param aSource Source database URI
     */
    SyncMLSync( int aCmdID, const QString& aTarget, const QString& aSource );

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLSync();

    /*! \brief Add NumberOfChanges object as a child
     *
     * @param aNumberOfChanges Number of changes
     */
    void addNumberOfChanges( int aNumberOfChanges );

private:

};
}
#endif  //  SYNCMLSYNC_H
