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

#include "LocalMappingsPackage.h"
#include "SyncMLMessage.h"
#include "SyncMLMap.h"
#include "SyncMLMapItem.h"
#include "SyncMLLogging.h"

using namespace DataSync;

LocalMappingsPackage::LocalMappingsPackage( const QString& aSourceDatabase,
                                            const QString& aTargetDatabase,
                                            const QList<UIDMapping>& aMappings )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    iSourceDatabase = aSourceDatabase;
    iTargetDatabase = aTargetDatabase;
    iMappings = aMappings;
}

LocalMappingsPackage::~LocalMappingsPackage()
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);
}

bool LocalMappingsPackage::write( SyncMLMessage& aMessage, int& aSizeThreshold, bool aWBXML, const ProtocolVersion& aVersion )
{
    FUNCTION_CALL_TRACE(lcSyncMLTrace);

    if( !iMappings.isEmpty() ) {
        int cmdId = aMessage.getNextCmdId();
        SyncMLMap* map = new SyncMLMap( cmdId,
                                        iTargetDatabase,
                                        iSourceDatabase );

        aSizeThreshold -= map->calculateSize(aWBXML, aVersion);

        // Add at least one MapItem to the Map package
        do {
            SyncMLMapItem* mapItem = new SyncMLMapItem( iMappings[0].iRemoteUID, iMappings[0].iLocalUID );
            aSizeThreshold -= mapItem->calculateSize(aWBXML, aVersion);
            map->addChild(mapItem);
            iMappings.removeFirst();
        } while( aSizeThreshold > 0 && !iMappings.isEmpty() );

        aMessage.addToBody(map);
    }

    return iMappings.isEmpty();
}
