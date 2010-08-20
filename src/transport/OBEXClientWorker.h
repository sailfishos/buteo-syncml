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
#ifndef OBEXCLIENTWORKER_H
#define OBEXCLIENTWORKER_H

#include <QObject>
#include <openobex/obex.h>

namespace DataSync {

class OBEXConnection;

/*! \brief Worker class for handling OBEX client functionality
 *
 */
class OBEXClientWorker : public QObject
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     * @param aConnection Connection to use
     * @param aTimeOut Timeout to use in OBEX operations
     */
    OBEXClientWorker( OBEXConnection* aConnection, int aTimeOut );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXClientWorker();

    /*! \brief Returns whether connection is current established
     *
     * @return True if connected, otherwise false
     */
    bool isConnected();

public slots:

    /*! \brief Slot for doing OBEX CONNECT
     *
     */
    void connect();

    /*! \brief Slot for doing OBEX DISCONNECT
     *
     */
    void disconnect();

    /*! \brief Slot for doing OBEX PUT
     *
     */
    void send( const QByteArray& aBuffer, const QString& aContentType );

    /*! \brief Slot for doing OBEX GET
     *
     */
    void receive( const QString& aContentType );

signals:

    /*! \brief Emitted when data is available as a response to OBEX GET
     *
     * @param aData Data received
     * @param aContentType Content type of data
     */
    void incomingData( QByteArray aData, QString aContentType );

    /*! \brief Emitted when connection cannot be established
     *
     */
    void connectionFailed();

    /*! \brief Emitted when connection times out
     *
     */
    void connectionTimeout();

    /*! \brief Emitted when error occurs in connection
     *
     */
    void connectionError();

    /*! \brief Emitted when remote side has rejected the session
     *
     */
    void sessionRejected();

private:

    int process();

    static void handleEvent( obex_t *aHandle, obex_object_t *aObject, int aMode,
                             int aEvent, int aObexCmd, int aObexRsp );

    void linkError();

    void RequestCompleted( obex_object_t *aObject, int aMode, int aObexCmd, int aObexRsp );

    void ConnectResponse( obex_object_t *aObject, int aObexRsp );

    void DisconnectResponse( obex_object_t *aObject, int aObexRsp );

    void PutResponse( obex_object_t *aObject, int aObexRsp );

    void GetResponse( obex_object_t *aObject, int aObexRsp );

    OBEXConnection* iConnection;
    int             iTimeOut;
    bool            iConnected;
    int             iConnectionId;

    obex_t*         iTransportHandle;
    bool            iProcessing;

    QString         iGetContentType;

};

}

#endif  //  OBEXCLIENTWORKER_H
