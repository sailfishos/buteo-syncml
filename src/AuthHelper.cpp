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

#include "AuthHelper.h"

#include <QCryptographicHash>

#include "LogMacros.h"

using namespace DataSync;

AuthHelper::AuthHelper()
{
    FUNCTION_CALL_TRACE;
}

AuthHelper::~AuthHelper()
{
    FUNCTION_CALL_TRACE;
}

QByteArray AuthHelper::encodeBasicB64Auth( const AuthData& aAuthData ) const
{
    FUNCTION_CALL_TRACE;

    QByteArray in;

    in.append( aAuthData.iUsername );
    in.append( ':' );
    in.append( aAuthData.iPassword );

    return in.toBase64();

}

bool AuthHelper::decodeBasicB64EncodedAuth( const QByteArray& aEncodedData, AuthData& aAuthData )
{
    FUNCTION_CALL_TRACE;

    bool successful = false;

    QByteArray in = QByteArray::fromBase64( aEncodedData );

    QList<QByteArray> splits = in.split( ':' );

    if( splits.count() == 2 ) {
        aAuthData.iUsername = splits[0];
        aAuthData.iPassword = splits[1];
        successful = true;
    }

    return successful;
}

QByteArray AuthHelper::encodeMD5B64Auth( const AuthData& aAuthData, const QByteArray& aNonce ) const
{
    FUNCTION_CALL_TRACE;

    QByteArray in;

    in.append( aAuthData.iUsername );
    in.append( ':' );
    in.append( aAuthData.iPassword );

    QByteArray tmp = toMD5( in );
    tmp = tmp.toBase64();

    tmp.append( ':' );
    tmp.append( aNonce );

    tmp = toMD5( tmp );

    return tmp.toBase64();

}

QByteArray AuthHelper::toMD5( const QByteArray& aString ) const
{
    FUNCTION_CALL_TRACE;

    QCryptographicHash md5Hash( QCryptographicHash::Md5 );
    md5Hash.addData( aString );


    return md5Hash.result();

}
