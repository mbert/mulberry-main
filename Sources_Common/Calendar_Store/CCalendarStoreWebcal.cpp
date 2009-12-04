/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
	CCalendarStoreWebcal.cpp

	Author:
	Description:	maintains state for a publish/subscribe webcal
*/

#include "CCalendarStoreWebcal.h"

#include "CCalendarStoreXML.h"

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLObject.h"

using namespace calstore;

void CCalendarStoreWebcal::WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent) const
{
	// Create new node
	xmllib::XMLNode* xmlnode = new xmllib::XMLNode(doc, parent, cXMLElement_webcal);
	
	xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_webcal_url, mURL);
	
	xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_webcal_refresh, mPeriodicRefresh);
	
	xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_webcal_interval, mRefreshInterval);
	
	xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_webcal_autopublish, mAutoPublish);
	
	xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_webcal_readonly, mReadOnly);
}

void CCalendarStoreWebcal::ReadXML(const xmllib::XMLNode* xmlnode)
{
	// Must have right type of node
	if (!xmlnode->CompareFullName(cXMLElement_webcal))
		return;
	
	// Get child nodes
	
	xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_webcal_url, mURL);
	
	xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_webcal_refresh, mPeriodicRefresh);
	
	xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_webcal_interval, mRefreshInterval);
	
	xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_webcal_autopublish, mAutoPublish);
	
	xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_webcal_readonly, mReadOnly);
}
