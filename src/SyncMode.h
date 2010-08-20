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
#ifndef SYNCMODE_H
#define SYNCMODE_H

#include "SyncAgentConsts.h"
#include <QMap>

namespace DataSync {

class SyncModeTest;

/*! \brief Utility class for handling SyncML Alert codes related to sync modes
 *
 */
class SyncMode {

public:

    /*! \brief Construct using direction and initiator
     *
     * By default, direction is two-way, initiator is client and type if fast
     * sync
     *
     * @param aDirection Direction of the sync
     * @param aInitiator Initiator of the sync
     * @param aType Type of the sync
     */
    SyncMode( SyncDirection aDirection = DIRECTION_TWO_WAY,
              SyncInitiator aInitiator = INIT_CLIENT,
              SyncType aType = TYPE_FAST );

    /*! \brief Construct using SyncML Alert code
     *
     * @param aSyncMLCode SyncML Alert code
     */
    explicit SyncMode( int aSyncMLCode );

    /*! \brief Destructor
     *
     */
    ~SyncMode();

    /*! \brief Returns true if class instance holds a valid SyncML Alert code
     *
     * If false is returned, functioning of other functions is undefined
     *
     * @return
     */
    bool isValid() const;

    /*! \brief Return the direction of the sync
     *
     * @return
     */
    SyncDirection syncDirection() const;

    /*! \brief Return the initiator of the sync
     *
     * @return
     */
    SyncInitiator syncInitiator() const;

    /*!  \brief Return the type of the sync
     *
     * @return
     */
    SyncType syncType() const;

    /*! \brief Turn sync mode to slow sync
     *
     */
    void toSlowSync();

    /*! \brief Turn sync mode to refresh sync
     *
     */
    void setRefresh();

    /*! \brief Turn sync mode to client initiated sync
     *
     */
    void toClientInitiated();

    /*! \brief Output SyncML Alert code that matches presented sync mode
     *
     * @return
     */
    int toSyncMLCode() const;

protected:

private:

    SyncDirection   iDirection;
    SyncInitiator   iInitiator;
    SyncType        iType;
    bool            iValid;

    friend class SyncModeTest;

};

}

#endif  //  SYNCMODE_H
