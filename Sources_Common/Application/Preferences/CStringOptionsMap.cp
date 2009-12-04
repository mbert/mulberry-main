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


// Source for CStringOptionsMap class

#include "CStringOptionsMap.h"

#include "CLog.h"
#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C D E F A U L T O P T I O N S M A P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
CStringOptionsMap::CStringOptionsMap()
{
	mBuffer = NULL;
	mUpper = false;
	mIMSPFormat = false;
}

// Default destructor
CStringOptionsMap::~CStringOptionsMap(void)
{
	mBuffer = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Read map
bool CStringOptionsMap::ReadMap(void)
{
	// Check buffer specified
	if (!mBuffer) return false;

	// Wipe any existing map
	mMap.clear();

	// Get key/values one line at a time

	try
	{
		char* p = mBuffer->c_str_mod();
		char* line = p;
		while(p && *p)
		{
			// Get line of text
			while(*p && (*p != '\r') && (*p != '\n')) p++;

			if (*p)
			{
				*p++ = 0;
				while((*p == '\r') && (*p == '\n')) p++;
			}

			// Split into key & value
			char* key = ::strtok(line, "=");
			char* value = ::strtok(NULL, "\r\n");

			// Insert into map - ignore duplicates
			if (key)
			{
				if (value)
					mMap.insert(cdstrmap::value_type(key, value));
				else
					mMap.insert(cdstrmap::value_type(key, cdstring::null_str));
			}
			
			// Set to next line
			line = p;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return true;
}

// Write map
bool CStringOptionsMap::WriteMap(bool verify)
{
	// Check buffer specified
	if (!mBuffer) return false;

	// Write map to file
	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		*mBuffer += (*iter).first;
		if (mIMSPFormat)
			*mBuffer += " W ";
		else
			*mBuffer += "=";
		*mBuffer += (*iter).second;
		*mBuffer += os_endl;
	}

	return true;
}

// Delete map
void CStringOptionsMap::DeleteMap(void)
{
	// Check buffer specified
	if (!mBuffer) return;

	// Clear map on close
	mMap.clear();
}

void CStringOptionsMap::SetSection(const cdstring& section)
{
	// Use upper case section and replace ' ' with '_'
	cdstring usection = section;
	if (mUpper)
	{
		::strupper(usection);
		::strreplacespace(usection, '_');
	}
	CBlockOptionsMap::SetSection(usection);
}

void CStringOptionsMap::PushSection(const cdstring& sub_section)
{
	// Use upper case section and replace ' ' with '_'
	cdstring usub_section = sub_section;
	if (mUpper)
	{
		::strupper(usub_section);
		::strreplacespace(usub_section, '_');
	}
	CBlockOptionsMap::PushSection(usub_section);
}

// Look for values under a key
bool CStringOptionsMap::EnumValue(int num, cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	if (mUpper)
	{
		::strupper(ukey);
		::strreplacespace(ukey, '_');
	}
	return CBlockOptionsMap::EnumValue(num, ukey);
}

// Look for keys under a key
bool CStringOptionsMap::EnumKey(int num, cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	if (mUpper)
	{
		::strupper(ukey);
		::strreplacespace(ukey, '_');
	}
	return CBlockOptionsMap::EnumKey(num, key);
}

// Write key/value
bool CStringOptionsMap::WriteKeyValue(const cdstring& key, const cdstring& value)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	if (mUpper)
	{
		::strupper(ukey);
		::strreplacespace(ukey, '_');
	}
	return CBlockOptionsMap::WriteKeyValue(ukey, value);
}

// Read value from key
bool CStringOptionsMap::ReadKeyValue(const cdstring& key, cdstring& value)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	if (mUpper)
	{
		::strupper(ukey);
		::strreplacespace(ukey, '_');
	}
	return CBlockOptionsMap::ReadKeyValue(ukey, value);
}

// Remove key/value
bool CStringOptionsMap::RemoveKeyValue(const cdstring& key)
{
	// Use upper case key and replace ' ' with '_'
	cdstring ukey = key;
	if (mUpper)
	{
		::strupper(ukey);
		::strreplacespace(ukey, '_');
	}
	return CBlockOptionsMap::RemoveKeyValue(ukey);
}
