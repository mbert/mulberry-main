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


// Header for CBlockOptionsMap class

#ifndef __CBLOCKOPTIONSMAP__MULBERRY__
#define __CBLOCKOPTIONSMAP__MULBERRY__

#include "COptionsMap.h"
#include "cdstring.h"

// Classes
class CBlockOptionsMap : public COptionsMap
{

public:
	CBlockOptionsMap(void);
	virtual ~CBlockOptionsMap(void);

	// Read/Write map
	virtual bool ReadMap(void) = 0;
	virtual bool WriteMap(bool verify) = 0;
	virtual bool IsEmpty() const
		{ return mMap.size(); }
	virtual void DeleteMap(void) = 0;

	// Look for values under a key
	virtual bool EnumValue(int num, cdstring& key);
	virtual bool EnumKey(int num, cdstring& key);
	virtual bool EnumKeys(cdstrvect& found);

	// Remove whole section
	virtual void ClearSection(void);

protected:
	cdstrmap	mMap;

	virtual bool WriteKeyValue(const cdstring& key, const cdstring& value);
	virtual bool ReadKeyValue(const cdstring& key, cdstring& value);
	virtual bool RemoveKeyValue(const cdstring& key);

};

#endif
