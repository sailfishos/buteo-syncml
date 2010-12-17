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
#ifndef OBEXWORKER_H
#define OBEXWORKER_H

#include <QObject>
#include <openobex/obex.h>

namespace DataSync {

/*! \brief Base class for OBEX workers
 *
 */
class OBEXWorker : public QObject
{
    Q_OBJECT;
public:
    /*! \brief Constructor
     *
     * @param aParent Parent of this object
     */
    OBEXWorker( QObject* aParent = NULL);

    /*! \brief Destructor
     *
     */
    virtual ~OBEXWorker();

public:

    /*! \brief Returns connection status
     *
     * @return True if connected, otherwise false
     */
    bool isConnected() const;

protected:

    /*! \brief Setup OpenOBEX
     *
     * @param aFd File descriptor to use
     * @param aMTU MTU to use
     * @param aEventHandler Event handler to use
     */
    bool setupOpenOBEX( int aFd, qint32 aMTU, obex_event_t aEventHandler );

    /*! \brief Close OpenOBEX
     *
     */
    void closeOpenOBEX();

    /*! \brief Retrieve current OBEX handle
     *
     * @return
     */
    obex_t* getHandle() const;

    /*! \brief Sets connection status
     *
     * @param aConnected Connection status to set
     */
    void setConnected( bool aConnected );

    /*! \brief Returns link error status
     *
     * @return True if link error occurred, otherwise false
     */
    bool isLinkError() const;

    /*! \brief Sets link error status
     *
     * @param aLinkError Link error status to set
     */
    void setLinkError( bool aLinkError );

private:

    obex_t* iTransportHandle;

    bool    iConnected;
    bool    iLinkError;
};

}

#endif // OBEXWORKER_H
