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
	CCardDAVReportParser.h

	Author:			
	Description:	<describe the CCardDAVReportParser class here>
*/

#ifndef CCardDAVReportParser_H
#define CCardDAVReportParser_H

#include "CWebDAVMultiResponseParser.h"

#include "cdstring.h"

using namespace http; 
using namespace webdav; 

namespace vCard
{
	class CVCardAddressBook;
}

class CAddressBook;
class CAddressList;

namespace http {

namespace carddav {

class CCardDAVReportParser: public CWebDAVMultiResponseParser
{
public:
	CCardDAVReportParser(vCard::CVCardAddressBook& vadbk);
	CCardDAVReportParser(vCard::CVCardAddressBook& vadbk, CAddressBook* adbk, bool add = false);
	CCardDAVReportParser(vCard::CVCardAddressBook& vadbk, CAddressList* addrs, bool add = false);
	virtual ~CCardDAVReportParser();
	
	
protected:
	vCard::CVCardAddressBook*	mVAdbk;
	CAddressBook*				mAdbk;
	CAddressList*				mAddrs;
	bool						mAddItems;

	// Parse the response element down to the properties

	virtual void ParseResponse(const xmllib::XMLNode* response);
	virtual void ParsePropStat(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag);
	virtual void ParseProp(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag);
	
	// Parsing of property elements
	virtual void ParsePropElement(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag);
	
	// Calendar data
	virtual void ParseAdbkData(const xmllib::XMLNode* response, const cdstring& href, const cdstring& etag);

};

}	// namespace carddav

}	// namespace http

#endif	// CCardDAVReportParser_H
