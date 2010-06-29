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
#ifndef SYNCMLITEM_H
#define SYNCMLITEM_H

#include "SyncMLCmdObject.h"
#include "SyncItemKey.h"

namespace DataSync {



/*! \brief SyncMLItem is class for generating SyncML Item XML object
 */
class SyncMLItem : public SyncMLCmdObject
{
public:

    /*! \brief Constructor
     *
     */
    SyncMLItem();

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLItem();

    /*! \brief Insert Target object as a child
     *
     * @param aLocURI Target locURI
     */
    void insertTarget( const SyncItemKey& aLocURI );

    /*! \brief Insert Source object as a child
     *
     * @param aLocURI Source locURI
     */
    void insertSource( const SyncItemKey& aLocURI );

    /*! \brief Insert TargetParent object as a child
     *
     * @param aLocURI TargetParent locURI
     */
    void insertTargetParent( const SyncItemKey& aLocURI );

    /*! \brief Insert SourceParent object as a child
     *
     * @param aLocURI SourceParent locURI
     */
    void insertSourceParent( const SyncItemKey& aLocURI );

    /*! \brief Insert Data object as a child
     *
     * @param aData Data buffer to use
     */
    void insertData( const QByteArray& aData );

    /*! \brief Insert MoreData object as a child
     *
     * MoreData object should be used when inserted data is only a part of the actual data of an item.
     * In other words, with large objects MoreData should be used to signal remote device that more data
     * should be expected
     *
     */
    void insertMoreData();

protected:

private:
};
}
#endif  //  SYNCMLITEM_H
