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


// Header for CDefaultOptionsMap class

#ifndef __CDEFAULTOPTIONSMAP__MULBERRY__
#define __CDEFAULTOPTIONSMAP__MULBERRY__

#include "COptionsMap.h"

// Classes

class CDefaultOptionsMap : public COptionsMap
{

public:
	CDefaultOptionsMap(void);
	virtual ~CDefaultOptionsMap(void);

	// Read/Write map
	virtual bool ReadMap(void);
	virtual bool WriteMap(bool validate);
	virtual bool IsEmpty() const
		{ return false; }

	virtual bool WriteEmptyEnums() const
		{ return true; }

	// Look for values under a key
	virtual bool EnumValue(int num, cdstring& key);
	virtual bool EnumKey(int num, cdstring& key);
	virtual bool EnumKeys(cdstrvect& found);

	// Remove whole section
	virtual void ClearSection(void);

private:
	virtual bool WriteKeyValue(const cdstring& key, const cdstring& value);
	virtual bool ReadKeyValue(const cdstring& key, cdstring& value);
	virtual bool RemoveKeyValue(const cdstring& key);

	HKEY	GetSectionKey(void);		// Get nested section key

};

#endif
