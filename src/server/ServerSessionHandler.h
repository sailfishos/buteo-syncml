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


#ifndef SERVERSESSIONHANDLER_H
#define SERVERSESSIONHANDLER_H

#include "SessionHandler.h"

#include <QString>
#include <QList>
#include <QPair>

class ServerSessionHandlerTest;

namespace DataSync {

/*! \brief Handles SyncML server related session management
 *
 */
class ServerSessionHandler: public SessionHandler {
    Q_OBJECT

public:
    /*! \brief Constructor
     *
     * @param aConfig A pointer to configuration object
     * @param aParent A pointer to parent object
     */
    explicit ServerSessionHandler( const SyncAgentConfig* aConfig,
                                   QObject* aParent = 0 );

    /*! \brief Destructor
     *
     */
	virtual ~ServerSessionHandler();

public slots:

    virtual void initiateSync();

    virtual void suspendSync();

    virtual void resumeSync();

    /*! \brief Initiate a new synchronization session as a server by serving the
     *         request sent by remote client
     *
     * @param aFragments Protocol fragments of the request sent by client
     */
    void serveRequest( QList<Fragment*>& aFragments );

protected:

    virtual void messageReceived( HeaderParams& aHeaderParams );

    virtual ResponseStatusCode syncAlertReceived( const SyncMode& aSyncMode, AlertParams& aAlertParams );

    virtual bool syncReceived();

    virtual bool mapReceived();

    virtual void finalReceived();

    virtual void messageParsed();

    virtual void resendPackage();


private:

    ResponseStatusCode setupTargetByClient( const SyncMode& aSyncMode, AlertParams& aAlertParams );

    ResponseStatusCode acknowledgeTarget( const SyncMode& aSyncMode, AlertParams& aAlertParams );

    void composeSyncML11ServerAlertedSyncPackage( const QList< QPair<QString, QString> >& aStorages );

    void composeAndSendSyncML12ServerAlertedSyncPackage( const QList< QPair<QString, QString> >& aStorages );

    void composeServerInitializationPackage();

    void composeServerModificationsPackage();

    void composeMapAcknowledgementPackage();

    void composeServerInitialization();

    void serverInitiatedSyncDS11( const QList< QPair<QString, QString> >& aStorages );

    void serverInitiatedSyncDS12( const QList< QPair<QString, QString> >& aStorages );


private: // data

    const DataSync::SyncAgentConfig*    iConfig;            ///< A pointer to configuration

    friend class ::ServerSessionHandlerTest;
};

}
#endif /* SERVERSESSIONHANDLER_H */
