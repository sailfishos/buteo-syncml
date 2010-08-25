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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <QMetaType>
#include <QString>
#include <QPair>
#include <QList>
#include <QDateTime>

#include <QBitArray>
#include "SyncItemKey.h"
#include "SyncMLCommand.h"

namespace DataSync
{

    /*! \brief OMA DS role
     *
     */
    enum Role {
        ROLE_CLIENT,    /*!<Act as OMA DS client*/
        ROLE_SERVER     /*!<Act as OMA DS server*/
    };

    enum SyncTypes {
        SYNCTYPE_TWOWAY = 1,
        SYNCTYPE_TWOWAYSLOW,
        SYNCTYPE_FROMCLIENT,
        SYNCTYPE_FROMCLIENTSLOW,
        SYNCTYPE_FROMSERVER,
        SYNCTYPE_FROMSERVERSLOW,
        SYNCTYPE_SERVERALERTED
    };

    enum AlertType
    {
        // Invalid type for initializations etc.
        INVALID_ALERT = 0,

        // Alert codes used for user alerts
        DISPLAY = 100,

        // Alert codes used at the sync init
        TWO_WAY_SYNC = 200,
        SLOW_SYNC = 201,
        ONE_WAY_FROM_CLIENT_SYNC = 202,
        REFRESH_FROM_CLIENT_SYNC = 203,
        ONE_WAY_FROM_SERVER_SYNC = 204,
        REFRESH_FROM_SERVER_SYNC = 205,

        // Alert codes used by the server when alerting the sync
        TWO_WAY_BY_SERVER = 206,
        ONE_WAY_FROM_CLIENT_BY_SERVER = 207,
        REFRESH_FROM_CLIENT_BY_SERVER = 208,
        ONE_WAY_FROM_SERVER_BY_SERVER = 209,
        REFRESH_FROM_SERVER_BY_SERVER = 210,

        // Special alert codes
        RESULT_ALERT = 221,
        NEXT_MESSAGE = 222,
        NO_END_OF_DATA = 223,
        ALERT_SUSPEND = 224,
        ALERT_RESUME = 225
    };

    enum ResponseStatusCode
    {
        // Informational 1xx
        IN_PROGRESS = 101,

        // Successful 2xx
        SUCCESS = 200,
        ITEM_ADDED = 201,
        ACCEPTED_FOR_PROCESSING = 202,
        NONAUTHORITATIVE_RESPONSE = 203,
        NO_CONTENT = 204,
        RESET_CONTENT = 205,
        PARTIAL_CONTENT = 206,
        RESOLVED_WITH_MERGE = 207,
        RESOLVED_CLIENT_WINNING = 208,
        RESOLVED_WITH_DUPLICATE = 209,
        DELETE_WITHOUT_ARCHIVE = 210,
        ITEM_NOT_DELETED = 211,
        AUTH_ACCEPTED = 212,
        CHUNKED_ITEM_ACCEPTED = 213,
        CANCELLED = 214,
        NOT_EXECUTED = 215,
        ATOMIC_ROLLBACK_OK = 216,

        // Redirection 3xx
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        FOUND = 302,
        SEE_OTHER = 303,
        NOT_MODIFIED = 304,
        USE_PROXY = 305,

        // Originator exceptions 4xx
        BAD_REQUEST = 400,
        INVALID_CRED = 401,
        PAYMENT_NEEDED = 402,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        COMMAND_NOT_ALLOWED = 405,
        NOT_SUPPORTED = 406,
        MISSING_CRED = 407,
        REQUEST_TIMEOUT = 408,
        CONFLICT = 409,
        GONE = 410,
        SIZE_REQUIRED = 411,
        INCOMPLETE_COMMAND = 412,
        REQUEST_ENTITY_TOO_LARGE = 413,
        URI_TOO_LONG = 414,
        UNSUPPORTED_FORMAT = 415,
        REQUEST_SIZE_TOO_BIG = 416,
        RETRY_LATER = 417,
        ALREADY_EXISTS = 418,
        RESOLVED_WITH_SERVER_DATA = 419,
        DEVICE_FULL = 420,
        UNKNOWN_SEARCH_GRAMMAR = 421,
        BAD_CGI_SCRIPT = 422,
        SOFTDELETE_CONFLICT = 423,
        SIZE_MISMATCH = 424,
        PERMISSION_DENIED = 425,
        PARTIAL_ITEM_NOT_ACCEPTED = 426,
        ITEM_NOT_EMPTY = 427,
        MOVE_FAILED = 428,

        // Recipient exception 5xx
        COMMAND_FAILED = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503,
        GATEWAY_TIMEOUT = 504,
        UNSUPPORTED_DTD_VERSION = 505,
        PROCESSING_ERROR = 506,
        ATOMIC_FAILED = 507,
        REFRESH_REQUIRED = 508,
        // 509 is reserved for future use
        DATA_STORE_FAILURE = 510,
        SERVER_FAILURE = 511,
        SYNC_FAILED = 512,
        UNSUPPORTED_PROTOCOL_VERSION = 513,
        OPERATION_CANCELLED = 514,
        // 515 not specified
        ATOMIC_ROLLBACK_FAILED = 516,
        ATOMIC_RESPONSE_TOO_LARGE = 517
    };

    #define SYNCML_ELEMENT_SYNCHDR "SyncHdr"
    #define SYNCML_ELEMENT_SYNCBODY "SyncBody"
    #define SYNCML_ELEMENT_SYNCML "SyncML"
    #define SYNCML_ELEMENT_CMDID "CmdID"
    #define SYNCML_ELEMENT_STATUS "Status"
    #define SYNCML_ELEMENT_ALERT "Alert"
    #define SYNCML_ELEMENT_ADD "Add"
    #define SYNCML_ELEMENT_DELETE "Delete"
    #define SYNCML_ELEMENT_REPLACE "Replace"
    #define SYNCML_ELEMENT_SYNC "Sync"
    #define SYNCML_ELEMENT_VERDTD "VerDTD"
    #define SYNCML_ELEMENT_VERPROTO "VerProto"
    #define SYNCML_ELEMENT_ITEM "Item"
    #define SYNCML_ELEMENT_SOURCE "Source"
    #define SYNCML_ELEMENT_TARGET "Target"
    #define SYNCML_ELEMENT_SOURCEPARENT "SourceParent"
    #define SYNCML_ELEMENT_TARGETPARENT "TargetParent"
    #define SYNCML_ELEMENT_LOCURI "LocURI"
    #define SYNCML_ELEMENT_DATA "Data"
    #define SYNCML_ELEMENT_DEVINF "DevInf"
    #define SYNCML_ELEMENT_PUT "Put"
    #define SYNCML_ELEMENT_GET "Get"
    #define SYNCML_ELEMENT_MAN "Man"
    #define SYNCML_ELEMENT_MOD "Mod"
    #define SYNCML_ELEMENT_FWVERSION "FwV"
    #define SYNCML_ELEMENT_SWVERSION "SwV"
    #define SYNCML_ELEMENT_HWVERSION "HwV"
    #define SYNCML_ELEMENT_DEVID "DevID"
    #define SYNCML_ELEMENT_DEVTYPE "DevTyp"
    #define SYNCML_ELEMENT_UTC "UTC"
    #define SYNCML_ELEMENT_SUPPORTLARGEOBJS "SupportLargeObjs"
    #define SYNCML_ELEMENT_SUPPORTNUMBEROFCHANGES "SupportNumberOfChanges"
    #define SYNCML_ELEMENT_DATASTORE "DataStore"
    #define SYNCML_ELEMENT_SOURCEREF "SourceRef"
    #define SYNCML_ELEMENT_TARGETREF "TargetRef"
    #define SYNCML_ELEMENT_MAXGUIDSIZE "MaxGUIDSize"
    #define SYNCML_ELEMENT_RX_PREF "Rx-Pref"
    #define SYNCML_ELEMENT_TX_PREF "Tx-Pref"
    #define SYNCML_ELEMENT_RX "Rx"
    #define SYNCML_ELEMENT_TX "Tx"
    #define SYNCML_ELEMENT_CTCAP "CTCap"
    #define SYNCML_ELEMENT_CTCAPS "CTCaps"
    #define SYNCML_ELEMENT_CTTYPE "CTType"
    #define SYNCML_ELEMENT_VERCT "VerCT"
    #define SYNCML_ELEMENT_SYNCCAP "SyncCap"
    #define SYNCML_ELEMENT_SYNCTYPE "SyncType"
    #define SYNCML_ELEMENT_NUMOFCHANGES "NumberOfChanges"
    #define SYNCML_ELEMENT_FINAL "Final"
    #define SYNCML_ELEMENT_CRED "Cred"
    #define SYNCML_ELEMENT_SESSIONID "SessionID"
    #define SYNCML_ELEMENT_MSGID "MsgID"
    #define SYNCML_ELEMENT_MOREDATA "MoreData"
    #define SYNCML_ELEMENT_MAP "Map"
    #define SYNCML_ELEMENT_MAPITEM "MapItem"
    #define SYNCML_ELEMENT_RESULTS "Results"
    #define SYNCML_ELEMENT_MAXOBJSIZE "MaxObjSize"
    #define SYNCML_ELEMENT_MAXMSGSIZE "MaxMsgSize"
    #define SYNCML_ELEMENT_RESPURI "RespURI"
    #define SYNCML_ELEMENT_MSGREF "MsgRef"
    #define SYNCML_ELEMENT_CMDREF "CmdRef"
    #define SYNCML_ELEMENT_CMD "Cmd"
    #define SYNCML_ELEMENT_NORESP "NoResp"
    #define SYNCML_ELEMENT_META "Meta"
    #define SYNCML_ELEMENT_ATOMIC "Atomic"
    #define SYNCML_ELEMENT_COPY "Copy"
    #define SYNCML_ELEMENT_MOVE "Move"
    #define SYNCML_ELEMENT_SEQUENCE "Sequence"
    #define SYNCML_ELEMENT_LAST "Last"
    #define SYNCML_ELEMENT_NEXT "Next"
    #define SYNCML_ELEMENT_LANG "Lang"
    #define SYNCML_ELEMENT_FORMAT "Format"
    #define SYNCML_ELEMENT_SIZE "Size"
    #define SYNCML_ELEMENT_TYPE "Type"
    #define SYNCML_ELEMENT_ANCHOR "Anchor"
    #define SYNCML_ELEMENT_VERSION "Version"
    #define SYNCML_ELEMENT_EMI "EMI"
    #define SYNCML_ELEMENT_CHAL "Chal"
    #define SYNCML_ELEMENT_NEXTNONCE "NextNonce"

    #define SYNCML_CONTENT_TYPE "Content-Type"
    #define SYNCML_CONTTYPE_DEVINF_XML "application/vnd.syncml-devinf+xml"
    #define SYNCML_CONTTYPE_XML "application/vnd.syncml+xml"
    #define SYNCML_CONTTYPE_WBXML "application/vnd.syncml+wbxml"
    #define SYNCML_CONTTYPE_SAN "application/vnd.syncml.ds.notification"

    #define SYNCML_DEVINFO_MANUFACTURER "Nokia"
    #define SYNCML_DEVINFO_MODEL ""
    #define SYNCML_DEVINFO_FW_VERSION ""
    #define SYNCML_DEVINFO_SW_VERSION ""
    #define SYNCML_DEVINFO_HW_VERSION ""
    #define SYNCML_DEVINFO_DEVID ""
    #define SYNCML_DEVINFO_DEVTYPE "phone"
    #define SYNCML_DSINFO_MAXGUIDSIZE "32"
    #define SYNCML_DEVINF_PATH_11 "./devinf11"
    #define SYNCML_DEVINF_PATH_12 "./devinf12"

    #define XML_NAMESPACE          "xmlns"
    #define XML_NAMESPACE_VALUE_SYNCML11  "SYNCML:SYNCML1.1"
    #define XML_NAMESPACE_VALUE_SYNCML12  "SYNCML:SYNCML1.2"
    #define XML_NAMESPACE_VALUE_DEVINF    "syncml:devinf"
    #define XML_NAMESPACE_VALUE_METINF    "syncml:metinf"
    #define SYNCML_DTD_VERSION_1_2     "1.2"
    #define SYNCML12_VERPROTO "SyncML/1.2"

    #define SYNCML_DTD_VERSION_1_1     "1.1"
    #define SYNCML11_VERPROTO "SyncML/1.1"
    #define SYNCML_UNKNOWN_DEVICE "/"
    #define SYNCML_URI_PREFIX "./"

    #define SYNCML_FORMAT_ENCODING_B64 "b64"

    // Silly Ovi.com developers don't want to follow standards. This encoding
    // is the same as "b64"
    #define OVI_FORMAT_ENCODING_B64 "bin"

    #define SYNCML_FORMAT_AUTH_BASIC "syncml:auth-basic"
    #define SYNCML_FORMAT_AUTH_MD5 "syncml:auth-md5"

    #define SYNCML_SCHEMA_HTTP "http"
    #define SYNCML_SCHEMA_HTTPS "https"

    #define HTTP_HDRSTR_POST "POST"
    #define HTTP_HDRSTR_UA "User-Agent"
    #define HTTP_UA_VALUE "libmeegosyncml"
    #define HTTP_HDRSTR_ACCEPT "Accept"
    #define HTTP_ACCEPT_VALUE  "*/*"

    #define DEFAULT_MAX_CHANGES_TO_SEND 22
    #define DEFAULT_MAX_MESSAGESIZE     16384
    #define DEFAULT_OBEX_MTU            1024

    #define MAXMSGOVERHEADRATIO      0.1f
    #define MINMSGOVERHEADBYTES      256
    #define WBXMLCOMPRESSIONRATE     0.66f

} // end namespace DataSync

#endif // DATATYPES_H



