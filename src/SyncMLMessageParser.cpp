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

#include "SyncMLMessageParser.h"
#include "LogMacros.h"
#include "RemoteDeviceInfo.h"

using namespace DataSync;


SyncMLMessageParser::SyncMLMessageParser()
 : iLastMessageInPackage( false ), iError( PARSER_ERROR_LAST ),
   iSyncHdrFound( false ), iSyncBodyFound( false ),
   iIsNewPacket( false )
{
    FUNCTION_CALL_TRACE

    qRegisterMetaType<DataSync::ParserError>("DataSync::ParserError");

#ifdef QT_DEBUG
    // additional functions for debugging the status and alert codes from server
    initMaps();
#endif
}

SyncMLMessageParser::~SyncMLMessageParser()
{
    FUNCTION_CALL_TRACE

    qDeleteAll(iFragments);
    iFragments.clear();
}


QList<DataSync::Fragment*> SyncMLMessageParser::takeFragments()
{
    FUNCTION_CALL_TRACE

    QList<DataSync::Fragment*> fragments = iFragments;
    iFragments.clear();
    return fragments;
}


void SyncMLMessageParser::parseResponse( QIODevice *aDevice, bool aIsNewPacket )
{
    FUNCTION_CALL_TRACE

    iIsNewPacket = aIsNewPacket;
    if( aDevice->bytesAvailable() == 0 ) {
        LOG_CRITICAL( "Zero-sized message detected, aborting parsing");
        emit parsingError( PARSER_ERROR_INVALID_DATA );
    }
    else
    {

        LOG_DEBUG( "Beginning to parse incoming message..." );

        iReader.setDevice( aDevice );
        startParsing();

        LOG_DEBUG( "Incoming message parsed");
    }

}

void SyncMLMessageParser::startParsing()
{

    FUNCTION_CALL_TRACE

    qDeleteAll(iFragments);
    iFragments.clear();
    iLastMessageInPackage = false;

    iSyncHdrFound = false;
    iSyncBodyFound = false;

    iError = PARSER_ERROR_LAST;

    while( shouldContinue() ) {

        iReader.readNext();

        QXmlStreamReader::TokenType token = iReader.tokenType();
        switch( token )
            {
            case QXmlStreamReader::StartDocument:
            {
                break;
            }
            case QXmlStreamReader::EndDocument:
            {
                break;
            }
            case QXmlStreamReader::StartElement:
            {
                QStringRef name = iReader.name();
                if( name == SYNCML_ELEMENT_SYNCHDR ) {
                    readHeader();
                } else if( name == SYNCML_ELEMENT_SYNCBODY ) {
                    readBody();
                } else if( name != SYNCML_ELEMENT_SYNCML ){
                    LOG_CRITICAL("Unexpected element in SyncML message:" << name );
                    iError = PARSER_ERROR_UNEXPECTED_DATA;
                }
                break;

            }
            case QXmlStreamReader::EndElement:
            case QXmlStreamReader::Characters:
            case QXmlStreamReader::DTD:
            case QXmlStreamReader::Comment:
            {
                break;
            }
            case QXmlStreamReader::Invalid:
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::EntityReference:
            case QXmlStreamReader::ProcessingInstruction:
            {
                LOG_CRITICAL("Unexpected token in SyncML message" << iReader.tokenType());
                iError = PARSER_ERROR_UNEXPECTED_DATA;
                break;
            }
        }
    }

    if( iError != PARSER_ERROR_LAST )
    {
        LOG_CRITICAL( "Error while parsing SyncML document:" << iError );
        // Check if the parsing error happened due to invalid XML characters
        if( iIsNewPacket && ( QXmlStreamReader::PrematureEndOfDocumentError  == iReader.error()
                || QXmlStreamReader::NotWellFormedError == iReader.error() ) )
        {
            // Change error here to Invalid character found error
            // so that session handler can retry after removing illegal
            // characters
            iError = PARSER_ERROR_INVALID_CHARS;
        }
        emit parsingError( iError );
    }
    else if( !iSyncHdrFound || !iSyncBodyFound )
    {
        LOG_CRITICAL( "Malformed SyncML document, missing either SyncHdr or SyncBody" );
        emit parsingError( PARSER_ERROR_INCOMPLETE_DATA );
    }
    else
    {
        emit parsingComplete(iLastMessageInPackage);
    }

}

void SyncMLMessageParser::readBody()
{
    FUNCTION_CALL_TRACE

    if( iSyncBodyFound )
    {
        LOG_CRITICAL( "Invalid SyncML message, multiple SyncBody elements found" );
        iError = PARSER_ERROR_INVALID_DATA;
        return;
    }

    iSyncBodyFound = true;

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_SYNCBODY ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if( name == SYNCML_ELEMENT_STATUS) {
                readStatus();
            } else if (name == SYNCML_ELEMENT_SYNC) {
                readSync();
            } else if (name == SYNCML_ELEMENT_PUT) {
                readPut();
            } else if (name == SYNCML_ELEMENT_RESULTS) {
                readResults();
            } else if (name == SYNCML_ELEMENT_MAP ) {
                readMap();
            }else if (name == SYNCML_ELEMENT_FINAL) {
                iLastMessageInPackage = true;
            } else {
                CommandParams* command = new CommandParams();

                if( readCommand( name, *command ) ) {
                    iFragments.append( command );
                }
                else {
                    delete command;
                    command = 0;
                    LOG_WARNING("UNKNOWN  TOKEN TYPE in BODY:NOT HANDLED BY PARSER" << name );
                }

            }

        }

    }

    if( iReader.atEnd() ) {
        LOG_CRITICAL( "Incomplete SyncML message" );
        iError = PARSER_ERROR_INCOMPLETE_DATA;
    }

}

void SyncMLMessageParser::readHeader()
{
    FUNCTION_CALL_TRACE

    if( iSyncHdrFound )
    {
        LOG_CRITICAL( "Invalid SyncML message, multiple SyncHdr elements found" );
        iError = PARSER_ERROR_INVALID_DATA;
        return;
    }

    iSyncHdrFound = true;

    HeaderParams *header = new HeaderParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_SYNCHDR ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if (name == SYNCML_ELEMENT_VERDTD) {
                header->verDTD = readString();
            }
            else if (name == SYNCML_ELEMENT_VERPROTO) {
                header->verProto = readString();
            }
            else if (name == SYNCML_ELEMENT_SESSIONID) {
                header->sessionID = readString();
            }
            else if (name == SYNCML_ELEMENT_MSGID) {
                header->msgID = readInt();
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                header->targetDevice = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                header->sourceDevice = readURI();
            }
            else if (name == SYNCML_ELEMENT_RESPURI) {
                header->respURI = readString();
            }
            else if (name == SYNCML_ELEMENT_NORESP) {
                header->noResp = true;
            }
            else if (name == SYNCML_ELEMENT_CRED) {
                readCred( header->cred );
            }
            else if (name == SYNCML_ELEMENT_META) {
                readMeta( header->meta );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in HEADER:NOT HANDLED BY PARSER" << name );
            }
        }
    }

    iFragments.append(header);

    if( iReader.atEnd() ) {
        LOG_CRITICAL( "Incomplete SyncML message" );
        iError = PARSER_ERROR_INCOMPLETE_DATA;
    }

}

void SyncMLMessageParser::readChal( ChalParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        if( iReader.isEndElement() && iReader.name() == SYNCML_ELEMENT_CHAL ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if( iReader.name() == SYNCML_ELEMENT_META ) {
                readMeta( aParams.meta );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in CHAL:NOT HANDLED BY PARSER" << iReader.name() );
            }
        }
    }

}

void SyncMLMessageParser::readStatus()
{
    FUNCTION_CALL_TRACE

    StatusParams *status = new StatusParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_STATUS ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if(name == SYNCML_ELEMENT_CMDID) {
                status->cmdId = readInt();
            }
            else if (name == SYNCML_ELEMENT_MSGREF) {
                status->msgRef = readInt();
            }
            else if (name == SYNCML_ELEMENT_CMDREF) {
                status->cmdRef = readInt();
            }
            else if (name == SYNCML_ELEMENT_CMD) {
                status->cmd = readString();
            }
            else if (name == SYNCML_ELEMENT_TARGETREF) {
                status->targetRef = readString();
            }
            else if (name == SYNCML_ELEMENT_SOURCEREF) {
                status->sourceRef = readString();
            }
            else if (name == SYNCML_ELEMENT_DATA) {
                status->data = (ResponseStatusCode)readInt();
                LOG_DEBUG( iStatusCodeMap[status->data] << ":" << status->data );
            }
            else if (name == SYNCML_ELEMENT_ITEM) {
                ItemParams item;
                readItem( item );
                status->items.append( item );
            }
            else if (name == SYNCML_ELEMENT_CHAL) {
                status->hasChal = true;
                readChal( status->chal );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in STATUS:NOT HANDLED BY PARSER" << name);
            }

         }

    }

    iFragments.append(status);
}

void SyncMLMessageParser::readSync()
{
    FUNCTION_CALL_TRACE

    SyncParams *sync = new SyncParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_SYNC ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if (name == SYNCML_ELEMENT_CMDID) {
                sync->cmdId = readInt();
            }
            else if (name == SYNCML_ELEMENT_NORESP) {
                sync->noResp = true;
            }
            else if (name == SYNCML_ELEMENT_META) {
                readMeta( sync->meta );
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                sync->target = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                sync->source = readURI();
            }
            else if( name == SYNCML_ELEMENT_NUMOFCHANGES ) {
                sync->numberOfChanges = readInt();
            }
            else {
                CommandParams command;
                if( readCommand( name, command ) ) {
                    sync->commands.append(command);
                }
                else {
                    LOG_WARNING("UNKNOWN TOKEN TYPE in SYNC:NOT HANDLED BY PARSER" << name);
                }

            }

        }

    }

    iFragments.append(sync);

}

void SyncMLMessageParser::readMap()
{
    FUNCTION_CALL_TRACE

    MapParams *map = new MapParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_MAP ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if( name == SYNCML_ELEMENT_CMDID) {
                map->cmdId = readInt();
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                map->target = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                map->source = readURI();
            }
            else if (name == SYNCML_ELEMENT_META) {
                readMeta( map->meta );
            }
            else if (name == SYNCML_ELEMENT_MAPITEM) {
                MapItemParams item;
                readMapItem( item );
                map->mapItems.append( item );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in MAP:NOT HANDLED BY PARSER" << name );
            }

        }

    }

    iFragments.append(map);

}

void SyncMLMessageParser::readMapItem( MapItemParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_MAPITEM ) {
            break;
        }
        if( iReader.isStartElement() ) {
            if (name == SYNCML_ELEMENT_TARGET) {
                aParams.target = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                aParams.source = readURI();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in MAPITEM:NOT HANDLED BY PARSER" << name );
            }

        }

    }

}

void SyncMLMessageParser::readPut()
{
    FUNCTION_CALL_TRACE;

    PutParams* put = new PutParams;

    while( shouldContinue() )
    {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_PUT )
        {
            break;
        }

        if( iReader.isStartElement() )
        {
            if (name == SYNCML_ELEMENT_CMDID)
            {
                put->cmdId = readInt();
            }
            else if (name == SYNCML_ELEMENT_NORESP)
            {
                put->noResp = true;
            }
            else if (name == SYNCML_ELEMENT_META)
            {
                readMeta( put->meta );
            }
            else if (name == SYNCML_ELEMENT_ITEM)
            {
                readDevInfItem( put->devInf );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in PUT:NOT HANDLED BY PARSER" << name);
            }
        }
    }

    // Ensure that the PUT fragment is kept next only to the HEADER, or RESULTS fragment.
    if( iFragments.count() > 0 )
    {
        iFragments.insert( 1, put );
    }
    else
    {
        iFragments.append(put);
    }
}

void SyncMLMessageParser::readResults()
{
    FUNCTION_CALL_TRACE

    ResultsParams *results = new ResultsParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_RESULTS ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if (name == SYNCML_ELEMENT_CMDID) {
                results->cmdId = readInt();
            }
            else if (name == SYNCML_ELEMENT_MSGREF) {
                results->msgRef = readInt();
            }
            else if (name == SYNCML_ELEMENT_CMDREF) {
                results->cmdRef = readInt();
            }
            else if (name == SYNCML_ELEMENT_META) {
                readMeta( results->meta );
            }
            else if (name == SYNCML_ELEMENT_TARGETREF) {
                results->targetRef = readString();
            }
            else if (name == SYNCML_ELEMENT_SOURCEREF) {
                results->sourceRef = readString();
            }
            else if (name == SYNCML_ELEMENT_ITEM) {
                readDevInfItem( results->devInf );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in RESULTS:NOT HANDLED BY PARSER" << name);
            }

        }

    }

    // Ensure that the RESULTS fragment is kept next only to the HEADER, or PUT fragment.
    if( iFragments.count() > 0 )
    {
        iFragments.insert(1, results);
    }
    else
    {
        iFragments.append(results);
    }

}

bool SyncMLMessageParser::readCommand( const QStringRef& aName, CommandParams& aCommand )
{
    FUNCTION_CALL_TRACE;

    bool found = true;

    if( aName == SYNCML_ELEMENT_ALERT ) {
        aCommand.commandType = CommandParams::COMMAND_ALERT;
        readLeafCommand( aCommand, SYNCML_ELEMENT_ALERT );
    }
    else if (aName == SYNCML_ELEMENT_ADD) {
        aCommand.commandType = CommandParams::COMMAND_ADD;
        readLeafCommand( aCommand, SYNCML_ELEMENT_ADD );
    }
    else if (aName == SYNCML_ELEMENT_REPLACE) {
        aCommand.commandType = CommandParams::COMMAND_REPLACE;
        readLeafCommand( aCommand, SYNCML_ELEMENT_REPLACE );
    }
    else if (aName == SYNCML_ELEMENT_DELETE) {
        aCommand.commandType = CommandParams::COMMAND_DELETE;
        readLeafCommand( aCommand, SYNCML_ELEMENT_DELETE );
    }
    else if (aName == SYNCML_ELEMENT_GET) {
        aCommand.commandType = CommandParams::COMMAND_GET;
        readLeafCommand( aCommand, SYNCML_ELEMENT_GET );
    }
    else if (aName == SYNCML_ELEMENT_COPY) {
        aCommand.commandType = CommandParams::COMMAND_COPY;
        readLeafCommand( aCommand, SYNCML_ELEMENT_COPY );
    }
    else if (aName == SYNCML_ELEMENT_MOVE) {
        aCommand.commandType = CommandParams::COMMAND_MOVE;
        readLeafCommand( aCommand, SYNCML_ELEMENT_MOVE );
    }
    else if (aName == SYNCML_ELEMENT_EXEC ) {
        aCommand.commandType = CommandParams::COMMAND_EXEC;
        readLeafCommand( aCommand, SYNCML_ELEMENT_EXEC );
    }
    else if (aName == SYNCML_ELEMENT_ATOMIC) {
        aCommand.commandType = CommandParams::COMMAND_ATOMIC;
        readContainerCommand( aCommand, SYNCML_ELEMENT_ATOMIC );
    }
    else if (aName == SYNCML_ELEMENT_SEQUENCE) {
        aCommand.commandType = CommandParams::COMMAND_SEQUENCE;
        readContainerCommand( aCommand, SYNCML_ELEMENT_SEQUENCE );
    }
    else {
        found = false;
    }

    return found;
}

void SyncMLMessageParser::readLeafCommand( CommandParams& aParams, const QString& aCommand )
{
    FUNCTION_CALL_TRACE;

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == aCommand ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if( name == SYNCML_ELEMENT_CMDID ) {
                aParams.cmdId = readInt();
            }
            else if( name == SYNCML_ELEMENT_NORESP ) {
                aParams.noResp = true;
            }
            else if (name == SYNCML_ELEMENT_DATA) {
                aParams.data = readString();
            }
            else if( name == SYNCML_ELEMENT_CORRELATOR ) {
                aParams.correlator = readString();
            }
            else if( name == SYNCML_ELEMENT_META ) {
                readMeta( aParams.meta );
            }
            else if( name == SYNCML_ELEMENT_ITEM ) {
                ItemParams item;
                readItem( item );
                aParams.items.append( item );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in COMMAND:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readContainerCommand( CommandParams& aParams, const QString& aCommand )
{
    FUNCTION_CALL_TRACE;

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == aCommand ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if( name == SYNCML_ELEMENT_CMDID ) {
                aParams.cmdId = readInt();
            }
            else if( name == SYNCML_ELEMENT_NORESP ) {
                aParams.noResp = true;
            }
            else if( name == SYNCML_ELEMENT_META ) {
                readMeta( aParams.meta );
            }
            else {
                CommandParams command;

                if( readCommand( name, command ) )
                {
                    aParams.subCommands.append(command);
                }
                else {
                    LOG_WARNING("UNKNOWN TOKEN TYPE in COMMAND:NOT HANDLED BY PARSER" << name);
                }

            }

        }

    }

}

void SyncMLMessageParser::readCred( CredParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_CRED ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if( name == SYNCML_ELEMENT_META ) {
                readMeta( aParams.meta );
            }
            else if( name == SYNCML_ELEMENT_DATA ) {
                aParams.data = readString();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in CRED:NOT HANDLED BY PARSER" << name);
            }
        }
    }

}

void SyncMLMessageParser::readMeta( MetaParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_META ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if (name == SYNCML_ELEMENT_FORMAT) {
                aParams.format = readString();
            }
            else if (name == SYNCML_ELEMENT_SIZE) {
                aParams.size = readInt();
            }
            else if (name == SYNCML_ELEMENT_TYPE) {
                aParams.type = readString();
            }
            else if (name == SYNCML_ELEMENT_ANCHOR) {
                readAnchor( aParams.anchor );
            }
            else if (name == SYNCML_ELEMENT_VERSION) {
                aParams.version = readString();
            }
            else if (name == SYNCML_ELEMENT_NEXTNONCE) {
                aParams.nextNonce = readString();
            }
            else if (name == SYNCML_ELEMENT_MAXMSGSIZE) {
                aParams.maxMsgSize = readInt();
            }
            else if (name == SYNCML_ELEMENT_MAXOBJSIZE) {
                aParams.maxObjSize = readInt();
            }
            else if (name == SYNCML_ELEMENT_EMI) {
                aParams.EMI.append( readString() );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in META:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readAnchor( AnchorParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_ANCHOR ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if (name == SYNCML_ELEMENT_NEXT) {
                aParams.next = readString();
            }
            else if (name == SYNCML_ELEMENT_LAST) {
                aParams.last = readString();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in ANCHOR:NOT HANDLED BY PARSER" << name);
            }

        }
    }

}

void SyncMLMessageParser::readDevInfItem( DevInfItemParams& aParams )
{
    FUNCTION_CALL_TRACE;


    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_ITEM )
        {
            break;
        }

        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_SOURCE )
            {
                aParams.source = readURI();
            }
            else if( name == SYNCML_ELEMENT_DEVINF )
            {
                readDevInf( aParams );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }
        }

    }

}

void SyncMLMessageParser::readDevInf( DevInfItemParams& aParams )
{
    FUNCTION_CALL_TRACE;

    QString dtd;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_DEVINF )
        {
            break;
        }

        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_VERDTD )
            {
                dtd = readString();

                if( dtd != SYNCML_DTD_VERSION_1_1 &&
                    dtd != SYNCML_DTD_VERSION_1_2 )
                {
                    LOG_CRITICAL( "Unrecognized DevInf verDTD:" << dtd );
                    iError = PARSER_ERROR_INVALID_DATA;
                }

            }
            else if( name == SYNCML_ELEMENT_MAN )
            {
                aParams.devInfo.deviceInfo().setManufacturer( readString() );
            }
            else if( name == SYNCML_ELEMENT_MOD )
            {
                aParams.devInfo.deviceInfo().setModel( readString() );
            }
            else if( name == SYNCML_ELEMENT_OEM )
            {
                aParams.devInfo.deviceInfo().setOEM( readString() );
            }
            else if( name == SYNCML_ELEMENT_FWVERSION )
            {
                aParams.devInfo.deviceInfo().setFirmwareVersion( readString() );
            }
            else if( name == SYNCML_ELEMENT_SWVERSION )
            {
                aParams.devInfo.deviceInfo().setSoftwareVersion( readString() );
            }
            else if( name == SYNCML_ELEMENT_HWVERSION )
            {
                aParams.devInfo.deviceInfo().setHardwareVersion( readString() );
            }
            else if( name == SYNCML_ELEMENT_DEVID )
            {
                aParams.devInfo.deviceInfo().setDeviceID( readString() );
            }
            else if( name == SYNCML_ELEMENT_DEVTYPE )
            {
                aParams.devInfo.deviceInfo().setDeviceType( readString() );
            }
            else if( name == SYNCML_ELEMENT_UTC )
            {
                aParams.devInfo.setSupportsUTC( true );
            }
            else if( name == SYNCML_ELEMENT_SUPPORTLARGEOBJS )
            {
                aParams.devInfo.setSupportsLargeObjs( true );
            }
            else if( name == SYNCML_ELEMENT_SUPPORTNUMBEROFCHANGES )
            {
                aParams.devInfo.setSupportsNumberOfChanges( true );
            }
            else if( name == SYNCML_ELEMENT_DATASTORE )
            {
                Datastore newDatastore;
                readDataStore( newDatastore, dtd );
                aParams.devInfo.datastores().append( newDatastore );
            }
            else if( name == SYNCML_ELEMENT_CTCAP )
            {
                // CTCap element resides under DevInf only 1.1, in 1.2 it's under
                // DataStore
                if( dtd == SYNCML_DTD_VERSION_1_1 )
                {
                    readCTCap11( aParams.devInfo.datastores() );
                }
                else
                {
                    LOG_CRITICAL( SYNCML_ELEMENT_CTCAP << "under DevInf allowed only for DS 1.1" );
                    iError = PARSER_ERROR_INVALID_DATA;
                }
            }
            else{
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }
        }
    }
}

void SyncMLMessageParser::readDataStore( Datastore& aDatastore, const QString& aDTD )
{
    FUNCTION_CALL_TRACE;


    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_DATASTORE )
        {
            break;
        }
        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_SOURCEREF )
            {
                QString URI = readString();
                LOG_DEBUG( "URI of the new datastore instance:" << URI );
                aDatastore.setSourceURI( URI );
            }
            else if( name == SYNCML_ELEMENT_RX_PREF )
            {
                ContentFormat rxPref;
                readContentFormat( rxPref, SYNCML_ELEMENT_RX_PREF );
                aDatastore.formatInfo().setPreferredRx( rxPref );
            }
            else if( name == SYNCML_ELEMENT_RX )
            {
                ContentFormat rx;
                readContentFormat( rx, SYNCML_ELEMENT_RX );
                aDatastore.formatInfo().rx().append( rx );
            }
            else if( name == SYNCML_ELEMENT_TX_PREF )
            {
                ContentFormat txPref;
                readContentFormat( txPref, SYNCML_ELEMENT_TX_PREF );
                aDatastore.formatInfo().setPreferredTx( txPref );
            }
            else if( name == SYNCML_ELEMENT_TX )
            {
                ContentFormat tx;
                readContentFormat( tx, SYNCML_ELEMENT_TX );
                aDatastore.formatInfo().tx().append( tx );
            }
            else if( name == SYNCML_ELEMENT_SYNCCAP )
            {
                readSyncCaps( aDatastore );
            }
            else if( name == SYNCML_ELEMENT_CTCAP )
            {
                readCTCap12( aDatastore );
            }
            else if( name == SYNCML_ELEMENT_SUPPORTHIERARCHICALSYNC )
            {
                if( aDTD == SYNCML_DTD_VERSION_1_2 )
                {
                    aDatastore.setSupportsHierarchicalSync( true );
                }
                else
                {
                    LOG_CRITICAL( SYNCML_ELEMENT_SUPPORTHIERARCHICALSYNC << "under DevInf allowed only for DS 1.2" );
                    iError = PARSER_ERROR_INVALID_DATA;
                }
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }
        }

    }

}

void SyncMLMessageParser::readContentFormat( ContentFormat& aFormat, const QString& aEndElement )
{
    FUNCTION_CALL_TRACE;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == aEndElement )
        {
            break;
        }
        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_CTTYPE )
            {
                aFormat.iType = readString();
            }
            else if( name == SYNCML_ELEMENT_VERCT )
            {
                aFormat.iVersion = readString();
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readSyncCaps( Datastore& aDatastore )
{
    FUNCTION_CALL_TRACE;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_SYNCCAP )
        {
            break;
        }
        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_SYNCTYPE )
            {
                int syncType = readInt();
                aDatastore.syncCaps().append( static_cast<SyncTypes>( syncType ) );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }
        }
    }
}

void SyncMLMessageParser::readCTCap11( QList<Datastore>& aDataStores )
{
    FUNCTION_CALL_TRACE;

    QList<CTCap> caps;

    CTCap* currentCap = 0;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_CTCAP )
        {
            break;
        }
        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_CTTYPE )
            {
                QString type = readString();

                currentCap = 0;

                for( int i = 0; i < caps.count(); ++i )
                {
                    if( caps[i].getFormat().iType == type )
                    {
                        currentCap = &caps[i];
                        break;
                    }
                }

                if( !currentCap )
                {
                    LOG_DEBUG( "Creating new CTCap instance with type" << type );
                    CTCap newCap;
                    ContentFormat format;
                    format.iType = type;
                    newCap.setFormat(format);
                    caps.append(newCap);
                    currentCap = &caps.last();
                }

            }
            else
            {
                if( !currentCap )
                {
                    LOG_CRITICAL( "Cannot process" << name <<"as no" << SYNCML_ELEMENT_CTTYPE << "was found!" );
                    iError = PARSER_ERROR_INVALID_DATA;
                }
                else if( name == SYNCML_ELEMENT_PROPNAME )
                {
                    CTCapProperty newProp;
                    newProp.iName = readString();
                    currentCap->properties().append( newProp );
                }
                else if( name == SYNCML_ELEMENT_VALENUM )
                {
                    QString val = readString();
                    currentCap->properties().last().iValues.append( val );
                }
                else if( name == SYNCML_ELEMENT_DATATYPE )
                {
                    QString type = readString();
                    currentCap->properties().last().iType = type;
                }
                else if( name == SYNCML_ELEMENT_SIZE )
                {
                    int size = readInt();
                    currentCap->properties().last().iSize = size;
                }
                else if( name == SYNCML_ELEMENT_DISPLAYNAME )
                {
                    QString displayName = readString();
                    currentCap->properties().last().iDisplayName = displayName;
                }
                else if( name == SYNCML_ELEMENT_PARAMNAME )
                {
                    // In SyncML 1.1, parameter names (for example TYPE) are not conveyed, instead
                    // parameter values (for example WORK). So we must create an anonymous parameter
                    // that includes all the allowed parameter values
                    QString paramName = readString();
                    if( currentCap->properties().last().iParameters.isEmpty() )
                    {
                        CTCapParameter newParam;
                        newParam.iValues.append( paramName );
                        currentCap->properties().last().iParameters.append( newParam );
                    }
                    else
                    {
                        currentCap->properties().last().iParameters.last().iValues.append( paramName );
                    }
                            \

                }
                else
                {
                    LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
                }

            }
        }

    }

    // Sweep through declared datastores, check the content formats they are interested, and add
    // parsed CTCaps if a datastore is interested
    for( int i = 0; i < caps.count(); ++i )
    {

        for( int a = 0; a < aDataStores.count(); ++a )
        {
            QList<ContentFormat> interestedFormats;
            const StorageContentFormatInfo& formatInfo = aDataStores[a].formatInfo();
            interestedFormats.append( formatInfo.getPreferredRx() );
            interestedFormats.append( formatInfo.rx());
            interestedFormats.append( formatInfo.getPreferredTx() );
            interestedFormats.append( formatInfo.tx());

            for( int b = 0; b < interestedFormats.count(); ++b )
            {
                if( interestedFormats[b].iType == caps[i].getFormat().iType )
                {
                    LOG_DEBUG( "Datastore" << aDataStores[a].getSourceURI() << "is interested in CTType" << caps[i].getFormat().iType );
                    aDataStores[a].ctCaps().append( caps[i] );
                    break;
                }

            }

        }

    }

}

void SyncMLMessageParser::readCTCap12( Datastore& aDatastore )
{
    FUNCTION_CALL_TRACE;

    CTCap cap;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_CTCAP )
        {
            break;
        }

        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_CTTYPE )
            {
                ContentFormat format = cap.getFormat();
                format.iType = readString();
                cap.setFormat( format );

            }
            else if( name == SYNCML_ELEMENT_VERCT )
            {
                ContentFormat format = cap.getFormat();
                format.iVersion = readString();
                cap.setFormat( format );
            }
            else if( name == SYNCML_ELEMENT_PROPERTY )
            {
                CTCapProperty newProperty;
                readCTCap12Property( newProperty );
                cap.properties().append( newProperty );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }
        }

    }

    aDatastore.ctCaps().append( cap );
}

void SyncMLMessageParser::readCTCap12Property( CTCapProperty& aProperty )
{
    FUNCTION_CALL_TRACE;

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_PROPERTY )
        {
            break;
        }
        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_PROPNAME )
            {
                aProperty.iName = readString();
            }
            else if( name == SYNCML_ELEMENT_DATATYPE )
            {
                aProperty.iType = readString();
            }
            else if( name == SYNCML_ELEMENT_MAXOCCUR )
            {
                aProperty.iMaxOccur = readInt();
            }
            else if( name == SYNCML_ELEMENT_MAXSIZE )
            {
                aProperty.iSize = readInt();
            }
            else if( name == SYNCML_ELEMENT_NOTRUNCATE )
            {
                aProperty.iNoTruncate = true;
            }
            else if( name == SYNCML_ELEMENT_DISPLAYNAME )
            {
                aProperty.iDisplayName = readString();
            }
            else if( name == SYNCML_ELEMENT_VALENUM )
            {
                aProperty.iValues.append( readString() );
            }
            else if( name == SYNCML_ELEMENT_PROPPARAM )
            {
                CTCapParameter newParam;
                readCTCap12Parameter( newParam );
                aProperty.iParameters.append( newParam );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readCTCap12Parameter( CTCapParameter& aParameter )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() )
    {
        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_PROPPARAM )
        {
            break;
        }

        if( iReader.isStartElement() )
        {
            if( name == SYNCML_ELEMENT_PARAMNAME )
            {
                aParameter.iName = readString();
            }
            else if( name == SYNCML_ELEMENT_DATATYPE )
            {
                aParameter.iType = readString();
            }
            else if( name == SYNCML_ELEMENT_DISPLAYNAME )
            {
                aParameter.iDisplayName = readString();
            }
            else if( name == SYNCML_ELEMENT_VALENUM )
            {
                aParameter.iValues.append( readString() );
            }
            else
            {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINF:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readItem( ItemParams& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_ITEM ) {
            break;
        }

        if( iReader.isStartElement() ) {
            if (name == SYNCML_ELEMENT_META) {
                readMeta( aParams.meta );
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                aParams.target = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                aParams.source = readURI();
            }
            else if( name == SYNCML_ELEMENT_TARGETPARENT ) {
                aParams.targetParent = readURI();
            }
            else if( name == SYNCML_ELEMENT_SOURCEPARENT ) {
                aParams.sourceParent = readURI();
            }
            else if (name == SYNCML_ELEMENT_DATA) {
                aParams.data = readString();
            }
            else if (name == SYNCML_ELEMENT_MOREDATA) {
                aParams.moreData = true;
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in ITEM:NOT HANDLED BY PARSER" << name);
            }
        }
    }

}

QString SyncMLMessageParser::readURI()
{
    FUNCTION_CALL_TRACE

    QString uri;

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() &&
            ( name == SYNCML_ELEMENT_TARGET ||
              name == SYNCML_ELEMENT_SOURCE ||
              name == SYNCML_ELEMENT_TARGETPARENT ||
              name == SYNCML_ELEMENT_SOURCEPARENT ) ){
            break;
        }

        if( iReader.isStartElement() && name == SYNCML_ELEMENT_LOCURI ) {
            uri = readString();
            continue;
        }

    }

    return uri;
}


int SyncMLMessageParser::readInt()
{
    iReader.readNext();

    return iReader.text().toString().toInt();
}

QString SyncMLMessageParser::readString()
{

    QString string;

    while( shouldContinue() ) {

        iReader.readNext();

        if( iReader.isCharacters() ) {
            string.append( iReader.text().toString() );

        }
        else if( iReader.isEndElement() ) {
            break;
        }
    }

    return string;
}

bool SyncMLMessageParser::shouldContinue() const
{
    if( iError == PARSER_ERROR_LAST && !iReader.atEnd() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SyncMLMessageParser::initMaps() {

    FUNCTION_CALL_TRACE

    // for debugging alert codes
    iAlertCodeMap[100] = "Alert for DISPLAY";
    // Alert codes used at the sync init
    iAlertCodeMap[200] = "Alert for TWO_WAY_SYNC";
    iAlertCodeMap[201] = "Alert for SLOW_SYNC";
    iAlertCodeMap[202] = "Alert for ONE_WAY_FROM_CLIENT_SYNC";
    iAlertCodeMap[203] = "Alert for REFRESH_FROM_CLIENT_SYNC";
    iAlertCodeMap[204] = "Alert for ONE_WAY_FROM_SERVER_SYNC";
    iAlertCodeMap[205] = "Alert for REFRESH_FROM_SERVER_SYNC";

    // Alert codes used by the server when alerting the sync
    iAlertCodeMap[206] = "Alert for TWO_WAY_BY_SERVER";
    iAlertCodeMap[207] = "Alert for ONE_WAY_FROM_CLIENT_BY_SERVER";
    iAlertCodeMap[208] = "Alert for REFRESH_FROM_CLIENT_BY_SERVER";
    iAlertCodeMap[209] = "Alert for ONE_WAY_FROM_SERVER_BY_SERVER";
    iAlertCodeMap[210] = "Alert for REFRESH_FROM_SERVER_BY_SERVER";

    iAlertCodeMap[221] = "Alert for RESULT_ALERT";
    iAlertCodeMap[222] = "Alert for NEXT_MESSAGE";
    iAlertCodeMap[223] = "Alert for NO_END_OF_DATA";
    iAlertCodeMap[224] = "Alert for ALERT_SUSPEND";
    iAlertCodeMap[225] = "Alert for ALERT_RESUME";
    iAlertCodeMap[206] = "Alert for TWO_WAY_BY_SERVER";

    iStatusCodeMap[101] = "Status:IN_PROGRESS";

    // Successful 2xx
    iStatusCodeMap[200] = "Status:SUCCESS";
    iStatusCodeMap[201] = "Status:ITEM_ADDED";
    iStatusCodeMap[202] = "Status:ACCEPTED_FOR_PROCESSING";
    iStatusCodeMap[203] = "Status:NONAUTHORITATIVE_RESPONSE";
    iStatusCodeMap[204] = "Status:NO_CONTENT";
    iStatusCodeMap[205] = "Status:RESET_CONTENT";
    iStatusCodeMap[206] = "Status:PARTIAL_CONTENT";
    iStatusCodeMap[207] = "Status:RESOLVED_WITH_MERGE";

    iStatusCodeMap[208] = "Status:RESOLVED_CLIENT_WINNING";
    iStatusCodeMap[209] = "Status:RESOLVED_WITH_DUPLICATE";
    iStatusCodeMap[210] = "Status:DELETE_WITHOUT_ARCHIVE";
    iStatusCodeMap[211] = "Status:ITEM_NOT_DELETED";
    iStatusCodeMap[212] = "Status:AUTH_ACCEPTED";
    iStatusCodeMap[213] = "Status:CHUNKED_ITEM_ACCEPTED";
    iStatusCodeMap[214] = "Status:CANCELLED";
    iStatusCodeMap[215] = "Status:NOT_EXECUTED";

    iStatusCodeMap[216] = "Status:ATOMIC_ROLLBACK_OK";

    // Redirection 3xx
    iStatusCodeMap[300] = "Status:MULTIPLE_CHOICES";
    iStatusCodeMap[301] = "Status:MOVED_PERMANENTLY";
    iStatusCodeMap[302] = "Status:FOUND";
    iStatusCodeMap[303] = "Status:SEE_OTHER";
    iStatusCodeMap[304] = "Status:NOT_MODIFIED";
    iStatusCodeMap[305] = "Status:USE_PROXY";

    // Originator exceptions 4xx
    iStatusCodeMap[400] = "Status:BAD_REQUEST";
    iStatusCodeMap[401] = "Status:INVALID_CRED";
    iStatusCodeMap[402] = "Status:PAYMENT_NEEDED";
    iStatusCodeMap[403] = "Status:FORBIDDEN";
    iStatusCodeMap[404] = "Status:NOT_FOUND";
    iStatusCodeMap[405] = "Status:COMMAND_NOT_ALLOWED";

    iStatusCodeMap[406] = "Status:NOT_SUPPORTED";
    iStatusCodeMap[407] = "Status:MISSING_CRED";
    iStatusCodeMap[408] = "Status:REQUEST_TIMEOUT";
    iStatusCodeMap[409] = "Status:CONFLICT";
    iStatusCodeMap[410] = "Status:GONE";
    iStatusCodeMap[411] = "Status:SIZE_REQUIRED";

    iStatusCodeMap[412] = "Status:INCOMPLETE_COMMAND";
    iStatusCodeMap[413] = "Status:REQUEST_ENTITY_TOO_LARGE";
    iStatusCodeMap[414] = "Status:URI_TOO_LONG";
    iStatusCodeMap[415] = "Status:UNSUPPORTED_FORMAT";
    iStatusCodeMap[416] = "Status:REQUEST_SIZE_TOO_BIG";
    iStatusCodeMap[417] = "Status:RETRY_LATER";

    iStatusCodeMap[418] = "Status:ALREADY_EXISTS";
    iStatusCodeMap[419] = "Status:RESOLVED_WITH_SERVER_DATA";
    iStatusCodeMap[420] = "Status:DEVICE_FULL";
    iStatusCodeMap[421] = "Status:UNKNOWN_SEARCH_GRAMMAR";
    iStatusCodeMap[422] = "Status:BAD_CGI_SCRIPT";
    iStatusCodeMap[423] = "Status:SOFTDELETE_CONFLICT";

    iStatusCodeMap[424] = "Status:SIZE_MISMATCH";
    iStatusCodeMap[425] = "Status:PERMISSION_DENIED";
    iStatusCodeMap[426] = "Status:PARTIAL_ITEM_NOT_ACCEPTED";
    iStatusCodeMap[427] = "Status:ITEM_NOT_EMPTY";
    iStatusCodeMap[428] = "Status:MOVE_FAILED";

    // Recipient exception 5xx
    iStatusCodeMap[500] = "Status:COMMAND_FAILED";
    iStatusCodeMap[501] = "Status:NOT_IMPLEMENTED";
    iStatusCodeMap[502] = "Status:BAD_GATEWAY";
    iStatusCodeMap[503] = "Status:SERVICE_UNAVAILABLE";
    iStatusCodeMap[504] = "Status:GATEWAY_TIMEOUT";
    iStatusCodeMap[505] = "Status:UNSUPPORTED_DTD_VERSION";

    iStatusCodeMap[506] = "Status:PROCESSING_ERROR";
    iStatusCodeMap[507] = "Status:ATOMIC_FAILED";
    iStatusCodeMap[508] = "Status:REFRESH_REQUIRED";

    iStatusCodeMap[510] = "Status:DATA_STORE_FAILURE";
    iStatusCodeMap[511] = "Status:SERVER_FAILURE";
    iStatusCodeMap[512] = "Status:SYNC_FAILED";
    iStatusCodeMap[513] = "Status:UNSUPPORTED_PROTOCOL_VERSION";
    iStatusCodeMap[514] = "Status:OPERATION_CANCELLED";
    iStatusCodeMap[516] = "Status:ATOMIC_ROLLBACK_FAILED";
    iStatusCodeMap[517] = "Status:ATOMIC_RESPONSE_TOO_LARGE";

}
