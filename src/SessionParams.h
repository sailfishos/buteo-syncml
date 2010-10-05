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
#ifndef SESSIONPARAMS_H
#define SESSIONPARAMS_H

#include <QString>

namespace DataSync {

/*! \brief Holds global parameters of SyncML session
 *
 */
class SessionParams
{
public:
    /*! \brief Constructor
     *
     */
    SessionParams();

    /*! \brief Destructor
     *
     */
    ~SessionParams();

    /*! \brief Sets Id of the session
     *
     * @param aSessionId Id of the session
     */
    void setSessionId( const QString& aSessionId );

    /*! \brief Returns Id of the session
     *
     * @return
     */
    const QString& sessionId() const;

    /*! \brief Sets name of the local device
     *
     * @param aLocalDeviceName Name of the local device
     */
    void setLocalDeviceName( const QString& aLocalDeviceName );

    /*! \brief Returns the name of the local device
     *
     * @return
     */
    const QString& localDeviceName();

    /*! \brief Sets the name of the remote device
     *
     * @param aRemoteDeviceName Name of the remote device
     */
    void setRemoteDeviceName( const QString& aRemoteDeviceName );

    /*! \brief Returns the name of the remote device
     *
     * @return
     */
    const QString& remoteDeviceName();

    /*! \brief Sets the maximum message size for local device
     *
     * @param aLocalMaxMsgSize Maximum message size for local device
     */
    void setLocalMaxMsgSize( qint32 aLocalMaxMsgSize );

    /*! \brief Returns the maximum message size for local device
     *
     * @return
     */
    qint32 localMaxMsgSize();

    /*! \brief Sets the maximum message size for remote device
      *
      * @param aRemoteMaxMsgSize Maximum message size for remote device
      */
    void setRemoteMaxMsgSize( qint32 aRemoteMaxMsgSize );

    /*! \brief Returns the maximum message size for remote device
     *
     * @return
     */
    qint32 remoteMaxMsgSize();

protected:

private:


    QString iSessionId;
    QString iLocalDeviceName;
    QString iRemoteDeviceName;

    qint32  iLocalMaxMsgSize;
    qint32  iRemoteMaxMsgSize;

};

}

#endif // SESSIONPARAMS_H
