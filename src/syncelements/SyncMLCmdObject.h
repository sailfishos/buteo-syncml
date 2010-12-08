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

#ifndef SYNCMLCMDOBJECT_H
#define SYNCMLCMDOBJECT_H

#include <QString>
#include <QMap>

#include "SyncAgentConsts.h"

namespace DataSync {

/*! \brief SyncMLCmdObject is the base class for generating
 *         SyncML message tree
 *
 */
class SyncMLCmdObject {

public:

	/*! \brief Constructor
	 *
     * @param aName Name of this element
     * @param aValue Value of this element
     */
	explicit SyncMLCmdObject( const QString& aName = "", const QString& aValue = "" );

	/*! \brief Destructor
	 *
	 */
	virtual ~SyncMLCmdObject();

	/*! \brief Returns the name of the XML element represented by this object
	 *
	 * @return Name of the XML element
	 */
	const QString& getName() const;

    /*! \brief Sets the name of the XML element represented by this object
     *
     * @param aName Name of the XML element
     */
	void setName( const QString& aName );

    /*! \brief Returns the value of the XML element represented by this object
     *
     * @return Value of the XML element
     */
	const QString& getValue() const;

    /*! \brief Sets the value of the XML element represented by this object
     *
     * @param aValue Value of the XML element
     */
	void setValue( const QString& aValue );

	/*! \brief Returns whether the value of the XML element should be written as CDATA
	 *
	 * @return True if value of XML element should be written as CDATA, otherwise false
	 */
	bool getCDATA() const;

	/*! \brief Sets whether the value of the XML element should be written as CDATA
	 *
	 * @param aCDATA True if value of XML element should be written as CDATA, otherwise false
	 */
	void setCDATA( bool aCDATA );

	/*! \brief Adds an XML attribute
	 *
	 * @param aName Name of the attribute to add
	 * @param aValue Value of the attribute to addl
	 */
	void addAttribute( const QString& aName, const QString& aValue );

	/*! \brief Returns the XML attributes
	 *
	 * @return XML attributes
	 */
	const QMap<QString, QString>& getAttributes() const;

	/*! \brief Adds a child to this element
	 *
	 * @param aChild
	 */
	void addChild( SyncMLCmdObject* aChild );

	/*! \brief Returns the children of this object
	 *
	 * @return Children of this object
	 */
	const QList<SyncMLCmdObject*>& getChildren() const;

	/*! \brief Estimate the size of the present object when formatted as XML object
	 *
	 * @return Estimated size of the object, including all child objects
	 */
    int calculateSize(bool aWbXML, const ProtocolVersion& aVersion);

protected:

private:

    QString                 iName;

    QString                 iValue;
    bool                    iIsCDATA;

    QMap<QString, QString>  iAttributes;

    QList<SyncMLCmdObject*> iChildren;


};

}

Q_DECLARE_TYPEINFO(DataSync::SyncMLCmdObject,Q_COMPLEX_TYPE);

#endif // SYNCMLCMDOBJECT_H
