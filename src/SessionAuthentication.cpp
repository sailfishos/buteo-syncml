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
#include "AuthHelper.h"

#include "LogMacros.h"

using namespace DataSync;


SessionAuthentication::SessionAuthentication()
 : iAuthedToRemote( false ),
   iRemoteAuthPending( false ),
   iRemoteAuthed( false ),
   iLocalAuthPending( false )
{

}

SessionAuthentication::~SessionAuthentication()
{

}

void SessionAuthentication::setSessionParams( AuthType aAuthType,
                                              const QString& aRemoteUsername,
                                              const QString& aRemotePassword,
                                              const QString& aRemoteNonce,
                                              const QString& aLocalUsername,
                                              const QString& aLocalPassword,
                                              const QString& aLocalNonce )
{
    FUNCTION_CALL_TRACE;

    iAuthType = aAuthType;
    iRemoteUsername = aRemoteUsername;
    iRemotePassword = aRemotePassword;
    iRemoteNonce = aRemoteNonce;

    iLocalUsername = aLocalUsername;
    iLocalPassword = aLocalPassword;
    iLocalNonce = aLocalNonce;

    if( iAuthType == AUTH_NONE )
    {
        iAuthedToRemote = true;
        iRemoteAuthPending = false;
        iRemoteAuthed = true;
        iLocalAuthPending = false;
    }
    else if( aLocalUsername.isEmpty() || aLocalPassword.isEmpty() )
    {
        iAuthedToRemote = false;
        iRemoteAuthPending = false;
        iRemoteAuthed = true;
        iLocalAuthPending = false;
    }
    else
    {
        iAuthedToRemote = false;
        iRemoteAuthPending = false;
        iRemoteAuthed = false;
        iLocalAuthPending = false;
    }
}

bool SessionAuthentication::remoteIsAuthed() const
{
    return iRemoteAuthed;
}

bool SessionAuthentication::authedToRemote() const
{
    return iAuthedToRemote;
}

SessionAuthentication::HeaderStatus SessionAuthentication::analyzeHeader( const HeaderParams& aHeader,
                                                                          DatabaseHandler& aDbHandler,
                                                                          const QString& aLocalDeviceName,
                                                                          const QString& aRemoteDeviceName,
                                                                          ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    HeaderStatus status = HEADER_NOT_HANDLED;

    if( !aHeader.cred.data.isEmpty() )
    {
        // Remote device wants to authenticate to us

        if( !iRemoteAuthed )
        {
            // Remote device is not yet authenticated to us
            status = handleAuthentication( aHeader, aDbHandler, aLocalDeviceName, aRemoteDeviceName, aResponseGenerator );
        }
        else
        {
            // Remote device is already authenticated to us. This shouldn't happen as we don't do continuous authentication.
            iLastError = "Remote device attempted authentication when not expected";
            status = HEADER_HANDLED_ABORT;
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
        iAuthedToRemote = true;
        iRemoteAuthPending = false;
        status = STATUS_HANDLED_OK;
    }
    else if( aStatus.data == AUTH_ACCEPTED ||
             aStatus.data == INVALID_CRED ||
             aStatus.data == MISSING_CRED )
    {
        // Clear possible nonce, because it was intended only for auth in this session
        NonceStorage nonces( aDbHandler.getDbHandle(), aLocalDeviceName, aRemoteDeviceName );
        nonces.clearNonce();

        // If remote party sent us a next nonce, save it
        QByteArray nonce = decodeNonce( aStatus.chal );

        if( !nonce.isEmpty() )
        {
            nonces.setNonce( nonce );
        }

        if( aStatus.data == AUTH_ACCEPTED )
        {
            // Authentication was accepted
            iAuthedToRemote = true;
            iRemoteAuthPending = false;
            status = STATUS_HANDLED_OK;
        }
        else
        {

            // Authentication was rejected
            iAuthedToRemote = false;

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

    if( iAuthType == AUTH_BASIC )
    {

        aResponseGenerator.addPackage( new AuthenticationPackage( iAuthType,
                                                                  iRemoteUsername,
                                                                  iRemotePassword ) );
        iRemoteAuthPending = true;

    }
    else if( iAuthType == AUTH_MD5 )
    {

        QByteArray remoteNonce = iRemoteNonce.toUtf8();

        if( remoteNonce.isEmpty() )
        {
            NonceStorage nonces( aDbHandler.getDbHandle(), aLocalDeviceName, aRemoteDeviceName );
            remoteNonce = nonces.nonce();
        }

        aResponseGenerator.addPackage( new AuthenticationPackage( iAuthType,
                                                                  iRemoteUsername,
                                                                  iRemotePassword,
                                                                  remoteNonce ) );

        if( !remoteNonce.isEmpty() )
        {
            iRemoteAuthPending = true;
        }
        else
        {
            // We didn't have a nonce, so authentication will probably fail. We need to hope that
            // remote device challenges us with a nonce, so don't put auth pending flag up.
            LOG_WARNING( "MD5 authentication requested but no nonce found" );
        }

    }

}

QString SessionAuthentication::getLastError() const
{
    return iLastError;
}

SessionAuthentication::HeaderStatus SessionAuthentication::handleAuthentication( const HeaderParams& aHeader,
                                                                                 DatabaseHandler& aDbHandler,
                                                                                 const QString& aLocalDeviceName,
                                                                                 const QString& aRemoteDeviceName,
                                                                                 ResponseGenerator& aResponseGenerator )
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iAuthType != AUTH_NONE );

    // Check that format of the Cred data is something we can understand
    if( aHeader.cred.meta.format != SYNCML_FORMAT_ENCODING_B64 &&
        !aHeader.cred.meta.format.isEmpty() )
    {
        iLastError = "Unsupported format in Cred:" + aHeader.cred.meta.format;
        return HEADER_HANDLED_ABORT;
    }

    HeaderStatus status = HEADER_NOT_HANDLED;

    if( aHeader.cred.meta.type == SYNCML_FORMAT_AUTH_MD5 )
    {
        // * If remote device wants to authenticate with MD5, always bump up to MD5 as it's more secure
        iAuthType = AUTH_MD5;

        // * Check if credentials are OK

        NonceStorage nonces( aDbHandler.getDbHandle(), aRemoteDeviceName, aLocalDeviceName );

        QByteArray localNonce = iLocalNonce.toUtf8();
        iLocalNonce.clear();

        if( localNonce.isEmpty() )
        {
            localNonce = nonces.nonce();
        }

        AuthHelper helper;
        QByteArray md5 = helper.encodeMD5Auth( iLocalUsername, iLocalPassword, localNonce );

        if( aHeader.cred.meta.format == SYNCML_FORMAT_ENCODING_B64 )
        {
            md5 = md5.toBase64();
        }

        if( md5 == aHeader.cred.data )
        {
            // * Credentials OK, accept authentication
            LOG_DEBUG( "Authentication accepted" );
            iLocalAuthPending = false;
            iRemoteAuthed = true;

            ChalParams challenge = generateChallenge( nonces );
            aResponseGenerator.addStatus( aHeader, challenge, AUTH_ACCEPTED );
            status = HEADER_HANDLED_OK;
        }
        else if( iLocalAuthPending )
        {
            // * Credentials not OK and we have already sent a challenge, fail authentication
            LOG_WARNING( "Authentication failed" );
            iLastError = "Authentication failed";
            iLocalAuthPending = false;
            iRemoteAuthed = false;

            aResponseGenerator.addStatus( aHeader, INVALID_CRED );
            status = HEADER_HANDLED_ABORT;
        }
        else
        {
            // * Credentials not OK but we haven't yet sent a challenge, so send one
            LOG_WARNING( "Authentication failed, sending challenge" );
            iLocalAuthPending = true;
            iRemoteAuthed = false;

            // Send challenge
            ChalParams challenge = generateChallenge( nonces );
            aResponseGenerator.addStatus( aHeader, challenge, INVALID_CRED );
            status = HEADER_HANDLED_OK;
        }

    }
    // If basic is explicitly specified, or if no type is specified, use basic
    else if( aHeader.cred.meta.type == SYNCML_FORMAT_AUTH_BASIC ||
             aHeader.cred.meta.type.isEmpty() )
    {

        if( iAuthType == AUTH_MD5 )
        {
            // * Remote side wants to authenticate with BASIC when MD5 is enforced.
            if( iLocalAuthPending )
            {
                // * Fail authentication as we have already sent a challenge for MD5
                LOG_WARNING( "Authentication failed" );
                iLastError = "Authentication failed";
                iLocalAuthPending = false;
                iRemoteAuthed = false;

                aResponseGenerator.addStatus( aHeader, INVALID_CRED );
                status = HEADER_HANDLED_ABORT;
            }
            else
            {
                // * Challenge remote device to use MD5
                LOG_WARNING( "MD5 authentication required, sending challenge" );
                iLocalAuthPending = true;
                iRemoteAuthed = false;

                // Send challenge
                ChalParams challenge = generateChallenge();
                aResponseGenerator.addStatus( aHeader, challenge, INVALID_CRED );
                status = HEADER_HANDLED_OK;
            }
        }
        else
        {
            AuthHelper helper;
            QByteArray basic = helper.encodeBasicB64Auth( iLocalUsername, iLocalPassword );

            if( basic == aHeader.cred.data )
            {
                // * Credentials OK, accept authentication
                LOG_DEBUG( "Authentication accepted" );
                iLocalAuthPending = false;
                iRemoteAuthed = true;

                aResponseGenerator.addStatus( aHeader, AUTH_ACCEPTED );
                status = HEADER_HANDLED_OK;
            }
            else if( iLocalAuthPending )
            {
                // * Credentials not OK and we have already sent a challenge, fail authentication
                LOG_WARNING( "Authentication failed" );
                iLastError = "Authentication failed";
                iLocalAuthPending = false;
                iRemoteAuthed = false;

                aResponseGenerator.addStatus( aHeader, INVALID_CRED );
                status = HEADER_HANDLED_ABORT;
            }
            else
            {
                // * Credentials not OK but we haven't yet sent a challenge, so send one
                LOG_WARNING( "Authentication failed, sending challenge" );
                iLocalAuthPending = true;
                iRemoteAuthed = false;

                // Send challenge
                ChalParams challenge = generateChallenge();
                aResponseGenerator.addStatus( aHeader, challenge, INVALID_CRED );
                status = HEADER_HANDLED_OK;
            }

        }

    }
    else
    {
        status = HEADER_HANDLED_ABORT;
        iLastError = "Unsupported authentication type encountered:" + aHeader.cred.meta.type;
    }

    return status;
}

SessionAuthentication::StatusStatus SessionAuthentication::handleChallenge( const ChalParams& aChallenge,
                                                                            DatabaseHandler& aDbHandler,
                                                                            const QString& aLocalDeviceName,
                                                                            const QString& aRemoteDeviceName )
{
    FUNCTION_CALL_TRACE;

    StatusStatus status = STATUS_NOT_HANDLED;
    NonceStorage nonces( aDbHandler.getDbHandle(), aLocalDeviceName, aRemoteDeviceName );

    if( aChallenge.meta.type == SYNCML_FORMAT_AUTH_MD5 )
    {

        if( iAuthType == AUTH_MD5 )
        {
            // Configured to use MD5, so check if we have already sent auth info.
            // If we have, they were rejected and we must abort.
            // If we have not, we'll send auth info if we have a nonce. Otherwise we must abort.

            if( iRemoteAuthPending )
            {
                status = STATUS_HANDLED_ABORT;
                iLastError = "Authentication failed";
            }
            else
            {

                QByteArray nonce = nonces.nonce();

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
            QByteArray nonce = nonces.nonce();

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

        if( iRemoteAuthPending && iAuthType == AUTH_BASIC )
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

ChalParams SessionAuthentication::generateChallenge()
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iAuthType == AUTH_BASIC );

    ChalParams challenge;

    challenge.meta.type = SYNCML_FORMAT_AUTH_BASIC;
    challenge.meta.format = SYNCML_FORMAT_ENCODING_B64;

    return challenge;
}

ChalParams SessionAuthentication::generateChallenge( NonceStorage& aNonces )
{
    FUNCTION_CALL_TRACE;

    Q_ASSERT( iAuthType == AUTH_MD5 );

    ChalParams challenge;

    challenge.meta.type = SYNCML_FORMAT_AUTH_MD5;

    QByteArray nonce = iLocalNonce.toUtf8();
    iLocalNonce.clear();

    if( nonce.isEmpty() )
    {
        nonce = aNonces.generateNonce();
    }

    // @todo: wouldn't necessarily have to B64-encode NextNonce over WbXML
    challenge.meta.format = SYNCML_FORMAT_ENCODING_B64;
    challenge.meta.nextNonce = nonce.toBase64();

    aNonces.setNonce( nonce );

    return challenge;
}
