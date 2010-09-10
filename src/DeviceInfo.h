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

/*! \brief DeviceInfo includes information about a device
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

    /*! \brief Sets the device manufacturer
     *
     * @param aManufacturer Device manufacturer
     */
    void setManufacturer( const QString& aManufacturer );

    /*! \brief Returns the manufacturer
     *
     * @return
     */
    const QString& getManufacturer() const;

    /*! \brief Sets the device model
     *
     * @param aModel Device model
     */
    void setModel( const QString& aModel );

    /*! \brief Returns the device model
     *
     * @return
     */
    const QString& getModel() const;

    /*! \brief Sets the device OEM
     *
     * @param aOEM Device OEM
     */
    void setOEM( const QString& aOEM );

    /*! \brief Returns the OEM
     *
     * @return
     */
    const QString& getOEM() const;

    /*! \brief Sets the device firmware version
     *
     * @param aFirmwareVersion Device firmware version
     */
    void setFirmwareVersion( const QString& aFirmwareVersion );

    /*! \brief Returns the device firmware version
     *
     * @return
     */
    const QString& getFirmwareVersion() const;

    /*! \brief Sets the software version
     *
     * @param aSoftwareVersion Device software version
     */
    void setSoftwareVersion( const QString& aSoftwareVersion );

    /*! \brief Returns the device software version
     *
     * @return
     */
    const QString& getSoftwareVersion() const;

    /*! \brief Sets the device hardware version
     *
     * @param aHardwareVersion Device hardware version
     */
    void setHardwareVersion( const QString& aHardwareVersion );

    /*! \brief Returns the device hardware version
     *
     * @return
     */
    const QString& getHardwareVersion() const;

    /*! \brief Sets the device type
     *
     * Possible device types are enumerated in OMA DS Device
     * Information document in section 5.3.7
     *
     * @param aDeviceType Device type
     */
    void setDeviceType( const QString& aDeviceType );

    /*! \brief Returns the device type
     *
     * Possible device types are enumerated in OMA DS Device
     * Information document in section 5.3.7
     *
     * @return
     */
    const QString& getDeviceType() const;

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

    /*! \brief Read device information from a file
     *
     * @param aFileName File to read
     * @return True on success, otherwise false
     */
    bool readFromFile(const QString &aFileName);

private:

    QString iDeviceID;
    QString iManufacturer;
    QString iModel;
    QString iOEM;
    QString iFirmwareVersion;
    QString iSoftwareVersion;
    QString iHardwareVersion;
    QString iDeviceType;

};


}


#endif  //  DEVICEINFO_H
