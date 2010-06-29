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

#include "DeviceInfo.h"

#include <QXmlStreamReader>
#include <QFile>

#include "datatypes.h"

#include "LogMacros.h"

using namespace DataSync;

const QString XML_KEY_ID("Id");

const QString XML_KEY_MODEL("Model");

const QString XML_KEY_HW_VER("HwVersion");

const QString XML_KEY_SW_VER("SwVersion");

const QString XML_KEY_FW_VER("FwVersion");

const QString XML_KEY_DEVICE_TYPE("DeviceType");


DeviceInfo::DeviceInfo()
: iManufacturer( SYNCML_DEVINFO_MANUFACTURER ), iModel( SYNCML_DEVINFO_MODEL),
  iFirmwareVersion(SYNCML_DEVINFO_FW_VERSION), iSoftwareVersion(SYNCML_DEVINFO_SW_VERSION),
  iHardwareVersion(SYNCML_DEVINFO_HW_VERSION), iDeviceType( SYNCML_DEVINFO_DEVTYPE)
{
    FUNCTION_CALL_TRACE
}

DeviceInfo::~DeviceInfo()
{
    FUNCTION_CALL_TRACE
}

void DeviceInfo::setDeviceID( const QString& aDeviceID )
{
    FUNCTION_CALL_TRACE;
    iDeviceID = aDeviceID;
}

const QString& DeviceInfo::getDeviceID() const
{
    FUNCTION_CALL_TRACE;
    return iDeviceID;
}

const QString& DeviceInfo::getManufacturer() const
{
    FUNCTION_CALL_TRACE;

    return iManufacturer;
}

void DeviceInfo::setManufacturer(const QString &aManufacturer)
{
    FUNCTION_CALL_TRACE;

    iManufacturer = aManufacturer;
}

const QString& DeviceInfo::getModel() const
{
    FUNCTION_CALL_TRACE;

    return iModel;
}


void DeviceInfo::setModel(const QString &aModel)
{
    FUNCTION_CALL_TRACE;

    iModel = aModel;
}

const QString& DeviceInfo::getFirmwareVersion() const
{
    FUNCTION_CALL_TRACE;

    return iFirmwareVersion;
}

void DeviceInfo::setFirmwareVersion(const QString &aFirmwareVersion)
{
    FUNCTION_CALL_TRACE;

    iFirmwareVersion = aFirmwareVersion;
}

const QString& DeviceInfo::getSoftwareVersion() const
{
    FUNCTION_CALL_TRACE;

    return iSoftwareVersion;
}

void  DeviceInfo::setSoftwareVersion(const QString& aSoftwareVersion)
{
    FUNCTION_CALL_TRACE;

    iSoftwareVersion = aSoftwareVersion;
}

const QString& DeviceInfo::getHardwareVersion() const
{
    FUNCTION_CALL_TRACE;

    return iHardwareVersion;
}

void DeviceInfo::setHardwareVersion(const QString& aHardwareVersion)
{
    FUNCTION_CALL_TRACE;

    iHardwareVersion = aHardwareVersion;
}

const QString& DeviceInfo::getDeviceType() const
{
    FUNCTION_CALL_TRACE;

    return iDeviceType;
}

void DeviceInfo::setDeviceType(const QString& aDeviceType)
{
    FUNCTION_CALL_TRACE;

    iDeviceType = aDeviceType;
}


bool DeviceInfo::readFromFile(QString &aFileName)
{
    FUNCTION_CALL_TRACE;
    bool status = false;
    QFile file(aFileName);

    if(file.open(QIODevice::ReadOnly)){
        QXmlStreamReader reader(file.readAll());
        // @TODO - Do Xml Validation here
        while(!reader.atEnd()) {

            if(reader.tokenType() == QXmlStreamReader::StartElement) {
                if(reader.name() == XML_KEY_ID) {
                    reader.readNext();
                    QString id = reader.text().toString();
                    setDeviceID(id);
                    LOG_DEBUG("Device Id read " << id);
                }else if (reader.name() == XML_KEY_MODEL) {
                    reader.readNext();
                    setModel(reader.text().toString());
                }else if (reader.name() == XML_KEY_SW_VER){
                    reader.readNext();
                    setSoftwareVersion(reader.text().toString());
                }else if (reader.name() == XML_KEY_HW_VER) {
                    reader.readNext();
                    setHardwareVersion(reader.text().toString());
                } else if (reader.name() == XML_KEY_FW_VER) {
                    reader.readNext();
                    setFirmwareVersion(reader.text().toString());
                } else if (reader.name() == XML_KEY_DEVICE_TYPE ) {
                    reader.readNext();
                    setDeviceType(reader.text().toString());
                } else {
                    LOG_DEBUG("Unknown Element Read" << reader.name());
                }
            }

            reader.readNext();
        }
        status = true;
    } else {
        LOG_DEBUG("Failed opening the device info file " << aFileName);
    }
    return status;
}
