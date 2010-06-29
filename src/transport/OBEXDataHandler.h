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

#ifndef OBEXDATAHANDLER_H
#define OBEXDATAHANDLER_H

#include <openobex/obex.h>

#include <QString>

namespace DataSync {

/*! \brief Utility class for doing various OBEX operations
 *
 */
class OBEXDataHandler
{

public:
    /*! \brief Constructor
     *
     */
    OBEXDataHandler();

    /*! \brief Destructor
     *
     */
    virtual ~OBEXDataHandler();

    /*! \brief Structure for sending SyncML Connect
     *
     */
    struct ConnectCmdData
    {
        QByteArray      iTarget;        ///< OBEX Target header
    };

    /*! \brief Structure for responding to SyncML Connect
     *
     */
    struct ConnectRspData
    {
        unsigned int    iConnectionId;  ///< OBEX Connection header
        QByteArray      iWho;           ///< OBEX Who header
    };

    /*! \brief Structure for sending SyncML Disconnect
     *
     */
    struct DisconnectCmdData
    {
        unsigned int    iConnectionId;  ///< OBEX Connection header
    };

    /*! \brief Structure for sending SyncML Put
     *
     */
    struct PutCmdData
    {
        unsigned int    iConnectionId;          ///< OBEX Connection header
        QByteArray      iContentType;           ///< OBEX Type header
        int             iLength;                ///< OBEX Length header
        QByteArray      iBody;                  ///< OBEX Body header
        bool            iUnsupportedHeaders;    ///< True if PUT included unsupported headers
    };

    /*! \brief Structure for sending SyncML Get
     *
     */
    struct GetCmdData
    {
        unsigned int    iConnectionId;  ///< OBEX Connection header
        QByteArray      iContentType;   ///< OBEX Type header
    };

    /*! \brief Structure for responding to SyncML Get
     *
     */
    struct GetRspData
    {
        unsigned int    iLength;        ///< OBEX Length header
        QByteArray      iBody;          ///< OBEX Body header
    };

    /*! \brief Generate OBEX object representing SyncML Connect
     *
     * @param aHandle OBEX handle
     * @param aData SyncML Connect data
     * @return OBEX object if successful, otherwise NULL
     */
    obex_object_t* createConnectCmd( obex_t* aHandle, const ConnectCmdData& aData );

    /*! \brief Parse OBEX object representing SyncML Connect
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid SyncML Connect, otherwise false
     */
    bool parseConnectCmd( obex_t* aHandle, obex_object_t* aObject, ConnectCmdData& aData );

    /*! \brief Generate a response object for SyncML Connect
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object to append data to
     * @param aData SyncML Connect response data
     * @return True if operation was successful, false otherwise
     */
    bool createConnectRsp( obex_t* aHandle, obex_object_t* aObject, const ConnectRspData& aData );

    /*! \brief Parse OBEX object representing response to SyncML Connect
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid response to SyncML Connect, otherwise false
     */
    bool parseConnectRsp( obex_t* aHandle, obex_object_t* aObject, ConnectRspData& aData );

    /*! \brief Generate OBEX object representing SyncML Disconnect
     *
     * @param aHandle OBEX handle
     * @param aData SyncML Disconnect data
     * @return OBEX object if successful, otherwise NULL
     */
    obex_object_t* createDisconnectCmd( obex_t* aHandle, const DisconnectCmdData& aData );

    /*! \brief Parse OBEX object representing SyncML Disconnect
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid SyncML Disconnect, otherwise false
     */
    bool parseDisconnectCmd( obex_t* aHandle, obex_object_t* aObject, DisconnectCmdData& aData );

    /*! \brief Generate OBEX object representing SyncML Put
     *
     * @param aHandle OBEX handle
     * @param aData SyncML Put data
     * @return OBEX object if successful, otherwise NULL
     */
    obex_object_t* createPutCmd( obex_t* aHandle, const PutCmdData& aData );

    /*! \brief Parse OBEX object representing SyncML Put
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid SyncML Put, otherwise false
     */
    bool parsePutCmd( obex_t* aHandle, obex_object_t* aObject, PutCmdData& aData );

    /*! \brief Generate OBEX object representing SyncML Get
     *
     * @param aHandle OBEX handle
     * @param aData SyncML Get data
     * @return OBEX object if successful, otherwise NULL
     */
    obex_object_t* createGetCmd( obex_t* aHandle, const GetCmdData& aData );

    /*! \brief Parse OBEX object representing SyncML Get
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid SyncML Get, otherwise false
     */
    bool parseGetCmd( obex_t* aHandle, obex_object_t* aObject, GetCmdData& aData );

    /*! \brief Generate a response object for SyncML Get
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object to append data to
     * @param aData SyncML Get response data
     * @return True if operation was successful, otherwise false
     */
    bool createGetRsp( obex_t* aHandle, obex_object_t* aObject, const GetRspData& aData );

    /*! \brief Parse OBEX object representing response to SyncML Get
     *
     * @param aHandle OBEX handle
     * @param aObject OBEX object
     * @param aData Structure where to place parsing results
     * @return True if object is valid response to SyncML Get, otherwise false
     */
    bool parseGetRsp( obex_t* aHandle, obex_object_t* aObject, GetRspData& aData );

};

}
#endif // OBEXDATAHANDLER_H
