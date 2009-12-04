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


// Source for CStreamOptionsMap class

#include "CStreamOptionsMap.h"

#include "CLog.h"
#include "CStringUtils.h"

// Copy from raw list
CStreamOptionsMap::CStreamOptionsMap()
{
	mIn = NULL;
	mOut = NULL;
	mUpper = false;
	mIMSPFormat = false;
}

// Default destructor
CStreamOptionsMap::~CStreamOptionsMap()
{
	mIn = NULL;
	mOut = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Read map
bool CStreamOptionsMap::ReadMap()
{
	// Check stream specified
	if (!mIn)
		return false;

	// Wipe any existing map
	mMap.clear();

	// Get key/values one line at a time

	try
	{
		// Loop until failure or empty line
		while(!mIn->fail())
		{
			// Get line of text - exit loop if empty
			cdstring line;
			::getline(*mIn, line, 0);
			if (line.empty())
				break;

			// Split into key & value
			char* key = ::strtok(line.c_str_mod(), "=");
			char* value = ::strtok(NULL, "\r\n");

			// Insert into map - ignore duplicates
			if (key)
			{
				if (value)
					mMap.insert(cdstrmap::value_type(key, value));
				else
					mMap.insert(cdstrmap::value_type(key, cdstring::null_str));
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return true;
}

// Write map
bool CStreamOptionsMap::WriteMap(bool verify)
{
	// Check stream specified
	if (!mOut)
		return false;

	// Write map to file
	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		*mOut << (*iter).first;
		if (mIMSPFormat)
			*mOut << " W ";
		else
			*mOut << "=";
		*mOut << (*iter).second;
		*mOut << os_endl;
	}

	return true;
}

// Delete map
void CStreamOptionsMap::DeleteMap()
{
	// Clear map on close
	mMap.clear();
}

void CStreamOptionsMap::SetSection(const cdstring& section)
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

void CStreamOptionsMap::PushSection(const cdstring& sub_section)
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
bool CStreamOptionsMap::EnumValue(int num, cdstring& key)
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
bool CStreamOptionsMap::EnumKey(int num, cdstring& key)
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
bool CStreamOptionsMap::WriteKeyValue(const cdstring& key, const cdstring& value)
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
bool CStreamOptionsMap::ReadKeyValue(const cdstring& key, cdstring& value)
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
bool CStreamOptionsMap::RemoveKeyValue(const cdstring& key)
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
