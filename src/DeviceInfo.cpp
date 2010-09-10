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

#include "DeviceInfo.h"

#include <QXmlStreamReader>
#include <QFile>

#include "LogMacros.h"
#include "datatypes.h"

using namespace DataSync;

const QString XML_KEY_MANUFACTURER("Manufacturer");
const QString XML_KEY_MODEL("Model");
const QString XML_KEY_OEM("OEM");
const QString XML_KEY_FW_VER("FwVersion");
const QString XML_KEY_HW_VER("HwVersion");
const QString XML_KEY_SW_VER("SwVersion");
const QString XML_KEY_ID("Id");
const QString XML_KEY_DEVICE_TYPE("DeviceType");


DeviceInfo::DeviceInfo()
{
    FUNCTION_CALL_TRACE
}

DeviceInfo::~DeviceInfo()
{
    FUNCTION_CALL_TRACE
}

const QString& DeviceInfo::getManufacturer() const
{
    return iManufacturer;
}

void DeviceInfo::setManufacturer(const QString &aManufacturer)
{
    iManufacturer = aManufacturer;
}

const QString& DeviceInfo::getModel() const
{
    return iModel;
}

void DeviceInfo::setModel(const QString &aModel)
{
    iModel = aModel;
}

void DeviceInfo::setOEM( const QString& aOEM )
{
    iOEM = aOEM;
}

const QString& DeviceInfo::getOEM() const
{
    return iOEM;
}

const QString& DeviceInfo::getFirmwareVersion() const
{
    return iFirmwareVersion;
}

void DeviceInfo::setFirmwareVersion(const QString &aFirmwareVersion)
{
    iFirmwareVersion = aFirmwareVersion;
}

const QString& DeviceInfo::getSoftwareVersion() const
{
    return iSoftwareVersion;
}

void  DeviceInfo::setSoftwareVersion(const QString& aSoftwareVersion)
{
    iSoftwareVersion = aSoftwareVersion;
}

const QString& DeviceInfo::getHardwareVersion() const
{
    return iHardwareVersion;
}

void DeviceInfo::setHardwareVersion(const QString& aHardwareVersion)
{
    iHardwareVersion = aHardwareVersion;
}

void DeviceInfo::setDeviceID( const QString& aDeviceID )
{
    iDeviceID = aDeviceID;
}

const QString& DeviceInfo::getDeviceID() const
{
    return iDeviceID;
}

const QString& DeviceInfo::getDeviceType() const
{
    return iDeviceType;
}

void DeviceInfo::setDeviceType(const QString& aDeviceType)
{
    iDeviceType = aDeviceType;
}

bool DeviceInfo::readFromFile(const QString &aFileName)
{
    FUNCTION_CALL_TRACE;

    QFile file(aFileName);

    if( !file.open(QIODevice::ReadOnly) )
    {
        LOG_WARNING("Failed open file:" << aFileName);
        return false;
    }

    QXmlStreamReader reader(file.readAll());

    while(!reader.atEnd()) {

        if(reader.tokenType() == QXmlStreamReader::StartElement) {
            if(reader.name() == XML_KEY_MANUFACTURER) {
                reader.readNext();
                setManufacturer(reader.text().toString());
            }else if (reader.name() == XML_KEY_MODEL) {
                reader.readNext();
                setModel(reader.text().toString());
            }else if (reader.name() == XML_KEY_OEM) {
                reader.readNext();
                setOEM(reader.text().toString());
            }else if (reader.name() == XML_KEY_SW_VER){
                reader.readNext();
                setSoftwareVersion(reader.text().toString());
            }else if (reader.name() == XML_KEY_HW_VER) {
                reader.readNext();
                setHardwareVersion(reader.text().toString());
            } else if (reader.name() == XML_KEY_FW_VER) {
                reader.readNext();
                setFirmwareVersion(reader.text().toString());
            } else if(reader.name() == XML_KEY_ID) {
                reader.readNext();
                setDeviceID(reader.text().toString());
            } else if (reader.name() == XML_KEY_DEVICE_TYPE ) {
                reader.readNext();
                setDeviceType(reader.text().toString());
            } else {
                LOG_WARNING("Ignoring unknown element:" << reader.name() );
            }
        }

        reader.readNext();
    }

    file.close();

    return true;

}
