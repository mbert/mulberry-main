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


// CWinRegistry.h

#ifndef __CWINREGISTRY__MULBERRY__
#define __CWINREGISTRY__MULBERRY__

#include "cdstring.h"

class CWinRegistry
{
public:
	static cdstrmap sMIME2Suffix;		// Cache for MIME -> extension
	static cdstrmap sFile2App;			// Cache for extension -> app

	static cdstring GetMIMEFromFile(const cdstring& suffix);
	static cdstring GetSuffixFromMIME(const cdstring& type, const cdstring& original);
	static cdstring GetAppFromFile(const cdstring& type);

	// Check for mailto helper
	static bool		CheckHelperMailto();
	static bool		SetHelperMailto();
	
	// Check for webcal helper
	static bool		CheckHelperWebcal();
	static bool		SetHelperWebcal();
	
	// Specials for HKEY_LOCAL_MACHINE registry
	static HKEY		GetMachineRegistryKey(bool write = false);
	static HKEY		GetMachineSectionKey(LPCTSTR lpszSection, bool write = false);
	static UINT		GetMachineProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	static CString	GetMachineProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	static BOOL		WriteMachineProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	static BOOL		WriteMachineProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	static BOOL		WriteRegString(HKEY root, const cdstring& path, const cdstring& valuename, const cdstring& value);
	static BOOL		ReadRegString(HKEY root, const cdstring& path, const cdstring& valuename, cdstring& value);
	static BOOL		WriteRegInt(HKEY root, const cdstring& path, const cdstring& valuename, const unsigned long& value);
	static BOOL		ReadRegInt(HKEY root, const cdstring& path, const cdstring& valuename, unsigned long& value);
	static BOOL		DeleteRegValue(HKEY root, const cdstring& path, const cdstring& valuename);
};

#endif
