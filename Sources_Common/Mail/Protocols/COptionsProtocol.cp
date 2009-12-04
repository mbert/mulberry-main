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


// COptionsProtocol

#include "COptionsProtocol.h"

#include "CACAPClient.h"
#include "CConnectionManager.h"
#include "CIMSPClient.h"
#include "CLocalPrefsClient.h"
#include "CMulberryApp.h"
#include "COptionsClient.h"
#include "CPreferences.h"
#include "CPreferencesFile.h"
#include "CPreferenceKeys.h"
#include "CWebDAVPrefsClient.h"

// COptionsProtocol: Handles quotas for all resources

// Constructor
COptionsProtocol::COptionsProtocol(COptionsAccount* account)
	: CINETProtocol(account)
{
	mMap = NULL;

	mClient = NULL;

	// Always able to disconnect
	SetFlags(eCanDisconnect, IsOfflineAllowed());

	CreateClient();
}

// Default destructor
COptionsProtocol::~COptionsProtocol()
{
	// Delete client
	RemoveClient();
	
	mMap = NULL;
	mValue = NULL;

} // CMboxProtocol::~CMboxProtocol

void COptionsProtocol::CreateClient()
{

	RemoveClient();

	// Check disconnected state
	if (CConnectionManager::sConnectionManager.IsConnected())
	{
		SetFlags(eIsOffline, false);
		SetFlags(eDisconnected, false);
		switch(GetAccountType())
		{
		case CINETAccount::eWebDAVPrefs:
			mClient = new prefsstore::CWebDAVPrefsClient(this);
			mFlags.Set(eCanPartialReadWrite, false);
			break;
		case CINETAccount::eIMSP:
			mClient = new CIMSPClient(this, NULL);
			mFlags.Set(eCanPartialReadWrite, true);
			break;
		case CINETAccount::eACAP:
			mClient = new CACAPClient(this, NULL);
			mFlags.Set(eCanPartialReadWrite, true);
			break;
		default:;
		}
	}
	else
	{
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, true);
		InitDisconnect();
		mClient = new CLocalPrefsClient(this);
		mFlags.Set(eCanPartialReadWrite, false);
	}

	mClient->SetVendor(cVendor);
	mClient->SetProduct(cProduct);

	CINETProtocol::mClient = mClient;
}

void COptionsProtocol::RemoveClient()
{
	delete mClient;
	mClient = NULL;
	CINETProtocol::mClient = NULL;
}

bool COptionsProtocol::DoesPartialPrefs() const
{
	// Test if partial read-write
	return mFlags.IsSet(eCanPartialReadWrite);
}

#pragma mark ____________________________Options

// Get all options from client
void COptionsProtocol::GetAllAttributes(const cdstring& entry)
{
	mClient->_FindAllAttributes(entry);
}

// Write current map to client
void COptionsProtocol::SetAllAttributes(const cdstring& entry)
{
	mClient->_SetAllAttributes(entry);
}

// Unset all in current map in client
void COptionsProtocol::DeleteEntry(const cdstring& entry)
{
	mClient->_DeleteEntry(entry);
}

// Get single option from client
void COptionsProtocol::GetAttribute(const cdstring& entry, const cdstring& key, cdstring& value)
{
	// Cache value ptr for reply
	mValue = &value;
	mClient->_GetAttribute(entry, key);
}

// Write current map to client
void COptionsProtocol::SetAttribute(const cdstring& entry, const cdstring& key, const cdstring& value)
{
	mClient->_SetAttribute(entry, key, value);
}

#pragma mark ____________________________Disconnected

const cdstring& COptionsProtocol::GetOfflineDefaultDirectory()
{
	static cdstring name("Preferences");
	return name;
}

void COptionsProtocol::GoOffline()
{
	// Do default to switch into disconnected mode
	CINETProtocol::GoOffline();

	// Now do save of current preferences
	if (CMulberryApp::sCurrentPrefsFile != NULL)
		CMulberryApp::sCurrentPrefsFile->SavePrefs(true, false);
}
