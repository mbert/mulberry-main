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
	CWebDAVXMLResponseParser.cpp

	Author:			
	Description:	<describe the CWebDAVXMLResponseParser class here>
*/

#include "CWebDAVXMLResponseParser.h"

#include "XMLSAXSimple.h"

using namespace http; 
using namespace webdav; 
using namespace xmllib; 

void CWebDAVXMLResponseParser::ParseData(const cdstring& data)
{
	// XML parse the data
	XMLSAXSimple parser;
	parser.ParseData(data.c_str());

	// See if we got any valid XML
	if (parser.Document())
	{
		// Grab control of document object
		mDocument = parser.ReleaseDocument();
		
		// Now parse the root node
		Parse(mDocument->GetRoot());
	}
}

void CWebDAVXMLResponseParser::ParseFile(const cdstring& fpath)
{
	// XML parse the data
	XMLSAXSimple parser;
	parser.ParseFile(fpath.c_str());

	// See if we got any valid XML
	if (parser.Document())
	{
		// Grab control of document object
		mDocument = parser.Document();
		
		// Now parse the root node
		Parse(mDocument->GetRoot());
	}
}
