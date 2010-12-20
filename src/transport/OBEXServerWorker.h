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
#ifndef OBEXSERVERWORKER_H
#define OBEXSERVERWORKER_H

#include "OBEXWorker.h"

namespace DataSync {

/*! \brief Data source for OBEX Server
 *
 */
class OBEXServerDataSource
{
public:

    /*! \brief Destructor
     *
     */
    virtual ~OBEXServerDataSource() { }

    /*! \brief Retrieve data from source
     *
     * @param aContentType Content type for data to retrieve
     * @param aData Retrieved data buffer
     * @return True on success, otherwise false
     */
    virtual bool getData( const QString& aContentType, QByteArray& aData ) = 0;
};

/*! \brief Worker class for handling OBEX server functionality
 *
 */
class OBEXServerWorker : public OBEXWorker
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     * @param aSource Data source to use
     * @param aFd File descriptor to use
     * @param aMTU MTU to use
     * @param aTimeOut Timeout to use in OBEX operations
     */
    OBEXServerWorker( OBEXServerDataSource& aSource,
                      int aFd,
                      qint32 aMTU,
                      int aTimeOut );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXServerWorker();

public slots:

    /*! \brief Slot for waiting to receive OBEX CONNECT
     *
     */
    void waitForConnect();

    /*! \brief Slot for waiting to receive OBEX DISCONNECT
     *
     */
    void waitForDisconnect();

    /*! \brief Slot for waiting to receive OBEX PUT
     *
     */
    void waitForPut();

    /*! \brief Slot for waiting to receive OBEX GET
     *
     */
    void waitForGet();


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

private:

    enum State
    {
        STATE_IDLE,
        STATE_CONNECT,
        STATE_DISCONNECT,
        STATE_PUT,
        STATE_GET
    };

    void process( State aNextState );

    static void handleEvent( obex_t *aHandle, obex_object_t *aObject, int aMode,
                             int aEvent, int aObexCmd, int aObexRsp );

    void linkError();

    void requestReceived( obex_object_t *aObject, int aMode, int aObexCmd );

    void ConnectRequest( obex_object_t *aObject );

    void DisconnectRequest( obex_object_t *aObject );

    void PutRequest( obex_object_t *aObject );

    void GetRequest( obex_object_t *aObject );

    OBEXServerDataSource&   iSource;
    int                     iFd;
    qint32                  iMTU;
    int                     iTimeOut;
    unsigned int            iConnectionId;

    bool                    iProcessing;
    State                   iState;

};

}

#endif  //  OBEXSERVERWORKER_H
