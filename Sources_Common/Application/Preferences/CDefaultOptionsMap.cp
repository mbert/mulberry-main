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


// Source for CDefaultOptionsMap class

#include "CDefaultOptionsMap.h"

#include "CMulberryApp.h"

// __________________________________________________________________________________________________
// C L A S S __ C D E F A U L T O P T I O N S M A P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
CDefaultOptionsMap::CDefaultOptionsMap()
{
}

// Default destructor
CDefaultOptionsMap::~CDefaultOptionsMap(void)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Read map
bool CDefaultOptionsMap::ReadMap(void)
{
	// No special processing for read
	return true;
}

// Write map
bool CDefaultOptionsMap::WriteMap(bool validate)
{
	// No special processing for write
	return true;
}

// Look for values under a key
bool CDefaultOptionsMap::EnumValue(int num, cdstring& key)
{
	if (CMulberryApp::sApp->m_pszRegistryKey != NULL)
	{
		// Open current key
		HKEY hSecKey = GetSectionKey();
		if (hSecKey == NULL)
			return false;

		// Get next sub-key
		TCHAR szSubKeyName[256];
		DWORD bufSize = 256;
		DWORD dwResult = ::RegEnumValue(hSecKey, num, szSubKeyName, &bufSize, NULL, NULL, NULL, NULL);
		if (dwResult == ERROR_SUCCESS)
			key = szSubKeyName;

		::RegCloseKey(hSecKey);
		return (dwResult == ERROR_SUCCESS);
	}
	else
	{
		// This is not quite right as it assumes that there are either keys or values
		// but not both in the current key to enumerate

		TCHAR buf[8192];
		
		// Get all data in section
		if (::GetPrivateProfileSection(mSection.win_str(), buf, 8192, CMulberryApp::sApp->m_pszProfileName))
		{
			// Iterate over all strings untl count
			cdstring temp_buf(buf);
			char* entry = temp_buf.c_str_mod();
			char* p = entry;
			while(*p && num)
			{
				// Get pointer to current entry
				p = entry;

				// Step up to and past end of string
				while(*p++) ;

				// Reduce count
				num--;
			}
			
			// If empty gone past the end
			if (!*p)
				return false;
			
			// Get key
			if ((p = ::strtok(p, "=")) != NULL)
				key = p;
			return true;
		}
	}
	
	return false;
}

// Look for keys under a key
bool CDefaultOptionsMap::EnumKey(int num, cdstring& key)
{
	if (CMulberryApp::sApp->m_pszRegistryKey != NULL)
	{
		// Open current key
		HKEY hSecKey = GetSectionKey();
		if (hSecKey == NULL)
			return false;

		// Get next sub-key
		TCHAR szSubKeyName[256];
		DWORD bufSize = 256;
		DWORD dwResult = ::RegEnumKey(hSecKey, num, szSubKeyName, bufSize);
		if (dwResult == ERROR_SUCCESS)
			key = szSubKeyName;

		::RegCloseKey(hSecKey);
		return (dwResult == ERROR_SUCCESS);
	}
	else
	{
		// This is not quite right as it assumes that there are either keys or values
		// but not both in the current key to enumerate

		TCHAR buf[8192];
		
		// Get all data in section
		if (::GetPrivateProfileSection(mSection.win_str(), buf, 8192, CMulberryApp::sApp->m_pszProfileName))
		{
			// Iterate over all strings until count
			cdstring temp_buf(buf);
			char* entry = temp_buf.c_str_mod();
			char* p = entry;
			while(*p && num)
			{
				// Get pointer to current entry
				p = entry;

				// Step up to and past end of string
				while(*p++) ;

				// Reduce count
				num--;
			}
			
			// If empty gone past the end
			if (!*p)
				return false;
			
			// Get key
			if ((p = ::strtok(p, "=")) != NULL)
				key = p;
			return true;
		}
	}
	
	return false;
}

// Look for values under a key
bool CDefaultOptionsMap::EnumKeys(cdstrvect& found)
{
	if (CMulberryApp::sApp->m_pszRegistryKey != NULL)
	{
		// Open current key
		HKEY hSecKey = GetSectionKey();
		if (hSecKey == NULL)
			return false;

		// Get next sub-key
		TCHAR szSubKeyName[256];
		DWORD bufSize = 256;
		int num = 0;
		while(::RegEnumValue(hSecKey, num++, szSubKeyName, &bufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			found.push_back(szSubKeyName);
			bufSize = 256;
		}

		::RegCloseKey(hSecKey);
		return true;
	}
	else
	{
		// No longer support .ini files!!
	}
	
	return false;
}


// Remove whole section
void CDefaultOptionsMap::ClearSection(void)
{
	WriteKeyValue(cdstring::null_str, cdstring::null_str);
}

// Write key/value
bool CDefaultOptionsMap::WriteKeyValue(const cdstring& key, const cdstring& value)
{
	if (CMulberryApp::sApp->m_pszRegistryKey != NULL)
	{
		LONG lResult;
		if (key.length() == 0) //delete whole section
		{
			// Pop bottom of section stack off and save
			cdstring old_section = mSectionStack.back();
			mSectionStack.pop_back();

			// Get key for parent section
			HKEY hSecKey = GetSectionKey();
			if (hSecKey == NULL)
			{
				// Put section back on stack before exit
				mSectionStack.push_back(old_section);
				return FALSE;
			}
			
			// Delete child section and close parent
			lResult = ::RegDeleteKey(hSecKey, old_section.win_str());
			::RegCloseKey(hSecKey);
			
			// Put section back on stack
			mSectionStack.push_back(old_section);
		}
#if 0
		else if (value.length() == 0)
		{
			HKEY hSecKey = GetSectionKey();
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, key);
			::RegCloseKey(hSecKey);
		}
#endif
		else
		{
			HKEY hSecKey = GetSectionKey();
			if (hSecKey == NULL)
				return FALSE;
			CString temp(value);
			const TCHAR* value_p = temp;
			lResult = ::RegSetValueEx(hSecKey, key.win_str(), NULL, REG_SZ, (LPBYTE)value_p, (lstrlen(value_p)+1)*sizeof(TCHAR));
			::RegCloseKey(hSecKey);
		}
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		// Handle case of deleting section/value
		const char* keyp = key;
		const char* valp = value;
		if (!*keyp)
			keyp = NULL;
		if (!*valp)
			valp = NULL;
		return CMulberryApp::sApp->WriteProfileString(mSection.win_str(), cdstring(keyp).win_str(), cdstring(valp).win_str());
	}
}

// Read value from key
bool CDefaultOptionsMap::ReadKeyValue(const cdstring& key, cdstring& value)
{
	if (CMulberryApp::sApp->m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetSectionKey();
		if (hSecKey == NULL)
			return false;
		CString strValue;
		DWORD dwType, dwCount;
		LONG lResult = ::RegQueryValueEx(hSecKey, key.win_str(), NULL, &dwType, NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lResult = ::RegQueryValueEx(hSecKey, key.win_str(), NULL, &dwType, (LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		::RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			value = strValue;
			return true;
		}
		return false;
	}
	else
	{
		CString str = CMulberryApp::sApp->GetProfileString(mSection.win_str(), cdstring(key).win_str(), NULL);
		value = str;
		return (str.GetLength() != 0);
	}
}

// Remove key/value
bool CDefaultOptionsMap::RemoveKeyValue(const cdstring& key)
{
	return WriteKeyValue(key, cdstring::null_str);
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CDefaultOptionsMap::GetSectionKey(void)
{
	HKEY hSectionKey = NULL;
	HKEY hAppKey = CMulberryApp::sApp->GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	// Iterate over nested section specification
	for(cdstrvect::const_iterator iter = mSectionStack.begin(); iter != mSectionStack.end(); iter++)
	{
		// Create new key and close old one
		DWORD dw;
		::RegCreateKeyEx(hAppKey, (*iter).win_str(), 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hSectionKey, &dw);
		::RegCloseKey(hAppKey);
		
		// Reassign new to old
		hAppKey = hSectionKey;
	}
	
	// Return final key
	return hSectionKey;
}

