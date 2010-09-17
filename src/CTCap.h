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
#ifndef CTCAPS_H
#define CTCAPS_H

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

/*! \brief Parameter of a property
 *
 */
struct CTCapParameter
{
    QString         iName;          /*!< Name of the parameter*/
    QString         iType;          /*!< Type of the parameter*/
    QString         iDisplayName;   /*!< Human-readable name of the parameter*/
    QList<QString>  iValues;        /*!< Allowed values for the parameter*/
};

/*! \brief CTCap property
 *
 */
struct CTCapProperty
{
    QString                 iName;          /*!< Name of the property*/
    QString                 iType;          /*!< Type of the property*/
    int                     iSize;          /*!< Maximum size of the property*/
    QString                 iDisplayName;   /*!< Human-readable name of the property*/
    QList<QString>          iValues;        /*!< Allowed values for the property*/
    QList<CTCapParameter>   iParameters;    /*!< Parameters of the property*/

    CTCapProperty() : iSize( -1 ) { }
};

/*! \brief Describes single content format and its properties
 *
 */
class CTCap
{
public:
    /*! \brief Constructor
     *
     */
    CTCap();

    /*! \brief Destructor
     *
     */
    ~CTCap();

    /*! \brief Set content format
     *
     * @param aFormat Content format
     */
    void setFormat( const ContentFormat& aFormat );

    /*! \brief Returns content format
     *
     * @return
     */
    const ContentFormat& getFormat() const;

    /*! \brief Returns properties
     *
     * @return
     */
    const QList<CTCapProperty>& properties() const;

    /*! \brief Returns properties
     *
     * @return
     */
    QList<CTCapProperty>& properties();

private:
    ContentFormat           iFormat;        /*!< Format*/
    QList<CTCapProperty>    iProperties;    /*!< Properties*/
};

}

#endif // CTCAPS_H
