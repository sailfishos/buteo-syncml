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
            } else if (name == SYNCML_ELEMENT_ALERT) {
                readAlert();
            } else if (name == SYNCML_ELEMENT_GET) {
                SyncActionData *actionData = new SyncActionData();
                readSyncActionData(name.toString(), *actionData );
                actionData->action = SYNCML_GET;
                iFragments.append(actionData);
            } else if (name == SYNCML_ELEMENT_PUT) {
                SyncActionData *actionData = new SyncActionData();
                readSyncActionData(name.toString(), *actionData );
                actionData->action = SYNCML_PUT;
                iFragments.append(actionData);
            } else if (name == SYNCML_ELEMENT_RESULTS) {
                readResults();
            } else if (name == SYNCML_ELEMENT_MAP ) {
                readMap();
            }else if (name == SYNCML_ELEMENT_FINAL) {
                iLastMessageInPackage = true;
            } else {
                LOG_WARNING("UNKNOWN  TOKEN TYPE in BODY:NOT HANDLED BY PARSER" << name );
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
            else if (name == SYNCML_ELEMENT_NORESP) {
                header->noResp = true;
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
            else if (name == SYNCML_ELEMENT_MAXMSGSIZE) {
                header->maxMsgSize = readInt();
            }
            else if (name == SYNCML_ELEMENT_MAXOBJSIZE) {
                header->maxObjSize = readInt();
            }
            else if (name == SYNCML_ELEMENT_EMI) {
                header->EMI.append( readString() );
            }
            else if (name == SYNCML_ELEMENT_RESPURI) {
                header->respURI = readString();
            }
            else if (name == SYNCML_ELEMENT_CRED) {
                readCred( header->cred );
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
                status->cmdID = readInt();
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
                status->itemList.append( item );
            }
            else if (name == SYNCML_ELEMENT_CHAL) {
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
                sync->cmdID = readInt();
            }
            else if (name == SYNCML_ELEMENT_NORESP) {
                sync->noResp = true;
            }
            else if (name == SYNCML_ELEMENT_META) {
                readMeta( sync->meta );
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                sync->targetDatabase = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                sync->sourceDatabase = readURI();
            }
            else if (name == SYNCML_ELEMENT_ADD) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_ADD;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_ATOMIC) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_ATOMIC;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_COPY) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_COPY;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_DELETE) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_DELETE;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_MOVE) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_MOVE;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_SEQUENCE) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_SEQUENCE;
                sync->actionList << actionData;
            }
            else if (name == SYNCML_ELEMENT_REPLACE) {
                SyncActionData actionData;
                readSyncActionData( name.toString(), actionData );
                actionData.action = SYNCML_REPLACE;
                sync->actionList << actionData;
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in SYNC:NOT HANDLED BY PARSER" << name);
            }

        }

    }

    iFragments.append(sync);

}

void SyncMLMessageParser::readAlert()
{
    FUNCTION_CALL_TRACE

    AlertParams *alert = new AlertParams();

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_ALERT ) {
            break;
        }
        if( iReader.isStartElement() ) {

            if (name == SYNCML_ELEMENT_CMDID) {
                alert->cmdID = readInt();
            }
            else if (name == SYNCML_ELEMENT_NORESP) {
                alert->noResp = true;
            }
            else if (name == SYNCML_ELEMENT_DATA) {
                alert->data = (AlertType)readInt();
                LOG_DEBUG(iAlertCodeMap[alert->data] << ":" << alert->data);
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                alert->targetDatabase = readURI();
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                alert->sourceDatabase = readURI();
            }
            else if (name == SYNCML_ELEMENT_LAST) {
                alert->lastAnchor = readString();
            }
            else if (name == SYNCML_ELEMENT_NEXT) {
                alert->nextAnchor = readString();
            }
            else if (name == SYNCML_ELEMENT_TYPE) {
                alert->type = readString();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in ALERT:NOT HANDLED BY PARSER" << name );
            }

        }

    }

    iFragments.append(alert);

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
                map->cmdID = readInt();
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
            else if (name == SYNCML_ELEMENT_CRED) {
                readCred( map->cred );
            }
            else if (name == SYNCML_ELEMENT_MAPITEM) {
                MapItem item;
                readMapItem( item );
                map->mapItemList.append( item );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in MAP:NOT HANDLED BY PARSER" << name );
            }

        }

    }

    iFragments.append(map);

}

void SyncMLMessageParser::readMapItem( MapItem& aParams )
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
                results->cmdID = readInt();
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
                readDevInfData();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in RESULTS:NOT HANDLED BY PARSER" << name);
            }

        }

    }

    iFragments.append(results);

}

void SyncMLMessageParser::readDevInfData()
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
                MetaParams meta;
                readMeta( meta );
                RemoteDeviceInfo::instance()->populateMeta( meta );
            }
            else if (name == SYNCML_ELEMENT_TARGET) {
                RemoteDeviceInfo::instance()->populateTargetURI( readURI() );
            }
            else if (name == SYNCML_ELEMENT_SOURCE) {
                RemoteDeviceInfo::instance()->populateSourceURI( readURI() );
            }
            else if (name == SYNCML_ELEMENT_DATA) {
                readDevInfItem();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in ITEM:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}


void SyncMLMessageParser::readDevInfItem()
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == SYNCML_ELEMENT_DATA ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if( name == SYNCML_ELEMENT_SUPPORTLARGEOBJS ) {
                RemoteDeviceInfo::instance()->setLargeObjectSupported();
            }
            else if( name == SYNCML_ELEMENT_SYNCTYPE ) {
                RemoteDeviceInfo::instance()->populateSupportedSyncType( static_cast<DataSync::SyncTypes>(readInt()) );
            }
            else if( name == SYNCML_ELEMENT_MAN ) {
                RemoteDeviceInfo::instance()->populateManufacturer( readString() );
            }
            else if( name == SYNCML_ELEMENT_MOD ) {
                RemoteDeviceInfo::instance()->populateModel( readString() );
            }
            else if( name == SYNCML_ELEMENT_SWVERSION ) {
                RemoteDeviceInfo::instance()->populateSwVersion( readString() );
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in DEVINFDATA:NOT HANDLED BY PARSER" << name);
            }

        }

    }

}

void SyncMLMessageParser::readSyncActionData( const QString& aAction, SyncActionData& aParams )
{
    FUNCTION_CALL_TRACE

    while( shouldContinue() ) {

        iReader.readNext();

        QStringRef name = iReader.name();

        if( iReader.isEndElement() && name == aAction ) {
            break;
        }

        if( iReader.isStartElement() ) {

            if( name == SYNCML_ELEMENT_CMDID ) {
                aParams.cmdID = readInt();
            }
            else if( name == SYNCML_ELEMENT_NORESP ) {
                aParams.noResp = true;
            }
            else if( name == SYNCML_ELEMENT_LANG ) {
                aParams.lang = readString();
            }
            else if( name == SYNCML_ELEMENT_CRED ) {
                readCred( aParams.cred );
            }
            else if( name == SYNCML_ELEMENT_META ) {
                readMeta( aParams.meta );
            }
            else if( name == SYNCML_ELEMENT_ITEM ) {
                ItemParams item;
                readItem( item );
                aParams.items.append( item );
            }
            else if( name == SYNCML_ELEMENT_NUMOFCHANGES ) {
                aParams.numberOfChanges = readInt();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in SYNC ACTIONDATA:NOT HANDLED BY PARSER" << name);
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
                aParams.Version = readString();
            }
            else if (name == SYNCML_ELEMENT_NEXTNONCE) {
                aParams.nextNonce = readString();
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
                aParams.Next = readString();
            }
            else if (name == SYNCML_ELEMENT_LAST) {
                aParams.Last = readString();
            }
            else {
                LOG_WARNING("UNKNOWN TOKEN TYPE in ANCHOR:NOT HANDLED BY PARSER" << name);
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
                aParams.Data = readString();
            }
            else if (name == SYNCML_ELEMENT_MOREDATA) {
                aParams.moreData = true;
            }
	    else if( name == SYNCML_ELEMENT_SUPPORTLARGEOBJS ) {
                RemoteDeviceInfo::instance()->setLargeObjectSupported();
            }
            else if( name == SYNCML_ELEMENT_SYNCTYPE ) {
                RemoteDeviceInfo::instance()->populateSupportedSyncType( static_cast<DataSync::SyncTypes>(readInt()) );
            }
            else if( name == SYNCML_ELEMENT_MAN ) {
                RemoteDeviceInfo::instance()->populateManufacturer( readString() );
            }
            else if( name == SYNCML_ELEMENT_MOD ) {
                RemoteDeviceInfo::instance()->populateModel( readString() );
            }
            else if( name == SYNCML_ELEMENT_SWVERSION ) {
                RemoteDeviceInfo::instance()->populateSwVersion( readString() );
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
