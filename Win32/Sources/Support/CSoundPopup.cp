/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CSoundPopup class

#include "CSoundPopup.h"

#include "CUnicodeUtils.h"

#include <regstr.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSoundPopup::CSoundPopup()
{
}

// Default destructor
CSoundPopup::~CSoundPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CSoundPopup::SyncMenu(void)
{
	// Remove any existing items from main menu
	short num_menu = GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		GetPopupMenu()->RemoveMenu(0, MF_BYPOSITION);
	
	cdstrvect items;
	cdstrvect default_items;

	HKEY hSchemesKey = NULL;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPS_DEFAULT, 0, KEY_READ, &hSchemesKey) == ERROR_SUCCESS)
	{
		// First look up default items only
		int index = 0;
		TCHAR szSubKeyName[256];
		DWORD bufSize = 256;
		while(::RegEnumKey(hSchemesKey, index++, szSubKeyName, bufSize) == ERROR_SUCCESS)
			default_items.push_back(szSubKeyName);

		::RegCloseKey(hSchemesKey);
	}


	HKEY hEventLabelsKey = NULL;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EVENTLABELS, 0, KEY_READ, &hEventLabelsKey) == ERROR_SUCCESS)
	{
		// Get friendly names of each default item
		for(cdstrvect::const_iterator iter = default_items.begin(); iter != default_items.end(); iter++)
		{
			HKEY hItemKey = NULL;
			if (::RegOpenKeyEx(hEventLabelsKey, (*iter).win_str(), 0, KEY_READ, &hItemKey) == ERROR_SUCCESS)
			{
				DWORD dwType, dwCount;
				LONG lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, NULL, &dwCount);
				if ((lResult == ERROR_SUCCESS) && (dwType == REG_SZ))
				{
					CString temp;
					lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
					temp.ReleaseBuffer();
					if (lResult == ERROR_SUCCESS)
					{
						items.push_back(cdstring(temp));
					}
				}
			}
				
			::RegCloseKey(hItemKey);
		}

		::RegCloseKey(hEventLabelsKey);
	}

	// Sort list
	std::stable_sort(items.begin(), items.end());
	
	// Add items to menu
	short menu_id = mCommandBase;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(GetPopupMenu(), MF_STRING, menu_id++, (*iter).c_str());


}

cdstring CSoundPopup::GetSound() const
{
	cdstring result;

	// Get title of selected item
	CString item_title;
	GetPopupMenu()->GetMenuString(mValue, item_title, MF_BYCOMMAND);
	
	// Now find it in registry
	HKEY hEventLabelsKey = NULL;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EVENTLABELS, 0, KEY_READ, &hEventLabelsKey) == ERROR_SUCCESS)
	{
		int index = 0;
		TCHAR szSubKeyName[256];
		DWORD bufSize = 256;
		while(::RegEnumKey(hEventLabelsKey, index++, szSubKeyName, bufSize) == ERROR_SUCCESS)
		{
			HKEY hItemKey = NULL;
			if (::RegOpenKeyEx(hEventLabelsKey, szSubKeyName, 0, KEY_READ, &hItemKey) == ERROR_SUCCESS)
			{
				DWORD dwType, dwCount;
				LONG lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, NULL, &dwCount);
				if ((lResult == ERROR_SUCCESS) && (dwType == REG_SZ))
				{
					CString temp;
					lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, (LPBYTE)temp.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
					temp.ReleaseBuffer();
					if (lResult == ERROR_SUCCESS)
					{
						// Check for match
						if (item_title == temp)
						{
							result = szSubKeyName;
							break;
						}
					}
				}
				
				::RegCloseKey(hItemKey);
			}
		}

		::RegCloseKey(hEventLabelsKey);
	}
	
	return result;
}

void CSoundPopup::SetSound(const cdstring& snd_title)
{
	CString menu_title;

	// Now find it in registry
	HKEY hEventLabelsKey = NULL;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EVENTLABELS, 0, KEY_READ, &hEventLabelsKey) == ERROR_SUCCESS)
	{
		HKEY hItemKey = NULL;
		if (::RegOpenKeyEx(hEventLabelsKey, snd_title.win_str(), 0, KEY_READ, &hItemKey) == ERROR_SUCCESS)
		{
			DWORD dwType, dwCount;
			LONG lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, NULL, &dwCount);
			if ((lResult == ERROR_SUCCESS) && (dwType == REG_SZ))
			{
				lResult = ::RegQueryValueEx(hItemKey, NULL, NULL, &dwType, (LPBYTE)menu_title.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
				menu_title.ReleaseBuffer();
				if (lResult != ERROR_SUCCESS)
				{
					menu_title = _T("");
				}
			}
				
			::RegCloseKey(hItemKey);
		}

		::RegCloseKey(hEventLabelsKey);
	}

	// Find item that matches
	short num_menu = GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
	{
		CString item_title;
		GetPopupMenu()->GetMenuString(i, item_title, MF_BYPOSITION);
		if (item_title == menu_title)
		{
			SetValue(i + mCommandBase);
			return;
		}
	}
	
	SetValue(mCommandBase);
}
