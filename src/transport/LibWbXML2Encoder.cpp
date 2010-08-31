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

#include "LibWbXML2Encoder.h"

#include "SyncMLMessage.h"
#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

LibWbXML2Encoder::LibWbXML2Encoder()
{
    FUNCTION_CALL_TRACE;
}

LibWbXML2Encoder::~LibWbXML2Encoder()
{
    FUNCTION_CALL_TRACE;
}

bool LibWbXML2Encoder::encodeToXML( const SyncMLCmdObject& aRootObject,
                                    ProtocolVersion aVersion,
                                    QByteArray& aXMLDocument,
                                    bool aPrettyPrint ) const
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Encoding to XML" );

    WBXMLTree* tree = generateTree( aRootObject, aVersion );

    if( !tree ) {
        LOG_CRITICAL( "Could not generate WBXMLTree" );
        return false;
    }

    WBXMLEncoder* encoder = wbxml_encoder_create();

    if( !encoder ) {
        LOG_CRITICAL( "Could not create WBXMLEncoder" );
        wbxml_tree_destroy( tree );
        return false;
    }
    
    wbxml_encoder_set_wbxml_version(encoder, WBXML_VERSION_12);

    if( aPrettyPrint ) {
        wbxml_encoder_set_xml_gen_type( encoder, WBXML_GEN_XML_INDENT );
        wbxml_encoder_set_indent( encoder, 3 );
    }
    else {
        wbxml_encoder_set_xml_gen_type( encoder, WBXML_GEN_XML_COMPACT );
        wbxml_encoder_set_indent( encoder, 0 );
    }

    wbxml_encoder_set_tree( encoder, tree );
    

    WB_UTINY* xml;
    WB_ULONG xml_len;

    WBXMLError error = wbxml_encoder_encode_tree_to_xml( encoder, &xml, &xml_len );
    bool success = false;

    if( error == WBXML_OK ) {
        aXMLDocument.append( (char *)xml, xml_len );
        wbxml_free( xml );
        success = true;
        LOG_DEBUG( "Encoding successful" );
        LOG_DEBUG( "XML buffer size:" << xml_len );
    }
    else {
        success = false;
        LOG_CRITICAL( "XML conversion failed:" << (const char* )wbxml_errors_string( error ) );
    }

    wbxml_tree_destroy( tree );
    wbxml_encoder_destroy( encoder );

    return success;

}

bool LibWbXML2Encoder::encodeToWbXML( const SyncMLCmdObject& aRootObject,
                                      ProtocolVersion aVersion,
                                      QByteArray& aWbXMLDocument ) const
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Encoding to wbXML" );

    WBXMLTree* tree = generateTree( aRootObject, aVersion );

    if( !tree ) {
        LOG_CRITICAL( "Could not generate WBXMLTree" );
        return false;
    }
   

    WBXMLEncoder* encoder = wbxml_encoder_create();

    if( !encoder ) {
        LOG_CRITICAL( "Could not create WBXMLEncoder" );
        wbxml_tree_destroy( tree );
        return false;
    }
    
    wbxml_encoder_set_wbxml_version(encoder, WBXML_VERSION_12);
    //Workaround : For N900 combo sync
    wbxml_encoder_set_use_strtbl (encoder, false);
    wbxml_encoder_set_tree( encoder, tree );

    WB_UTINY* wbxml;
    WB_ULONG wbxml_len;

    WBXMLError error = wbxml_encoder_encode_tree_to_wbxml( encoder, &wbxml, &wbxml_len );
    bool success = false;

    if( error == WBXML_OK ) {
        aWbXMLDocument.append( (char *)wbxml, wbxml_len );
        wbxml_free( wbxml );
        success = true;
        LOG_DEBUG( "Encoding successful" );
        LOG_DEBUG( "wbXML buffer size:" << wbxml_len );
    }
    else {
        LOG_CRITICAL( "wbXML conversion failed:" << (const char* )wbxml_errors_string( error ) );
        success = false;
    }

    destroyTree( tree );

    wbxml_encoder_destroy( encoder );

    return success;
}

bool LibWbXML2Encoder::decodeFromWbxML( const QByteArray& aWbXMLDocument, QByteArray& aXMLDocument,
                                        bool aPrettyPrint ) const
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Converting WbXML to XML");

    WBXMLGenXMLParams params;
    params.lang = WBXML_LANG_UNKNOWN;

    if( aPrettyPrint ) {
        params.gen_type = WBXML_GEN_XML_INDENT;
        params.indent = 3;
    }
    else {
        params.gen_type = WBXML_GEN_XML_COMPACT;
        params.indent = 0;
    }

    params.keep_ignorable_ws = TRUE;

    const WB_UTINY* wbxml = reinterpret_cast<const WB_UTINY*>( aWbXMLDocument.constData() );
    WB_ULONG wbxml_len = aWbXMLDocument.size();

    WB_UTINY* xml;
    WB_ULONG xml_len;

    LOG_DEBUG( "WbXML buffer size: " << wbxml_len );

    WBXMLError error = wbxml_conv_wbxml2xml_withlen( const_cast<WB_UTINY*>( wbxml ), wbxml_len,
                                                     &xml, &xml_len,
                                                     &params );

    QByteArray data;

    if( error == WBXML_OK ) {
        LOG_DEBUG("WbXML to XML conversion succeeded");
        LOG_DEBUG( "XML buffer size: " << xml_len );
        aXMLDocument.append( reinterpret_cast<char*>( xml ), xml_len );
        wbxml_free( xml );
        return true;
    }
    else {
        LOG_DEBUG("WbXML to XML conversion failed: " << (const char* )wbxml_errors_string( error ) );
        return false;
    }

}

WBXMLTree* LibWbXML2Encoder::generateTree( const SyncMLCmdObject& aRootObject,
                                            ProtocolVersion aVersion ) const
{
    FUNCTION_CALL_TRACE;

    WBXMLTree* tree = createTree( aRootObject, aVersion );

    if( !tree ) {
        return NULL;
    }

    if( !createNode( aRootObject, tree, NULL, aVersion) ) {
        destroyTree( tree );
        tree = NULL;
    }

    return tree;
}

WBXMLTree* LibWbXML2Encoder::createTree( const SyncMLCmdObject& aObject,
                                         ProtocolVersion aVersion ) const
{

    FUNCTION_CALL_TRACE;

    WBXMLLanguage language = namespaceToLanguage( aObject, aVersion );

    if( language == WBXML_LANG_UNKNOWN ) {
        LOG_CRITICAL( "Cannot create tree, unknown language!" );
        return NULL;
    }
    else {
        return wbxml_tree_create( language, WBXML_CHARSET_UTF_8 );
    }

}

void LibWbXML2Encoder::destroyTree( WBXMLTree* aTree ) const
{
    FUNCTION_CALL_TRACE;

    wbxml_tree_destroy( aTree );
}

bool LibWbXML2Encoder::createNode( const SyncMLCmdObject& aObject,
                                   WBXMLTree* aTree,
                                   WBXMLTreeNode* aParent,
                                   ProtocolVersion aVersion ) const
{

    // ** Write element name
    QByteArray name = aObject.getName().toUtf8();
    WBXMLTreeNode* node = wbxml_tree_add_xml_elt( aTree, aParent, (unsigned char*)name.constData() );

    if( !node ) {
        LOG_CRITICAL( "Could not create tree node for element" << aObject.getName() );
        return false;
    }

    // ** Write element attributes
    const QMap<QString, QString>& attributes = aObject.getAttributes();
    bool attributesOk = true;

    QMapIterator<QString, QString> i( attributes );
    while( i.hasNext() ) {
        i.next();

        QByteArray attrName = i.key().toUtf8();
        QByteArray attrValue = i.value().toUtf8();
        WBXMLError error = wbxml_tree_node_add_xml_attr( aTree->lang, node,
                                                         (unsigned char*)attrName.constData(),
                                                         (unsigned char*)attrValue.constData() );

        if( error != WBXML_OK ) {
            LOG_CRITICAL( "Could not add attribute" << attrName << "for element" << name );
            attributesOk = false;
            break;
        }
    }

    if( !attributesOk ) {
        return false;
    }

    // ** Write element value
    QByteArray value = aObject.getValue().toUtf8();

    bool valueOk = true;

    if( !value.isEmpty() ) {

        if( aObject.getCDATA() ) {

            WBXMLTreeNode* cdata = wbxml_tree_add_cdata( aTree, node );

            if( !cdata || !wbxml_tree_add_text( aTree, cdata,
                                                (unsigned char*)value.constData(), value.length() ) ) {
                LOG_CRITICAL( "Could not add CDATA for element" << aObject.getName() );
                valueOk = false;
            }

        }
        else if( !wbxml_tree_add_text( aTree, node, (unsigned char*)value.constData(), value.length() ) ) {
            LOG_CRITICAL( "Could not add TEXT for element" << aObject.getName() );
            valueOk = false;
        }

    }

    if( !valueOk ) {
        return false;
    }

    // ** Process element children

    const QList<SyncMLCmdObject*>& children = aObject.getChildren();

    for( int i = 0; i < children.count(); ++i ) {

        SyncMLCmdObject* child = children[i];

        WBXMLLanguage language = namespaceToLanguage( *child, aVersion );

        if( language != WBXML_LANG_UNKNOWN &&
            language != aTree->lang->langID ) {

            LOG_DEBUG( "Children using language" << language << "while parent is using"
                       << aTree->lang->langID );
            LOG_DEBUG( "Creating subtree for children" );

            WBXMLTree* subTree = createTree( *child, aVersion );

            if( !subTree ) {
                LOG_CRITICAL( "Could not create subtree for" << child->getName() );
                return false;
            }

            if( !createNode( *child, subTree, NULL, aVersion ) ) {
                LOG_CRITICAL( "Could not add child to subtree" << child->getName() << "for element" << name );
                return false;
            }

            if( !wbxml_tree_add_tree( aTree, node, subTree ) ) {
                destroyTree( subTree );
                LOG_CRITICAL( "Could not add subtree for" << child->getName() );
                return false;
            }


        }
        else {

            if( !createNode( *child, aTree, node, aVersion ) ) {
                LOG_CRITICAL( "Could not add child" << child->getName() << "for element" << name );
                return false;
            }
        }
    }

    return true;

}

WBXMLLanguage LibWbXML2Encoder::namespaceToLanguage( const SyncMLCmdObject& aObject,
                                                     ProtocolVersion aVersion ) const
{

    QString ns = aObject.getAttributes().value( XML_NAMESPACE );

    if( ns == XML_NAMESPACE_VALUE_SYNCML11 ) {
        return WBXML_LANG_SYNCML_SYNCML11;
    }
    else if( ns == XML_NAMESPACE_VALUE_SYNCML12 ) {
        return WBXML_LANG_SYNCML_SYNCML12;
    }
    else if( ns == XML_NAMESPACE_VALUE_METINF ) {

        if( aVersion == DS_1_1 ) {
            return WBXML_LANG_SYNCML_SYNCML11;
        }
        else if( aVersion == DS_1_2 ) {
            return WBXML_LANG_SYNCML_SYNCML12;
        }
        else {
            return WBXML_LANG_UNKNOWN;
        }

    }
    else if( ns == XML_NAMESPACE_VALUE_DEVINF ) {

        if( aVersion == DS_1_1 ) {
            return WBXML_LANG_SYNCML_DEVINF11;
        }
        else if( aVersion == DS_1_2 ) {
            return WBXML_LANG_SYNCML_DEVINF12;
        }
        else {
            return WBXML_LANG_UNKNOWN;
        }

    }
    else {
        return WBXML_LANG_UNKNOWN;
    }
}
