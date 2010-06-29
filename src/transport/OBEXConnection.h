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
#ifndef OBEXCONNECTION_H
#define OBEXCONNECTION_H

#include <QObject>
#include <openobex/obex.h>

namespace DataSync {

/*! \brief Base class for OBEX transport layer
 *
 */
class OBEXConnection : public QObject
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     */
    OBEXConnection( int aMTUSize );

    /*! \brief Destructor
     *
     */
    virtual ~OBEXConnection();

    /*! \brief Connect transport layer
     *
     * @param aEventHandler OpenObex event handler to register to transport layer
     * @return Openobex handle if successful, otherwise false
     */
    obex_t* connect( obex_event_t aEventHandler );

    /*! \brief Disconnect transport layer
     *
     */
    void disconnect();

signals:
    /*! \brief Signal that is emitted when transport layer is connected
     *
     */
    void connected();

    /*! \brief Signal that is emitted when transport layer is disconnected
     *
     */
    void disconnected();

protected:

    /*! \brief Connects a link
     *
     * @return File descriptor on success, otherwise -1
     */
    virtual int connectLink() = 0;

    /*! \brief Disconnects a link
     *
     */
    virtual void disconnectLink() = 0;

private:

    bool setupOpenOBEX( int aFd, obex_event_t aEventHandler );

    void closeOpenOBEX();

    obex_t*         iTransportHandle;
    int             iMTUSize;

};

}

#endif  //  OBEXCONNECTION_H
