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
#ifndef STORAGECONTENTFORMATINFO_H
#define STORAGECONTENTFORMATINFO_H

#include <QString>
#include <QList>

namespace DataSync {

/*! \brief Information about a content format
 *
 */
struct ContentFormat
{
    QString iType;      /*!< MIME type of the content, for example "text/x-vcard"*/
    QString iVersion;   /*!< Version of the supported MIME type*/
};

/*! \brief Describes general information about content formats supported by
 *         a storage
 */
class StorageContentFormatInfo
{
public:

    /*! \brief Constructor
     *
     */
    StorageContentFormatInfo();

    /*! \brief Destructor
     *
     */
    ~StorageContentFormatInfo();

    /*! \brief Set preferred receive format
     *
     * @param aFormat Format to set
     */
    void setPreferredRx( const ContentFormat& aFormat );

    /*! \brief Retrieve preferred receive format
     *
     * @return
     */
    const ContentFormat& getPreferredRx() const;

    /*! \brief Set preferred transmit format
     *
     * @param aFormat
     */
    void setPreferredTx( const ContentFormat& aFormat );

    /*! \brief Retrieve preferred transmit format
     *
     * @return
     */
    const ContentFormat& getPreferredTx() const;

    /*! \brief Access receive formats
     *
     * @return
     */
    const QList<ContentFormat>& rx() const;

    /*! \brief Access receive formats
     *
     * @return
     */
    QList<ContentFormat>& rx();

    /*! \brief Access transmit formats
     *
     * @return
     */
    const QList<ContentFormat>& tx() const;

    /*! \brief Access transmit formats
     *
     * @return
     */
    QList<ContentFormat>& tx();

private:


    ContentFormat           iRxPref;
    ContentFormat           iTxPref;
    QList<ContentFormat>    iRx;
    QList<ContentFormat>    iTx;

};

}

#endif // STORAGECONTENTFORMATINFO_H
