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

#include "SessionAuthentication.h"

#include "ResponseGenerator.h"
#include "DatabaseHandler.h"
#include "NonceStorage.h"
#include "AuthenticationPackage.h"

#include "LogMacros.h"

using namespace DataSync;


SessionAuthentication::SessionAuthentication()
 : iAuthenticated( false ),
   iAuthenticationPending( false )
{

}

SessionAuthentication::~SessionAuthentication()
{

}

void SessionAuthentication::setSessionParams( AuthType aAuthType,
                                              const QString& aUsername,
                                              const QString& aPassword,
                                              const QString& aNonce )
{
    FUNCTION_CALL_TRACE;

    iAuthType = aAuthType;
    iUsername = aUsername;
    iPassword = aPassword;
    iNonce = aNonce;

    if( iAuthType == AUTH_NONE )
    {
        iAuthenticated = true;
    }
}

bool SessionAuthentication::authenticated()
{
    return iAuthenticated;
}

SessionAuthentication::HeaderStatus SessionAuthentication::analyzeHeader( const HeaderParams& aHeader,
                                                                          ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    HeaderStatus status = HEADER_NOT_HANDLED;

    if( !iAuthenticated )
    {

        if( !aHeader.cred.data.isEmpty() )
        {
            // Handling of Cred's is not implemented, as we're fully supporting Basic+MD5 authentication only in client
            // mode, and in client we do not do challenges. Server side does not support authentication at all, because
            // it's purpose is to serve only D2D sync where authentication is not used. Therefore, abort the session
            // if we receive Creds

            // For now, do not support Creds sent by remote device. There's multiple reason why this is not implemented:
            // 1. Primary use-case for authentication is in client mode, and we always send the Cred there. So remote device
            //    should never send us Creds.
            // 2. Primary use-case for server mode is D2D sync, where authentication is not commonly used
            // 3. Handling of credentials should be better if we want to support authentication in server mode. They're directly
            //    configured to SyncAgentConfig, which is pretty OK for client mode, but for server mode this is bad as user
            //    might have multiple accounts. Also password handling should be more secure, for example by querying them
            //    from outside the stack through some interface.

            aResponseGenerator.addStatus( aHeader, INVALID_CRED );
            status = HEADER_HANDLED_OK;
        }
    }

    return status;

}

SessionAuthentication::StatusStatus SessionAuthentication::analyzeHeaderStatus( const StatusParams& aStatus,
                                                                                DatabaseHandler& aDbHandler,
                                                                                const QString& aLocalDeviceName,
                                                                                const QString& aRemoteDeviceName )
{
    FUNCTION_CALL_TRACE;

    StatusStatus status = STATUS_NOT_HANDLED;

    if( aStatus.data == SUCCESS )
    {
        iAuthenticated = true;
        status = STATUS_HANDLED_OK;
    }
    else if( aStatus.data == AUTH_ACCEPTED ||
             aStatus.data == INVALID_CRED ||
             aStatus.data == MISSING_CRED )
    {
        // Clear possible nonce, because it was intended only for auth in this session
        NonceStorage nonces( aDbHandler.getDbHandle() );
        nonces.clearNonce( aLocalDeviceName, aRemoteDeviceName );

        // If remote party sent us a next nonce, save it
        QByteArray nonce = decodeNonce( aStatus.chal );


        if( !nonce.isEmpty() )
        {
            nonces.addNonce( aLocalDeviceName, aRemoteDeviceName, nonce );
        }

        if( aStatus.data == AUTH_ACCEPTED )
        {
            // Authentication was accepted
            iAuthenticated = true;
            iAuthenticationPending = false;
            status = STATUS_HANDLED_OK;
        }
        else
        {

            // Authentication was rejected

            iAuthenticated = false;

            if( iAuthType == AUTH_NONE )
            {
                status = SessionAuthentication::STATUS_HANDLED_ABORT;
                iLastError = "Authentication required";
            }
            else if( aStatus.hasChal )
            {
                // Handle challenge
                status = handleChallenge( aStatus.chal, aDbHandler, aLocalDeviceName, aRemoteDeviceName );
            }
            else
            {
                status = STATUS_HANDLED_ABORT;
                iLastError = "Authentication failed";
            }

        }

    }

    return status;

}

void SessionAuthentication::composeAuthentication( ResponseGenerator& aResponseGenerator,
                                                   DatabaseHandler& aDbHandler,
                                                   const QString& aLocalDeviceName,
                                                   const QString& aRemoteDeviceName )
{
    FUNCTION_CALL_TRACE;

    if( iAuthType == AUTH_BASIC ){

        aResponseGenerator.addPackage( new AuthenticationPackage( iUsername,
                                                                  iPassword ) );
        iAuthenticationPending = true;

    }
    else if( iAuthType == AUTH_MD5 ) {

        QByteArray nonce = iNonce.toUtf8();

        if( nonce.isEmpty() )
        {
            NonceStorage nonces( aDbHandler.getDbHandle() );
            nonce = nonces.retrieveNonce( aLocalDeviceName, aRemoteDeviceName );
        }

        if( !nonce.isEmpty() )
        {

            aResponseGenerator.addPackage( new AuthenticationPackage( iUsername,
                                                                      iPassword,
                                                                      nonce ) );

            iAuthenticationPending = true;
        }
        else
        {
            LOG_WARNING( "MD5 authentication requested but no nonce found. Attempting without authentication." );
        }

    }
}

QString SessionAuthentication::getLastError() const
{
    return iLastError;
}

SessionAuthentication::StatusStatus SessionAuthentication::handleChallenge( const ChalParams& aChallenge,
                                                                            DatabaseHandler& aDbHandler,
                                                                            const QString& aLocalDeviceName,
                                                                            const QString& aRemoteDeviceName )
{
    FUNCTION_CALL_TRACE;

    StatusStatus status = STATUS_NOT_HANDLED;

    if( aChallenge.meta.type == SYNCML_FORMAT_AUTH_MD5 )
    {

        if( iAuthType == AUTH_MD5 )
        {
            // Configured to use MD5, so check if we have already sent auth info.
            // If we have, they were rejected and we must abort.
            // If we have not, we'll send auth info if we have a nonce. Otherwise we must abort.

            if( iAuthenticationPending )
            {
                status = STATUS_HANDLED_ABORT;
                iLastError = "Authentication failed";
            }
            else
            {
                NonceStorage nonces( aDbHandler.getDbHandle() );
                QByteArray nonce = nonces.retrieveNonce( aLocalDeviceName, aRemoteDeviceName );

                if( !nonce.isEmpty() )
                {
                    status = STATUS_HANDLED_RESEND;
                }
                else
                {
                    status = STATUS_HANDLED_ABORT;
                    iLastError = "Challenged for MD5 authentication, but do not have a nonce!";
                }
            }


        }
        else if( iAuthType == AUTH_BASIC )
        {
            // Configured to use Basic authentication, so retry if we have a nonce
            NonceStorage nonces( aDbHandler.getDbHandle() );
            QByteArray nonce = nonces.retrieveNonce( aLocalDeviceName, aRemoteDeviceName );

            if( !nonce.isEmpty() )
            {
                iAuthType = AUTH_MD5;
                status = STATUS_HANDLED_RESEND;
            }
            else
            {
                status = STATUS_HANDLED_ABORT;
                iLastError = "Challenged for MD5 authentication, but do not have a nonce!";
            }
        }
        else
        {
            status = STATUS_HANDLED_ABORT;
            iLastError = "Authentication required";
        }

    }
    // If basic is explicitly specified, or if no type is specified, use basic
    else if( aChallenge.meta.type == SYNCML_FORMAT_AUTH_BASIC ||
             aChallenge.meta.type.isEmpty() )
    {

        if( iAuthenticationPending && iAuthType == AUTH_BASIC )
        {
            // We have already sent auth using basic authentication, re-challenge
            // means authentication has failed
            status = STATUS_HANDLED_ABORT;
            iLastError = "Authentication failed";
        }
        else if( iAuthType == AUTH_MD5 )
        {
            // We are to use MD5 authentication, so don't allow revertion to a more
            // unsafe authentication scheme
            status = STATUS_HANDLED_ABORT;
            iLastError = "Received Basic authentication challenge when MD5 is reinforced";
        }
        else
        {
            iAuthType = AUTH_BASIC;
            status = STATUS_HANDLED_RESEND;
        }
    }
    else
    {
        status = STATUS_HANDLED_ABORT;
        iLastError = "Unsupported authentication type encountered:" + aChallenge.meta.type;
    }

    return status;
}

QByteArray SessionAuthentication::decodeNonce( const ChalParams& aChallenge ) const
{
    FUNCTION_CALL_TRACE;

    QByteArray nonce;

    if( aChallenge.meta.format == SYNCML_FORMAT_ENCODING_B64 )
    {
        nonce = QByteArray::fromBase64( aChallenge.meta.nextNonce.toUtf8() );
    }
    else if( aChallenge.meta.format.isEmpty() )
    {
        nonce = aChallenge.meta.nextNonce.toUtf8();
    }
    else
    {
        LOG_WARNING( "Unknown format" << aChallenge.meta.format << "specified for NextNonce, ignoring" );
    }

    return nonce;
}
