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
#ifndef REQUESTLISTENER_H
#define REQUESTLISTENER_H

#include <QObject>

#include "SyncMLMessageParser.h"
#include "ServerAlertedNotification.h"
#include "Transport.h"

namespace DataSync {

/*! \brief Class that listens for incoming requests for server session by OMA
 *         DS clients, or incoming server alerted notifications by OMA DS
 *         servers
 */
class RequestListener : public QObject
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     * @param aParent Parent of this object
     */
    RequestListener( QObject* aParent = 0 );

    /*! \brief Destructor
     *
     */
    virtual ~RequestListener();

    /*! \brief Types of requests supported
     *
     */
    enum RequestType
    {
        REQUEST_SAN_XML,        ///< SAN notification message (usually OMA DS 1.1)
        REQUEST_SAN_PACKAGE,    ///< SAN notification package (usually OMA DS 1.2)
        REQUEST_CLIENT          ///< OMA DS XML initialization request sent by client
    };

    /*! \brief Processed data of a request
     *
     */
    struct RequestData
    {
        RequestType         iType;      ///< Type of the request
        SANData             iSANData;   ///< Package fields of the request if SAN package
        QList<Fragment*>    iFragments; ///< Protocol fragments if the request was XML
    };

    /*! \brief Start listening for incoming requests
     *
     * @param aTransport Transport to use
     * @return True if listening was successfully started, otherwise false
     */
    bool start( Transport* aTransport );

    /*! \brief Stop listening for incoming requests
     *
     */
    void stop();

    /*! \brief Retrieve data of a pending request
     *
     * @return Data of pending request. Ownership is transferred
     */
    RequestData takeRequestData();

signals:

    /*! \brief Signal emitted when there's a new pending request
     *
     * Listening is automatically stopped when a new request comes
     */
    void newPendingRequest();

    /*! \brief Signal emitted when an error has occurred during listening
     *
     * Listening is automatically stopped when an error occurs
     * @param aState Error state
     * @param aErrorString Developer-oriented, human-readable description
     */
    void error( DataSync::SyncState aState, QString aErrorString );

private slots:
    void transportEvent( DataSync::TransportStatusEvent aEvent, QString aErrorString );

    void parsingComplete( bool aLastMessageInPackage );

    void parserError( DataSync::ParserError aError );

    void SANPackageReceived( QIODevice* aDevice );

private:

    Transport*          iTransport;
    SyncMLMessageParser iParser;
    RequestData         iRequestData;
};

}

#endif  //  REQUESTLISTENER_H
