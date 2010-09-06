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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QObject>
#include <QMetaType>

class QIODevice;

namespace DataSync {

class SyncMLMessage;


/*! \brief Transport layer events that are emitted by transports to
 *         SyncML stack
 */
enum TransportStatusEvent {
    TRANSPORT_CONNECTION_FAILED,
    TRANSPORT_CONNECTION_OPENED,
    TRANSPORT_CONNECTION_TIMEOUT,
    TRANSPORT_CONNECTION_ABORTED,
    TRANSPORT_CONNECTION_CLOSED,
    TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED,
    TRANSPORT_DATA_SENT,
    TRANSPORT_DATA_INVALID_CONTENT_TYPE,
    TRANSPORT_DATA_INVALID_CONTENT,
    TRANSPORT_SESSION_REJECTED
};

/*! \brief Transport interface that is exposed to SyncML stack
 *
 */
class Transport: public QObject
{
    Q_OBJECT
public:
    /*! \brief Constructor
     *
     * @param aParent Parent of this QObject
     */
	Transport( QObject* aParent ) : QObject( aParent )
	{
	    qRegisterMetaType<DataSync::TransportStatusEvent>("DataSync::TransportStatusEvent");
	}

    /*! \brief Destructor
     *
     */
    virtual ~Transport() { };


    /*! \brief Sets the URI to use in next send operation
     *
     * @param aURI New URI to remote side
     */
    virtual void setRemoteLocURI( const QString& aURI ) = 0;

    /*! \brief Returns true if this transport employes wbXML, and it should be taken
     *         into account when the stack is estimating the size of the message to send.
     * @return True if transport employs wbXML, otherwise false
     */
    virtual bool usesWbXML() = 0;

    /*! \brief Send SyncML message using transport
     *
     * When data is sent, sendEvent() signal with TRANSPORT_DATA_SENT is emitted.
     * In case of errors or timeouts, sendEvent() signal is emitted with error code.
     *
     * @param aMessage Message to send. Ownership is transferred
     * @return True if sending of data was started, false otherwise
     */
    virtual bool sendSyncML( SyncMLMessage* aMessage ) = 0;

    /*! \brief Send SAN message using transport
     *
     * When data is sent, sendEvent() signal with TRANSPORT_DATA_SENT is emitted.
     * In case of errors or timeouts, sendEvent() signal is emitted with error code.
     *
     * @param aMessage Message to send.
     * @return True if sending of data was started, false otherwise
     */
    virtual bool sendSAN( const QByteArray& aMessage ) = 0;

    /*! \brief Receive XML data using transport
     *
     * When data is available, readData() signal is emitted. In case of errors or
     * timeouts, sendEvent() signal is emitted
     *
     * @return True if listening for incoming data was started, false otherwise
     */
    virtual bool receive() = 0;

    /*! \brief Sets a transport property
     *
     * This function is called by SyncAgent to enable properties set with
     * SyncAgentConfig::setTransportProperty(). Users generally should not
     * call this function directly.
     *
     * @param aProperty Property to set
     * @param aValue Value to set
     */
    virtual void setProperty( const QString& aProperty, const QString& aValue ) = 0;

    /*! \brief Initiates the transport
     *
     * This function is called by SyncAgent when it is sure that transport
     * layer is required to proceed with the synchronization. Users generally
     * should not call this function directly. This function should take care
     * of any operations needed to be performed before data can be sent or
     * received, such as establishing a link layer connection and/or session.
     *
     * @return True if initiation was successful, otherwise false
     */
    virtual bool init() = 0;

    /*! \brief Closes the transport
     *
     * This function is called by SyncAgent when it is sure that transport layer
     * is no longer required to proceed with the synchronization. Users generally
     * should not call this function directly. This function should take care of
     * any operations needed to close link layer connection and/or session.
     */
    virtual void close() = 0;

signals:

    /*! \brief Signal that is emitted when transport level event occurs
     *
     * @param aEvent Event that has occurred
     * @param aDescription Description of the event that has occurred
     *
     */
    void sendEvent( DataSync::TransportStatusEvent aEvent, const QString& aDescription );

    /*! \brief Signal that is emitted when new XML data is available
     *
     * @param aDevice QIODevice that can be used to read data
     * @param aIsNewPacket bool To indicate if this is a newly received packet
     * or a purged one.
     */
    void readXMLData( QIODevice* aDevice, bool aIsNewPacket );

    /*! \brief Signal that is emitted when new SAN data is available
     *
     * @param aDevice QIODevice that can be used to read data
     */
    void readSANData( QIODevice* aDevice );

private slots:

    /*! \brief Remove any illegal XML characters from the previous message
     *
     * Removes illegal XML characters (NULLs and control characters)
     */
    virtual void purgeAndResendBuffer() = 0;
};

}

Q_DECLARE_METATYPE( DataSync::TransportStatusEvent );

#endif
