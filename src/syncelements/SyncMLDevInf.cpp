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

#include "SyncMLDevInf.h"
#include "DeviceInfo.h"
#include "StoragePlugin.h"
#include "SyncMLCTCap.h"
#include "internals.h"
#include "datatypes.h"
#include "LogMacros.h"
#include <QDomDocument>
#include <QTextStream>

using namespace DataSync;

SyncMLDevInf::SyncMLDevInf( const QList<StoragePlugin*> &aDataStores,
                            const DeviceInfo& aDeviceInfo,
                            const ProtocolVersion& aVersion,
                            const Role& aRole )
 : SyncMLCmdObject( SYNCML_ELEMENT_ITEM ), iDeviceInfo( aDeviceInfo )
{
    SyncMLCmdObject* sourceObject = new SyncMLCmdObject(SYNCML_ELEMENT_SOURCE);

    SyncMLCmdObject* sourceURIObject = new SyncMLCmdObject(SYNCML_ELEMENT_LOCURI,
            aVersion == DS_1_2 ? SYNCML_DEVINF_PATH_12 : SYNCML_DEVINF_PATH_11 );
    sourceObject->addChild(sourceURIObject);
    addChild(sourceObject);

    SyncMLCmdObject* dataObject = new SyncMLCmdObject(SYNCML_ELEMENT_DATA);

    SyncMLCmdObject* devInfObject = new SyncMLCmdObject(SYNCML_ELEMENT_DEVINF);
    devInfObject->addAttribute( XML_NAMESPACE,XML_NAMESPACE_VALUE_DEVINF );

    SyncMLCmdObject* verDTDObject = new SyncMLCmdObject(SYNCML_ELEMENT_VERDTD,
            aVersion == DS_1_2 ? SYNCML_DTD_VERSION_1_2 : SYNCML_DTD_VERSION_1_1 );
    devInfObject->addChild( verDTDObject );

    SyncMLCmdObject* manObject = new SyncMLCmdObject( SYNCML_ELEMENT_MAN,
                                                      iDeviceInfo.getManufacturer() );
    devInfObject->addChild( manObject );


    SyncMLCmdObject* modObject = new SyncMLCmdObject( SYNCML_ELEMENT_MOD,
                                                      iDeviceInfo.getModel() );
    devInfObject->addChild( modObject );


    SyncMLCmdObject* fwVObject = new SyncMLCmdObject( SYNCML_ELEMENT_FWVERSION,
                                                      iDeviceInfo.getFirmwareVersion() );
    devInfObject->addChild( fwVObject );


    SyncMLCmdObject* swVObject = new SyncMLCmdObject( SYNCML_ELEMENT_SWVERSION,
                                                      iDeviceInfo.getSoftwareVersion() );
    devInfObject->addChild( swVObject );


    SyncMLCmdObject* hwVObject = new SyncMLCmdObject( SYNCML_ELEMENT_HWVERSION,
                                                      iDeviceInfo.getHardwareVersion() );
    devInfObject->addChild( hwVObject );


    SyncMLCmdObject* devIDObject = new SyncMLCmdObject( SYNCML_ELEMENT_DEVID,
                                                        iDeviceInfo.getDeviceID() );
    devInfObject->addChild( devIDObject );


    SyncMLCmdObject* devTypObject = new SyncMLCmdObject( SYNCML_ELEMENT_DEVTYPE,
                                                         iDeviceInfo.getDeviceType() );
    devInfObject->addChild( devTypObject );

    SyncMLCmdObject* utcObject = new SyncMLCmdObject( SYNCML_ELEMENT_UTC );
    devInfObject->addChild( utcObject );

    SyncMLCmdObject* supportLargeObjsObject = new SyncMLCmdObject(SYNCML_ELEMENT_SUPPORTLARGEOBJS);
    devInfObject->addChild( supportLargeObjsObject );

    SyncMLCmdObject* nocObject = new SyncMLCmdObject( SYNCML_ELEMENT_SUPPORTNUMBEROFCHANGES );
    devInfObject->addChild( nocObject );

    // In SyncML 1.1, CTCaps are sent with DevInf element as parent and a single CTCaps element includes
    // capabilities of every storage plugin. In SyncML 1.2 CTCaps are sent with corresponding
    // DataStore element as parent

    if( aVersion == DS_1_1 ) {

        SyncMLCTCap* ctcap = NULL;

        for( int i = 0; i < aDataStores.count(); ++i ) {
            devInfObject->addChild( generateDataStore( *aDataStores[i], aRole ) );

            QByteArray cap = aDataStores[i]->getPluginCTCaps( DS_1_1 );

            if( !cap.isEmpty() ) {

                if( !ctcap ) {
                    ctcap = new SyncMLCTCap();
                }

                ctcap->addCTCap( cap );

            }
        }

        if( ctcap ) {
            devInfObject->addChild( ctcap );
        }

    }
    else if( aVersion == DS_1_2 ) {

        for( int i = 0; i < aDataStores.count(); ++i ) {
            SyncMLCmdObject* dataStore = generateDataStore( *aDataStores[i], aRole );

            QByteArray cap = aDataStores[i]->getPluginCTCaps( DS_1_2 );
            QDomDocument doc;
            if( !cap.isEmpty() && doc.setContent( cap ) ) {

                // Make sure that CtCap elements are under one root element named CtCaps.
                QDomElement root = doc.documentElement();
                if ( root.tagName() != SYNCML_ELEMENT_CTCAPS ) {
                    QDomElement newRoot = doc.createElement( SYNCML_ELEMENT_CTCAPS );
                    newRoot.appendChild( root );
                    root = newRoot;
                }

                QDomElement ctCapElement = root.firstChildElement( SYNCML_ELEMENT_CTCAP );
                for ( ; !ctCapElement.isNull(); ctCapElement = ctCapElement.nextSiblingElement( SYNCML_ELEMENT_CTCAP ) )
                {
                    QByteArray ctCapData;
                    QTextStream ctCapStream( &ctCapData );
                    ctCapElement.save( ctCapStream, 1 );

                    SyncMLCTCap* ctcap = new SyncMLCTCap();
                    ctcap->addCTCap( ctCapData );
                    dataStore->addChild( ctcap );
                }
            }

            devInfObject->addChild( dataStore );
        }

    }

    dataObject->addChild( devInfObject );

    addChild( dataObject );

}

SyncMLDevInf::~SyncMLDevInf()
{
}

SyncMLCmdObject* SyncMLDevInf::generateDataStore( const StoragePlugin& aPlugin,
                                                  const Role& aRole )
{
    FUNCTION_CALL_TRACE;

    SyncMLCmdObject* dataStoreObject = new SyncMLCmdObject(SYNCML_ELEMENT_DATASTORE);

    SyncMLCmdObject* sourceRef = new SyncMLCmdObject( SYNCML_ELEMENT_SOURCEREF,
                                                      aPlugin.getSourceURI() );

    dataStoreObject->addChild( sourceRef );

    // If in client mode, send maximum GUID size
    if( aRole == ROLE_CLIENT ) {

        SyncMLCmdObject* maxGUIDSize = new SyncMLCmdObject( SYNCML_ELEMENT_MAXGUIDSIZE,
                                                            SYNCML_DSINFO_MAXGUIDSIZE );
        dataStoreObject->addChild( maxGUIDSize );

    }

    const StoragePlugin::ContentFormat& preferredFormat = aPlugin.getPreferredFormat();

    if( !preferredFormat.iType.isEmpty() ) {

        SyncMLCmdObject *rxPrefObject = new SyncMLCmdObject(SYNCML_ELEMENT_RX_PREF);
        SyncMLCmdObject *txPrefObject = new SyncMLCmdObject(SYNCML_ELEMENT_TX_PREF);

        SyncMLCmdObject* rxCTTypeObject = new SyncMLCmdObject( SYNCML_ELEMENT_CTTYPE, preferredFormat.iType );
        rxPrefObject->addChild( rxCTTypeObject );

        SyncMLCmdObject* rxVerCTObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERCT, preferredFormat.iVersion );
        rxPrefObject->addChild( rxVerCTObject );

        dataStoreObject->addChild( rxPrefObject );

        SyncMLCmdObject* txCTTypeObject = new SyncMLCmdObject( SYNCML_ELEMENT_CTTYPE, preferredFormat.iType);
        txPrefObject->addChild( txCTTypeObject );

        SyncMLCmdObject* txVerCTObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERCT, preferredFormat.iVersion );
        txPrefObject->addChild( txVerCTObject );

        dataStoreObject->addChild( txPrefObject );

    }

    const QList<StoragePlugin::ContentFormat>& supportedFormats = aPlugin.getSupportedFormats();

    for( int i = 0; i < supportedFormats.count(); ++i ) {
        const StoragePlugin::ContentFormat& format = supportedFormats[i];

        SyncMLCmdObject *rxObject = new SyncMLCmdObject(SYNCML_ELEMENT_RX);
        SyncMLCmdObject *txObject = new SyncMLCmdObject(SYNCML_ELEMENT_TX);

        SyncMLCmdObject* rxCTTypeObject = new SyncMLCmdObject( SYNCML_ELEMENT_CTTYPE, format.iType );
        rxObject->addChild( rxCTTypeObject );

        SyncMLCmdObject* rxVerCTObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERCT, format.iVersion );
        rxObject->addChild( rxVerCTObject );

        dataStoreObject->addChild( rxObject );

        SyncMLCmdObject* txCTTypeObject = new SyncMLCmdObject( SYNCML_ELEMENT_CTTYPE, format.iType);
        txObject->addChild( txCTTypeObject );

        SyncMLCmdObject* txVerCTObject = new SyncMLCmdObject( SYNCML_ELEMENT_VERCT, format.iVersion );
        txObject->addChild( txVerCTObject );

        dataStoreObject->addChild( txObject );


    }

    SyncMLCmdObject* syncCapObject = new SyncMLCmdObject(SYNCML_ELEMENT_SYNCCAP);

    QList<int> syncTypes;

    // Two-way sync
    syncTypes.append( SYNCTYPE_TWOWAY );

    // Slow two-way sync
    syncTypes.append( SYNCTYPE_TWOWAYSLOW );

    // One-way sync from client only
    syncTypes.append( SYNCTYPE_FROMCLIENT );

    // Refresh sync from client only
    syncTypes.append( SYNCTYPE_FROMCLIENTSLOW );

    // One-way sync from server only
    syncTypes.append( SYNCTYPE_FROMSERVER );

    // Refresh sync from server only
    syncTypes.append( SYNCTYPE_FROMSERVERSLOW );

    // Server alerted sync
    syncTypes.append( SYNCTYPE_SERVERALERTED );

    for( int i = 0; i < syncTypes.count(); ++i )
    {
        SyncMLCmdObject* syncType = new SyncMLCmdObject( SYNCML_ELEMENT_SYNCTYPE,
                                                         QString::number( syncTypes[i] ));
        syncCapObject->addChild( syncType );
    }

    dataStoreObject->addChild( syncCapObject );


    return dataStoreObject;
}
