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
	CWebDAVPropFindParser.h

	Author:			
	Description:	<describe the CWebDAVPropFindParser class here>
*/

#ifndef CWebDAVPropFindParser_H
#define CWebDAVPropFindParser_H

#include "CWebDAVMultiResponseParser.h"

#include "cdstring.h"

namespace http {

namespace webdav {

class CWebDAVPropFindParser: public CWebDAVMultiResponseParser
{
public:
	class CPropFindResult
	{
	public:
		typedef std::map<cdstring, cdstring>				CTextPropertyMap;
		typedef std::map<cdstring, const xmllib::XMLNode*>	CNodePropertyMap;

		CPropFindResult() {}
		~CPropFindResult() {}
		
		void SetResource(const cdstring& resource)
		{
			mResource = resource;
		}
		const cdstring& GetResource() const
		{
			return mResource;
		}
		
		void AddTextProperty(const cdstring& name, const cdstring& value)
		{
			mTextPropertyMap.insert(CTextPropertyMap::value_type(name, value));
		}
		const CTextPropertyMap& GetTextProperties() const
		{
			return mTextPropertyMap;
		}

		void AddNodeProperty(const cdstring& name, const xmllib::XMLNode* node)
		{
			mNodePropertyMap.insert(CNodePropertyMap::value_type(name, node));
		}
		const CNodePropertyMap& GetNodeProperties() const
		{
			return mNodePropertyMap;
		}
	
	private:
		cdstring			mResource;
		CTextPropertyMap	mTextPropertyMap;
		CNodePropertyMap	mNodePropertyMap;
	};
	typedef std::vector<CPropFindResult*> CPropFindResults;

	CWebDAVPropFindParser();
	virtual ~CWebDAVPropFindParser();
	
	
	const CPropFindResults& Results() const
	{
		return mResults;
	}
	
	// Useful for debugging
	void DebugPrint(std::ostream& os) const;

protected:
	static std::set<cdstring> sSimpleProperties;

	CPropFindResults	mResults;

	// Parse the response element down to the properties
	virtual void ParseResponse(const xmllib::XMLNode* response);
	virtual void ParsePropStat(const xmllib::XMLNode* response, CPropFindResult* result);
	virtual void ParseProp(const xmllib::XMLNode* response, CPropFindResult* result);
	
	// Parsing of property elements
	virtual void ParsePropElement(const xmllib::XMLNode* response, CPropFindResult* result);
	virtual void ParsePropElementSimple(const xmllib::XMLNode* response, CPropFindResult* result);
	virtual void ParsePropElementUnknown(const xmllib::XMLNode* response, CPropFindResult* result);
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVPropFindParser_H
