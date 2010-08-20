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
#ifndef SYNCAGENTCONFIGPROPERTIES_H
#define SYNCAGENTCONFIGPROPERTIES_H

namespace DataSync {

// Property to control maximum size of accepted messages
const QString MAXMESSAGESIZEPROP( "max-message-size" );

// Property to control the maximum number of changes to send per message
const QString MAXCHANGESPERMESSAGEPROP( "max-changes-per-message" );

// Property to control the conflict resolution policy
const QString CONFLICTRESOLUTIONPOLICYPROP( "conflict-resolution-policy" );

// Property to control whether item ID mappings should be sent fast
// (alongside Status of the operation in Pck#4), or separately in Pck#5
const QString FASTMAPSSENDPROP( "fast-maps-send" );

// Property to control the maximum transfer unit of OBEX over BT
const QString BTOBEXMTUPROP( "bt-obex-mtu" );

// Property to control the maximum transfer unit of OBEX over USB
const QString USBOBEXMTUPROP( "usb-obex-mtu" );

// Property to control the number of times the sending of first message is
// attempted
const QString HTTPNUMBEROFRESENDATTEMPTSPROP( "http-number-of-resend-attempts" );

// Property to control the host address of http proxy
const QString HTTPPROXYHOSTPROP( "http-proxy-host" );

// Property to control the port of http proxy
const QString HTTPPROXYPORTPROP( "http-proxy-port" );

// Property to control EMI tags extension
const QString EMITAGSEXTENSION( "emi-tags" );

// Property to control sync without initialization phase extension
const QString SYNCWITHOUTINITPHASEEXTENSION( "sync-without-init-phase" );

}

#endif  //   SYNCAGENTCONFIGPROPERTIES_H
