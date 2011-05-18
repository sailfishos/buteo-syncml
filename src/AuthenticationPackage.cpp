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

#include "AuthenticationPackage.h"

#include "SyncMLCred.h"
#include "SyncMLMessage.h"
#include "AuthHelper.h"
#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

AuthenticationPackage::AuthenticationPackage( const AuthType& aAuthType,
                                              const QString& aUsername,
                                              const QString& aPassword,
                                              const QByteArray& aNonce )
 : iAuthType( aAuthType ), iUsername( aUsername ), iPassword( aPassword ),
   iNonce( aNonce )
{
    FUNCTION_CALL_TRACE;

}

AuthenticationPackage::~AuthenticationPackage()
{
    FUNCTION_CALL_TRACE;

}

bool AuthenticationPackage::write( SyncMLMessage& aMessage, int& aSizeThreshold, bool aWBXML, const ProtocolVersion& aVersion )
{
    FUNCTION_CALL_TRACE;

    SyncMLCred* cred = NULL;
    
    AuthHelper helper;

    if( iAuthType == AUTH_BASIC )
    {
        QByteArray data = helper.encodeBasicB64Auth( iUsername, iPassword );
        cred = new SyncMLCred( SYNCML_FORMAT_ENCODING_B64, SYNCML_FORMAT_AUTH_BASIC, data );
    }
    else if( iAuthType == AUTH_MD5 )
    {
        QByteArray data = helper.encodeMD5Auth( iUsername, iPassword, iNonce );
        // @todo: wouldn't necessarily have to B64-encode over WbXML
        data = data.toBase64();
        cred = new SyncMLCred( SYNCML_FORMAT_ENCODING_B64, SYNCML_FORMAT_AUTH_MD5, data );
    }
    else
    {
        Q_ASSERT( 0 );
    }

    if(cred) {
        aMessage.addToHeader( cred );
        aSizeThreshold -= cred->calculateSize(aWBXML, aVersion);
    }

    return true;
}
