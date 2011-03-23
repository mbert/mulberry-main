/*
    Copyright (c) 2010 Cyrus Daboo. All rights reserved.
    
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
	CWebDAVErrorResponseParser.h

	Author:			
	Description:	<describe the CWebDAVErrorResponseParser class here>
*/

#ifndef CWebDAVErrorResponseParser_H
#define CWebDAVErrorResponseParser_H

#include "CWebDAVXMLResponseParser.h"

#include "XMLName.h"

namespace http {

namespace webdav {

class CWebDAVErrorResponseParser : public CWebDAVXMLResponseParser
{
public:
	CWebDAVErrorResponseParser() {}
	virtual ~CWebDAVErrorResponseParser() {}

	const xmllib::XMLName& GetElement() const
	{
		return mErrorName;
	}

protected:
	xmllib::XMLName	mErrorName;

	virtual void Parse(const xmllib::XMLNode* error_node);
	
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVErrorResponseParser_H
