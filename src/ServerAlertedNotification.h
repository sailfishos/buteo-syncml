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
#ifndef SERVERALERTEDNOTIFICATION_H
#define SERVERALERTEDNOTIFICATION_H

#include <QByteArray>
#include <QString>

#include "SyncMode.h"
#include "SyncAgentConsts.h"

namespace DataSync {

/*! \brief SAN UI Mode
 *
 */
enum SANUIMode
{
    SANUIMODE_NOT_SPECIFIED     = 0, /*!< Not specified*/
    SANUIMODE_BACKGROUND        = 1, /*!< Background*/
    SANUIMODE_INFORMATIVE       = 2, /*!< Informative*/
    SANUIMODE_USER_INTERACTION  = 3  /*!< User interaction*/
};

/*! \brief SAN Initiator
 *
 */
enum SANInitiator
{
    SANINITIATOR_USER   = 0, /*!< User initiated action*/
    SANINITIATOR_SERVER = 1  /*!< Server initiated action*/
};

/*! \brief SAN sync information
 *
 */
struct SANSyncInfo
{
    int             iSyncType;      /*!< Sync type to user*/
    QString         iContentType;   /*!< Content type*/
    QString         iServerURI;     /*!< Remote database URI*/
};

/*! \brief SAN message data
 *
 */
struct SANData
{
    QByteArray              iDigest;            /*!< MD5 digest*/
    ProtocolVersion         iVersion;           /*!< Protocol version*/
    SANUIMode               iUIMode;            /*!< User interaction mode*/
    SANInitiator            iInitiator;         /*!< Notification initiator*/
    qint16                  iSessionId;         /*!< Session ID*/
    QString                 iServerIdentifier;  /*!< Server identifier*/
    QList<SANSyncInfo>      iSyncInfo;          /*!< Message sync info payload*/

};

/*! \brief Class for parsing and generating OMA DS 1.2 Server Alerted
 *         Notification (SAN) message
 */
class SANHandler
{

public:

    /*! \brief Constructor
     *
     */
    SANHandler();

    /*! \brief Destructor
     *
     */
    ~SANHandler();

    /*! \brief Check MD5 digest of a SAN message
     *
     * @param aMessage Message to check
     * @param aServerIdentifier Identifier of the server
     * @param aPassword Password
     * @param aNonce Nonce
     * @return True if digest check passes, otherwise false
     */
    bool checkDigest( const QByteArray& aMessage,
                      const QString& aServerIdentifier,
                      const QString& aPassword,
                      const QString& aNonce );

    /*! \brief Parse SAN message
     *
     * @param aMessage Message to parse
     * @param aData Parsed message data on success
     * @return True on success, otherwise false
     */
    bool parseSANMessage( const QByteArray& aMessage,
                          SANData& aData );

    /*! \brief Generate SAN message
     *
     * @param aData Message data to use
     * @param aPassword Password for MD5 Digest
     * @param aNonce Nonce for MD5 Digest
     * @param aMessage Generated message on success
     * @return True on success, otherwise false
     */
    bool generateSANMessage( const SANData& aData,
                             const QString& aPassword,
                             const QString& aNonce,
                             QByteArray& aMessage );


protected:

private:

    QByteArray generateDigest( const QString& aServerIdentifier,
                               const QString& aPassword,
                               const QString& aNonce,
                               const QByteArray& aNotification );

};

}

#endif  //  SERVERALERTEDNOTIFICATION_H
