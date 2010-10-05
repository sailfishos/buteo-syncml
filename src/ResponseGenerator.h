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
#ifndef RESPONSEGENERATOR_H
#define RESPONSEGENERATOR_H

#include <QtGlobal>
#include "SyncAgentConsts.h"
#include "Fragments.h"

class QString;

namespace DataSync {

class Package;
class SyncMLMessage;
struct StatusParams;

/*! \brief Class that contains all of the data of one request sent to the server.
 *
 */
class ResponseGenerator
{
public:
    /*! \brief Constructor
     *
     */
    ResponseGenerator();

    /*! \brief Destructor
     *
     */
    virtual ~ResponseGenerator();

    /*! \brief Retrieve SyncML header parameters
     *
     * @return
     */
    const HeaderParams& getHeaderParams() const;

    /*! \brief Set SyncML header parameters
     *
     * msgId field is ignored as this is filled by response generator
     *
     * @param aHeaderParams
     */
    void setHeaderParams( const HeaderParams& aHeaderParams );

    /*! \brief Set remote message id
     *
     * Remote message id is used when status elements are generated, as they require
     * msgRef field
     *
     * @param aRemoteMsgId Remote message id
     */
    void setRemoteMsgId( int aRemoteMsgId );

    /*! \brief Get remote message id
     *
     * Remote message id is used when status elements are generated, as they require
     * msgRef field
     *
     * @return
     */
    int getRemoteMsgId() const;

    /*! \brief Generate next message
     *
     * @param aMaxSize Maximum size of the message
     * @param aVersion Protocol version to use
     * @param aWbXML If generated message will be converted to WbXML, take increased
     *               compression into account in size calculations
     * @return SyncML message on success, otherwise NULL
     */
    SyncMLMessage* generateNextMessage( int aMaxSize, const ProtocolVersion& aVersion,
                                        bool aWbXML = false );

    /*! \brief Add package to package queue for sending
     *
     * @param aPackage Package. Ownership IS transferred
     */
    void addPackage( Package* aPackage );

    /*! \brief Clears the contents of package queue by removing all packages
     *
     */
    void clearPackageQueue();

    /*! \brief Returns true if package queue is empty
     *
     * @return
     */
    bool packageQueueEmpty() const;

    /*! \brief Get list of current packages
     *
     * Ownership is NOT transferred
     * @return
     */
    const QList<Package*>& getPackages() const;

    /*! \brief Sets whether status messages added should be ignored or written
     *
     * This is used when there's noResp specified in SyncHdr and no status messages
     * should be sent back. Exception is status messages with challenges; they are
     * never ignored
     */
    void ignoreStatuses( bool aIgnore );

    /*! \brief Adds a status to the next outgoing message from already constructed Status data
     *
     * @param aParams Parameters of the status. Ownership IS transferred
     */
    void addStatus( StatusParams* aParams );

    /*! \brief Adds a status to the next outgoing message from SyncHdr data
     *
     * @param aParams SyncHdr params
     * @param aStatusCode Status code
     */
    void addStatus( const HeaderParams& aParams, ResponseStatusCode aStatusCode );

    /*! \brief Adds a status to the next outgoing message from SyncHdr data
     *
     * @param aParams SyncHdr params
     * @param aChalParams Authentication challenge params
     * @param aStatusCode Status code
     */
    void addStatus( const HeaderParams& aParams, const ChalParams& aChalParams,
                    ResponseStatusCode aStatusCode );

    /*! \brief Adds a status to the next outgoing message from command data
     *
     * @param aParams Command params
     * @param aStatusCode Status code
     * @param aWriteItemRefs True if SourceRef/TargetRef about Item(s) of the command should be written to the status
     * @param aItemIndex Override index for item to reference
     */
    void addStatus( const CommandParams& aParams, ResponseStatusCode aStatusCode,
                    bool aWriteItemRefs = true, int aItemIndex = -1);

    /*! \brief Adds a status to the next outgoing message from Sync data
     *
     * @param aParams Sync params
     * @param aStatusCode Status code
     */
    void addStatus( const SyncParams& aParams, ResponseStatusCode aStatusCode );

    /*! \brief Adds a status to the next outgoing message from Map data
     *
     * @param aParams Map element parameters
     * @param aStatusCode Status code
     */
    void addStatus( const MapParams& aParams, ResponseStatusCode aStatusCode );

    /*! \brief Adds a status to the next outgoing message from PutParams data
     *
     * @param aParams PutParams element parameters
     * @param aStatusCode Status code
     */
    void addStatus( const PutParams& aParams, ResponseStatusCode aStatusCode );

    /*! \brief Adds a status to the next outgoing message from ResultsParams data
     *
     * @param aParams ResultsParams element parameters
     * @param aStatusCode Status code
     */
    void addStatus( const ResultsParams& aParams, ResponseStatusCode aStatusCode );

    /*! \brief Returns current list of statuses
     *
     * @return
     */
    const QList<StatusParams*>& getStatuses() const;

protected:

    /*! \brief Retrieve id that should be used for next message
     *
     * @return
     */
    int getNextMsgId();

private:

    int                     iMaxMsgSize;

    int                     iMsgId;
    int                     iRemoteMsgId;
    HeaderParams            iHeaderParams;

    QList<StatusParams*>    iStatuses;
    QList<Package*>         iPackages;

    bool                    iIgnoreStatuses;

};
}
#endif  //  RESPONSEGENERATOR_H
