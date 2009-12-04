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


// Source for CINETProtocol class

#include "CINETProtocol.h"

#include "CAdminLock.h"
#include "CConnectionManager.h"
#include "CGeneralException.h"
#include "CINETClient.h"
#include "CLocalCommon.h"
#include "CMailAccount.h"
#include "CMailControl.h"
#include "CMulberryApp.h"
#include "CPasswordManager.h"
#include "CPreferences.h"
#include "CURL.h"

#include __stat_header
#include <unistd.h>

// __________________________________________________________________________________________________
// C L A S S __ C I N E T P R O T O C O L
// __________________________________________________________________________________________________

cdstrmap CINETProtocol::sUserPswdCache;					// Cache of user id & password

// Default constructor
CINETProtocol::CINETProtocol(CINETAccount* account)
{
	mMPState = eINETNotOpen;
	mClient = NULL;

	// Init instance variables
	SetAccount(account);

	SetFlags(eIsOffline);

} // CINETProtocol::CINETProtocol

// Copy constructor
CINETProtocol::CINETProtocol(const CINETProtocol& copy)
{
	// Init instance variables
	mFlags = copy.mFlags;
	mAccount = copy.mAccount;
	mAccountName = copy.mAccountName;
	mAccountType = copy.mAccountType;
	mAccountUniqueness = copy.mAccountUniqueness;
	mAuthenticatorUniqueness = copy.mAuthenticatorUniqueness;
	mMPState = eINETNotOpen;
	SetErrorProcess(false);
	SetNoErrorAlert(false);
	SetNoRecovery(false);
	mDescriptor = copy.mDescriptor;
	mType = copy.mType;

	mClient = NULL;

	mOfflineCWD = copy.mOfflineCWD;

} // CINETProtocol::CINETProtocol

// Default destructor
CINETProtocol::~CINETProtocol()
{
	// Always remove this just in case!
	CMailControl::RegisterPeriodic(this, false);
	
	mAccount = NULL;
	mClient = NULL;
	
	// Clean any free connections in cache - just in case
	CleanConnections();
}

void CINETProtocol::SetAccount(CINETAccount* account)
{
	// Change account first
	mAccount = account;
	mAccount->SetProtocol(this);

	// Look for change in name
	bool rename = (!mAccountName.empty() && (mAccountName != account->GetName()));

	// Now change cached items
	mAccountName = account->GetName();
	mAccountType = account->GetServerType();
	mAccountUniqueness = account->GetUniqueness();
	SetAuthenticatorUniqueness(mAccount->GetAuthenticator().GetUniqueness());
	
	if (IsOffline())
	{
		if (rename)
			RenameOffline();
		else
			InitOffline();
	}
	
	if (mClient)
	{
		// Don't allow throw
		try
		{
			mClient->Reset();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}

// Account removed from prefs
void CINETProtocol::RemoveAccount()
{
	// Must remove offline cache
	RemoveOffline();
}

void CINETProtocol::DirtyAccount()
{
	// Must override in derived classes
}

bool CINETProtocol::IsSecure() const
{
	// Check for TLS
	return (GetAccount()->GetTLSType() != CINETAccount::eNoTLS);
}

bool CINETProtocol::IsOfflineAllowed() const
{
	// Not if disabled by admin
	switch(GetAccountType())
	{
	case CINETAccount::eIMAP:
		// Only if disconnect allowed
		return !CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::ePOP3:
	case CINETAccount::eLocal:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalMbox;
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		// Only if disconnect allowed
		return !CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalAdbks;
	case CINETAccount::eLocalCalendar:
	case CINETAccount::eHTTPCalendar:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalCalendars;
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		// Only if disconnect allowed
		return !CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::eCardDAVAdbk:
		// Only if disconnect allowed
		return !CAdminLock::sAdminLock.mNoDisconnect;
	default:
		return true;
	}
}

bool CINETProtocol::IsOpenAllowed() const
{
	// Not if disabled by admin
	switch(GetAccountType())
	{
	case CINETAccount::eIMAP:
		// Only if disconnect allowed
		return CConnectionManager::sConnectionManager.IsConnected() ||
				!CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::ePOP3:
	case CINETAccount::eLocal:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalMbox;
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		// Only if disconnect allowed
		return CConnectionManager::sConnectionManager.IsConnected() ||
				!CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalAdbks;
	case CINETAccount::eLocalCalendar:
	case CINETAccount::eHTTPCalendar:
		// Only if local allowed
		return !CAdminLock::sAdminLock.mNoLocalCalendars;
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		// Only if disconnect allowed
		return CConnectionManager::sConnectionManager.IsConnected() ||
				!CAdminLock::sAdminLock.mNoDisconnect;
	case CINETAccount::eCardDAVAdbk:
		// Only if disconnect allowed
		return CConnectionManager::sConnectionManager.IsConnected() ||
				!CAdminLock::sAdminLock.mNoDisconnect;
	default:
		return true;
	}
}

const cdstring& CINETProtocol::GetGreeting() const
{
	return mClient->GetGreeting();
}

const cdstring& CINETProtocol::GetCapability() const
{
	return mClient->GetCapability();
}

const cdstring& CINETProtocol::GetCertText() const
{
	return mClient->GetCertText();
}

// Get URL for protocol
cdstring CINETProtocol::GetURL(bool full) const
{
	// Get scheme
	cdstring url;
	switch(GetAccountType())
	{
	case CINETAccount::eIMAP:
		url += cIMAPURLScheme;
		break;
	case CINETAccount::ePOP3:
		url += cPOPURLScheme;
		break;
	case CINETAccount::eLocal:
		url += cMboxURLScheme;
		break;
	case CINETAccount::eSMTP:
		url += cSMTPURLScheme;
		break;
	case CINETAccount::eIMSP:
		url += cIMSPURLScheme;
		break;
	case CINETAccount::eACAP:
		url += cACAPURLScheme;
		break;
	case CINETAccount::eLDAP:
		url += cLDAPURLScheme;
		break;
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
		url += cADBKURLScheme;
		break;
	case CINETAccount::eLocalCalendar:
	case CINETAccount::eHTTPCalendar:
		url += cCALURLScheme;
		break;
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
	case CINETAccount::eCardDAVAdbk:
		url += cHTTPURLScheme;
		break;
	default:;
	}

	// For user id based accounts add the user id if required
	if (full)
	{
		bool added = false;
		switch(GetAccountType())
		{
		case CINETAccount::eIMAP:
		case CINETAccount::ePOP3:
		case CINETAccount::eSMTP:
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
		case CINETAccount::eLDAP:
		case CINETAccount::eWebDAVCalendar:
		case CINETAccount::eCalDAVCalendar:
		case CINETAccount::eCardDAVAdbk:
			// Add user id based on auth method
			if (GetAccount()->GetAuthenticator().RequiresUserPswd())
			{
				cdstring temp = GetAccount()->GetAuthenticatorUserPswd()->GetUID();
				temp.EncodeURL();
				url += temp;
				added = true;
			}

			// Add auth method if plugin (sasl) based
			if (GetAccount()->GetAuthenticatorType() == CAuthenticator::ePlugin)
			{
				url += cURLAUTH;
				cdstring temp = GetAccount()->GetAuthenticator().GetSASLID();
				temp.EncodeURL();
				url += temp;
				added = true;
			}
			
			// Special for APOP
			else if ((GetAccountType() == CINETAccount::ePOP3) &&
						(GetAccount()->GetAuthenticatorType() == CAuthenticator::ePlainText) &&
						static_cast<const CMailAccount*>(GetAccount())->GetUseAPOP())
			{
				url += cURLAPOP;
				added = true;
			}
			break;
		default:;
		}
		
		if (added)
			url += "@";
	}
	
	// Now add server address or account name
	switch(GetAccountType())
	{
	case CINETAccount::eIMAP:
	case CINETAccount::ePOP3:
	case CINETAccount::eSMTP:
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
	case CINETAccount::eLDAP:
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
	case CINETAccount::eCardDAVAdbk:
		url += mDescriptor;
		break;
	case CINETAccount::eLocal:
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
	case CINETAccount::eLocalCalendar:
	case CINETAccount::eHTTPCalendar:
	default:
		url += GetAccountName();
		break;
	}
	
	return url;
}

// Open connection to protocol server
void CINETProtocol::Open()
{
	// Only bother if not already open
	if (IsOpenAllowed() && IsNotOpen())
	{
		// Get client to open
		SetErrorProcess(false);
		mClient->Open();
		mMPState = eINETOpen;
	}

} // CINETProtocol::Open

// Close connection to protocol server
void CINETProtocol::Close()
{
	// Only bother if not already closed
	if (IsNotOpen())
		return;

	try
	{
		// Logoff if required
		if (IsLoggedOn())
			Logoff();

		// Get client to close
		mClient->Close();

		mMPState = eINETNotOpen;
		SetErrorProcess(false);
		
		// Clean any free connections in cache
		CleanConnections();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		mMPState = eINETNotOpen;
		SetErrorProcess(false);
		
		// Clean any free connections in cache
		CleanConnections();

		CLOG_LOGRETHROW;
		throw;
	}

} // CINETProtocol::Close

// Logon to server
void CINETProtocol::Logon()
{
	// No need to block since if its not logged in there can be no other network
	// operation in progress. If it is logged in it won't issue a network call either.

	if (IsOpenAllowed() && !IsLoggedOn())
	{
		// Recovering after a failure should be on here as it can be turned off
		// at logoff
		SetNoRecovery(false);

		// Get client to logon
		mClient->Logon();

		// Recache user id & password after successful logon
		if (GetAccount()->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

			// Only bother if it contains something
			if (!auth->GetPswd().empty())
			{
				CINETProtocol::SetCachedPswd(auth->GetUID(), auth->GetPswd());
				CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
			}
		}

		// Make copy of current authenticator
		SetAuthenticatorUniqueness(GetAccount()->GetAuthenticator().GetUniqueness());

		// Add to list of periodic items
		CMailControl::RegisterPeriodic(this, true);

		mMPState = eINETLoggedOn;
		SetErrorProcess(false);

		// Broadcast change in state
		Broadcast_Message(eBroadcast_Logon, this);
	}

} // CINETProtocol::Logon

// Logoff server
void CINETProtocol::Logoff()
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	if (IsLoggedOn())
	{
		// Remove from list of periodic items
		CMailControl::RegisterPeriodic(this, false);

		// Do without errors appearing on screen as the user
		// is not really interested in failures during logoff
		bool old_error_alert = GetNoErrorAlert();
		SetNoErrorAlert(true);

		// No point in recovering after a failure
		SetNoRecovery(true);

		try
		{
			// Get client to logoff
			mClient->Logoff();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		SetNoErrorAlert(old_error_alert);

		// Set flag
		mMPState = eINETLoggedOff;
		SetErrorProcess(false);

		// Broadcast change in state
		Broadcast_Message(eBroadcast_Logoff, this);
		
		// Clean any free connections in cache
		CleanConnections();
	}

} // CINETProtocol::Logoff

// Forced reconnect to server
void CINETProtocol::Forceon()
{
	// Remove from list of periodic items
	CMailControl::RegisterPeriodic(this, false);

	// Set flag
	mMPState = eINETLoggedOff;
	SetErrorProcess(false);

	// Get client to logon
	mClient->Logon();
	
	// Add to list of periodic items
	CMailControl::RegisterPeriodic(this, true);

	mMPState = eINETLoggedOn;
	SetErrorProcess(false);

} // CINETProtocol::Forceoff

// Forced off server
void CINETProtocol::Forceoff()
{
	// Remove from list of periodic items
	CMailControl::RegisterPeriodic(this, false);

	// Make sure client clean-up is done too
	if (mClient != NULL)
		mClient->Forceoff();

	// Set flag
	mMPState = eINETLoggedOff;
	SetErrorProcess(false);

	// Broadcast change in state
	Broadcast_Message(eBroadcast_Logoff, this);

	// Clean any free connections in cache
	CleanConnections();

} // CINETProtocol::Forceoff

// Porgram initiated abort
void CINETProtocol::Abort(bool silent, bool recover)
{
	SetNoErrorAlert(silent);
	SetNoRecovery(!recover);

	mClient->Abort();
}

// Get cached password
cdstring CINETProtocol::GetCachedPswd(const cdstring& uid)
{
	// Recache user id & password after successful logon
	cdstrmap::iterator found = sUserPswdCache.find(uid);

	if (found != sUserPswdCache.end())
		return (*found).second;
	else
		return cdstring::null_str;

} // CINETProtocol::GetCachedPswd

void CINETProtocol::SetCachedPswd(const cdstring& uid, const cdstring& pswd)
{
	cdstrmap::iterator found = sUserPswdCache.find(uid);
	if (found == sUserPswdCache.end())
		sUserPswdCache.insert(cdstrmap::value_type(uid, pswd));
	else if (pswd != (*found).second)
		(*found).second = pswd;
}

// Called during idle
void CINETProtocol::SpendTime(bool force_tickle)
{
	// Only do if logged on and no error and will not block
	if (IsLoggedOn() && !IsErrorProcess())
	{
		// Must try block
		if (_mutex.try_lock())
		{

			try
			{
				// Do tickle if logged on
				mClient->_Tickle(force_tickle);

				_mutex.release();
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				_mutex.release();
				CLOG_LOGRETHROW;
				throw;
			}
		}
	}

	ConnectionIdle();

} // CINETProtocol::SpendTime

#pragma mark ____________________________Offline

void CINETProtocol::InitOffline()
{
	InitOfflineCWD();
}

void CINETProtocol::InitOfflineCWD(bool check)
{
	// Not if disabled by admin
	if (!IsOfflineAllowed())
		return;

	try
	{
		// Get any previous CWD
		cdstring old_cwd = mOfflineCWD;

		// Get CWD from account (only for specific types
		cdstring account_wd;
		switch(GetAccountType())
		{
		case CINETAccount::eLocal:
		case CINETAccount::ePOP3:
		case CINETAccount::eLocalAdbk:
		case CINETAccount::eOSAdbk:
		case CINETAccount::eLocalCalendar:
		case CINETAccount::eHTTPCalendar:
			account_wd = GetAccount()->GetCWD().GetName();
			break;
		default:
			if (CanDisconnect())
				account_wd = CPreferences::sPrefs->mDisconnectedCWD.GetValue();
		}

		// If empty then use relative default
		if (account_wd.empty())
		{
			// Use default CWD
			mOfflineCWD = CConnectionManager::sConnectionManager.GetCWD();
			::chkdir(mOfflineCWD);

			// Append default items
			DefaultOfflineCWD(mOfflineCWD, check);
		}
		else
		{
			// Determine absolute or relative path
			if (IsRelativePath(account_wd))
			{
				// Prefix with CWD
				mOfflineCWD = CConnectionManager::sConnectionManager.GetCWD();
				::chkdir(mOfflineCWD);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
				mOfflineCWD += &account_wd.c_str()[1];
#elif __dest_os == __win32_os
				mOfflineCWD += &account_wd.c_str()[1];
#else
				mOfflineCWD += account_wd;
#endif
			}
			else
				mOfflineCWD = account_wd;

			if (CanDisconnect())
			{
				::chkdir(mOfflineCWD);
				DefaultOfflineCWD(mOfflineCWD, check);
			}
			else if (check)
				::chkdir(mOfflineCWD);
		}
	}
	catch (CGeneralException& gex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Report error
		::report_file_error(mOfflineCWD, gex.GetErrorCode());

		// Could not initialise account
		mOfflineCWD = cdstring::null_str;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Could not initialise account
		mOfflineCWD = cdstring::null_str;
	}
}

void CINETProtocol::DefaultOfflineCWD(cdstring& cwd, bool check)
{
	// Not if disabled by admin
	if (!IsOfflineAllowed())
		return;

	try
	{
		// Create disconnected hierarchy if required
		if (CanDisconnect())
		{
			cwd += "Disconnected";
			cwd += os_dir_delim;
			::chkdir(cwd);
		}

		// Create first level of default
		cwd += GetOfflineDefaultDirectory();
		cwd += os_dir_delim;
		::chkdir(cwd);

		// Create second level of default
		if (!IsDisconnected())
		{
			switch(GetAccountType())
			{
			case CINETAccount::eLocal:
				cwd += "Offline";
				cwd += os_dir_delim;
				::chkdir(cwd);
				break;
			case CINETAccount::ePOP3:
				cwd += "POP3";
				cwd += os_dir_delim;
				::chkdir(cwd);
				break;
			default:;
			}
		}

		// Add account name
		if ((GetAccountType() != CINETAccount::eLocalAdbk) &&
			(GetAccountType() != CINETAccount::eOSAdbk) &&
			(GetAccountType() != CINETAccount::eLocalCalendar) &&
			(GetAccountType() != CINETAccount::eHTTPCalendar))
		{
			cwd += LocalFileName(GetAccountName(), 0, false, true);
			cwd += os_dir_delim;
			if (check)
				::chkdir(cwd);
		}
	}
	catch(CGeneralException& gex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Report error
		::report_file_error(cwd, gex.GetErrorCode());
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CINETProtocol::InitDisconnect()
{
	// Must init disconnected CWD
	InitOfflineCWD();
}

void CINETProtocol::RenameOffline()
{
	// Not if disabled by admin
	if (!IsOfflineAllowed())
		return;

	if (!mOfflineCWD.empty())
	{
		cdstring old_cwd = mOfflineCWD;
		
		// Reinit CWD
		InitOfflineCWD(false);
		cdstring new_cwd = mOfflineCWD;

		if (!new_cwd.empty())
		{
			// Strip trailing dir delims
			old_cwd[old_cwd.length() - 1] = 0;
			new_cwd[new_cwd.length() - 1] = 0;

			// Do rename
			::moverename_file(old_cwd, new_cwd);
		}
	}
}

void CINETProtocol::RemoveOffline()
{
	// Recursive delete of entire offline/disconnected mailstore
	//if (!mOfflineCWD.empty())
	//	::delete_dir(mOfflineCWD);
}

void CINETProtocol::GoOffline()
{
	// Only if already online
	if (!IsOffline())
	{
		// Check logged in state to recover it
		bool is_open = IsOpen();
		bool logged_in = IsLoggedOn();

		// Close current proto - do without errors appearing on screen as the user
		// is not really interested in failures during close
		bool old_error_alert = GetNoErrorAlert();
		SetNoErrorAlert(true);
		try
		{
			Close();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		SetNoErrorAlert(old_error_alert);

		// Create new local client
		CreateClient();

		try
		{
			if (is_open || logged_in)
				Open();

			if (logged_in)
				Logon();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}

void CINETProtocol::GoOnline()
{
	// Only if already offline
	// NB Disconnected state could be turned on while disconnected so client is still IMAP
	if (IsOffline())
	{
		// Check logged in state to recover it
		bool is_open = IsOpen();
		bool logged_in = IsLoggedOn();

		// Close current proto
		try
		{
			Close();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}

		// Create new local client
		CreateClient();

		// Allow these to throw up
		if (is_open || logged_in)
			Open();

		if (logged_in)
			CMulberryApp::sApp->BeginINET(this);
	}
}

void CINETProtocol::ForceDisconnect(bool force)
{
	if (IsForceDisconnect() ^ force)
	{
		// Change flag
		mFlags.Set(eForceDisconnect, force);
		
		// Do offline/online change based on global state
		if (CConnectionManager::sConnectionManager.IsConnected())
		{
			if (IsForceDisconnect() && CanDisconnect() && !IsDisconnected())
			{
				GoOffline();
				
				// Update account's login at startup to make this change persistent
				GetAccount()->SetLoginAtStart(false);
				DirtyAccount();
			}
			else if (!IsForceDisconnect() && IsDisconnected())
			{
				GoOnline();
				
				// Update account's login at startup to make this change persistent
				GetAccount()->SetLoginAtStart(true);
				DirtyAccount();
			}
		}
		else
		{
			// Must already be disconnected so leave as-is
		}
	}
}

// Set into synchronising mode
void CINETProtocol::SetSynchronising()
{
	// Do nothing here
}

#pragma mark ____________________________Connection caching

// Get a new connection cloned from this one
CINETProtocol* CINETProtocol::NewConnection()
{
	// Lock the mutex first
	cdmutex::lock_cdmutex _lock(_cache_mutex);
	
	// Look for an unused connection in the cache
	CINETProtocol* proto = NULL;
	for(CConnections::iterator iter = mCachedConnections.begin(); iter != mCachedConnections.end(); iter++)
	{
		// Look for one not being used
		if (!(*iter).mInUse)
		{
			// Should check whether its free from errors here
			
			// Now mark it as being used prior to returning it
			proto = (*iter).mConnection;
			(*iter).mInUse = true;
			(*iter).mFreeTime = 0;

			// Turn on standard error reporting for connection
			(*iter).mConnection->SetErrorProcess(false);
			(*iter).mConnection->SetNoErrorAlert(false);
			(*iter).mConnection->SetNoRecovery(false);
			break;
		}
	}
	
	// Return the one found
	if (proto)
		return proto;
	
	// Create a clone
	proto = CloneConnection();
	if (!proto)
		return NULL;

	// Add record to cache list
	SConnectionCache cache;
	cache.mConnection = proto;
	cache.mInUse = true;
	cache.mFreeTime = 0;
	mCachedConnections.push_back(cache);
	
	// Return it
	return proto;
}

// Connection is freed
void CINETProtocol::EndConnection(CINETProtocol* proto, bool close_it)
{
	// Lock the mutex first
	cdmutex::lock_cdmutex _lock(_cache_mutex);
	
	// Find matching connection in the cache
	bool found = false;
	for(CConnections::iterator iter = mCachedConnections.begin(); iter != mCachedConnections.end(); iter++)
	{
		// Look for this one
		if ((*iter).mConnection == proto)
		{
			// If close required, close it
			if (close_it)
			{
				// Close connection and erase record
				CloseConnection((*iter).mConnection);
				mCachedConnections.erase(iter);
			}
			
			// Otherwise mark it as free in the cache
			else
			{
				// Should check whether its free from errors here
				
				// Now mark it as being free
				(*iter).mInUse = false;
				(*iter).mFreeTime = ::time(NULL);

				// Turn off standard error reporting for connection
				// as any failures at this point should be silent
				(*iter).mConnection->SetErrorProcess(false);
				(*iter).mConnection->SetNoErrorAlert(true);
				(*iter).mConnection->SetNoRecovery(true);
			}
			found = true;
			break;
		}
	}

	// Done if found
	if (found)
		return;
	
	// Hmm - get here if the connection is not in the cache list
	// This is really an error state as all connections should be cached
	
	// Close it
	CloseConnection(proto);
}

// Idle time processing
void CINETProtocol::ConnectionIdle()
{
	const int cCacheExpirySecs = 5 * 60;	// 5 minutes
	const unsigned long cMaxFreeCount = 3;

	// Lock the mutex first
	cdmutex::lock_cdmutex _lock(_cache_mutex);
	
	// 1. check ones in an error state

	// 2. look for ones that are not in use and have expired
	unsigned long free_count = 0;
	for(CConnections::iterator iter = mCachedConnections.begin(); iter != mCachedConnections.end(); )
	{
		// Check whether its free and expired
		if (!(*iter).mInUse && (::time(NULL) > (*iter).mFreeTime + cCacheExpirySecs))
		{
			// Close it out
			CloseConnection((*iter).mConnection);
			
			// Delete the record
			iter = mCachedConnections.erase(iter);
			continue;
		}
		
		// Count remaining free ones
		else if (!(*iter).mInUse)
			free_count++;
		
		iter++;
	}
	
	// 3. remove excess free
	if (free_count > cMaxFreeCount)
	{
		for(CConnections::iterator iter = mCachedConnections.begin(); iter != mCachedConnections.end(); )
		{
			// Remove if free
			if (!(*iter).mInUse)
			{
				// Close it out
				CloseConnection((*iter).mConnection);
				
				// Delete the record
				iter = mCachedConnections.erase(iter);

				// Bump down count and exit when max free reached
				free_count--;
				if (free_count <= cMaxFreeCount)
					break;
				
				continue;
			}
			
			iter++;
		}
	}
}

// Close out a connection
void CINETProtocol::CloseConnection(CINETProtocol* proto)
{
	// Do logout
	try
	{
		// Prevent error reporting or recovery attempts when closing it
		proto->SetErrorProcess(false);
		proto->SetNoErrorAlert(true);
		proto->SetNoRecovery(true);
		
		// Logoff and close
		proto->Logoff();
		proto->Close();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	// Always delete it to prevent leak
	delete proto;
}

// Remove all free connections
void CINETProtocol::CleanConnections()
{
	// Lock the mutex first
	cdmutex::lock_cdmutex _lock(_cache_mutex);
	
	for(CConnections::iterator iter = mCachedConnections.begin(); iter != mCachedConnections.end(); )
	{
		// Remove if free
		if (!(*iter).mInUse)
		{
			// Close it out
			CloseConnection((*iter).mConnection);
			
			// Delete the record
			iter = mCachedConnections.erase(iter);
			continue;
		}
		
		iter++;
	}
}

// Create duplicate, empty connection
CINETProtocol* CINETProtocol::CloneConnection()
{
	// Child classes that want to do connection caching override this
	return NULL;
}

#pragma mark ____________________________StProtocolLogin

StProtocolLogin::StProtocolLogin(CINETProtocol* proto)
{
	mProtocol = proto;
	mWasLoggedIn = mProtocol->IsLoggedOn();
	if (!mWasLoggedIn)
		mProtocol->Logon();
}

StProtocolLogin::~StProtocolLogin()
{
	if (!mWasLoggedIn)
		mProtocol->Logoff();
}
