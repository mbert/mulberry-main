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


// CWinRegistry.cp

// Source file to handle access to Windows registry or .INI files


#include "CWinRegistry.h"

#include "CStringUtils.h"

cdstrmap CWinRegistry::sMIME2Suffix;
cdstrmap CWinRegistry::sFile2App;

cdstring CWinRegistry::GetMIMEFromFile(const cdstring& suffix)
{
	cdstring type;

	// Look in registry
	HKEY hSuffixKey = NULL;
	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, suffix.win_str(), 0, KEY_READ, &hSuffixKey) == ERROR_SUCCESS)
	{
		DWORD dwType, dwCount;
		if (::RegQueryValueEx(hSuffixKey, _T("Content Type"), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
		{
			CString temp;
			LONG lresult = ::RegQueryValueEx(hSuffixKey, _T("Content Type"), NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			temp.ReleaseBuffer();
			if (lresult == ERROR_SUCCESS)
			{
				type = temp;
			}
		}
		::RegCloseKey(hSuffixKey);
	}
	
	return type;
}

cdstring CWinRegistry::GetSuffixFromMIME(const cdstring& type, const cdstring& original)
{
	// Cache is is content type/subtype/original extension
	// This ensures that attachments with the same type/subtype but different original extensions
	// will be mapped separately
	cdstring cache_key(type);
	cache_key += "/";
	cache_key += original;
	::strlower(cache_key.c_str_mod());

	// First see if its in the cache
	cdstrmap::const_iterator found = sMIME2Suffix.find(cache_key);
	if (found != sMIME2Suffix.end())
		return (*found).second;

	// New policy - based on what unix client does:
	
	// First - lookup the original suffix and see if its MIME type/subtype match the
	//         current MIME type/subtype. If so just return that extension.
	
	// Second - leave application/octet-streams alone.
	
	// Third - lookup MIME type/subtype in HKEY_CLASSES_ROOT/MIME/Database. If a match is
	//         found use that.
	
	// Fourth - scan extension list looking for first matching MIME type/subtype and use that.
	
	// Fifth - just return the original if no match.

	// First:
	cdstring suffixtype = CWinRegistry::GetMIMEFromFile(original);
	if (!::strcmpnocase(suffixtype.c_str(), type.c_str()))
	{
		// Cache result and return
		sMIME2Suffix.insert(cdstrmap::value_type(cache_key, original));
		return original;
	}
	
	// Second:
	if (!::strcmpnocase(type.c_str(), "application/octet-stream"))
	{
		// Cache result and return
		sMIME2Suffix.insert(cdstrmap::value_type(cache_key, original));
		return original;
	}

	// Third:
	cdstring suffix;
	// Look in registry HKEY_CLASSES_ROOT/MIME/Database/Content Type/Extension
	HKEY hMIMEKey = NULL;
	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("MIME"), 0, KEY_READ, &hMIMEKey) == ERROR_SUCCESS)
	{
		HKEY hDBaseKey = NULL;
		if (::RegOpenKeyEx(hMIMEKey, _T("Database"), 0, KEY_READ, &hDBaseKey) == ERROR_SUCCESS)
		{
			HKEY hContentTypeKey = NULL;
			if (::RegOpenKeyEx(hDBaseKey, _T("Content Type"), 0, KEY_READ, &hContentTypeKey) == ERROR_SUCCESS)
			{
				HKEY hContentKey = NULL;
				if (::RegOpenKeyEx(hContentTypeKey, type.win_str(), 0, KEY_READ, &hContentKey) == ERROR_SUCCESS)
				{
					DWORD dwType, dwCount;
					if (::RegQueryValueEx(hContentKey, _T("Extension"), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
					{
						ASSERT(dwType == REG_SZ);
						CString temp;
						LONG lresult = ::RegQueryValueEx(hContentKey, _T("Extension"), NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
						temp.ReleaseBuffer();
						if (lresult == ERROR_SUCCESS)
						{
							suffix = temp;
						}
					}
					::RegCloseKey(hContentKey);
				}
				::RegCloseKey(hContentTypeKey);
			}
			::RegCloseKey(hDBaseKey);
		}
		::RegCloseKey(hMIMEKey);
	}
	
	// Iterate over all suffix and match first type/subtype
	if (suffix.empty())
	{
		// Fourth:

		int ctr = 0;
		bool found = false;
		TCHAR temp[256];
		while (::RegEnumKey(HKEY_CLASSES_ROOT, ctr++, temp, 256) == ERROR_SUCCESS)
		{
			suffix = temp;
			HKEY hRootKey = NULL;
			::RegOpenKeyEx(HKEY_CLASSES_ROOT, suffix.win_str(), 0, KEY_READ, &hRootKey);
			DWORD dwType, dwCount;
			if (::RegQueryValueEx(hRootKey, _T("Content Type"), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
			{
				// Get Content Type Regentry and cobvert to lowercase for comparison
				ASSERT(dwType == REG_SZ);
				CString temp;
				LONG lresult = ::RegQueryValueEx(hRootKey, _T("Content Type"), NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
				temp.ReleaseBuffer();
				if (lresult == ERROR_SUCCESS)
				{
					cdstring value = temp;
					::strlower(value.c_str_mod());

					// Compare
					if (value == type)
					{
						::RegCloseKey(hRootKey);
						found = true;
						break;
					}
				}
			}
			::RegCloseKey(hRootKey);
		}
		
		// Fifth:
		if (!found)
			suffix = original;
	}

	// Cache result and return
	sMIME2Suffix.insert(cdstrmap::value_type(cache_key, suffix));
	return suffix;
}

cdstring CWinRegistry::GetAppFromFile(const cdstring& suffix)
{
	// Lowercase suffix
	cdstring lower_suffix(suffix);
	::strlower(lower_suffix.c_str_mod());

	// First see if its in the cache
	cdstrmap::const_iterator found = sFile2App.find(lower_suffix);
	if (found != sFile2App.end())
		return (*found).second;

	cdstring apptype;
	cdstring app;

	// Look in registry for app type
	HKEY hSuffixKey = NULL;
	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, suffix.win_str(), 0, KEY_READ, &hSuffixKey) == ERROR_SUCCESS)
	{
		CString strValue;
		DWORD dwType, dwCount;
		if (::RegQueryValueEx(hSuffixKey, _T(""), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			CString temp;
			LONG lresult = ::RegQueryValueEx(hSuffixKey, _T(""), NULL, &dwType,(LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			temp.ReleaseBuffer();
			if (lresult == ERROR_SUCCESS)
			{
				apptype = temp;
			}
		}
		::RegCloseKey(hSuffixKey);
	}
	
	// Look in registry for apptype's open command
	if (!apptype.empty())
	{
		cdstring command;
	
		HKEY hAppTypeKey = NULL;
		if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, apptype.win_str(), 0, KEY_READ, &hAppTypeKey) == ERROR_SUCCESS)
		{
			HKEY hShellKey = NULL;
			if (::RegOpenKeyEx(hAppTypeKey, _T("shell"), 0, KEY_READ, &hShellKey) == ERROR_SUCCESS)
			{
				HKEY hOpenKey = NULL;
				if (::RegOpenKeyEx(hShellKey, _T("open"), 0, KEY_READ, &hOpenKey) == ERROR_SUCCESS)
				{
					HKEY hCommandKey = NULL;
					if (::RegOpenKeyEx(hOpenKey, _T("command"), 0, KEY_READ, &hCommandKey) == ERROR_SUCCESS)
					{
						DWORD dwType, dwCount;
						if (::RegQueryValueEx(hCommandKey, _T(""), NULL, &dwType, NULL, &dwCount) == ERROR_SUCCESS)
						{
							ASSERT(dwType == REG_SZ);
							CString temp;
							LONG lresult = ::RegQueryValueEx(hCommandKey, _T(""), NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
							temp.ReleaseBuffer();
							if (lresult == ERROR_SUCCESS)
							{
								command = temp;
							}
						}
						::RegCloseKey(hCommandKey);
					}
					::RegCloseKey(hOpenKey);
				}
				::RegCloseKey(hShellKey);
			}
			::RegCloseKey(hAppTypeKey);
		}
		
		// Find name from command
		if (!command.empty())
		{
			// Terminate at end of (possibly quoted) command (ignore any arguments)
			char* pos = NULL;
			if (command[(cdstring::size_type)0] == '"')
				pos = ::strchr(command.c_str_mod() + 1, '"');
			else
				pos = ::strchr(command.c_str_mod(), ' ');
			if (pos)
				*pos = 0;
			
			// Find last directory separator
			pos = ::strrchr(command.c_str_mod(), '\\');
			if (pos)
				pos++;
			else
				pos = command;
			
			// Copy application name
			app = pos;
		}
	}

	// Cache result and return
	sFile2App.insert(cdstrmap::value_type(lower_suffix, app));
	return app;
}

#pragma mark ________________________mailto helper

// Check for mailto helper
bool CWinRegistry::CheckHelperMailto()
{
	// Look at registry's mailto item
	cdstring value;
	if (ReadRegString(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\command", "", value))
	{
		// Check that it contains mulberry
		if (::strstr(value.c_str(), "Mulberry.exe"))
		{
			// Look at HKEY_LOCAL_MACHINE setting
			if (ReadRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail", "", value))
				return value == "Mulberry";
		}
	}

	return false;
}

// Set mailto helper
bool CWinRegistry::SetHelperMailto()
{
	// Get path of this app
	CString temp;
	::GetModuleFileName(AfxGetInstanceHandle(), temp.GetBuffer(MAX_PATH), MAX_PATH);
	temp.ReleaseBuffer(-1);
	cdstring appl(temp);

	// Get directory of this app
	cdstring appl_dir = appl;
	char* p = ::strrchr(appl_dir.c_str_mod(), '\\');
	if (p)
		*p = 0;

	// Always quote the app path
	cdstring applname = "\"";
	applname += appl;
	applname += "\"";

	// HKEY_CLASSES_ROOT area
	WriteRegString(HKEY_CLASSES_ROOT, "mailto\\DefaultIcon", "", applname + ",0");
	WriteRegString(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\ddeexec\\Application", "", "Mulberry");

	// HKEY_LOCAL_MACHINE area
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry", "", "Mulberry");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry", "DLLPath", appl_dir + "\\Plug-ins\\MAPI32.dll");

	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto", "", "URL: Mailto Protocol");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto", "URL Protocol", "");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto\\DefaultIcon", "", applname + ",0");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\Protocols\\mailto\\shell\\open\\ddeexec\\Application", "", "Mulberry");

	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail\\Mulberry\\shell\\open\\ddeexec\\Application", "", "Mulberry");
	
	// This actually makes Mulberry the default client for IE, MAPI etc
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Mail", "", "Mulberry");

	return true;
}	

#pragma mark ________________________webcal helper

// Check for webcal helper
bool CWinRegistry::CheckHelperWebcal()
{
	// Look at registry's webcal item
	cdstring value;
	if (ReadRegString(HKEY_CLASSES_ROOT, "webcal\\shell\\open\\command", "", value))
	{
		// Check that it contains mulberry
		if (::strstr(value.c_str(), "Mulberry.exe"))
		{
			// Look at HKEY_LOCAL_MACHINE setting
			if (ReadRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar", "", value))
				return value == "Mulberry";
		}
	}

	return false;
}

// Set webcal helper
bool CWinRegistry::SetHelperWebcal()
{
	// Get path of this app
	CString temp;
	::GetModuleFileName(AfxGetInstanceHandle(), temp.GetBuffer(MAX_PATH), MAX_PATH);
	temp.ReleaseBuffer(-1);
	cdstring appl(temp);

	// Get directory of this app
	cdstring appl_dir = appl;
	char* p = ::strrchr(appl_dir.c_str_mod(), '\\');
	if (p)
		*p = 0;

	// Always quote the app path
	cdstring applname = "\"";
	applname += appl;
	applname += "\"";

	// HKEY_CLASSES_ROOT area
	WriteRegString(HKEY_CLASSES_ROOT, "webcal", "", "URL: Webcal Protocol");
	WriteRegString(HKEY_CLASSES_ROOT, "webcal", "URL Protocol", "");
	WriteRegString(HKEY_CLASSES_ROOT, "webcal\\DefaultIcon", "", applname + ",0");
	WriteRegString(HKEY_CLASSES_ROOT, "webcal\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_CLASSES_ROOT, "webcal\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_CLASSES_ROOT, "webcal\\shell\\open\\ddeexec\\Application", "", "Mulberry");

	// HKEY_LOCAL_MACHINE area
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry", "", "Mulberry");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry", "DLLPath", appl_dir + "\\Plug-ins\\MAPI32.dll");

	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal", "", "URL: Webcal Protocol");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal", "URL Protocol", "");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal\\DefaultIcon", "", applname + ",0");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\Protocols\\webcal\\shell\\open\\ddeexec\\Application", "", "Mulberry");

	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\shell\\open\\command", "", applname + " %1");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\shell\\open\\ddeexec", "", "[url(\"%1\")]");
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar\\Mulberry\\shell\\open\\ddeexec\\Application", "", "Mulberry");
	
	// This actually makes Mulberry the default client for IE, MAPI etc
	WriteRegString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\Calendar", "", "Mulberry");

	return true;
}	

#pragma mark ________________________HKEY_LOCAL_MACHINE Registry

static const TCHAR szMachineSoftware[] = _T("SOFTWARE");

// returns key for HKEY_LOCAL_MACHINE\"SOFTWARE"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinRegistry::GetMachineRegistryKey(bool write)
{
	ASSERT(::AfxGetApp()->m_pszRegistryKey != NULL);
	ASSERT(::AfxGetApp()->m_pszProfileName != NULL);

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	REGSAM  samDesired = KEY_READ | (write ? KEY_WRITE : 0);
	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMachineSoftware, 0, samDesired,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (::RegCreateKeyEx(hSoftKey, ::AfxGetApp()->m_pszRegistryKey, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, samDesired, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			::RegCreateKeyEx(hCompanyKey, ::AfxGetApp()->m_pszProfileName, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, samDesired, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		::RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		::RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_LOCAL_MACHINE\"SOFTWARE"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY CWinRegistry::GetMachineSectionKey(LPCTSTR lpszSection, bool write)
{
	ASSERT(lpszSection != NULL);

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetMachineRegistryKey(write);
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	REGSAM  samDesired = KEY_READ | (write ? KEY_WRITE : 0);
	::RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, samDesired, NULL,
		&hSectionKey, &dw);
	::RegCloseKey(hAppKey);
	return hSectionKey;
}

UINT CWinRegistry::GetMachineProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (AfxGetApp()->m_pszRegistryKey != NULL) // use registry
	{
		HKEY hSecKey = GetMachineSectionKey(lpszSection);
		if (hSecKey == NULL)
			return nDefault;
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof(DWORD);
		LONG lResult =::RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType, (LPBYTE)&dwValue, &dwCount);
		::RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_DWORD);
			ASSERT(dwCount == sizeof(dwValue));
			return (UINT)dwValue;
		}
		return nDefault;
	}
	else
	{
		ASSERT(AfxGetApp()->m_pszProfileName != NULL);
		return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, AfxGetApp()->m_pszProfileName);
	}
}

CString CWinRegistry::GetMachineProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (AfxGetApp()->m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetMachineSectionKey(lpszSection);
		if (hSecKey == NULL)
			return lpszDefault;
		CString strValue;
		DWORD dwType, dwCount;
		LONG lResult = ::RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType, NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lResult = ::RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType, (LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		::RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			return strValue;
		}
		return lpszDefault;
	}
	else
	{
		ASSERT(AfxGetApp()->m_pszProfileName != NULL);

		if (lpszDefault == NULL)
			lpszDefault = _T("");    // don't pass in NULL
		TCHAR szT[4096];
		DWORD dw = ::GetPrivateProfileString(lpszSection, lpszEntry,
			lpszDefault, szT, sizeof(szT), AfxGetApp()->m_pszProfileName);
		ASSERT(dw < 4095);
		return szT;
	}
}

BOOL CWinRegistry::WriteMachineProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	ASSERT(lpszSection != NULL);
	ASSERT(lpszEntry != NULL);
	if (AfxGetApp()->m_pszRegistryKey != NULL)
	{
		HKEY hSecKey = GetMachineSectionKey(lpszSection, true);
		if (hSecKey == NULL)
			return FALSE;
		LONG lResult = ::RegSetValueEx(hSecKey, lpszEntry, NULL, REG_DWORD, (LPBYTE)&nValue, sizeof(nValue));
		::RegCloseKey(hSecKey);
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT(AfxGetApp()->m_pszProfileName != NULL);

		TCHAR szT[16];
		wsprintf(szT, _T("%d"), nValue);
		return ::WritePrivateProfileString(lpszSection, lpszEntry, szT, AfxGetApp()->m_pszProfileName);
	}
}

BOOL CWinRegistry::WriteMachineProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	ASSERT(lpszSection != NULL);
	if (AfxGetApp()->m_pszRegistryKey != NULL)
	{
		LONG lResult;
		if (lpszEntry == NULL) //delete whole section
		{
			HKEY hAppKey = GetMachineRegistryKey(true);
			if (hAppKey == NULL)
				return FALSE;
			lResult = ::RegDeleteKey(hAppKey, lpszSection);
			::RegCloseKey(hAppKey);
		}
		else if (lpszValue == NULL)
		{
			HKEY hSecKey = GetMachineSectionKey(lpszSection, true);
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
			::RegCloseKey(hSecKey);
		}
		else
		{
			HKEY hSecKey = GetMachineSectionKey(lpszSection, true);
			if (hSecKey == NULL)
				return FALSE;
			lResult = ::RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
				(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
			::RegCloseKey(hSecKey);
		}
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT(AfxGetApp()->m_pszProfileName != NULL);
		ASSERT(lstrlen(AfxGetApp()->m_pszProfileName) < 4095); // can't read in bigger
		return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, AfxGetApp()->m_pszProfileName);
	}
}

BOOL CWinRegistry::WriteRegString(HKEY root, const cdstring& path, const cdstring& valuename, const cdstring& value)
{
	bool result = false;

	// Open the key for the full path (creating if required)
	HKEY key;
	DWORD dw;
	if (::RegCreateKeyEx(root, path.win_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &key, &dw) == ERROR_SUCCESS)
	{
		// Set the key's named value
		CString temp(value.win_str());
		const TCHAR* value_p = temp;
		if (::RegSetValueEx(key, valuename.win_str(), 0, REG_SZ, (LPBYTE)value_p, (lstrlen(value_p)+1)*sizeof(TCHAR)) == ERROR_SUCCESS)
			result = true;
		
		// Close the key
		::RegCloseKey(key);
	}
	
	return result;
}

BOOL CWinRegistry::ReadRegString(HKEY root, const cdstring& path, const cdstring& valuename, cdstring& value)
{
	bool result = false;

	// Open the key for the full path
	HKEY key;
	if (::RegOpenKeyEx(root, path.win_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		// Determine the space required
		DWORD bufsize = 0;
		if (::RegQueryValueEx(key, valuename.win_str(), 0, NULL, NULL, &bufsize) == ERROR_SUCCESS)
		{
			// Get the key's named value
			CString temp;
			LONG lresult = ::RegQueryValueEx(key, valuename.win_str(), 0, NULL, (LPBYTE)temp.GetBuffer(bufsize/sizeof(TCHAR)), &bufsize);
			temp.ReleaseBuffer();
			if (lresult == ERROR_SUCCESS)
			{
				value = temp;
				result = true;
			}
		}
		
		// Close the key
		::RegCloseKey(key);
	}
	
	return result;
}

BOOL CWinRegistry::WriteRegInt(HKEY root, const cdstring& path, const cdstring& valuename, const unsigned long& value)
{
	bool result = false;

	// Open the key for the full path (creating if required)
	HKEY key;
	DWORD dw;
	if (::RegCreateKeyEx(root, path.win_str(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &key, &dw) == ERROR_SUCCESS)
	{
		// Set the key's named value
		if (::RegSetValueEx(key, valuename.win_str(), 0, REG_DWORD, (LPBYTE)&value, 4) == ERROR_SUCCESS)
			result = true;
		
		// Close the key
		::RegCloseKey(key);
	}
	
	return result;
}

BOOL CWinRegistry::ReadRegInt(HKEY root, const cdstring& path, const cdstring& valuename, unsigned long& value)
{
	bool result = false;

	// Open the key for the full path
	HKEY key;
	if (::RegOpenKeyEx(root, path.win_str(), 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		// Get the key's named value
		DWORD dwCount = 4;
		if (::RegQueryValueEx(key, valuename.win_str(), 0, NULL, (LPBYTE)&value, &dwCount) == ERROR_SUCCESS)
			result = true;
		
		// Close the key
		::RegCloseKey(key);
	}
	
	return result;
}

BOOL CWinRegistry::DeleteRegValue(HKEY root, const cdstring& path, const cdstring& valuename)
{
	bool result = false;

	// Open the key for the full path
	HKEY key;
	if (::RegOpenKeyEx(root, path.win_str(), 0, KEY_READ|KEY_WRITE, &key) == ERROR_SUCCESS)
	{
		// Get the key's named value
		if (::RegDeleteValue(key, valuename.win_str()) == ERROR_SUCCESS)
			result = true;
		
		// Close the key
		::RegCloseKey(key);
	}
	
	return result;
}
