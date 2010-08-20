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

#include "AlertPackage.h"

#include "SyncMLMessage.h"
#include "SyncMLAlert.h"
#include "SyncTarget.h"
#include "SyncMode.h"

#include "LogMacros.h"

using namespace DataSync;

AlertPackage::AlertPackage( AlertType aAlertCode, const QString& aSourceDatabase, const QString& aTargetDatabase )
{
    FUNCTION_CALL_TRACE;

    iParams.data = aAlertCode;
    iParams.sourceDatabase = aSourceDatabase;
    iParams.targetDatabase = aTargetDatabase;
}

AlertPackage::AlertPackage( AlertType aAlertCode, const QString& aSourceDatabase, const QString& aTargetDatabase,
                            const QString& aLocalLastAnchor, const QString& aLocalNextAnchor )
{
    FUNCTION_CALL_TRACE;

    iParams.data = aAlertCode;

    iParams.sourceDatabase = aSourceDatabase;
    iParams.targetDatabase = aTargetDatabase;
    iParams.lastAnchor = aLocalLastAnchor;
    iParams.nextAnchor = aLocalNextAnchor;

    // This is needed for S60, do NOT remove
    if( iParams.lastAnchor.isEmpty() ) {
        iParams.lastAnchor  = '0';
    }
}

AlertPackage::AlertPackage( const QString& aSourceDatabase, const QString& aMIMEType, AlertType aAlertCode )
{
    FUNCTION_CALL_TRACE;

    iParams.data = aAlertCode;
    iParams.sourceDatabase = aSourceDatabase;
    iParams.type = aMIMEType;
}

AlertPackage::~AlertPackage()
{
}

bool AlertPackage::write( SyncMLMessage& aMessage, int& aSizeThreshold )
{
    FUNCTION_CALL_TRACE

    iParams.cmdID = aMessage.getNextCmdId();
    SyncMLAlert* alertObject = new SyncMLAlert( iParams );
    aMessage.addToBody( alertObject );
    aSizeThreshold -= alertObject->sizeAsXML();

    return true;

}
