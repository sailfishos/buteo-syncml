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

#include "RequestListener.h"

#include "SyncMLLogging.h"

using namespace DataSync;

RequestListener::RequestListener( QObject* aParent )
 : QObject( aParent ), iTransport( 0 )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

RequestListener::~RequestListener()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    qDeleteAll( iRequestData.iFragments);
    iRequestData.iFragments.clear();
}

bool RequestListener::start( Transport* aTransport )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !aTransport )
    {
        return false;
    }

    iTransport = aTransport;

    connect( iTransport, SIGNAL(sendEvent(DataSync::TransportStatusEvent, QString )),
             this, SLOT(transportEvent(DataSync::TransportStatusEvent , QString )) );

    connect( iTransport, SIGNAL(readXMLData(QIODevice *, bool)) ,
             &iParser, SLOT(parseResponse(QIODevice *, bool)) );

    connect( &iParser, SIGNAL(parsingComplete(bool)),
             this, SLOT(parsingComplete(bool)) );

    connect( &iParser, SIGNAL( parsingError(DataSync::ParserError)),
            this, SLOT(parserError(DataSync::ParserError)) );

    connect( iTransport, SIGNAL(readSANData(QIODevice *)) ,
             this, SLOT(SANPackageReceived(QIODevice *)) );

    return iTransport->receive();
}

void RequestListener::stop()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    disconnect( &iParser, 0, this, 0 );
    disconnect( iTransport, SIGNAL(readXMLData(QIODevice *, bool)) ,
             &iParser, SLOT(parseResponse(QIODevice *, bool)) );
    if( iTransport )
    {
        disconnect( iTransport, 0, this, 0 );
        iTransport = 0;
    }
}

RequestListener::RequestData RequestListener::takeRequestData()
{
    RequestData data = iRequestData;

    iRequestData.iFragments.clear();

    return data;

}


void RequestListener::transportEvent( DataSync::TransportStatusEvent aEvent, QString aErrorString )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    stop();

    switch (aEvent)
    {

        case TRANSPORT_CONNECTION_OPENED:
        case TRANSPORT_CONNECTION_CLOSED:
        {
            break;
        }
        case TRANSPORT_DATA_INVALID_CONTENT_TYPE:
        case TRANSPORT_DATA_INVALID_CONTENT:
        {
            emit error( INVALID_SYNCML_MESSAGE, aErrorString);
            break;
        }
        case TRANSPORT_CONNECTION_FAILED:
        case TRANSPORT_CONNECTION_TIMEOUT:
        case TRANSPORT_CONNECTION_ABORTED:
        case TRANSPORT_CONNECTION_AUTHENTICATION_NEEDED:
        {
            emit error( CONNECTION_ERROR , aErrorString );
            break;
        }
        default:
        {
            qCDebug(lcSyncML) << "Unknown transport status code: " << aEvent;
            emit error( INTERNAL_ERROR, aErrorString );
            break;
        }
    }
}

void RequestListener::parsingComplete( bool aLastMessageInPackage )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    Q_UNUSED( aLastMessageInPackage );

    stop();

    qCDebug(lcSyncML) << "SyncAgent: Received XML data, analyzing...";

    QList<DataSync::Fragment*> fragments = iParser.takeFragments();

    QList<SyncMode> syncModes;

    for( int i = 0; i < fragments.count(); ++i )
    {
        if( fragments[i]->fragmentType == Fragment::FRAGMENT_COMMAND )
        {
            CommandParams* command = static_cast<CommandParams*>(fragments[i]);

            if( command->commandType == CommandParams::COMMAND_ALERT )
            {
                SyncMode syncMode( command->data.toInt() );

                if( syncMode.isValid() )
                {
                    syncModes.append( syncMode );
                }
            }

        }
    }

    bool isValid = false;

    if( !syncModes.isEmpty() )
    {

        if( syncModes[0].syncInitiator() == INIT_CLIENT )
        {
            qCDebug(lcSyncML) << "SyncAgent: XML data recognized as sent by client";
            iRequestData.iType = REQUEST_CLIENT;
            isValid = true;
        }
        else if( syncModes[0].syncInitiator() == INIT_SERVER )
        {
            qCDebug(lcSyncML) << "SyncAgent: XML data recognized as sent by server";
            iRequestData.iType = REQUEST_SAN_XML;
            isValid = true;
        }
        else
        {
            qCCritical(lcSyncML) << "Could not recognize alert code";
        }
    }
    else
    {
        qCCritical(lcSyncML) << "Could not find any alerts from received XML data!";
    }

    if( isValid )
    {
        iRequestData.iFragments = fragments;
        emit newPendingRequest();
    }
    else
    {
        qDeleteAll(fragments);
        fragments.clear();
        emit error( INVALID_SYNCML_MESSAGE, "Invalid SyncML message" );
    }

}

void RequestListener::parserError( DataSync::ParserError aError )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    stop();

    switch (aError)
    {
        case PARSER_ERROR_INCOMPLETE_DATA:
        {
            emit error( INVALID_SYNCML_MESSAGE, "Parser error: incomplete data" );
            break;
        }
        case PARSER_ERROR_UNEXPECTED_DATA:
        {
            emit error( INVALID_SYNCML_MESSAGE, "Parser error: unexpected data" );
            break;
        }
        case PARSER_ERROR_INVALID_DATA:
        {
            emit error( INVALID_SYNCML_MESSAGE, "Parser error: invalid data" );
            break;
        }
        default:
        {
            emit error( INVALID_SYNCML_MESSAGE, "Unspecified error" );
            break;
        }
    }
}

void RequestListener::SANPackageReceived( QIODevice* aDevice )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    qCDebug(lcSyncML) << "SyncAgent: Received SAN data, processing...";

    QByteArray package = aDevice->readAll();

    SANHandler parser;

    iRequestData.iType = REQUEST_SAN_PACKAGE;
    qDeleteAll( iRequestData.iFragments );
    iRequestData.iFragments.clear();

    if( parser.parseSANMessageDS( package, iRequestData.iSANData ) )
    {
        qCDebug(lcSyncML) << "SyncAgent: SAN package processed";
        emit newPendingRequest();
    }
    else
    {
        qCCritical(lcSyncML) << "Parsing of SAN package failed";
        emit error( INVALID_SYNCML_MESSAGE, "Parsing of 1.2 SAN package failed" );
    }

}
