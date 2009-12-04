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
	CXStringResources.h

	Author:			
	Description:	<describe the CXStringResources class here>
*/

#ifndef CXStringResources_H
#define CXStringResources_H

#include "cdstring.h"

namespace rsrc {

class CXStringResources
{
public:
	static CXStringResources sStringResources;

	CXStringResources()
	{
		mInitialised = false;
	}
	~CXStringResources() {}
	
	void LoadAllStrings();
	void LoadStrings(const cdstring& rsrc_name);
	void ClearStrings()
	{
		mStrings.clear();
		mIndexedStrings.clear();
	}
	void FullReset()
	{
		mInitialised = false;
		mPaths.clear();
		ClearStrings();
		LoadAllStrings();
	}
	const cdstring& GetString(const char* rsrc_id) const;
	const cdstring& GetIndexedString(const char* rsrc_id, uint32_t index) const;

private:
	static cdstring sEmptyString;
	bool			mInitialised;
	cdstrvect		mPaths;

	typedef cdstrmap CStringResourceMap;
	typedef std::map<cdstring, cdstrvect> CIndexedStringResourceMap;

	CStringResourceMap 			mStrings;
	CIndexedStringResourceMap	mIndexedStrings;
	
	void InitRsrcPaths();
	void LoadStringsFile(const cdstring& rsrc_name);
};

static const cdstring& GetString(const char* rsrc_id)
{
	return rsrc::CXStringResources::sStringResources.GetString(rsrc_id);
}
static const cdstring& GetIndexedString(const char* rsrc_id, uint32_t index)
{
	return rsrc::CXStringResources::sStringResources.GetIndexedString(rsrc_id, index);
}

}	// namespace rsrc

#endif	// CXStringResources_H
