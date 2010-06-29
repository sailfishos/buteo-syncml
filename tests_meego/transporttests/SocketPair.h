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
#ifndef SOCKETPAIR_H
#define SOCKETPAIR_H

#include <QObject>

class QTcpSocket;
class QTcpServer;

namespace DataSync {

/*! \brief Creates a local TCP socket pair and connects them together.
 *
 * Data written to the client socket can be read from the server socket
 * and vice versa. Despite their names, the client and server sockets are
 * similar and they can be used in peer-to-peer communication.
 */
class SocketPair : public QObject
{
    Q_OBJECT

public:
    //! \brief Constructor
    SocketPair(QObject *aParent = 0);

    //! \brief Destructor
    virtual ~SocketPair();

    /*! \brief Initializes the socket pair.
     *
     * Must be called before getting client or server socket pointers.
     * \return Success indicator.
     */
    bool init();

    /*! \brief Gets client socket pointer. Ownership is not transferred.
     *
     * The instance of this class is the parent of the returned socket
     * and must exist as long as the socket is used.
     * socketDescriptor can be called for the returned socket to get a
     * descriptor that can be used for example with QFile.
     * \return Client socket pointer. 0 if not available.
     */
    QTcpSocket *clientSocket();

    /*! \brief Gets server socket pointer. Ownership is not transferred.
     *
     * The instance of this class is the parent of the returned socket
     * and must exist as long as the socket is used.
     * socketDescriptor can be called for the returned socket to get a
     * descriptor that can be used for example with QFile.
     * \return Server socket pointer. 0 if not available.
     */
    QTcpSocket *serverSocket();

private:

    QTcpServer *iServer;

    QTcpSocket *iClientSocket;

    QTcpSocket *iServerSocket;
};

}
#endif // SOCKETPAIR_H
