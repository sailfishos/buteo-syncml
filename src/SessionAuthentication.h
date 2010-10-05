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
#ifndef SESSIONAUTHENTICATION_H
#define SESSIONAUTHENTICATION_H

#include "SyncAgentConsts.h"
#include "Fragments.h"

namespace DataSync {

class ResponseGenerator;
class DatabaseHandler;

/*! \brief Manages session authentication happening over SyncML protocol
 *
 */
class SessionAuthentication
{
public:

    /*! \brief Describes analysis result of SyncML header
     *
     */
    enum HeaderStatus
    {
        HEADER_HANDLED_OK,          ///< Header was handled, all OK
        HEADER_HANDLED_ABORT,       ///< Header was handled, should abort session
        HEADER_NOT_HANDLED          ///< Header was not handled
    };

    /*! \brief Describes analysis result of Status of SyncML header
     *
     */
    enum StatusStatus
    {
        STATUS_HANDLED_OK,          ///< Status was handled, all OK
        STATUS_HANDLED_ABORT,       ///< Status was handled, should abort session
        STATUS_HANDLED_RESEND,      ///< Status was handled, should resend last message
        STATUS_NOT_HANDLED          ///< Status was not handled
    };

    /*! \brief Constructor
     *
     */
    SessionAuthentication();

    /*! \brief Destructor
     *
     */
    ~SessionAuthentication();

    /*! \brief Sets parameters for this session
     *
     * @param aAuthType Authentication type to use in this session
     * @param aUsername Username to use in this session
     * @param aPassword Password to use in this session
     * @param aNonce Explicitly defined nonce that should be used in this session
     */
    void setSessionParams( AuthType aAuthType,
                           const QString& aUsername,
                           const QString& aPassword,
                           const QString& aNonce );

    /*! \brief Returns whether current session is authenticated
     *
     * @return True if current session is authenticated, otherwise false
     */
    bool authenticated();

    /*! \brief Analyze SyncML header sent by remote device
     *
     * @param aHeader Header to analyze
     * @param aResponseGenerator Response generator to utilize
     */
    HeaderStatus analyzeHeader( const HeaderParams& aHeader, ResponseGenerator& aResponseGenerator );

    /*! \brief Analyze Status sent by remote device in response to SyncML header sent by us
     *
     * @param aStatus Status to analyze
     * @param aDbHandler DatabaseHandler to utilize
     * @param aLocalDeviceName Local device name
     * @param aRemoteDeviceName Remote device name
     */
    StatusStatus analyzeHeaderStatus( const StatusParams& aStatus,
                                      DatabaseHandler& aDbHandler,
                                      const QString& aLocalDeviceName,
                                      const QString& aRemoteDeviceName );

    /*! \brief Compose authentication to be sent in the next message
     *
     * @param aResponseGenerator Response generator to use
     * @param aDbHandler DatabaseHandler to utilize
     * @param aLocalDeviceName Local device name
     * @param aRemoteDeviceName Remote device name
     */
    void composeAuthentication( ResponseGenerator& aResponseGenerator,
                                DatabaseHandler& aDbHandler,
                                const QString& aLocalDeviceName,
                                const QString& aRemoteDeviceName );

    /*! \brief Return human-readable description of the last occurred error
     *
     * @return
     */
    QString getLastError() const;

protected:

private:

    StatusStatus handleChallenge( const ChalParams& aChallenge,
                                  DatabaseHandler& aDbHandler,
                                  const QString& aLocalDeviceName,
                                  const QString& aRemoteDeviceName );

    QByteArray decodeNonce( const ChalParams& aChallenge ) const;

    bool                iAuthenticated;
    bool                iAuthenticationPending;
    AuthType            iAuthType;
    QString             iUsername;
    QString             iPassword;
    QString             iNonce;
    QString             iLastError;

};

}

#endif // SESSIONAUTHENTICATION_H
