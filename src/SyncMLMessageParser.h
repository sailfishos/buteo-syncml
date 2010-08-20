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

#ifndef SYNCMLMESSAGEPARSER_H
#define SYNCMLMESSAGEPARSER_H

#include <QXmlStreamReader>
#include <QHash>

#include "internals.h"


namespace DataSync {

/*! \brief Enumeration describing possible parser errors
 *
 */
enum ParserError {
    PARSER_ERROR_UNEXPECTED_DATA,   /*!< Parser encountered unexpected data*/
    PARSER_ERROR_INCOMPLETE_DATA,   /*!< Parser encountered incomplete data*/
    PARSER_ERROR_INVALID_DATA,      /*!< Parser encountered invalid data*/
    PARSER_ERROR_INVALID_CHARS,     /*!< Parser encountered invalid XML characters in data */
    PARSER_ERROR_LAST               /*!< Placeholder for last parser error*/
};

/*! \brief Constructs SyncML Commands into usable data structures.
 *
 * This Class reads the XML data from the incoming stream and builds
 * individual structs for the SyncML commands received in the SyncML
 * Message.
 */
class SyncMLMessageParser : public QObject
{
    Q_OBJECT

public:
	/*! \brief Constructor
	 */
    SyncMLMessageParser();

    /*! \brief Destructor
     */
    virtual ~SyncMLMessageParser();

public:

    /*! \brief Retrieves the fragments of the last parsing operation
     *
     * Ownership of the fragments is transferred.
     * @return
     */
    QList<DataSync::Fragment*> takeFragments();

public slots:

	/*! \brief Parse incoming data
	 *
	 * @param aDevice QIODevice from which to retrieve data
	 * @param aIsNewPacket To indicate if the packet is a newly received or a
     * purged one
	 */
    void parseResponse( QIODevice *aDevice, bool aIsNewPacket );

signals:

    /*! \brief Emitted when parsing of a message has been completed
     *
     * @param aLastMessageInPackage True if the parsed message contained
     *        Final element
     */
    void parsingComplete( bool aLastMessageInPackage );

    /*! \brief Emitted when error occurred during parsing of a message
     *
     * @param aEvent Occurred error
     */
    void parsingError( DataSync::ParserError aEvent );

private:

    /**
     * \brief trigger function for the parser
     */
    void startParsing();

	/**
     * \brief reads Header and emits cmdSyncHeaderReceived signal
     */
	void readHeader();

	/**
     * \brief reads Body inside a SyncML message
     */
	void readBody();

	/**
     * \brief reads Status element and emits cmdStatusReceived signal
     */
	void readStatus();

	/**
     * \brief reads Sync element and emits cmdSyncReceived signal
     */
	void readSync();

	/**
     * \brief reads Alert Element and emits cmdAlertReceived signal
     */
	void readAlert();

	/**
     * \brief reads Results and emits cmdResultsReceived signal
     */
	void readResults();

    /**
     * \brief reads Map and emits cmdMapReceived signal
     */
    void readMap();

    void readMapItem( MapItem& aParams );

	void readDevInfData();

	void readDevInfItem();

	void readSyncActionData( const QString& aAction, SyncActionData& aParams );

    void readChal( ChalParams& aParams );

	void readCred( CredParams& aParams );

	void readMeta( MetaParams& aParams );

	void readItem( ItemParams& aParams );

	void readAnchor( AnchorParams& aParams );

    QString readURI();

	int readInt();

	QString readString();

	bool shouldContinue() const;

	void initMaps();

    QXmlStreamReader            iReader;
    QList<DataSync::Fragment*>  iFragments;
    bool                        iLastMessageInPackage;
    ParserError                 iError;
    QHash<int, QString>         iAlertCodeMap;
    QHash<int, QString>         iStatusCodeMap;
    bool                        iSyncHdrFound;
    bool                        iSyncBodyFound;
    bool                        iIsNewPacket;
};
}

Q_DECLARE_METATYPE(DataSync::ParserError);

#endif // SYNCMLMESSAGEPARSER_H

