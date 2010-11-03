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

#include "SyncAgentConfig.h"

#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlStreamReader>
#include <QFile>
#include <QStringList>

#include "SyncAgentConfigProperties.h"
#include "datatypes.h"
#include "Transport.h"

#include "LogMacros.h"

using namespace DataSync;

SyncAgentConfig::SyncAgentConfig()
 : iTransport( NULL ),
   iStorageProvider( NULL ),
   iDatabaseFilePath( "/etc/sync/syncml.db" ),
   iProtocolVersion( SYNCML_1_2 ),
   iAuthenticationType( AUTH_NONE )
{
    FUNCTION_CALL_TRACE
}


SyncAgentConfig::~SyncAgentConfig()
{
    FUNCTION_CALL_TRACE
}

bool SyncAgentConfig::fromFile( const QString& aFile, const QString& aSchemaFile )
{
    FUNCTION_CALL_TRACE

    // @todo: The QXmlSchema class has a memory leak (http://bugreports.qt.nokia.com/browse/QTBUG-8948).
    // As this function is called for every sync performed, this leads to large leaks.
    // Since the schema validation part is already commented out,
    // there is no need to create a QXmlSchema object here.

    /*
    QByteArray schemaData;

    if( !readFile( aSchemaFile, schemaData ) ) {
        LOG_CRITICAL( "Could not open schema file:" << aSchemaFile );
        return false;
    }

    QXmlSchema schema;
    schema.load(schemaData);

    if( !schema.isValid() ) {
        LOG_CRITICAL( "Schema file is invalid:" << aSchemaFile );
        return false;
    }
    */

    QByteArray xmlData;

    if( !readFile( aFile, xmlData ) ) {
        return false;
    }

    // @todo: Schema validation is currently disabled, because QXmlSchemaValidator
    //        does not seem to work properl atleast in arm7 environment. After adding
    //        new configuration parameters the time taken by validation was over 5 minutes!
    //        Checked the situation without new configuration parameters, and even then
    //        validation takes about a second, which is a _lot_ when actual parsing
    //        about 50ms. So, disabling this for now. Situation should be checked again
    //        in the future.

    /*
    QXmlSchemaValidator validator( schema );
    if( !validator.validate( xmlData ) ) {
        LOG_CRITICAL( "File did not pass validation:" << aFile );
        return false;
    }
    */

    return parseConfFile(xmlData);
}

void SyncAgentConfig::setTransport( Transport* aTransport )
{
    if( aTransport )
    {
        QList<QString> properties = iTransportProperties.keys();

        for( int i = 0; i < properties.count(); ++i )
        {
            aTransport->setProperty( properties[i],
                                     iTransportProperties.value( properties[i] ) );
        }

    }

    iTransport = aTransport;
}

Transport* SyncAgentConfig::getTransport() const
{
    return iTransport;
}

void SyncAgentConfig::setStorageProvider( StorageProvider* aProvider )
{
    iStorageProvider = aProvider;
}

StorageProvider* SyncAgentConfig::getStorageProvider() const
{
    return iStorageProvider;
}

void SyncAgentConfig::setDeviceInfo( const DeviceInfo &aDeviceInfo )
{
    iDeviceInfo = aDeviceInfo;
}

const DeviceInfo& SyncAgentConfig::getDeviceInfo() const
{
    return iDeviceInfo;
}

void SyncAgentConfig::setDatabaseFilePath( const QString& aPath )
{
    iDatabaseFilePath = aPath;
}

const QString& SyncAgentConfig::getDatabaseFilePath() const
{
    return iDatabaseFilePath;
}

void SyncAgentConfig::setLocalDeviceName ( const QString& aDeviceName )
{
    iLocalDeviceName = aDeviceName;
}

const QString& SyncAgentConfig::getLocalDeviceName() const
{
    return iLocalDeviceName;
}

void SyncAgentConfig::setAgentProperty( const QString& aProperty, const QString& aValue )
{
    iAgentProperties.insert( aProperty, aValue );
}

QString SyncAgentConfig::getAgentProperty( const QString& aProperty ) const
{
    return iAgentProperties.value( aProperty );
}

void SyncAgentConfig::setTransportProperty( const QString& aProperty, const QString& aValue )
{
    iTransportProperties.insert( aProperty, aValue );
}

QString SyncAgentConfig::getTransportProperty( const QString& aProperty ) const
{
    return iTransportProperties.value( aProperty );
}

void SyncAgentConfig::setSyncParams( const QString& aRemoteDeviceName,
                                     ProtocolVersion aVersion,
                                     const SyncMode& aSyncMode )
{
    iRemoteDeviceName = aRemoteDeviceName;
    iProtocolVersion = aVersion,
    iSyncMode = aSyncMode;
}

const QString& SyncAgentConfig::getRemoteDeviceName() const
{
    return iRemoteDeviceName;
}

const ProtocolVersion& SyncAgentConfig::getProtocolVersion() const
{
    return iProtocolVersion;

}

const SyncMode& SyncAgentConfig::getSyncMode() const
{
    return iSyncMode;
}

void SyncAgentConfig::setAuthParams( const AuthType& aAuthType,
                                     const QString& aUsername,
                                     const QString& aPassword,
                                     const QString& aNonce )
{
    iAuthenticationType = aAuthType;
    iUsername = aUsername;
    iPassword = aPassword;
    iNonce = aNonce;
}

const AuthType& SyncAgentConfig::getAuthType() const
{
    return iAuthenticationType;
}

const QString& SyncAgentConfig::getUsername() const
{
    return iUsername;
}

const QString& SyncAgentConfig::getPassword() const
{
    return iPassword;
}

const QString& SyncAgentConfig::getNonce() const
{
    return iNonce;
}

void SyncAgentConfig::addSyncTarget( const QString& aSourceDb, const QString& aTargetDb )
{
    FUNCTION_CALL_TRACE

    iTargets[aSourceDb] = aTargetDb;
    iTargetDbs.append(aSourceDb);
}

void SyncAgentConfig::addDisabledSyncTarget( const QString& aSourceDb)
{
    FUNCTION_CALL_TRACE
    iDTargetDbs.append(aSourceDb);
}

QList<QString> SyncAgentConfig::getDisabledSourceDbs() const
{
    FUNCTION_CALL_TRACE
    return iDTargetDbs;
}

QList<QString> SyncAgentConfig::getSourceDbs() const
{
    return iTargetDbs;
}

QString SyncAgentConfig::getTarget( const QString& aSourceDb ) const
{
    return iTargets.value( aSourceDb );
}

const QMap<QString, QString>* SyncAgentConfig::getTargets() const
{
    return &iTargets;
}

void SyncAgentConfig::setExtension( const QString& aName, const QVariant& aData )
{
    // ** Validate extensions data

    bool valid = false;

    if( aName == EMITAGSEXTENSION )
    {

        QStringList tags = aData.toStringList();

        if( tags.count() == 2 )
        {
            valid =  true;
        }
        else
        {
            LOG_WARNING( "EMI tags extension: missing required data!" );
        }

    }
    else if( aName == SYNCWITHOUTINITPHASEEXTENSION )
    {
        if( !aData.isValid() )
        {
            valid = true;
        }
        else
        {
            LOG_WARNING( "Sync without init phase extension: data should be invalid!" );
        }
    }
    else if( aName == SANMAPPINGSEXTENSION )
    {
        QStringList mappings = aData.toStringList();

        if( !mappings.isEmpty() && mappings.count() % 2 == 0 )
        {
            valid = true;
        }
        else
        {
            LOG_WARNING( "SAN mappings extension: missing required data!" );
        }

    }
    else
    {
        LOG_WARNING( "Unknown extension" << aName );
    }

    if( valid )
    {
        iExtensions.insert( aName, aData );
    }
}

bool SyncAgentConfig::extensionEnabled( const QString& aName ) const
{
    return iExtensions.contains( aName );
}

QVariant SyncAgentConfig::getExtensionData( const QString& aName ) const
{
    return iExtensions.value( aName );
}

void SyncAgentConfig::clearExtension( const QString& aName )
{
    iExtensions.remove( aName );
}

bool SyncAgentConfig::readFile( const QString& aFileName, QByteArray& aData ) const
{
    FUNCTION_CALL_TRACE

    QFile file( aFileName );

    if( file.open( QIODevice::ReadOnly ) ) {

        aData = file.readAll();
        file.close();
        return true;
    }
    else {
        LOG_DEBUG( "Could not read file:" << aFileName );
        return false;
    }
}

bool SyncAgentConfig::parseConfFile( const QByteArray& aData )
{
    FUNCTION_CALL_TRACE

    QXmlStreamReader reader( aData );

    const QString ROOTELEMENT( "meego-syncml-conf" );
    const QString DBPATH( "dbpath" );
    const QString LOCALDEVICENAME( "local-device-name" );
    const QString AGENTPROPS( "agent-props" );
    const QString TRANSPORTPROPS( "transport-props" );
    const QString EXTENSIONS( "extensions" );

    while( !reader.atEnd() )
    {
        if( reader.tokenType() == QXmlStreamReader::StartElement )
        {

            if( reader.name() == DBPATH )
            {
                reader.readNext();
                QString dbPath = reader.text().toString();
                LOG_DEBUG( "Found critical property" << DBPATH <<":" << dbPath );
                setDatabaseFilePath( dbPath );
            }
            else if( reader.name() == LOCALDEVICENAME )
            {
                reader.readNext();
                QString localDeviceName = reader.text().toString();
                LOG_DEBUG( "Found critical property" << LOCALDEVICENAME <<":" << localDeviceName );
                setLocalDeviceName( localDeviceName );
            }
            else if( reader.name() == AGENTPROPS )
            {
                parseAgentProps( reader );
            }
            else if( reader.name() == TRANSPORTPROPS )
            {
                parseTransportProps( reader );
            }
            else if( reader.name() == EXTENSIONS )
            {
                parseSyncExtensions( reader );
            }

        }

        reader.readNext();
    }

    return true;
}

bool SyncAgentConfig::parseAgentProps( QXmlStreamReader& aReader )
{
    FUNCTION_CALL_TRACE

    const QString AGENTPROPS( "agent-props" );

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement )
        {

            if( aReader.name() == MAXCHANGESPERMESSAGEPROP )
            {
                aReader.readNext();
                QString maxChangesPerMessage = aReader.text().toString();
                LOG_DEBUG( "Found agent property" << MAXCHANGESPERMESSAGEPROP <<":" << maxChangesPerMessage );
                setAgentProperty( MAXCHANGESPERMESSAGEPROP, maxChangesPerMessage );
            }
            else if( aReader.name() == MAXMESSAGESIZEPROP )
            {
                aReader.readNext();
                QString maxMessageSize = aReader.text().toString();
                LOG_DEBUG( "Found agent property" << MAXMESSAGESIZEPROP <<":" << maxMessageSize );
                setAgentProperty( MAXMESSAGESIZEPROP, maxMessageSize );
            }
            else if( aReader.name() == CONFLICTRESOLUTIONPOLICYPROP )
            {
                aReader.readNext();
                QString conflictResolutionPolicy = aReader.text().toString();
                LOG_DEBUG( "Found agent property" << CONFLICTRESOLUTIONPOLICYPROP <<":" << conflictResolutionPolicy );
                setAgentProperty( CONFLICTRESOLUTIONPOLICYPROP, conflictResolutionPolicy );
            }
            else if( aReader.name() == FASTMAPSSENDPROP )
            {
                aReader.readNext();
                QString fastMapsSend = aReader.text().toString();
                LOG_DEBUG( "Found agent property" << FASTMAPSSENDPROP <<":" << fastMapsSend );
                setAgentProperty( FASTMAPSSENDPROP, fastMapsSend );
            }
            else if( aReader.name() == OMITDATAUPDATESTATUSPROP )
            {
                aReader.readNext();
                QString omitDataUpdateStatus = aReader.text().toString();
                LOG_DEBUG( "Found agent property" << OMITDATAUPDATESTATUSPROP <<":" << omitDataUpdateStatus );
                setAgentProperty( OMITDATAUPDATESTATUSPROP, omitDataUpdateStatus );
            }

        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == AGENTPROPS )
        {
            break;
        }

        aReader.readNext();
    }

    return true;
}

bool SyncAgentConfig::parseTransportProps( QXmlStreamReader& aReader )
{
    FUNCTION_CALL_TRACE

    const QString TRANSPORTPROPS( "transport-props" );

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement )
        {

            if( aReader.name() == OBEXMTUBTPROP )
            {
                aReader.readNext();
                QString btObexMtu = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << OBEXMTUBTPROP <<":" << btObexMtu );
                setTransportProperty( OBEXMTUBTPROP, btObexMtu );
            }
            else if( aReader.name() == OBEXMTUUSBPROP )
            {
                aReader.readNext();
                QString usbObexMtu = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << OBEXMTUUSBPROP <<":" << usbObexMtu );
                setTransportProperty( OBEXMTUUSBPROP, usbObexMtu );
            }
            else if( aReader.name() == OBEXMTUOTHERPROP )
            {
                aReader.readNext();
                QString otherObexMtu = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << OBEXMTUOTHERPROP <<":" << otherObexMtu );
                setTransportProperty( OBEXMTUOTHERPROP, otherObexMtu );
            }
            else if( aReader.name() == OBEXTIMEOUTPROP )
            {
                aReader.readNext();
                QString obexTimeout= aReader.text().toString();
                LOG_DEBUG( "Found transport property" << OBEXTIMEOUTPROP <<":" << obexTimeout );
                setTransportProperty( OBEXTIMEOUTPROP, obexTimeout );
            }
            else if( aReader.name() == HTTPNUMBEROFRESENDATTEMPTSPROP )
            {
                aReader.readNext();
                QString numAttempts = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << HTTPNUMBEROFRESENDATTEMPTSPROP <<":" << numAttempts );
                setTransportProperty( HTTPNUMBEROFRESENDATTEMPTSPROP, numAttempts );
            }
            else if( aReader.name() == HTTPPROXYHOSTPROP )
            {
                aReader.readNext();
                QString proxyHost = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << HTTPPROXYHOSTPROP <<":" << proxyHost );
                setTransportProperty( HTTPPROXYHOSTPROP, proxyHost );
            }
            else if( aReader.name() == HTTPPROXYPORTPROP )
            {
                aReader.readNext();
                QString proxyPort = aReader.text().toString();
                LOG_DEBUG( "Found transport property" << HTTPPROXYPORTPROP <<":" << proxyPort );
                setTransportProperty( HTTPPROXYPORTPROP, proxyPort );
            }

        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == TRANSPORTPROPS )
        {
            break;
        }

        aReader.readNext();
    }

    return true;

}

bool SyncAgentConfig::parseSyncExtensions( QXmlStreamReader& aReader )
{
    FUNCTION_CALL_TRACE

    const QString EXTENSIONS( "extensions" );

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement )
        {

            if( aReader.name() == EMITAGSEXTENSION )
            {
                parseEMITagsExtension( aReader );
            }
            else if( aReader.name() == SYNCWITHOUTINITPHASEEXTENSION )
            {
                aReader.readNext();
                LOG_DEBUG( "Found extension" << SYNCWITHOUTINITPHASEEXTENSION );

                QVariant data;
                setExtension( SYNCWITHOUTINITPHASEEXTENSION, data );
            }
            else if( aReader.name() == SANMAPPINGSEXTENSION )
            {
                parseSANMappingsExtension( aReader );
            }

        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == EXTENSIONS )
        {
            break;
        }

        aReader.readNext();
    }

    return true;
}

bool SyncAgentConfig::parseEMITagsExtension( QXmlStreamReader& aReader )
{
    FUNCTION_CALL_TRACE

    const QString EMITAGSTOKEN( "token" );
    const QString EMITAGSRESPONSE( "response" );

    QString token;
    QString response;

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement )
        {

            if( aReader.name() == EMITAGSTOKEN )
            {
                aReader.readNext();
                token = aReader.text().toString();
                LOG_DEBUG( "Found EMI tag" << EMITAGSTOKEN <<":" << token );

            }
            else if( aReader.name() == EMITAGSRESPONSE )
            {
                aReader.readNext();
                response = aReader.text().toString();
                LOG_DEBUG( "Found EMI tag" << EMITAGSRESPONSE <<":" << response );
            }

        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == EMITAGSEXTENSION )
        {
            break;
        }

        aReader.readNext();
    }

    QStringList data;
    data << token << response;
    setExtension( EMITAGSEXTENSION, QVariant( data ) );

    return true;
}

bool SyncAgentConfig::parseSANMappingsExtension( QXmlStreamReader& aReader )
{
    FUNCTION_CALL_TRACE;

    const QString SANMAPPING( "san-mapping" );

    QStringList data;

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement &&
            aReader.name() == SANMAPPING )
        {
            parseSANMappingData( aReader, data );
        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == SANMAPPINGSEXTENSION )
        {
            break;
        }
        aReader.readNext();
    }

    setExtension( SANMAPPINGSEXTENSION, QVariant( data ) );

    return true;

}

bool SyncAgentConfig::parseSANMappingData( QXmlStreamReader& aReader,
                                           QStringList& aMappings )
{
    FUNCTION_CALL_TRACE;

    const QString SANMAPPING( "san-mapping" );
    const QString URI( "uri" );
    const QString MIME( "mime" );

    QString uri;
    QString mime;

    while( !aReader.atEnd() )
    {
        if( aReader.tokenType() == QXmlStreamReader::StartElement )
        {
            if( aReader.name() == URI )
            {
                aReader.readNext();
                uri = aReader.text().toString();
                LOG_DEBUG( "Found SAN URI:" << uri );
            }
            else if( aReader.name() == MIME )
            {
                aReader.readNext();
                mime = aReader.text().toString();
                LOG_DEBUG( "Found SAN MIME:" << mime );
            }
        }
        else if( aReader.tokenType() == QXmlStreamReader::EndElement &&
                 aReader.name() == SANMAPPING )
        {
            break;
        }

        aReader.readNext();
    }

    aMappings << uri << mime;

    return true;
}
