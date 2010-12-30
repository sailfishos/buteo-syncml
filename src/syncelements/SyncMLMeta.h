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

#ifndef SYNCMLMETA_H
#define SYNCMLMETA_H

#include "SyncMLCmdObject.h"

namespace DataSync {

/*! \brief SyncMLMeta is class for generating SyncML Meta XML object
 */
class SyncMLMeta : public SyncMLCmdObject
{
public:
    /*! \brief Constructor
     *
     */
    SyncMLMeta();

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLMeta();

    /*! \brief Adds metadata element as child about type. This can be
     *         for example content (MIME) type, or authentication type
     *
     * @param aType Type value
     */
    void addType( const QString& aType );

    /*! \brief Adds metadata element as child about content size
     *
     * @param aSize Size of the content
     */
    void addSize( qint64 aSize );

    /*! \brief Adds metadata element as child about content format
     *
     * @param aFormat Format of the content
     */
    void addFormat( const QString& aFormat );

    /*! \brief Adds metadata element as child about sync anchors
     *
     * @param aLastAnchor Last sync anchor. If empty, Last element is omitted
     * @param aNextAnchor Next sync anchor. If empty, Next element is omitted
     */
    void addAnchors( const QString& aLastAnchor, const QString& aNextAnchor );

    /*! \brief Adds metadata element as child about maximum message size
     *
     * @param aMaxMsgSize Maximum message size
     */
    void addMaxMsgSize( qint64 aMaxMsgSize );

    /*! \brief Adds metadata element as child about maximum object size
     *
     * @param aMaxObjSize Maximum object size
     */
    void addMaxObjSize( qint64 aMaxObjSize );

    /*! \brief Adds metadata element as child about EMI
     *
     * @param aEMI EMI field
     */
    void addEMI( const QString& aEMI );

    /*! \brief Adds metadata element as child about nonce for md5 authentication
     *
     * @param aNextNonce NextNonce field
     */
    void addNextNonce( const QString& aNextNonce );

    /*! \brief Adds metadat element as a child about mark
     *
     * @param aMark aMark field
     */
    void addMark( const QString& aMark );

    /*! \brief Adds version element as a child.
     *
     * @param aVersion field
     */
    void addVersion( const QString& aVersion );

protected:

private:

    void addNamespace( SyncMLCmdObject& aObject );

};

}

#endif
