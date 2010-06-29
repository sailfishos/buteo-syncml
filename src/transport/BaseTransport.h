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
#ifndef BASETRANSPORT_H
#define BASETRANSPORT_H

#include "Transport.h"

#include <QBuffer>

namespace DataSync {

/*! \brief Base implementation class for all transports
 *
 * Handles WbXml and other common operations
 */
class BaseTransport : public Transport
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aParent Parent of this object
     */
    BaseTransport( QObject* aParent );

    /*! \brief Destructor
     *
     */
    virtual ~BaseTransport();

    virtual void setRemoteLocURI( const QString& aURI );

    virtual bool sendSyncML( SyncMLMessage* aMessage );

    virtual bool sendSAN( const QByteArray& aMessage );

    virtual bool receive();

    /*! \brief Enable/disable WbXML
     *
     * @param aUse True/false to enable/disable WbXML encoding
     */
    void setWbXml( bool aUse );

protected:

    /*! \brief Prepare for sending data
     *
     * If problems occur in preparation, this function should return false. It should also
     * emit sendEvent() signal related to the error
     *
     * @return True on success, otherwise false
     */
    virtual bool prepareSend() = 0;

    /*! \brief Send via network layer
     *
     * All transports derived from BaseTransport must implement this function. It should do
     * the actual sending of data
     *
     * @param aData Content data
     * @param aContentType Content type
     */
    virtual bool doSend( const QByteArray& aData, const QString& aContentType ) = 0;

    /*! \brief Receive via network layer
     *
     * All transports derived from BaseTransport must implement this function. It should start
     * the actual receiving of data
     *
     * @param aContentType Content type
     */
    virtual bool doReceive( const QString& aContentType ) = 0;

    /*! \brief Receive incoming data
     *
     * All transports derived from BaseTransport. Data received should be passed to this function
     *
     * @param aData Content data
     * @param aContentType Content type
     */
    void receive( const QByteArray& aData, const QString& aContentType );

    /*! \brief Retrieves remote location URI
     *
     * @return Remote URI
     */
    const QString& getRemoteLocURI() const;

    /*! \brief Encodes a SyncML message
     *
     * SyncML message can be encoded to either XML or WbXML, depending on the current mode
     *
     * @param aMessage Message to encode
     * @param aData Resulting data
     * @return True on success, false otherwise
     */
    bool encodeMessage( const SyncMLMessage& aMessage, QByteArray& aData );

private:

    void emitReadSignal();

    bool useWbXml() const;

    void receiveWbXMLData( const QByteArray& aData );
    void receiveXMLData( const QByteArray& aData );
    void receiveSANData( const QByteArray& aData );

    QString             iRemoteLocURI;
    QString             iContentType;
    QByteArray          iIncomingData;
    QByteArray          iIODeviceData;
    QBuffer             iIODevice;
    bool                iHandleIncomingData;
    bool                iWbXml;

};

}


#endif  //  BASETRANSPORT_H
