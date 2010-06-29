/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "HTTPTransport.h"

#include <QtNetwork>

#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

HTTPTransport::HTTPTransport( QObject* aParent )
: BaseTransport( aParent), iManager( 0 ), iProxy( 0 ), iFirstMessageSent( false ),
  iMaxNumberOfResendAttempts( 0 ), iNumberOfResendAttempts( 0 )
{
    FUNCTION_CALL_TRACE;

    iManager = new QNetworkAccessManager;
    iProxy = new QNetworkProxy;
}

HTTPTransport::~HTTPTransport()
{
    FUNCTION_CALL_TRACE;

    delete iProxy;
    iProxy = NULL;

    delete iManager;
    iManager = NULL;
}

void HTTPTransport::init()
{
    FUNCTION_CALL_TRACE;

    if( iProxy->type() == QNetworkProxy::DefaultProxy ) {
        iProxy->setType( QNetworkProxy::NoProxy );
    }

    iManager->setProxy( *iProxy );

    connect( iManager, SIGNAL(finished(QNetworkReply *)),
              this, SLOT(httpRequestFinished(QNetworkReply *)), Qt::QueuedConnection);
    connect( iManager,SIGNAL(authenticationRequired(QNetworkReply *,QAuthenticator *)),
              this,SLOT(authRequired(QNetworkReply *,QAuthenticator * )), Qt::QueuedConnection);

#ifndef QT_NO_OPENSSL
    connect( iManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>& )),
             this, SLOT(sslErrors(QNetworkReply*, const QList<QSslError>& )));
#endif

    iFirstMessageSent = false;
}

qint64 HTTPTransport::getMaxTxSize()
{
    FUNCTION_CALL_TRACE;
    return HTTP_MAX_MESSAGESIZE;
}

qint64 HTTPTransport::getMaxRxSize()
{
    FUNCTION_CALL_TRACE;
    return HTTP_MAX_MESSAGESIZE;
}

bool HTTPTransport::prepareSend()
{
    // Don't need to do nothing in prepareSend as HTTP is sessionless
    return true;
}

bool HTTPTransport::doSend( const QByteArray& aData, const QString& aContentType )
{
    FUNCTION_CALL_TRACE;

    if( sendRequest( aData, aContentType ) )
    {

        // If this is the first message to be sent, save it
        if( !iFirstMessageSent )
        {
            iNumberOfResendAttempts = 0;
            iFirstMessageData = aData;
            iFirstMessageContentType = aContentType;
        }

        return true;
    }
    else
    {
        return false;
    }

}

bool HTTPTransport::doReceive( const QString& aContentType )
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED( aContentType );
    return true;
}

void HTTPTransport::setProxyConfig( const QNetworkProxy& aProxy )
{
    FUNCTION_CALL_TRACE;
    *iProxy = aProxy;
}

QNetworkProxy& HTTPTransport::getProxyConfig()
{
    return *iProxy;
}

void HTTPTransport::setResendAttempts( int aNumber )
{
    iMaxNumberOfResendAttempts = aNumber;
}

int HTTPTransport::getResendAttempts() const
{
    return iMaxNumberOfResendAttempts;
}

void HTTPTransport::setAuthToken( const QString &aToken )
{
    iAuthToken = aToken;
}

QString HTTPTransport::getAuthToken() const
{
    return iAuthToken;
}

void HTTPTransport::prepareRequest( QNetworkRequest& aRequest, const QByteArray& aContentType,
                                    int aContentLength )
{
    FUNCTION_CALL_TRACE;

    QUrl url( getRemoteLocURI() );

    aRequest.setRawHeader( HTTP_HDRSTR_POST, url.path().toAscii());
    aRequest.setUrl( url );
    aRequest.setRawHeader( HTTP_HDRSTR_UA, HTTP_UA_VALUE);
    aRequest.setRawHeader( SYNCML_CONTENT_TYPE, aContentType );
    aRequest.setRawHeader( HTTP_HDRSTR_ACCEPT,HTTP_ACCEPT_VALUE );
    aRequest.setRawHeader( HTTP_HDRSTR_AUTHTYPE, HTTP_AUTHTYPE_VALUE );
    if(false == iAuthToken.isEmpty())
    {
        aRequest.setRawHeader( HTTP_HDRSTR_TOKEN, iAuthToken.toAscii() );
    }
    aRequest.setHeader( QNetworkRequest::ContentLengthHeader, QVariant( aContentLength ) );

#ifndef QT_NO_OPENSSL
    //do it only for https
    if( url.toString().contains(SYNCML_SCHEMA_HTTPS)) {
        LOG_DEBUG("HTTPS protocol detected");
//        Don't remove the below commented code.
//        this can be used while adding full fledged ssl support.
//        QNetworkAccessManager sets the default configuration needed for https
//        if we want to change the protocol , we will have do it as follows.
//        QSslConfiguration ssl = request.sslConfiguration();
//        if( ssl.isNull() )
//        {
//            ssl.setProtocol(QSsl::SslV3);
//        }
//        request.setSslConfiguration(ssl);
    }
#endif  //  QT_NO_OPENSSL

}

bool HTTPTransport::sendRequest( const QByteArray& aData, const QString& aContentType )
{
    FUNCTION_CALL_TRACE;
    // build the message, and send it
    QNetworkRequest request;
    prepareRequest( request, aContentType.toAscii(), aData.size() );

#ifndef QT_NO_DEBUG
    // Print the message
    LOG_PROTOCOL("Sending request to" << request.url().host());
    LOG_PROTOCOL("Headers:");
    QList<QByteArray> headers = request.rawHeaderList();
    foreach( const QByteArray& ar, headers ) {
            LOG_PROTOCOL(ar << ": " << request.rawHeader(ar));
    }
#endif  //  QT_NO_DEBUG

    if( iManager->post(request, aData) ) {
        // send succeeded
        return true;
    }
    else {
        // send failed
        return false;
    }
}

bool HTTPTransport::shouldResend() const
{
    FUNCTION_CALL_TRACE;

    // We should try to re-send a message if it's the first message to be sent,
    // and if we have some retry attempts left
    if( !iFirstMessageSent && iNumberOfResendAttempts < iMaxNumberOfResendAttempts ) {
        return true;
    }
    else {
        return false;
    }
}

bool HTTPTransport::resend()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Attempting to re-send initialization package, attempt number:" << iNumberOfResendAttempts + 1 );

    if( sendRequest( iFirstMessageData, iFirstMessageContentType ) ) {
        ++iNumberOfResendAttempts;
        return true;
    }
    else {
        return false;
    }

}

void HTTPTransport::httpRequestFinished( QNetworkReply *aReply ) {
    FUNCTION_CALL_TRACE;

    Q_ASSERT( aReply );

    if( aReply->error() != QNetworkReply::NoError )
    {
        switch( aReply->error() )
        {
            case QNetworkReply::TimeoutError:
            {

                // In case the remote side times out, possibly try to re-send the message.
                // If message should not be re-sent, or the re-send fails, handle as
                // an error
                if( !shouldResend() || !resend() ) {
                    LOG_DEBUG("Connection timeout:" << aReply->errorString());
                    emit sendEvent(TRANSPORT_CONNECTION_TIMEOUT, aReply->errorString());
                }
                break;
            }
            default:
            {
                LOG_DEBUG("TRANSPORT ERROR REASON:" << aReply->errorString());
                emit sendEvent(TRANSPORT_CONNECTION_FAILED, aReply->errorString());
                break;
            }
        };
    }
    else {

#ifndef QT_NO_DEBUG
        LOG_PROTOCOL( "Received response" );
        LOG_PROTOCOL( "Headers:" );

        QList<QByteArray> headers = aReply->rawHeaderList();
        foreach( const QByteArray& ar, headers ) {
                LOG_PROTOCOL(ar << ": " << aReply->rawHeader(ar));
        }
#endif  //  QT_NO_DEBUG

        QByteArray data = aReply->readAll();

        // In case of zero-length response, possibly try to re-send the message. If the message
        // should not be re-sent, or if re-send fails, let the zero-length response through.
        // BaseTransport::receive() will mark it as TRANSPORT_DATA_INVALID_CONTENT error.
        if( !data.isEmpty() || !shouldResend() || !resend() ) {

            QString contentType = aReply->header( QNetworkRequest::ContentTypeHeader).toString();

            if( !iFirstMessageSent ) {
                iFirstMessageSent = true;
                iFirstMessageData.clear();
                iFirstMessageContentType.clear();
            }

            receive( data, contentType );

        }

    }

    aReply->deleteLater();

}

void HTTPTransport::authRequired(QNetworkReply* /*aReply*/, QAuthenticator* /*aAuth*/ ) {
    FUNCTION_CALL_TRACE
    LOG_DEBUG("Network Connection needs authentication");
    emit sendEvent( TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED, HTTP_ERROR_AUTH_NEEDED );
}

void HTTPTransport::handleProxyAuthentication(QNetworkProxy& /*aProxy*/, QAuthenticator* /*aAuth*/ )
{
    FUNCTION_CALL_TRACE
    LOG_DEBUG( "Proxy needs authentication" );
    sendEvent( TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED, HTTP_ERROR_AUTH_NEEDED );
}

#ifndef QT_NO_OPENSSL
void HTTPTransport::sslErrors( QNetworkReply* aReply, const QList<QSslError>& aErrors ) {
    FUNCTION_CALL_TRACE
    LOG_DEBUG("SSL Errors received");
    LOG_DEBUG("list size :" << aErrors.size());
    foreach( const QSslError& sslError , aErrors) {
        LOG_DEBUG(sslError.errorString());
        if (sslError.certificate().isValid()) {
            LOG_DEBUG("StartDate: " << sslError.certificate().effectiveDate());
            LOG_DEBUG("ExpiryDate:" << sslError.certificate().expiryDate());
            LOG_DEBUG("Issuer Info:");
            LOG_DEBUG("Organization:" << sslError.certificate().issuerInfo( QSslCertificate::Organization ) );
        }
    }
    LOG_DEBUG("Ignoring SSL Errors");
    aReply->ignoreSslErrors();

}
#endif
