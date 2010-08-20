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
#ifndef OBEXTRANSPORT_H
#define OBEXTRANSPORT_H

#include <QThread>

#include "BaseTransport.h"
#include "OBEXClientWorker.h"
#include "OBEXServerWorker.h"

namespace DataSync {

class OBEXConnection;
class OBEXClientWorker;
class OBEXServerWorker;
class OBEXWorkerThread;

/*! \brief Transport that implements OBEX functionality
 *
 */
class OBEXTransport : public BaseTransport, public OBEXServerDataSource
{
    Q_OBJECT;
public:
    /*! \brief Operation mode for transport
     *
     */
    enum Mode
    {
        MODE_OBEX_CLIENT,    //!< Act as OBEX Client
        MODE_OBEX_SERVER     //!< Act as OBEX Server
    };

    /*! \brief Type for transport
     *
     */
    enum Type
    {
        TYPE_USB,       //!< Operate over USB
        TYPE_BT         //!< Operate over BT
    };

    /*! \brief Constructor using file descriptor
     *
     * @param aOpMode Operation mode for transport
     * @param aType Type for transport
     * @param aFd File descriptor to device
     * @param aTimeOut Timeout to use in OBEX operations (in seconds)
     * @param aParent Parent of this QObject
     */
    OBEXTransport( Mode aOpMode, Type aType, int aFd, int aTimeOut, QObject* aParent = NULL );

    /*! \brief Convenience constructor for OBEX Client over BT
     *
     * @param aBTAddress Address of the BT device to connect to
     * @param aServiceUUID UUID of the service (SyncML client or server) to connect to
     * @param aTimeOut Timeout to use in OBEX operations (in seconds)
     * @param aParent Parent of this QObject
     */
    OBEXTransport( const QString& aBTAddress, const QString& aServiceUUID,
                   int aTimeOut, QObject* aParent = NULL );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXTransport();

    /* \brief Sets a transport property
     *
     * Properties should usually be set before attempting to send or receive.
     * Supported properties:
     * bt-obex-mtu: MTU in OBEX over BT
     * usb-obex-mtu: MTU in OBEX over USB
     *
     * @param aProperty Property to set
     * @param aValue Value to set
     */
    virtual void setProperty( const QString& aProperty, const QString& aValue );

    virtual bool sendSyncML( SyncMLMessage* aMessage );

    virtual bool getData( const QString& aContentType, QByteArray& aData );

protected:

    virtual bool prepareSend();

    virtual bool doSend( const QByteArray& aData, const QString& aContentType );

    virtual bool doReceive( const QString& aContentType );

private slots:

    void incomingData( QByteArray aData, QString aContentType );

    void connectionFailed();

    void connectionTimeout();

    void connectionError();

    void sessionRejected();

private:

    void setupClient( OBEXConnection* connection );

    void setupServer( OBEXConnection* connection );

    Mode                iMode;
    Type                iType;
    OBEXWorkerThread*   iWorkerThread;
    OBEXClientWorker*   iClientWorker;
    OBEXServerWorker*   iServerWorker;

    int                 iTimeOut;

    SyncMLMessage*      iMessage;
};

/*! \brief Thread for OBEX worker
 *
 */
class OBEXWorkerThread : public QThread
{
    Q_OBJECT;
public:

    /*! \brief Constructor for client
     *
     * @param aConnection Connection to use
     * @param aWorker Worker to use
     */
    OBEXWorkerThread( OBEXConnection* aConnection, OBEXClientWorker* aWorker );

    /*! \brief Constructor for server
     *
     * @param aConnection Connection to use
     * @param aWorker Worker to use
     */
    OBEXWorkerThread( OBEXConnection* aConnection, OBEXServerWorker* aWorker );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXWorkerThread();

    /*! \brief Retrieves the connection
     *
     * @return
     */
    OBEXConnection* getConnection();

protected:

    /*! \brief Thread function
     *
     */
    virtual void run();

private:
    OBEXConnection*     iConnection;
    OBEXClientWorker*   iClientWorker;
    OBEXServerWorker*   iServerWorker;

};

}

#endif  //  OBEXTRANSPORT_H
