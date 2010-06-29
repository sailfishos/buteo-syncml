/*
* This file is part of meego-syncml package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "OBEXBTConnection.h"

#include <QtDBus>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#include "LogMacros.h"

using namespace DataSync;

#define BLUEZ_DEST "org.bluez"
#define BLUEZ_MANAGER_INTERFACE "org.bluez.Manager"
#define BLUEZ_ADAPTER_INTERFACE "org.bluez.Adapter"
#define BLUEZ_SERIAL_INTERFACE "org.bluez.Serial"
#define REQUEST_SESSION "RequestSession"
#define RELEASE_SESSION "ReleaseSession"
#define GET_DEFAULT_ADAPTER "DefaultAdapter"
#define FIND_DEVICE "FindDevice"
#define CONNECT "Connect"
#define DISCONNECT "Disconnect"

#define MAX_MTU 16384


OBEXBTConnection::OBEXBTConnection( const QString& aBTAddress,
                                    const QString& aServiceUUID )
 : OBEXConnection( MAX_MTU ),
   iBTAddress( aBTAddress ),
   iServiceUUID( aServiceUUID ),
   iFd( -1 )
{
    FUNCTION_CALL_TRACE;
}

OBEXBTConnection::OBEXBTConnection( int aFd )
 : OBEXConnection( MAX_MTU ),
   iFd( aFd )
{
    FUNCTION_CALL_TRACE;
}

OBEXBTConnection::~OBEXBTConnection()
{
    FUNCTION_CALL_TRACE;
}

int OBEXBTConnection::connectLink()
{
    FUNCTION_CALL_TRACE;

    if( iFd != -1 ) {
        LOG_DEBUG( "Using existing connection" );
        return iFd;
    }

    iDevice = connectDevice( iBTAddress, iServiceUUID );

    if( iDevice.isEmpty() ) {
        LOG_CRITICAL("Could not connect to device" << iBTAddress << ", aborting" );
        return -1;
    }

    iFd = open( iDevice.toAscii().constData(), O_RDWR | O_NOCTTY | O_SYNC );

    if( iFd == -1 ) {
        LOG_CRITICAL( "Could not open file descriptor of the connection, aborting" );
        disconnectDevice( iBTAddress, iDevice );
        return -1;
    }

    fdRawMode( iFd );

    return iFd;

}

void OBEXBTConnection::disconnectLink()
{
    FUNCTION_CALL_TRACE;

    if( iFd != -1 ) {
        close( iFd );
    }

    if( !iDevice.isEmpty() ) {
        disconnectDevice( iBTAddress, iDevice );
    }

}

QString OBEXBTConnection::connectDevice( const QString& aBTAddress, const QString& aServiceUUID )
{
    FUNCTION_CALL_TRACE;

    QDBusInterface managerInterface( BLUEZ_DEST, "/", BLUEZ_MANAGER_INTERFACE, QDBusConnection::systemBus() );

    if( !managerInterface.isValid() ) {
        LOG_CRITICAL( "Could not find BlueZ manager interface" );
        return "";
    }

    QDBusReply<QDBusObjectPath> pathReply = managerInterface.call( QLatin1String( GET_DEFAULT_ADAPTER ) );

    if( !pathReply.isValid() ) {
        LOG_CRITICAL( "Could not find default adapter path:" << pathReply.error() );
        return "";
    }

    QString defaultAdapterPath = pathReply.value().path();

    LOG_DEBUG("Using adapter path: " << defaultAdapterPath );

    QDBusInterface adapterInterface( BLUEZ_DEST, defaultAdapterPath, BLUEZ_ADAPTER_INTERFACE, QDBusConnection::systemBus() );

    if( !adapterInterface.isValid() ) {
        LOG_CRITICAL( "Could not find adapter interface: " << adapterInterface.lastError() );
        return "";
    }

    QDBusReply<void> voidReply = adapterInterface.call( QLatin1String( REQUEST_SESSION ) );

    if( !voidReply.isValid() ) {
        LOG_CRITICAL( "Session request failed" );
        LOG_CRITICAL( "Reason:" <<  voidReply.error() );
        return "";
    }

    LOG_DEBUG( "BT session created" );

    pathReply = adapterInterface.call( QLatin1String( FIND_DEVICE ), aBTAddress );

    if( !pathReply.isValid() ) {
        LOG_CRITICAL( "Couldn't find device " << aBTAddress );
        LOG_CRITICAL( "Reason:" <<  pathReply.error() );
        adapterInterface.call( QLatin1String( RELEASE_SESSION ) );
        LOG_CRITICAL( "BT session closed" );
        return "";
    }

    QString devicePath = pathReply.value().path();

    LOG_DEBUG( "Using path" << devicePath << "for device " << aBTAddress );

    QDBusInterface serialInterface( BLUEZ_DEST, devicePath, BLUEZ_SERIAL_INTERFACE, QDBusConnection::systemBus() );

    if( !serialInterface.isValid() ) {
        LOG_CRITICAL( "Could not find serial interface: " << serialInterface.lastError() );
        adapterInterface.call( QLatin1String( RELEASE_SESSION ) );
        LOG_CRITICAL( "BT session closed" );
        return "";
    }

    QDBusReply<QString> stringReply = serialInterface.call( QLatin1String( CONNECT ), aServiceUUID );

    if( !stringReply.isValid() ) {
        LOG_CRITICAL( "Could not connect to device " << devicePath << " with service uuid " << aServiceUUID );
        LOG_CRITICAL( "Reason:" <<  stringReply.error() );
        adapterInterface.call( QLatin1String( RELEASE_SESSION ) );
        LOG_CRITICAL( "BT session closed" );
        return "";
    }

    LOG_DEBUG("Device connected:" << aBTAddress );

    return stringReply.value();

}

void OBEXBTConnection::disconnectDevice( const QString& aBTAddress, const QString& aDevice )
{
    FUNCTION_CALL_TRACE;

    QDBusInterface managerInterface( BLUEZ_DEST, "/", BLUEZ_MANAGER_INTERFACE, QDBusConnection::systemBus() );

    if( !managerInterface.isValid() ) {
        LOG_CRITICAL( "Could not find BlueZ manager interface" );
        return;
    }

    QDBusReply<QDBusObjectPath> pathReply = managerInterface.call( QLatin1String( GET_DEFAULT_ADAPTER ) );

    if( !pathReply.isValid() ) {
        LOG_CRITICAL( "Could not find default adapter path" );
        LOG_CRITICAL( "Reason:" <<  pathReply.error() );
        return;
    }

    QString defaultAdapterPath = pathReply.value().path();

    LOG_DEBUG("Using adapter path: " << defaultAdapterPath );

    QDBusInterface adapterInterface( BLUEZ_DEST, defaultAdapterPath, BLUEZ_ADAPTER_INTERFACE, QDBusConnection::systemBus() );

    if( !adapterInterface.isValid() ) {
        LOG_CRITICAL( "Could not find adapter interface: " << adapterInterface.lastError() );
        return;
    }

    pathReply = adapterInterface.call( QLatin1String( FIND_DEVICE ), aBTAddress );

    if( !pathReply.isValid() ) {
        LOG_CRITICAL( "Couldn't find device " << aBTAddress );
        LOG_CRITICAL( "Reason:" <<  pathReply.error() );
        return;
    }

    QString devicePath = pathReply.value().path();

    LOG_DEBUG( "Using path" << devicePath << "for device " << aBTAddress );

    QDBusInterface serialInterface( BLUEZ_DEST, devicePath, BLUEZ_SERIAL_INTERFACE, QDBusConnection::systemBus() );

    if( !serialInterface.isValid() ) {
        LOG_CRITICAL( "Could not find serial interface: " << serialInterface.lastError() );
        return;
    }

    QDBusReply<void> voidReply = serialInterface.call( QLatin1String( DISCONNECT ), aDevice );

    if( !voidReply.isValid() ) {
        LOG_CRITICAL( "Device disconnection failed" );
        LOG_CRITICAL( "Reason:" <<  voidReply.error() );
        return;
    }

    LOG_DEBUG( "Device disconnected:" << aBTAddress );

    voidReply = adapterInterface.call( RELEASE_SESSION );

    if( !voidReply.isValid() ) {
        LOG_CRITICAL( "Session release failed" );
        LOG_CRITICAL( "Reason:" <<  voidReply.error() );
        return;
    }

    LOG_DEBUG( "BT session closed" );

    iDevice.clear();

}

bool OBEXBTConnection::fdRawMode( int aFD )
{
    FUNCTION_CALL_TRACE;

    struct termios mode;

    if (tcgetattr(aFD, &mode)) {
        return false;
    }

    cfmakeraw(&mode);

    if (tcsetattr(aFD, TCSADRAIN, &mode)) {
        return false;
    }

    return true;
}