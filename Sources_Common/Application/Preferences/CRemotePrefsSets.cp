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


// CRemotePrefsSets.cp

// Class to handle remote sets of preferences

#include "CRemotePrefsSets.h"

#include "CLog.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPreferenceKeys.h"
#include "CRemoteOptionsMap.h"
#include "CStringUtils.h"
#include <algorithm>

CRemotePrefsSet::CRemotePrefsSet()
{
}

CRemotePrefsSet::~CRemotePrefsSet()
{
}

void CRemotePrefsSet::ListRemoteSets(void)
{
	try
	{
		cdstring key = cPrefsSetListKey;
		cdstring value;

		// Get single option from server
		{
			StProtocolLogin login(CMulberryApp::sOptionsProtocol);
			CMulberryApp::sOptionsProtocol->GetAttribute(key, cdstring::null_str, value);
		}

		// Parse S-Expression into list
		value.ParseSExpression(mSets);
		
		// Bug fix for empty prefs set names
		for(cdstrvect::iterator iter = mSets.begin(); iter != mSets.end(); )
		{
			if ((*iter).empty())
			{
				iter = mSets.erase(iter);
				continue;
			}
			
			iter++;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CRemotePrefsSet::UpdateRemoteSets(void)
{
	try
	{
		cdstring key = cPrefsSetListKey;
		cdstring value;

		// Create value from list (use old style bracketed string list for compatibility)
		value.CreateSExpression(mSets, true);

		// Set single option on server
		{
			StProtocolLogin login(CMulberryApp::sOptionsProtocol);
			CMulberryApp::sOptionsProtocol->SetAttribute(key, cdstring::null_str, value);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

bool CRemotePrefsSet::IsDuplicateName(const cdstring& test, cdstring& duplicate) const
{
	cdstring temp1 = test;
	ConvertName(temp1);

	// Scan each item doing IMSP comparison for duplicate
	for(cdstrvect::const_iterator iter = mSets.begin(); iter != mSets.end(); iter++)
	{
		cdstring temp2 = *iter;
		ConvertName(temp2);
		if (temp1 == temp2)
		{
			duplicate = *iter;
			return true;
		}
	}
	
	// Verify against default set
	cdstring temp2(cDefaultPrefsSetKey_2_0);
	ConvertName(temp2);
	if (temp1 == temp2)
	{
		duplicate = cDefaultPrefsSetKey_2_0;
		return true;
	}
	
	return false;
}

void CRemotePrefsSet::DeleteSet(cdstring set)
{
	cdstrvect::iterator name = std::find(mSets.begin(), mSets.end(), set);

	// Can only do if it exists
	if ((name == mSets.end()) && (set != cDefaultPrefsSetKey_2_0))
		return;

	CRemoteOptionsMap deleteMap;
	deleteMap.SetTitle(set);

	// Always reset map's protocol
	deleteMap.SetOptionsProtocol(CMulberryApp::sOptionsProtocol);

	try
	{
		// Login here so that all operations are done with single connection
		StProtocolLogin login(CMulberryApp::sOptionsProtocol);

		// Read map
		deleteMap.ReadMap();

		// Delete on server
		deleteMap.DeleteMap();

		if (name != mSets.end())
		{
			// Erase entry
			mSets.erase(name);

			// Update set list on servers
			UpdateRemoteSets();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CRemotePrefsSet::RenameSet(cdstring old_set, cdstring new_set)
{
	cdstrvect::iterator name = std::find(mSets.begin(), mSets.end(), old_set);

	// Can only do if it exists
	if (name == mSets.end())
		return;

	CRemoteOptionsMap renameMap;
	renameMap.SetTitle(old_set);

	// Always reset map's protocol
	renameMap.SetOptionsProtocol(CMulberryApp::sOptionsProtocol);

	try
	{
		// Login here so that all operations are done with single connection
		StProtocolLogin login(CMulberryApp::sOptionsProtocol);

		// Read map
		renameMap.ReadMap();

		// Change name
		renameMap.SetTitle(new_set);

		// Write out new map
		renameMap.WriteMap(false);

		// Change name back
		renameMap.SetTitle(old_set);

		// Delete on server
		renameMap.DeleteMap();

		// Rename entry
		*name = new_set;

		// Update set list on servers
		UpdateRemoteSets();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CRemotePrefsSet::ConvertName(cdstring& name) const
{
	// Make it upper case
	::strupper(name.c_str_mod());
	
	// Convert spaces to underscores
	for(char* p = name.c_str_mod(); *p; p++)
		if (*p == ' ') *p = '_';
}
