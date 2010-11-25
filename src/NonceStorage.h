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

#ifndef NONCESTORAGE_H
#define NONCESTORAGE_H

#include <QString>

class QSqlDatabase;

namespace DataSync {

/*! \brief Class for storing MD5 nonces
 *
 */
class NonceStorage
{

public:

    /*! \brief Constructor
     *
     * The order of devices means, that whenever aLocalDevice authenticates with
     * aRemoteDevice, it should use the specified nonce
     *
     * @param aDbHandle Database handle to use
     * @param aLocalDevice Local device to associate with
     * @param aRemoteDevice Remove device to associate with
     */
    explicit NonceStorage( QSqlDatabase& aDbHandle, const QString& aLocalDevice, const QString& aRemoteDevice );

    /*! \brief Destructor
     *
     */
    virtual ~NonceStorage();

    /*! \brief Generates a new nonce
     *
     * @return Generated nonce
     */
    QByteArray generateNonce() const;

    /*! \brief Retrieves a nonce from storage
     *
     * @return Nonce if found, otherwise empty
     */
    QByteArray nonce();

    /*! \brief Sets a new nonce to storage
     *
     *
     * @param aNonce Nonce to store
     */
    void setNonce( const QByteArray& aNonce );


    /*! \brief Clears a nonce from storage
     *
     */
    void clearNonce();

protected:

    /*! \brief Ensure that database table exists for Nonces
     *
     * @return True on success, otherwise false
     */
    bool createNonceTable();

private:

    QSqlDatabase&   iDbHandle;
    QString         iLocalDevice;
    QString         iRemoteDevice;

};

}

#endif  //  NONCESTORAGE_H
