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


// CAdminLock.h

// Class controls lockable items in prefs

#ifndef __CADMINLOCK__MULBERRY__
#define __CADMINLOCK__MULBERRY__

#include "CAdmin.h"

#include "CAuthenticator.h"

class COptionsMap;
class CINETAccount;
class CMessage;
class CMIMEContent;
class CPreferences;

class CAdminLock : public CAdmin
{
public:
	static CAdminLock 	sAdminLock;
	
	// Values retrieved from actual prefs
	cdstrvect			mLockedMailboxServerAddr;			// Locked server address
	cdstrvect			mLockedPrefsServerAddr;				// Locked server address
	cdstrvect			mLockedSMTPServerAddr;				// Locked server address
	cdstrvect			mLockedAdbkServerAddr;				// Locked server address
	cdstrvect			mLockedManageSIEVEServerAddr;				// Locked server address
	cdstring			mLockedReturnDomain;				// Locked user's return address domain from multi-user prefs
	cdstring			mLockedHeader;						// Header lines that always appear
	cdstring			mLoginID;							// Login id for Originator-Info
	cdstring			mServerID;							// Server id for Originator-Info
	
	cdstring			mInitialPswd;						// Password typed in MU dialog

	CAdminLock();
	~CAdminLock() {}
	
	void	ReadFromMap(COptionsMap* theMap);				// Read data from a stream
	void	ResetLock();									// Reset preferences locks
	void	ProcessPrefs(const CPreferences* prefs);		// Read 'special' values from prefs
	void	VirtualDomainPrefs(CPreferences* prefs,			// Do lock on preferences for virtual domain support
								 const cdstring& uid);
	void	LockPrefs(CPreferences* prefs);					// Do lock on preferences
	bool	ProcessLockedReturnDomain(cdstring& locked, const cdstring& email);

	void	UpgradeAuthenticators(CPreferences* prefs);		// Upgrade all authenticators if required
	void	UpgradeAuthenticator(CINETAccount* acct);		// Upgrade an authenticator if required

	void	PrepareRemote();								// Switching to use remote prefs rather than local

	void	SetLoginID(const cdstring& login)
		{ mLoginID = login; }
	const cdstring& GetLoginID() const
		{ return mLoginID; }

	void	SetServerID(const cdstring& server);
	const cdstring& GetServerID() const
		{ return mServerID; }
	
	cdstring GetLoginToken() const;
	
	bool CanSend(const CMessage* msg) const;
	bool CanDownload(const CMIMEContent& content) const;

};

#endif
