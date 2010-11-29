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

#include "HTTPTransport.h"

#include <QtNetwork>

#include "datatypes.h"
#include "SyncAgentConfigProperties.h"

#include "LogMacros.h"

using namespace DataSync;

HTTPTransport::HTTPTransport( const ProtocolContext& aContext, QObject* aParent )
: BaseTransport( aContext, aParent), iManager( 0 ), iFirstMessageSent( false ),
  iMaxNumberOfResendAttempts( 0 ), iNumberOfResendAttempts( 0 )
{
    FUNCTION_CALL_TRACE;

    iManager = new QNetworkAccessManager;
    // Workaround for NB #176070 -- QNetworkAccessManager cannot be used within
    // QThreads. @todo: Remove this when the bug is completely fixed.
    iManager->setConfiguration(QNetworkConfiguration());
    iManager->proxy().setType( QNetworkProxy::NoProxy );
}

HTTPTransport::~HTTPTransport()
{
    FUNCTION_CALL_TRACE;

    delete iManager;
    iManager = NULL;
}

void HTTPTransport::setProperty( const QString& aProperty, const QString& aValue )
{
    FUNCTION_CALL_TRACE;

    if( aProperty == HTTPNUMBEROFRESENDATTEMPTSPROP )
    {
        LOG_DEBUG( "Setting property" << aProperty <<":" << aValue );
        iMaxNumberOfResendAttempts = aValue.toInt();
    }
    else if( aProperty == HTTPPROXYHOSTPROP )
    {
        LOG_DEBUG( "Setting property" << aProperty <<":" << aValue );
        iManager->proxy().setType( QNetworkProxy::HttpProxy );
        iManager->proxy().setHostName(HTTPPROXYHOSTPROP);
    }
    else if( aProperty == HTTPPROXYPORTPROP )
    {
        LOG_DEBUG( "Setting property" << aProperty <<":" << aValue );
        iManager->proxy().setType( QNetworkProxy::HttpProxy );
        iManager->proxy().setPort( HTTPPROXYPORTPROP.toInt() );
    }

}

bool HTTPTransport::init()
{
    FUNCTION_CALL_TRACE;

    connect( iManager, SIGNAL(finished(QNetworkReply *)),
              this, SLOT(httpRequestFinished(QNetworkReply *)), Qt::QueuedConnection);
    connect( iManager,SIGNAL(authenticationRequired(QNetworkReply *,QAuthenticator *)),
              this,SLOT(authRequired(QNetworkReply *,QAuthenticator * )), Qt::QueuedConnection);

#ifndef QT_NO_OPENSSL
    connect( iManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>& )),
             this, SLOT(sslErrors(QNetworkReply*, const QList<QSslError>& )));
#endif

    iFirstMessageSent = false;

    return true;
}

void HTTPTransport::close()
{
    FUNCTION_CALL_TRACE;
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
    iManager->setProxy( aProxy );
}

QNetworkProxy HTTPTransport::getProxyConfig()
{
    return iManager->proxy();
}

void HTTPTransport::addXheader(const QString& aName, const QString& aValue)
{
    iXheaders.insert(aName, aValue);
}

void HTTPTransport::prepareRequest( QNetworkRequest& aRequest, const QByteArray& aContentType,
                                    int aContentLength )
{
    FUNCTION_CALL_TRACE;

    QUrl url;
    // The URL might be percent encoded
    url = QUrl::fromEncoded( getRemoteLocURI().toAscii() );
    if( !url.isValid() )
    {
        url = QUrl( getRemoteLocURI() );
    }
    aRequest.setRawHeader( HTTP_HDRSTR_POST, url.path().toAscii());
    aRequest.setUrl( url );
    aRequest.setRawHeader( HTTP_HDRSTR_UA, HTTP_UA_VALUE);
    aRequest.setRawHeader( HTTP_HDRSTR_CONTENT_TYPE, aContentType );
    aRequest.setRawHeader( HTTP_HDRSTR_ACCEPT,HTTP_ACCEPT_VALUE );
    aRequest.setHeader( QNetworkRequest::ContentLengthHeader, QVariant( aContentLength ) );
    QMap<QString, QString>::const_iterator i;
    for (i = iXheaders.constBegin(); i != iXheaders.constEnd(); i++) {
	    aRequest.setRawHeader(i.key().toAscii(), i.value().toAscii());
    }

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
    emit sendEvent( TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED, "Authentication required" );
}

void HTTPTransport::handleProxyAuthentication(QNetworkProxy& /*aProxy*/, QAuthenticator* /*aAuth*/ )
{
    FUNCTION_CALL_TRACE
    LOG_DEBUG( "Proxy needs authentication" );
    sendEvent( TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED, "Authentication required" );
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
