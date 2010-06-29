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
#ifndef LOCALMAPPINGSPACKAGE_H
#define LOCALMAPPINGSPACKAGE_H

#include <QObject>

#include "SyncMLGlobals.h"
#include "Package.h"
#include "internals.h"

class QString;
class LocalMappingsPackageTest;

namespace DataSync
{

/*! \brief LocalMappingsPackage handles sending local mappings phase for
 *         a single sync target
 */
class LocalMappingsPackage : public Package
{
    Q_OBJECT
public:

    /*! \brief Constructor
     *
     * @param aSourceDatabase Source database
     * @param aTargetDatabase Target database
     * @param aMappings UID mappings
     */
    LocalMappingsPackage( const QString& aSourceDatabase,
                          const QString& aTargetDatabase,
                          const QList<UIDMapping>& aMappings );

    /*! \brief Destructor
     *
     */
    virtual ~LocalMappingsPackage();

    virtual bool write( SyncMLMessage& aMessage, int& aSizeThreshold );

signals:

    /*! \brief Signal that is transmitted when map has been added to an outgoing message
     *
     * Information supplied in this signal can be used to track the map.
     *
     * @param aMsgId Message Id of the message where the map was written
     * @param aCmdId Command Id of the command where the map was written
     * @param aLocalDatabase Local database of the sent maps
     * @param aRemoteDatabase Remote database to where to send maps
     */
    void newMapWritten( int aMsgId, int aCmdId,
                        const QString& aLocalDatabase,
                        const QString& aRemoteDatabase );

protected:

private:

    QString             iSourceDatabase;
    QString             iTargetDatabase;
    QList<UIDMapping>   iMappings;

    friend class ::LocalMappingsPackageTest;

};

}

#endif  //  LOCALMAPPINGSPACKAGE_H
