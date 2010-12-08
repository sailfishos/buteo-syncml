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
#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>
#include "SyncAgentConsts.h"

namespace DataSync {

class SyncMLMessage;
struct StatusParams;

/*! \brief Package is a group of information that is sent to remote party
 *
 * This information can be contents of a single storage, or just a single
 * Alert or Final element. This information may or may not require a response
 * in the form of a SyncML Status element; package can handle responses by
 * itself, or give a signal that someone else should handle the response.
 *
 */
class Package : public QObject
{
    Q_OBJECT;
public:

    /*! \brief Destructor
     *
     */
    virtual ~Package() {}

    /*! \brief Serializes content of package to the given message
     *
     * The maximum size of data to serialize is given in aSizeThreshold. If true write was completed
     * and package can be deleted
     *
     * @param aMessage Message to serialize to
     * @param aSizeThreshold Maximum number of bytes to use. Is decreased for every
     *                       byte written.
     * @return
     *
     */
    virtual bool write( SyncMLMessage& aMessage, int& aSizeThreshold, bool aWBXML, const ProtocolVersion& aVersion ) = 0;

protected:

private:
};

}

#endif  //  PACKAGE_H
