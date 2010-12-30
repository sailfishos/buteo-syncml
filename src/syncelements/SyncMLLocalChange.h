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

#ifndef SYNCMLLOCALCHANGE_H
#define SYNCMLLOCALCHANGE_H

#include "SyncMLCmdObject.h"

namespace DataSync {

class SyncMLMeta;

/*! \brief SyncMLLocalChange is a base class for SyncML XML objects
 *  related to local changes ( Add, Delete, Replace commands )
 */
class SyncMLLocalChange : public SyncMLCmdObject
{
public:

    /*! \brief Constructor
     *
     * @param aElementName Name of this element. Filled out by derived class
     * @param aCmdID Command id for this element
     */
    SyncMLLocalChange( const QString &aElementName, int aCmdID );

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLLocalChange();

    /*! \brief Adds metadata element as child about content mime
     *
     * @param aMimeType Mime type of the content
     */
    void addMimeMetadata( const QString& aMimeType );

    /*! \brief Adds metadata element as child about content size
     *
     * @param aSize Size of the content
     */
    void addSizeMetadata( const qint64 aSize );

    /*! \brief Adds metadata element as child about content format
     *
     * @param aFormat Format of the content
     */
    void addFormatMetadata( const QString& aFormat );

    /*! \brief Adds version meta element as child about content format
     *
     * @param aVersion of the content
     */
    void addVersionMetadata(const QString& aVersion);

protected:

private:
    SyncMLCmdObject* generateCmdElement( int aCmdID ) const;

    void ensureMetaElement();

    SyncMLMeta* iMetaObject;
};

}
#endif  //  SYNCMLLOCALCHANGE_H
