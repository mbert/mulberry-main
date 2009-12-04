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


// Source for CRemoteOptionsMap class

#include "CRemoteOptionsMap.h"

#include "CLog.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C D E F A U L T O P T I O N S M A P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
CRemoteOptionsMap::CRemoteOptionsMap()
{
	mRemoteOptions = NULL;
}

// Default destructor
CRemoteOptionsMap::~CRemoteOptionsMap(void)
{
	mRemoteOptions = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Read map
bool CRemoteOptionsMap::ReadMap(void)
{
	// Assume file already open
	if (!mRemoteOptions) return false;

	try
	{
		// Logon to remote
		StProtocolLogin login(mRemoteOptions);

		// Wipe any existing map
		mMap.clear();

		// Give map to remote options protocol
		mRemoteOptions->SetMap(&mMap);

		// Get all options from remote
		mRemoteOptions->GetAllAttributes(mTitle);

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return true;
}

// Write map
bool CRemoteOptionsMap::WriteMap(bool verify)
{
	// Assume file already open
	if (!mRemoteOptions) return false;

	// Only bother if something in map
	if (mMap.empty()) return false;

	try
	{
		// Logon to remote
		StProtocolLogin login(mRemoteOptions);

		// Give map to remote options protocol
		mRemoteOptions->SetMap(&mMap);

		// Set all options in one go
		mRemoteOptions->SetAllAttributes(mTitle);

		// Must force re-read to cope with read-only prefs
		if (verify)
		{
			// Wipe any existing map
			mMap.clear();

			// Get all options from remote
			mRemoteOptions->GetAllAttributes(mTitle);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return true;
}

// Delete map
void CRemoteOptionsMap::DeleteMap(void)
{
	// Assume file already open
	if (!mRemoteOptions) return;

	try
	{
		// Logon to remote
		StProtocolLogin login(mRemoteOptions);

		// Give map to remote options protocol
		mRemoteOptions->SetMap(&mMap);

		// Unset all options from remote
		mRemoteOptions->DeleteEntry(mTitle);		//<- Too slow!

		// Clear map on close
		mMap.clear();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CRemoteOptionsMap::SetSection(const cdstring& section)
{
	// Use upper case section and replace ' ' with '_'
	cdstring usection = section;
	::strupper(usection);
	::strreplacespace(usection, '_');
	CBlockOptionsMap::SetSection(usection);
}

void CRemoteOptionsMap::PushSection(const cdstring& sub_section)
{
	// Use upper case section and replace ' ' with '_'
	cdstring usub_section = sub_section;
	::strupper(usub_section);
	::strreplacespace(usub_section, '_');
	CBlockOptionsMap::PushSection(usub_section);
}

// Look for values under a key
bool CRemoteOptionsMap::EnumValue(int num, cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	::strupper(ukey);
	::strreplacespace(ukey, '_');
	return CBlockOptionsMap::EnumValue(num, ukey);
}

// Look for keys under a key
bool CRemoteOptionsMap::EnumKey(int num, cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	::strupper(ukey);
	::strreplacespace(ukey, '_');
	return CBlockOptionsMap::EnumKey(num, key);
}

// Write key/value
bool CRemoteOptionsMap::WriteKeyValue(const cdstring& key, const cdstring& value)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	::strupper(ukey);
	::strreplacespace(ukey, '_');
	return CBlockOptionsMap::WriteKeyValue(ukey, value);
}

// Read value from key
bool CRemoteOptionsMap::ReadKeyValue(const cdstring& key, cdstring& value)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	::strupper(ukey);
	::strreplacespace(ukey, '_');
	return CBlockOptionsMap::ReadKeyValue(ukey, value);
}

// Remove key/value
bool CRemoteOptionsMap::RemoveKeyValue(const cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	::strupper(ukey);
	::strreplacespace(ukey, '_');
	return CBlockOptionsMap::RemoveKeyValue(ukey);
}
