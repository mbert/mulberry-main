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


// Header for CRemoteOptionsMap class

#ifndef __CREMOTEOPTIONSMAP__MULBERRY__
#define __CREMOTEOPTIONSMAP__MULBERRY__

#include "CBlockOptionsMap.h"
#include "cdstring.h"

// Classes
class COptionsProtocol;

class CRemoteOptionsMap : public CBlockOptionsMap
{
public:
	CRemoteOptionsMap(void);
	virtual ~CRemoteOptionsMap(void);

	// Read/Write map
	virtual bool ReadMap(void);
	virtual bool WriteMap(bool verify);
	virtual void DeleteMap(void);

	virtual bool WriteEmptyEnums() const
		{ return true; }

	// Look for values under a key
	virtual bool EnumValue(int num, cdstring& key);
	virtual bool EnumKey(int num, cdstring& key);

	// Section
	virtual void SetSection(const cdstring& section);
	virtual void PushSection(const cdstring& sub_section);

	// Set file
	virtual void SetOptionsProtocol(COptionsProtocol* remoteOptions)
					{ mRemoteOptions = remoteOptions; }

private:
	COptionsProtocol*	mRemoteOptions;

	virtual bool WriteKeyValue(const cdstring& key, const cdstring& value);
	virtual bool ReadKeyValue(const cdstring& key, cdstring& value);
	virtual bool RemoveKeyValue(const cdstring& key);
};

#endif
