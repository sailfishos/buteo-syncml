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
#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>

namespace DataSync
{

/*! \brief DeviceInfo includes information about the device
 *
 */
class DeviceInfo
{
public:
    /*! \brief Constructor
     *
     */
    DeviceInfo();

    /*! \brief Destructor
     *
     */
    ~DeviceInfo();

    /*! \brief Sets the globally unique device identification of this device
     *
     * @param aDeviceID Device identification
     */
    void setDeviceID( const QString& aDeviceID );

    /*! \brief Returns the globally unique device identification of this device
     *
     * @return
     */
    const QString& getDeviceID() const;


    /*! \brief Sets the Manufacturer
     *
     * @param aManufacturer Device manufacturer
     */
    void setManufacturer( const QString& aManufacturer );

    /*! \brief Returns the device manufacturer
     *
     * @return
     */
    const QString& getManufacturer() const;


    /*! \brief Sets the Model
     *
     * @param aModel
     */
    void setModel( const QString& aModel );

    /*! \brief Returns the device model
     *
     * @return
     */
    const QString& getModel() const;

    /*! \brief Sets the FirmwareVersion
     *
     * @param aFirmwareVersion
     */
    void setFirmwareVersion( const QString& aFirmwareVersion );

    /*! \brief Returns the firmware version of the device
     *
     * @return
     */
    const QString& getFirmwareVersion() const;

    /*! \brief Sets the SoftwareVersion
     *
     * @param aSoftwareVersion
     */
    void setSoftwareVersion( const QString& aSoftwareVersion );

    /*! \brief Returns the software version of the device
     *
     * @return
     */
    const QString& getSoftwareVersion() const;

    /*! \brief Sets the HardwareVersion
     *
     * @param aHardwareVersion
     */
    void setHardwareVersion( const QString& aHardwareVersion );

    /*! \brief Returns the hardware version of the device
     *
     * @return
     */
    const QString& getHardwareVersion() const;

    /*! \brief Sets the DeviceType
     *
     * @param aDeviceType
     */
    void setDeviceType( const QString& aDeviceType );

    /*! \brief Returns the type of the device.
     *
     * Possible device types are enumerated in OMA DS Device
     * Information document in section 5.3.7
     *
     * @return
     */
    const QString& getDeviceType() const;


    /*! \brief Returns the type of the device.
     *
     * Possible device types are enumerated in OMA DS Device
     * Information document in section 5.3.7
     *
     * @return
     */
    bool readFromFile(QString &aFileName);

protected:

private:

    QString iDeviceID;
    QString iManufacturer;
    QString iModel;
    QString iFirmwareVersion;
    QString iSoftwareVersion;
    QString iHardwareVersion;
    QString iDeviceType;

};


}


#endif  //  DEVICEINFO_H
