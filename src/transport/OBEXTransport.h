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

class OBEXWorker;
class OBEXWorkerThread;
class OBEXConnection;

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

    /*! \brief Connection type hint for transport
     *
     * Transport can use this information about the type of connection
     * to optimize for example used Maximum Transfer Unit (MTU).
     *
     */
    enum ConnectionTypeHint
    {
        TYPEHINT_USB,       //!< Operating over USB transport
        TYPEHINT_BT,        //!< Operating over BT transport
        TYPEHINT_OTHER      //!< Operating over other transport
    };

    /*! \brief Constructor
     *
     * @param aConnection OBEX connection to use
     * @param aOpMode Operation mode for transport
     * @param aTimeOut Timeout to use in OBEX operations (in seconds)
     * @param aTypeHint Connection type hint
     * @param aParent Parent of this QObject
     */
    OBEXTransport( OBEXConnection& aConnection, Mode aOpMode, int aTimeOut,
                   ConnectionTypeHint aTypeHint = TYPEHINT_OTHER,
                   QObject* aParent = NULL );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXTransport();

    virtual void setProperty( const QString& aProperty, const QString& aValue );

    virtual bool init();

    virtual void close();

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

    void setupClient( int aFd );

    void setupServer( int aFd );

    OBEXConnection&     iConnection;
    Mode                iMode;
    int                 iTimeOut;
    ConnectionTypeHint  iTypeHint;

    OBEXWorkerThread*   iWorkerThread;
    OBEXWorker*         iWorker;
    qint32              iMTU;
    SyncMLMessage*      iMessage;
};

/*! \brief Thread for OBEX worker
 *
 */
class OBEXWorkerThread : public QThread
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     * @param aWorker Worker to use
     */
    OBEXWorkerThread( OBEXWorker* aWorker );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXWorkerThread();

protected:

    /*! \brief Thread function
     *
     */
    virtual void run();

private:
    OBEXWorker* iWorker;

};

}

#endif  //  OBEXTRANSPORT_H
