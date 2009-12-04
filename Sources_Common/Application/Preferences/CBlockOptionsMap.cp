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


// Source for CBlockOptionsMap class

#include "CBlockOptionsMap.h"

#include "CMulberryApp.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#endif

// __________________________________________________________________________________________________
// C L A S S __ C D E F A U L T O P T I O N S M A P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
CBlockOptionsMap::CBlockOptionsMap()
{
}

// Default destructor
CBlockOptionsMap::~CBlockOptionsMap(void)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Look for values under a key
bool CBlockOptionsMap::EnumValue(int num, cdstring& key)
{
	size_t len_comp = ::strlen(mSection);

	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		// Partial compare
		if ((::strncmp((*iter).first, mSection, len_comp) == 0) &&
			((*iter).first[len_comp] == mSeparator[0UL]) &&
			(::strchr(((const char*) (*iter).first) + len_comp + 1, mSeparator[0UL]) == NULL) &&
			(num-- < 1))
		{
			cdstring full_key = (*iter).first;
			key = &full_key[len_comp + 1];
			return true;
		}
	}

	return false;
}

// Look for keys under a key
bool CBlockOptionsMap::EnumKey(int num, cdstring& key)
{
	size_t len_comp = ::strlen(mSection);

	cdstring last_key;
	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		// Partial compare
		cdstring match = (*iter).first;
		if ((::strncmp(match, mSection, len_comp) == 0) &&
			(match[len_comp] == mSeparator[0UL]) &&
			(::strchr(((const char*) match) + len_comp + 1, mSeparator[0UL]) != NULL))
		{
			// Extract key
			cdstring test_key = &match[len_comp + 1];
			*::strchr(test_key.c_str_mod(), mSeparator[0UL]) = 0;

			// Compare with last one
			if ((last_key != test_key) && (num-- < 1))
			{
				key = test_key;
				return true;
			}
			last_key = test_key;
		}
	}

	return false;
}

// Look for keys under a key
bool CBlockOptionsMap::EnumKeys(cdstrvect& found)
{
	size_t len_comp = ::strlen(mSection);

	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		// Partial compare
		cdstring match = (*iter).first;
		if (!::strncmp(match, mSection, len_comp) &&
			(match[len_comp] == mSeparator[0UL]))
		{
			// Add to found list
			found.push_back(&match.c_str()[len_comp + 1]);
		}
	}

	return found.size();
}

// Remove whole section
void CBlockOptionsMap::ClearSection(void)
{
	// Do nothing as file is always rewritten from scratch
}

// Write key/value
bool CBlockOptionsMap::WriteKeyValue(const cdstring& key, const cdstring& value)
{
	// Catenate section and key
	cdstring full_key = mSection;
	if (!full_key.empty())
		full_key += mSeparator;
	full_key += key;

	// Insert
	std::pair<cdstrmap::iterator, bool> result = mMap.insert(cdstrmap::value_type(full_key, value));

	// Does it exist already
	if (!result.second)
		// Replace existing
		(*result.first).second = value;

	return true;
}

// Read value from key
bool CBlockOptionsMap::ReadKeyValue(const cdstring& key, cdstring& value)
{
	// Catenate section and key
	cdstring full_key = mSection;
	if (!full_key.empty())
		full_key += mSeparator;
	full_key += key;

	// Find key
	cdstrmap::iterator found = mMap.find(full_key);

	// Return result if found
	if (found == mMap.end())
		return false;
	else
	{
		value = (*found).second;
		return true;
	}
}

// Remove key/value
bool CBlockOptionsMap::RemoveKeyValue(const cdstring& key)
{
	// Catenate section and key
	cdstring full_key = mSection;
	if (!full_key.empty())
		full_key += mSeparator;
	full_key += key;

	// erase element
	return mMap.erase(full_key);
}
