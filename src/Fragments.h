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

#ifndef FRAGMENTS_H
#define FRAGMENTS_H

#include "RemoteDeviceInfo.h"
#include "datatypes.h"

// @todo: StatusParams.nextAnchor
// @todo: Search


namespace DataSync {

struct CommandParams;

struct AnchorParams
{
    QString         last;
    QString         next;
};

struct MetaParams
{
    AnchorParams    anchor;
    QList<QString>  EMI;
    QString         format;
    qint64          maxMsgSize;
    qint64          maxObjSize;
    QString         nextNonce;
    qint64          size;
    QString         type;
    QString         version;  
    QString         mark;

    MetaParams() : maxMsgSize(0), maxObjSize(0),
                   size(0) {}
};

struct ItemParams
{

    QString         source;
    QString         target;
    QString         sourceParent;
    QString         targetParent;
    MetaParams      meta;
    QString         data;
    bool            moreData;

    ItemParams() : moreData(false) {}
};

struct DevInfItemParams
{
    QString             source;
    RemoteDeviceInfo    devInfo;
};

struct CredParams
{
    MetaParams      meta;
    QString         data;

};

struct MapItemParams
{
    QString         target;
    QString         source;
};

struct ChalParams
{
    MetaParams meta;
};

/*! \brief Base for protocol fragments
 *
 */
struct Fragment {

    /*! \brief Type of fragment
     *
     */
    enum FragmentType
    {
        FRAGMENT_HEADER = 0,
        FRAGMENT_STATUS,
        FRAGMENT_PUT,
        FRAGMENT_RESULTS,
        FRAGMENT_SYNC,
        FRAGMENT_MAP,
        FRAGMENT_COMMAND
    };

    FragmentType fragmentType;

    Fragment( FragmentType aType ) : fragmentType( aType ) {}

    virtual ~Fragment() { }

};

struct HeaderParams : public Fragment
{
    QString         verDTD;
    QString         verProto;
    QString         sessionID;
    int             msgID;
    QString         targetDevice;
    QString         sourceDevice;
    QString         respURI;
    bool            noResp;
    CredParams      cred;
    MetaParams      meta;

    HeaderParams() : Fragment( FRAGMENT_HEADER ), msgID(-1), noResp( false ) {}
};

struct StatusParams : public Fragment
{
    int                 cmdId;
    int                 msgRef;
    int                 cmdRef;
    QString             cmd;
    QString             targetRef;
    QString             sourceRef;
    ResponseStatusCode  data;
    bool                hasChal;
    ChalParams          chal;
    QString             nextAnchor;
    QList<ItemParams>   items;

    StatusParams() : Fragment( FRAGMENT_STATUS ), cmdId(-1), msgRef(-1),
                     cmdRef(-1), data(SERVER_FAILURE), hasChal( false ) {}
};

struct PutParams : public Fragment
{
    int                 cmdId;
    bool                noResp;
    MetaParams          meta;
    DevInfItemParams    devInf;

    PutParams() : Fragment( FRAGMENT_PUT ), cmdId( -1 ), noResp( false ) { }
};

struct ResultsParams : public Fragment
{
    int                 cmdId;
    int                 msgRef;
    int                 cmdRef;
    QString             targetRef;
    QString             sourceRef;
    MetaParams          meta;
    DevInfItemParams    devInf;
    ResultsParams() : Fragment( FRAGMENT_RESULTS ), cmdId(-1), msgRef(-1), cmdRef(-1) {}
};

struct SyncParams : public Fragment
{
    int                     cmdId;
    bool                    noResp;
    MetaParams              meta;
    QString                 target;
    QString                 source;
    qint32                  numberOfChanges;
    QList<CommandParams>    commands;
    SyncParams() : Fragment( FRAGMENT_SYNC ), cmdId(-1), noResp(false), numberOfChanges(0) {}
};

struct MapParams : public Fragment
{

    int                     cmdId;
    QString                 target;
    QString                 source;
    MetaParams              meta;
    QList<MapItemParams>    mapItems;

    MapParams() : Fragment( FRAGMENT_MAP ), cmdId(-1) {}
};

struct CommandParams : public Fragment
{
    enum CommandType
    {
        COMMAND_ALERT = 0,
        COMMAND_ADD,
        COMMAND_REPLACE,
        COMMAND_DELETE,
        COMMAND_GET,
        COMMAND_COPY,
        COMMAND_MOVE,
        COMMAND_EXEC,
        COMMAND_ATOMIC,
        COMMAND_SEQUENCE
    };

    CommandType             commandType;

    int                     cmdId;
    bool                    noResp;
    QString                 data;
    QString                 correlator;
    MetaParams              meta;
    QList<ItemParams>       items;
    QList<CommandParams>    subCommands;

    CommandParams( CommandType aType ) : Fragment( FRAGMENT_COMMAND ), commandType( aType ),
                                         cmdId( -1 ), noResp( false ) {}
    CommandParams() : Fragment( FRAGMENT_COMMAND ), commandType( COMMAND_ALERT ),
                      cmdId( -1 ), noResp( false ) {}
};

}

#endif  //  FRAGMENTS_H
