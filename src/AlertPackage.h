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
#ifndef ALERTPACKAGE_H
#define ALERTPACKAGE_H

#include "Package.h"

#include "Fragments.h"

class AlertPackageTest;

namespace DataSync {

class SyncTarget;
class SyncMode;


/*! \brief Handles sending an Alert to remote side
 *
 */
class AlertPackage : public Package
{
    Q_OBJECT;
public:

    /*! \brief Constructor for Alerts that include only the alert code
     *
     */
    AlertPackage( qint32 aAlertCode );

    /*! \brief Constructor for Alert that includes source and target databases
     *
     * This should be used for example which Alerts related to suspend & resume and
     * requesting next messages
     *
     * @param aAlertCode Code of this Alert
     * @param aSourceDatabase Source database
     * @param aTargetDatabase Target database
     */
    AlertPackage( qint32 aAlertCode, const QString& aSourceDatabase, const QString& aTargetDatabase );


    /*! \brief Constructor for Alert that includes source/target databases and anchor information
     *
     * This should be used for Alerts related to sync modes
     *
     * @param aAlertCode Code of this Alert
     * @param aSourceDatabase Source database
     * @param aTargetDatabase Target database
     * @param aLocalLastAnchor Local last anchor
     * @param aLocalNextAnchor Local next anchor
     */
    AlertPackage( qint32 aAlertCode, const QString& aSourceDatabase, const QString& aTargetDatabase,
                  const QString& aLocalLastAnchor, const QString& aLocalNextAnchor );


    /*! \brief Constructor for Alert that includes source database and MIME type
     *
     * This should be used for Alerts related to server initiated sync
     *
     * @param aSourceDatabase Source database
     * @param aMIMEType MIME type
     * @param aAlertCode Code of this Alert
     */
    AlertPackage( const QString& aSourceDatabase, const QString& aMIMEType, qint32 aAlertCode );

    /*! \brief Destructor
     *
     */
    virtual ~AlertPackage();

    virtual bool write( SyncMLMessage& aMessage, int& aSizeThreshold, bool aWBXML, const ProtocolVersion& aVersion);

protected:

private:

    CommandParams iParams;

    friend class ::AlertPackageTest;

};

}

#endif  //  ALERTPACKAGE_H
