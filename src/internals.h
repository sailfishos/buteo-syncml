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

#ifndef INTERNALS_H
#define INTERNALS_H

#include "datatypes.h"

namespace DataSync {

struct MemParams {
      int freeMem;
      int freeId;
      MemParams() : freeMem(-1),freeId(-1) {}
  };

  struct AnchorParams {
      QString Last;
      QString Next;
  };

  struct MetaParams {
      qint64 size;
      QString type;
      QString format;
      QString Version;
      QString nextNonce; // used for md5 digest challengs
      MemParams mem; // Optional parameter may not always present
      AnchorParams anchor;
      MetaParams() : size(0) {}
  };

  struct ItemParams {
      bool moreData;
      QString source;
      QString target;
      QString sourceParent;
      QString targetParent;
      QString Data;
      MetaParams meta;
      ItemParams() :moreData(false) {}
  };

  struct CredParams {
      QString data;
      MetaParams meta;
  };

/*! \brief Base for protocol fragments
 *
 */
struct Fragment {

    /*! \brief Type of fragment
     *
     */
    enum Type
    {
        FRAGMENT_HEADER,
        FRAGMENT_STATUS,
        FRAGMENT_ALERT,
        FRAGMENT_SYNC,
        FRAGMENT_MAP,
        FRAGMENT_RESULTS,
        FRAGMENT_COMMAND
    };

    Type iType;

    Fragment( Type aType ) : iType( aType ) {}

};

struct SyncActionData : public Fragment {
    SyncMLCommand action;
    int cmdID;
    bool noResp;//optional , if available protocol handler should not send a response to this action
    int numberOfChanges;
    QString lang;
    CredParams cred;//optional , if available should use this data for local database authentication
    MetaParams meta;//optional , if available this indicates the meta information available in this command
    QList<ItemParams> items;// 1 or more , must.specifies the source and target information
    QList<SyncActionData> syncActions; //valid in the case of Atomic , Sequence , right now these are optional
    SyncActionData() : Fragment( FRAGMENT_COMMAND ), action(SYNCML_ADD), cmdID(-1), noResp(false),
                       numberOfChanges(-1)  {}
};

struct HeaderParams : public Fragment {
    QString verDTD;
    QString verProto;
    QString sessionID;
    int msgID;
    QString targetDevice;
    QString sourceDevice;
    qint64 maxMsgSize;
    qint64 maxObjSize;
    QString respURI;
    QList<QString> EMI;
    CredParams cred;
    bool noResp;
    HeaderParams() : Fragment( FRAGMENT_HEADER ), msgID(-1), maxMsgSize(0),
                     maxObjSize(0), noResp( false ) {}
};

struct AlertParams : public Fragment {
    int cmdID;
    bool noResp;
    AlertType data;
    CredParams Cred;
    QList<ItemParams> itemList;
    // @to be removed , these can be read from itemList
    // parser supports pupulating these variables now ,
    // but they may not be valid.these should be read from
    // items , meta and anchor elements...
    QString targetDatabase;
    QString sourceDatabase;
    QString nextAnchor;
    QString lastAnchor;
    QString type;
    AlertParams() : Fragment( FRAGMENT_ALERT ), cmdID(-1),
                    noResp(false),data(SLOW_SYNC) {}
};

 struct ChalParams  {
      MetaParams meta;
      ChalParams() : meta() {}
 };

struct StatusParams : public Fragment {
    int cmdID;
    int msgRef;
    int cmdRef;
    QString cmd;
    QString targetRef;
    QString sourceRef;
    ResponseStatusCode data;
    QString nextAnchor;
    QList<ItemParams> itemList;
    ChalParams chal;
    StatusParams() : Fragment( FRAGMENT_STATUS ), cmdID(-1), msgRef(-1),
                     cmdRef(-1), data(SERVER_FAILURE) {}
};

struct ResultsParams : public Fragment {
    int cmdID;
    int msgRef;
    int cmdRef;
    QString targetRef;
    QString sourceRef;
    MetaParams meta;
    ResultsParams() : Fragment( FRAGMENT_RESULTS ), cmdID(-1), msgRef(-1), cmdRef(-1) {}
};

  struct MapItem {
      QString target;
      QString source;
  };

struct MapParams : public Fragment {
    // mandatory
    int cmdID;
    QString target;
    QString source;
    QList<MapItem> mapItemList;
    // optional
    CredParams cred;
    MetaParams meta;
    MapParams() : Fragment( FRAGMENT_MAP ), cmdID(-1) {}
};

struct SyncParams : public Fragment {
    int cmdID;
    bool noResp;
    CredParams cred;
    MetaParams meta;
    QString targetDatabase;
    QString sourceDatabase;
    QList<SyncActionData> actionList;
    SyncParams() : Fragment( FRAGMENT_SYNC ), cmdID(-1),noResp(false) {}
};

  struct LocalChanges {
      QList<SyncItemKey> added;
      QList<SyncItemKey> modified;
      QList<SyncItemKey> removed;
  };

}

#endif /* INTERNALS_H */
