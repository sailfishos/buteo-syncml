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

#ifndef HTTPTRANSPORT_H
#define HTTPTRANSPORT_H

#include <QHttp>

#include "BaseTransport.h"

class QNetworkAccessManager;
class QNetworkProxy;
class QNetworkReply;
class QNetworkRequest;
class QAuthenticator;

namespace DataSync {

/*! \brief HTTP Implementation of the Transport class
 */
class HTTPTransport : public BaseTransport
{
    Q_OBJECT

public:

    /*! \brief Constructor
     *
     * @param aParent Parent of this object
     */
    HTTPTransport( QObject* aParent = 0 );

    /*! \brief Destructor
     *
     */
    virtual ~HTTPTransport();

    virtual void setProperty( const QString& aProperty, const QString& aValue );

    /*! \brief Initialize HTTP transport
     *
     * Should be called before attempting to use the rest of the API
     *
     */
    void init();

    /*! \brief Sets proxy config to use
     *
     * @param aProxy Proxy configuration
     */
    void setProxyConfig( const QNetworkProxy& aProxy );

    /*! \brief Sets the authentication token used for SSO type
     * authentications.
     *
     * @param aToken The authentication token
     */
    void setAuthToken( const QString &aToken );

    /*! \brief Returns the authentication token used for SSO type
     * authentications.
     *
     * @return The authentication token
     */
    QString getAuthToken() const;

    /*! \brief Returns the proxy config in use
     *
     * @return Proxy configuration
     */
    QNetworkProxy getProxyConfig();

    /*! \brief Adds x-header in the form name - value
     *
     * @param aName X-header field name
     * @param aValue X-header field value
     */
    void addXheader(const QString& aName, const QString& aValue);

protected:

    virtual bool prepareSend();

    virtual bool doSend( const QByteArray& aData, const QString& aContentType );

    virtual bool doReceive( const QString& aContentType );

private slots:

    void httpRequestFinished( QNetworkReply* aReply );

    void handleProxyAuthentication( QNetworkProxy& aProxy, QAuthenticator* aAuth );

    void authRequired(QNetworkReply* aReply, QAuthenticator* aAuth );

#ifndef QT_NO_OPENSSL

    void sslErrors( QNetworkReply* aReply, const QList<QSslError>& aErrors );
#endif

private:

    void prepareRequest( QNetworkRequest& aRequest, const QByteArray& aContentType,
                         int aContentLength );

    bool sendRequest( const QByteArray& aData, const QString& aContentType );

    bool shouldResend() const;
    bool resend();



    QNetworkAccessManager*  iManager;

    bool                    iFirstMessageSent;
    QByteArray              iFirstMessageData;
    QString                 iFirstMessageContentType;
    int                     iMaxNumberOfResendAttempts;
    int                     iNumberOfResendAttempts;
    QString                 iAuthToken;
    QMap<QString, QString>  iXheaders;
};

}

#endif // HTTPTRANSPORT_H
