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

#ifndef SYNCMLMESSAGE_H
#define SYNCMLMESSAGE_H

#include "SyncMLCmdObject.h"
#include "SyncAgentConsts.h"

namespace DataSync {

class SyncMLHdr;
class SyncMLBody;
struct HeaderParams;

/*! \brief SyncMLMessage is class for generating SyncML root XML object
 */
class SyncMLMessage : public SyncMLCmdObject {
public:

    /*! \brief Constructor
     *
     * @param aHeaderParams SyncML header parameters
     * @param aProtocolVersion Protocol version to use
     */
    SyncMLMessage( const HeaderParams& aHeaderParams, ProtocolVersion aProtocolVersion );

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLMessage();

    /*! \brief Adds object to message header as a child
     *
     * @param aObject Object to add
     */
    void addToHeader( SyncMLCmdObject* aObject );

    /*! \brief Adds object to message body as a child
     *
     * @param aObject Object to add
     */
    void addToBody( SyncMLCmdObject* aObject );

    /*! \brief Assigns a command id that next command added to this
     *         message should use
     *
     * @return
     */
    int getNextCmdId();

    /*! \brief Returns message id of this message
     *
     * @return
     */
    int getMsgId() const;

    /*! \brief Returns the protocol version of this message
     *
     * @return Protocol version of this message
     */
    ProtocolVersion getProtocolVersion() const;

protected:

private:

    const int       iMsgId;
    int             iCmdId;
    ProtocolVersion iProtocolVersion;
    SyncMLHdr*      iSyncHdr;
    SyncMLBody*     iSyncBody;
};
}
#endif  //  SYNCMLMESSAGE_H
