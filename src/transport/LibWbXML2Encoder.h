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
#ifndef LIBWBXML2ENCODER_H
#define LIBWBXML2ENCODER_H

#include <QByteArray>

// Needed for compilation outside scratchbox, as only scratchbox version of
// libwbxml2-0 includes wbxml_config.h in wbxml.h.
#define WBXML_SUPPORT_SYNCML

#include <wbxml/wbxml.h>

#include "SyncAgentConsts.h"

// Defined opaquely to match the full definitions in <wbxml/wbxml_tree.h>
typedef struct WBXMLTree_s WBXMLTree;
typedef struct WBXMLTreeNode_s WBXMLTreeNode;

namespace DataSync {

class SyncMLCmdObject;

/*! \brief XML and WbXML encoder that uses libwbxml2
 *
 */
class LibWbXML2Encoder
{

public:

    /*! \brief Constructor
     *
     */
    LibWbXML2Encoder();

    /*! \brief Destructor
     *
     */
    ~LibWbXML2Encoder();

    /*! \brief Encode a SyncML message to XML document
     *
     * @param aRootObject Root object of the document
     * @param aVersion SyncML version
     * @param aXMLDocument Output XML document
     * @param aPrettyPrint If true prefer human-readable output, otherwise prefer compact size
     * @return True on success, otherwise false
     */
    bool encodeToXML( const SyncMLCmdObject& aRootObject, ProtocolVersion aVersion,
                      QByteArray& aXMLDocument, bool aPrettyPrint ) const;

    /*! \brief Encode a SyncML message to WbXML document
     *
     * @param aRootObject Root object of the document
     * @param aVersion SyncML version
     * @param aWbXMLDocument Output WbXML document
     * @return True on success, otherwise false
     */
    bool encodeToWbXML( const SyncMLCmdObject& aRootObject, ProtocolVersion aVersion,
                        QByteArray& aWbXMLDocument ) const;

    /*! \brief Decode a SyncML message from WbXML document
     *
     * @param aWbXMLDocument Message to decode
     * @param aXMLDocument Output XML document
     * @param aPrettyPrint If true prefer human-readable output, otherwise prefer compact size
     * @return True on success, otherwise false
     */
    bool decodeFromWbXML( const QByteArray& aWbXMLDocument, QByteArray& aXMLDocument,
                          bool aPrettyPrint ) const;

protected:

private:

    WBXMLTree* generateTree( const SyncMLCmdObject& aRootObject, ProtocolVersion aVersion ) const;

    WBXMLTree* createTree( const SyncMLCmdObject& aObject, ProtocolVersion aVersion ) const;

    void destroyTree( WBXMLTree* aTree ) const;

    bool createNode( const SyncMLCmdObject& aObject, WBXMLTree* aTree, WBXMLTreeNode* aParent,
                     ProtocolVersion aVersion ) const;

    WBXMLLanguage namespaceToLanguage( const SyncMLCmdObject& aObject, ProtocolVersion aVersion ) const;

};

}

#endif  //  LIBWBXML2ENCODER_H
