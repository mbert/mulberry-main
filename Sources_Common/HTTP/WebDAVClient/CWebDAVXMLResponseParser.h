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
	CWebDAVXMLResponseParser.h

	Author:			
	Description:	<describe the CWebDAVXMLResponseParser class here>
*/

#ifndef CWebDAVXMLResponseParser_H
#define CWebDAVXMLResponseParser_H

#include "cdstring.h"

#include "XMLDocument.h"

namespace http {

namespace webdav {

class CWebDAVXMLResponseParser
{
public:
	CWebDAVXMLResponseParser()
	{
		mDocument = NULL;
	}
	virtual ~CWebDAVXMLResponseParser()
	{
		// We have control over the document so delete it here
		delete mDocument;
	}

	void ParseData(const cdstring& data);
	void ParseFile(const cdstring& fpath);

protected:
	xmllib::XMLDocument*	mDocument;

	virtual void Parse(const xmllib::XMLNode* root_node) = 0;

};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVXMLResponseParser_H
