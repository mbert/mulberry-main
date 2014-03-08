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


// Source for CPreferences class

#include "CPreferences.h"

#include "CAddress.h"
#include "CAddressBook.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#if __dest_os == __win32_os
#include "CFontCache.h"
#endif
#include "CGeneralException.h"
#include "CICalendarManager.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#endif
#include "CIMAPCommon.h"
#include "CINETCommon.h"
#include "CMailAccountManager.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPluginManager.h"
#include "CPreferencesFile.h"
#include "CRegistration.h"
#include "CRFC822.h"
#include "CStringUtils.h"
#include "CTCPSocket.h"
#ifdef __MULBERRY
#include "CToolbarManager.h"
#endif
#include "CUtils.h"

#include <typeinfo>
#include <algorithm>
#include <memory>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F E R E N C E S
// __________________________________________________________________________________________________

// S T A T I C  I T E M S

// Init with default values at app startup
#if __framework == __jx
CPreferences* CPreferences::sPrefs = NULL;
#else
//CPreferences* CPreferences::sPrefs = new CPreferences;
CPreferences* CPreferences::sPrefs = NULL;
#endif

cdstring CPreferences::sFromAddr;

// Default constructor
CPreferences::CPreferences()
{
	// Create empty prefs
#ifdef __MULBERRY
	vers = CMulberryApp::GetVersionNumber();
#else
	long temp = 0L;
	vers = *(NumVersion*) &temp;
#endif
	mUpdateVers = false;

	mHelpState.mValue = true;
	mDetailedTooltips.mValue = true;
	mUse_IC.mValue = false;
	mCheckDefaultMailClient.mValue = true;
	mCheckDefaultWebcalClient.mValue = true;
	mWarnMailtoFiles.mValue = true;
	mSimple.mValue = true;
	mDisconnected.mValue = false;
	mSMTPQueueEnabled.mValue = true;

//--------------------Network Control Prefs
	InitNetworkControlPrefs();

//--------------------Account Prefs
	InitAccountPrefs();

//--------------------Alert Prefs
	InitAlertPrefs();

//--------------------Display Prefs
	InitDisplayPrefs();

//---------------------Formatting Prefs
	InitFormattingPrefs();

//--------------------Mailbox Prefs
	InitMailboxPrefs();

//--------------------Favourites Prefs
	InitFavouritesPrefs();

//--------------------Search Prefs
	InitSearchPrefs();

//--------------------Rules Prefs
	InitRulesPrefs();

//--------------------Message Prefs
	InitMessagePrefs();

//--------------------Draft Prefs
	InitLetterPrefs();

//--------------------Security Prefs
	InitSecurityPrefs();

//--------------------Identity Prefs
	InitIdentityPrefs();

//--------------------Address Prefs
	InitAddressPrefs();

//--------------------Calendar Prefs
	InitCalendarPrefs();

//--------------------Attachment Prefs
	InitAttachmentPrefs();

//--------------------Speech Prefs
#ifdef __use_speech
	InitSpeechPrefs();
#endif

//--------------------Miscellaneous Prefs
	InitMiscellaneousPrefs();

//--------------------Window Prefs
	// Reset default window states
	InitWindowPrefs();


//--------------------Post processing
	// Treat account as new
	NewAccount(mMailAccounts.mValue.front());

}

// Destructor
CPreferences::~CPreferences()
{
	// Dispose of filter manager
	delete mFilterManager;

	// Dispose of all window states
	for(CWindowStateArray::const_iterator iter = mServerWindowInfo.mValue.begin();
		iter != mServerWindowInfo.mValue.end(); iter++)
		delete *iter;

	for(CWindowStateArray::const_iterator iter = mMailboxWindowInfo.mValue.begin();
		iter != mMailboxWindowInfo.mValue.end(); iter++)
		delete *iter;

#if 0
	for(CWindowStateArray::const_iterator iter = mAddressBookWindowInfo.mValue.begin();
		iter != mAddressBookWindowInfo.mValue.end(); iter++)
		delete *iter;
#else
	for(CWindowStateArray::const_iterator iter = mNewAddressBookWindowInfo.mValue.begin();
		iter != mNewAddressBookWindowInfo.mValue.end(); iter++)
		delete *iter;
#endif

	// Do not allow prefs to dangle
	if (sPrefs == this)
		sPrefs = nil;
}

// Replace global prefs with this prefs
void CPreferences::SetAsDefault()
{
	// Delete current one
	if (sPrefs != this)
		delete sPrefs;

	// Make global equal to this one
	sPrefs = this;

	// Always reset prefs account assuming one already present and a new one is available
	if (CMulberryApp::sOptionsProtocol && mRemoteAccounts.GetValue().size())
		CMulberryApp::sOptionsProtocol->SetAccount(mRemoteAccounts.GetValue().front());

	// Do changed action
	Changed();
}

// Do prefs changed action
void CPreferences::Changed()
{
	// Update netwrk timeouts
	CTCPSocket::SetTimeouts(mConnectRetryTimeout.GetValue(), mConnectRetryMaxCount.GetValue());

	// Update smart address cache
	sFromAddr = (sPrefs->mIdentities.mValue.size() ? sPrefs->mIdentities.mValue.front().GetFrom() : cdstring::null_str);
	if (!sFromAddr.empty())
	{
		// Parse out email address part
		CAddress addr(sFromAddr);
		sFromAddr = addr.GetMailAddress();
	}

	// Update RFC822 wrapper
	CRFC822::SetWrapLength(sPrefs->wrap_length.mValue);

#if __dest_os == __win32_os
	// Force fonts update
	CFontCache::ResetFonts(sPrefs);
#endif

	// Force mailbox menu reset
	CCopyToMenu::SetMenuOptions(mUseCopyToMboxes.GetValue(), mUseAppendToMboxes.GetValue());

	// Force apply rules menu to change
	if (CApplyRulesMenu::sApplyRules && mFilterManager)
		mFilterManager->Add_Listener(CApplyRulesMenu::sApplyRules);
	
	// Force toolbars to update
#ifdef __MULBERRY
	CToolbarManager::sToolbarManager.PrefsChanged();
#endif

	// Force calendar item updates
	if (iCal::CICalendarManager::sICalendarManager != NULL)
		iCal::CICalendarManager::sICalendarManager->SetDefaultTimezoneID(CPreferences::sPrefs->mTimezone.GetValue());
}

// Check for valid prefs
bool CPreferences::Valid(bool showAlert)
{
	// Force Admin locks
	CAdminLock::sAdminLock.LockPrefs(this);

	// Check for empty strings
	bool error = false;
	bool extra = false;
	cdstring extra_txt;
	const char* str_index = NULL;

	try
	{
		// Test for default mail account
		if (!mMailAccounts.mValue.size())
		{
			str_index = "Alerts::Preferences::Invalid_MailAccount";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Test each mail account status
		for(CMailAccountList::const_iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
		{
			extra_txt = (*iter)->GetName();

			if (::strchr(extra_txt.c_str(), cMailAccountSeparator))
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_AccountName";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			if (!(*iter)->IsLocal() && (*iter)->GetServerIP().empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_IMAPServerIP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Check authenticators
			if ((*iter)->GetAuthenticator().RequiresKerberos())
			{
				CAuthenticatorKerberos* kerb = (*iter)->GetAuthenticatorKerberos();
				if (!kerb->GetDefaultPrincipal() && kerb->GetServerPrincipal().empty())
				{
					extra = true;
					str_index = "Alerts::Preferences::Invalid_KerberosRealm";
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}

		// Test for default SMTP account
		if (!mSMTPAccounts.mValue.size())
		{
			str_index = "Alerts::Preferences::Invalid_SMTPAccount";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Test each SMTP account status
		for(CSMTPAccountList::const_iterator iter = mSMTPAccounts.mValue.begin(); iter != mSMTPAccounts.mValue.end(); iter++)
		{
			extra_txt = (*iter)->GetName();

			if ((*iter)->GetServerIP().empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_SMTPServerIP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

		// Test each remote options account status
		for(COptionsAccountList::const_iterator iter = mRemoteAccounts.mValue.begin(); iter != mRemoteAccounts.mValue.end(); iter++)
		{
			extra_txt = (*iter)->GetName();

			if ((*iter)->GetServerIP().empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_RemoteServerIP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Check authenticators
			if ((*iter)->GetAuthenticator().RequiresKerberos())
			{
				CAuthenticatorKerberos* kerb = (*iter)->GetAuthenticatorKerberos();
				if (!kerb->GetDefaultPrincipal() && kerb->GetServerPrincipal().empty())
				{
					extra = true;
					str_index = "Alerts::Preferences::Invalid_KerberosRealm";
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}

		// Test each address account status
		for(CAddressAccountList::const_iterator iter = mAddressAccounts.mValue.begin(); iter != mAddressAccounts.mValue.end(); iter++)
		{
			extra_txt = (*iter)->GetName();

			if ((*iter)->GetServerIP().empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_AddressServerIP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Check authenticators
			if ((*iter)->GetAuthenticator().RequiresKerberos())
			{
				CAuthenticatorKerberos* kerb = (*iter)->GetAuthenticatorKerberos();
				if (!kerb->GetDefaultPrincipal() && kerb->GetServerPrincipal().empty())
				{
					extra = true;
					str_index = "Alerts::Preferences::Invalid_KerberosRealm";
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}

		// Test each sieve account status
		for(CManageSIEVEAccountList::const_iterator iter = mSIEVEAccounts.mValue.begin(); iter != mSIEVEAccounts.mValue.end(); iter++)
		{
			extra_txt = (*iter)->GetName();

			if ((*iter)->GetServerIP().empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_AddressServerIP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Check authenticators
			if ((*iter)->GetAuthenticator().RequiresKerberos())
			{
				CAuthenticatorKerberos* kerb = (*iter)->GetAuthenticatorKerberos();
				if (!kerb->GetDefaultPrincipal() && kerb->GetServerPrincipal().empty())
				{
					extra = true;
					str_index = "Alerts::Preferences::Invalid_KerberosRealm";
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}

		// Check default identity
		if (!mIdentities.mValue.size())
		{
			str_index = "Alerts::Preferences::Invalid_DefaultIdentity";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		{
			CIdentity& id = mIdentities.mValue.front();
			extra_txt = id.GetIdentity();

			if (id.GetSMTPAccount(false).empty())
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_DefaultIdentitySMTP";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			else
			{
				// Check for matching SMTP server
				bool found = false;
				for(CSMTPAccountList::const_iterator iter = mSMTPAccounts.mValue.begin(); iter != mSMTPAccounts.mValue.end(); iter++)
				{
					if ((*iter)->GetName() == id.GetSMTPAccount(false))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					extra = true;
					str_index = "Alerts::Preferences::Invalid_DefaultIdentitySMTP";
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}


			if (id.GetFrom().empty() &&
				(!CAdminLock::sAdminLock.mLockReturnAddress || CAdminLock::sAdminLock.mLockedReturnDomain.empty()))
			{
				extra = true;
				str_index = "Alerts::Preferences::Invalid_DefaultIdentityFrom";
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

		}

		if (mDoMailboxClear.mValue && clear_mailbox.mValue.empty())
		{
			str_index = "Alerts::Preferences::Invalid_MoveBoxName";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		if (mDoMailboxClear.mValue && !::strcmpnocase(clear_mailbox.mValue, cINBOX))
		{
			str_index = "Alerts::Preferences::Invalid_MoveBoxDup";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

#if __dest_os != __linux_os
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (mSaveCreator.mValue.length() != 4)
#elif __dest_os == __win32_os || __dest_os == __linux_os
		if (mSaveCreator.mValue.length() != 3)
#else
#error __dest_os
#endif
		{
			str_index = "Alerts::Preferences::Invalid_SaveCreator";
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
#endif

		// Make sure default directory is properly terminated
		if (!mAskDownload.mValue)
		{
			const char cOSSeparator = os_dir_delim;

			if (mDefaultDownload.mValue.length() &&
				(mDefaultDownload.mValue.c_str()[mDefaultDownload.mValue.length() -1] != cOSSeparator))
			{
				mDefaultDownload.mValue += cOSSeparator;
				mDefaultDownload.SetDirty();
			}
		}

		// Make sure default set of favourites exists
		for(unsigned long index = CMailAccountManager::eFavouriteStart; index < CMailAccountManager::eFavouriteOthers; index++)
		{
			if (mFavourites.mValue.size() == index)
			{
				mFavourites.mValue.push_back(cdstring::null_str.c_str());

				// Make sure only New Messages is initially visible
				if (index != CMailAccountManager::eFavouriteNew)
					mFavourites.mValue.at(index).SetVisible(false);
			}
		}

		// Check cache values - for now coerce to sensible values silently
		if (!mRAutoCacheIncrement.mValue && !(mRCacheIncrement.mValue > 0))
			mRCacheIncrement.mValue = 20;
		if (mRUseHiCache.mValue)
		{
			if (!mRAutoCacheIncrement.mValue && (mRHiCache.mValue <= mRCacheIncrement.mValue))
				mRHiCache.mValue = 2*std::max(mRCacheIncrement.mValue, 20L);
			else if (mRAutoCacheIncrement.mValue && (mRHiCache.mValue < 40L))
				mRHiCache.mValue = 40;
		}

		// Check cache values - for now coerce to sensible values silently
		if (!mLAutoCacheIncrement.mValue && !(mLCacheIncrement.mValue > 0))
			mLCacheIncrement.mValue = 50;
		if (mLUseHiCache.mValue)
		{
			if (!mLAutoCacheIncrement.mValue && (mLHiCache.mValue <= mLCacheIncrement.mValue))
				mLHiCache.mValue = 2*std::max(mLCacheIncrement.mValue, 20L);
			else if (mLAutoCacheIncrement.mValue && (mLHiCache.mValue < 40L))
				mLHiCache.mValue = 40;
		}
	}

	// Handle error now
	catch (...)
	{
		CLOG_LOGCATCH(...);

		error = true;

		if (showAlert)
		{
			if (extra)
				CErrorHandler::PutStopAlertRsrcStr(str_index, extra_txt.c_str());
			else
				CErrorHandler::PutStopAlertRsrc(str_index);
		}
	}


	return !error;
}

// Configure prefs from multi-user input
void CPreferences::MultiuserConfigure(const cdstring& uid, const cdstring& real_name, bool do_return_address)
{
	bool remote = (mRemoteAccounts.mValue.size() ? mRemoteAccounts.mValue.front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);

	// Do virtual domain config if required
	if (CAdminLock::sAdminLock.mLockVirtualDomain)
		CAdminLock::sAdminLock.VirtualDomainPrefs(this, uid);

	// Put user id into prefs if not from IC
	if (remote || !mUse_IC.mValue)
	{
		// Copy new uid
		if (remote)
		{
			// Copy to all remote accounts
			for(COptionsAccountList::iterator iter = mRemoteAccounts.mValue.begin(); iter != mRemoteAccounts.mValue.end(); iter++)
			{
				if ((*iter)->GetAuthenticator().RequiresUserPswd())
					(*iter)->GetAuthenticatorUserPswd()->SetUID(uid);
			}

		}
		else
		{
			// Copy to all mail accounts
			for(CMailAccountList::iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
			{
				if ((*iter)->GetAuthenticator().RequiresUserPswd())
					(*iter)->GetAuthenticatorUserPswd()->SetUID(uid);
			}

			// Copy to all remote accounts
			for(COptionsAccountList::iterator iter = mRemoteAccounts.mValue.begin(); iter != mRemoteAccounts.mValue.end(); iter++)
			{
				if ((*iter)->GetAuthenticator().RequiresUserPswd())
					(*iter)->GetAuthenticatorUserPswd()->SetUID(uid);
			}

			// Copy to all address accounts
			for(CAddressAccountList::iterator iter = mAddressAccounts.mValue.begin(); iter != mAddressAccounts.mValue.end(); iter++)
			{
				if ((*iter)->GetAuthenticator().RequiresUserPswd())
					(*iter)->GetAuthenticatorUserPswd()->SetUID(uid);
			}

			// Copy to all address accounts
			for(CManageSIEVEAccountList::iterator iter = mSIEVEAccounts.mValue.begin(); iter != mSIEVEAccounts.mValue.end(); iter++)
			{
				if ((*iter)->GetAuthenticator().RequiresUserPswd())
					(*iter)->GetAuthenticatorUserPswd()->SetUID(uid);
			}
		}

		// Lock the return address if required
		if (do_return_address)
			LockReturnAddress(uid, real_name);
		
		// Always set the real name if provided
		else if (!real_name.empty())
		{
			cdstring new_addr(real_name);
			new_addr += " <";
			if (mIdentities.mValue.size())
				new_addr += mIdentities.mValue.front().GetFrom();
			new_addr += ">";

			// Change all identities
			for(CIdentityList::iterator iter = mIdentities.mValue.begin(); iter != mIdentities.mValue.end(); iter++)
				iter->SetFrom(new_addr, iter->UseFrom());
		}
	}
}

// Configure prefs from multi-user input
void CPreferences::LockReturnAddress(const cdstring& uid, const cdstring& real_name)
{
	// Strip uid from return address
	cdstring SMTPuid;
	if (!mIdentities.mValue.empty())
	{
		CAddress addr(mIdentities.mValue.front().GetFrom());
		SMTPuid = addr.GetMailAddress();
	}

	cdstring locked;
	bool valid = CAdminLock::sAdminLock.ProcessLockedReturnDomain(locked, SMTPuid);

	// Update email address if its locked by the admin or it does not contain an '@' or an '@' right at the front
	// MU dialog may be in use without locked address to allow local and remote users to change the value
	if (CAdminLock::sAdminLock.mLockReturnAddress ||		// Lock required
		!valid)												// Was not valid to start with
	{
		// Concatenate user id with return address and save in prefs (overridden if remote)
		if (CAdminLock::sAdminLock.mLockReturnAddress)
			SMTPuid = uid + CAdminLock::sAdminLock.mLockedReturnDomain;
		else
			SMTPuid = uid + locked;

		// Convert to proper RFC822 address
		CAddress addr(SMTPuid, real_name);
		SMTPuid = addr.GetFullAddress();

		// Change all identities
		for(CIdentityList::iterator iter = mIdentities.mValue.begin(); iter != mIdentities.mValue.end(); iter++)
			iter->SetFrom(SMTPuid, iter->UseFrom());
		mIdentities.SetDirty();
	}
}

// Transfer UIDs
bool CPreferences::TransferUIDs(const cdstring& uid)
{
	// IMPORTANT: This should only be called during a multi-user remote prefs login
	// Its used to ensure that any default options onthe remote server get filled in with
	// the current user's user id


	bool transferred = false;

	// Check mail account uids
	for(CMailAccountList::iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = (*iter)->GetAuthenticatorUserPswd();
			if (auth->GetUID().empty())
			{
				auth->SetUID(uid);
				mMailAccounts.SetDirty();
				transferred = true;
			}
		}
	}

	// Check SMTP account uids
	for(CSMTPAccountList::iterator iter = mSMTPAccounts.mValue.begin(); iter != mSMTPAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = (*iter)->GetAuthenticatorUserPswd();
			if (auth->GetUID().empty())
			{
				auth->SetUID(uid);
				mSMTPAccounts.SetDirty();
				transferred = true;
			}
		}
	}

	// Check remote account uids
	for(COptionsAccountList::iterator iter = mRemoteAccounts.mValue.begin(); iter != mRemoteAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = (*iter)->GetAuthenticatorUserPswd();
			if (auth->GetUID().empty())
			{
				auth->SetUID(uid);
				mRemoteAccounts.SetDirty();
				transferred = true;
			}
		}
	}

	// Check each address account
	for(CAddressAccountList::iterator iter = mAddressAccounts.mValue.begin(); iter != mAddressAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = (*iter)->GetAuthenticatorUserPswd();
			if (auth->GetUID().empty())
			{
				auth->SetUID(uid);
				mAddressAccounts.SetDirty();
				transferred = true;
			}
		}
	}

	// Check each sieve account
	for(CManageSIEVEAccountList::iterator iter = mSIEVEAccounts.mValue.begin(); iter != mSIEVEAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = (*iter)->GetAuthenticatorUserPswd();
			if (auth->GetUID().empty())
			{
				auth->SetUID(uid);
				mSIEVEAccounts.SetDirty();
				transferred = true;
			}
		}
	}

	// Must always reset the return address (will dirty mIdentities)
	CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
	cdstring real_name = addr.GetName();
	LockReturnAddress(uid, real_name);

	return transferred || mIdentities.IsDirty();
}

// Synchronise with IC if required
void CPreferences::CheckIC()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (!mUse_IC.mValue || !CICSupport::ICInstalled()) return;

	// Get IMAPuid && IMAPServerIP
	{
		cdstring mIMAPuid;
		cdstring mIMAPServerIP;
		CICSupport::ICGetMailAccount(mIMAPuid, mIMAPServerIP);

		// Look for first IMAP account
		CMailAccount* imap = (mMailAccounts.mValue.size() ? mMailAccounts.mValue.front() : nil);
		if (imap)
		{
			// Not if admin locked
			if (!CAdminLock::sAdminLock.mLockServerAddress &&
				(!CAdminLock::sAdminLock.mLockServerDomain || mIMAPServerIP.compare_end(CAdminLock::sAdminLock.mLockedDomain)))
				imap->SetServerIP(mIMAPServerIP);

			if (imap->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = imap->GetAuthenticatorUserPswd();
				auth->SetUID(mIMAPuid);

				// Get password if saving
				if (auth->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd)
				{
					cdstring mPswd;
					CICSupport::ICGetPswd(mPswd);
					auth->SetPswd(mPswd);
				}
			}
		}
	}

	// Get userName
	if (!mIdentities.mValue.empty())
	{
		// Set server IP
		cdstring mSMTPServerIP;
		CICSupport::ICGetSMTPHost(mSMTPServerIP);
		CSMTPAccount* smtp = (mSMTPAccounts.mValue.size() ? mSMTPAccounts.mValue.front() : nil);
		if (smtp)
		{
			if (!CAdminLock::sAdminLock.mLockServerAddress &&
				(!CAdminLock::sAdminLock.mLockServerDomain || mSMTPServerIP.compare_end(CAdminLock::sAdminLock.mLockedDomain)))
				smtp->SetServerIP(mSMTPServerIP);
		}

		// Get real name
		cdstring mRealName;
		CICSupport::ICGetRealName(mRealName);

		// Get SMTPuid
		if (!CAdminLock::sAdminLock.mLockReturnAddress && !CAdminLock::sAdminLock.mLockIdentityFrom)
		{
			cdstring SMTPuid;
			CICSupport::ICGetEmail(SMTPuid);
			
			// Form full address and insert into identity
			CAddress addr(SMTPuid, mRealName);
			mIdentities.mValue.front().SetFrom(addr.GetFullAddress(), true);
		}

		// Get header details
		if (CAdminLock::sAdminLock.mAllowXHeaders)
		{
			cdstring header;
			CICSupport::ICGetMailHeaders(header);
			mIdentities.mValue.front().SetHeader(header, !header.empty());
		}

		// Get footer details
		cdstring footer;
		CICSupport::ICGetSignature(footer);
		mIdentities.mValue.front().SetSignature(footer, !footer.empty());
	}

	// Get notification details
	if (mMailNotification.mValue.size())
	{
		CMailNotification& notify = mMailNotification.mValue.front();
		bool result;
		CICSupport::ICGetFlashIcon(result);
		notify.SetFlashIcon(result);

		CICSupport::ICGetDisplayDialog(result);
		notify.SetShowAlertBackground(result);

		CICSupport::ICGetPlaySound(result);
		notify.SetPlaySound(result);

		cdstring snd;
		CICSupport::ICGetAlertSound(snd);
		notify.SetSoundID(snd);
	}

	// Get list FONT details
	ICFontRecord font;
	CICSupport::ICGetListFont(&font);
	mListTextTraits.mValue.traits.size = font.size;
	mListTextTraits.mValue.traits.style = font.face;
	mListTextTraits.mValue.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	::PLstrcpy(mListTextTraits.mValue.traits.fontName, font.font);

	// Get display FONT details
	CICSupport::ICGetScreenFont(&font);
	mDisplayTextTraits.mValue.traits.size = font.size;
	mDisplayTextTraits.mValue.traits.style = font.face;
	mDisplayTextTraits.mValue.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	::PLstrcpy(mDisplayTextTraits.mValue.traits.fontName, font.font);

	// Get print FONT details
	CICSupport::ICGetPrintFont(&font);
	mPrintTextTraits.mValue.traits.size = font.size;
	mPrintTextTraits.mValue.traits.style = font.face;
	mPrintTextTraits.mValue.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	::PLstrcpy(mPrintTextTraits.mValue.traits.fontName, font.font);

	// Get saveCreator details
	//CICSupport::ICGetTextCreator(mSaveCreator);

	// Get include details
	CICSupport::ICGetQuote(mReplyQuote.mValue);
	//CICSupport::ICGetQuoteChar(mForwardQuote);

#else
	// Do nothing in other OS's -> IC does not exist
#endif
}

// Get version as string
cdstring CPreferences::GetVersionText()
{
	return ::GetVersionText(vers);
}

// Get current 3pane options
CPreferenceValue<C3PaneOptions>& CPreferences::Get3PaneOptions()
{
	return CPreferences::sPrefs->mUse3Pane.GetValue() ?
			CPreferences::sPrefs->m3PaneOptions : CPreferences::sPrefs->m1PaneOptions;
}

// Format subject for forwarding
void CPreferences::ForwardSubject(cdstring& subj) const
{
	std::ostrstream out;
	const char* p = mForwardSubject.mValue.c_str();
	while(*p)
	{
		switch(*p)
		{
		case '*':
			out << subj;
			p++;
			break;
		case '\\':
			out << *p++;
			if (*p)
				out << *p++;
			break;
		default:
			out << *p++;
			break;
		}
	}
	out << std::ends;
	
	subj.steal(out.str());
}


void CPreferences::SignatureDashes(cdstring& sig) const
{
	// Is auto-insert of sig dashes required
	if (mSigDashes.GetValue() && !sig.empty())
	{
		// Check to see if they are already present in the signature
		const char* p = sig.c_str();
		while(*p && ((*p == '\r') || (*p == '\n'))) p++;
		
		if ((*p++ != '-') ||
			(*p++ != '-') ||
			(*p++ != ' ') ||
			(*p != '\r') && (*p != '\n'))
		{
			cdstring temp("-- ");
			temp += os_endl;
			temp += sig;
			sig = temp;
		}
	}
}

#pragma mark ____________________________Environment

const char cHDR_USERAGENT[] = "Mulberry/%s (%s)";
const char cHDR_USERAGENTDEMO[] = "Mulberry/%s (%s Demo)";
static cdstring cHDR_UACOMMENT;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
const char cHDR_UACOMMENT_SYSNAME[] = "Mac OS";
const char cHDR_UACOMMENT_SYSNAMEX[] = "Mac OS X";
#ifdef powerc
#if PP_Target_Classic
const char cHDR_UACOMMENT_CPU[] = "PPC";
#else
const char cHDR_UACOMMENT_CPU[] = "Carbon";
#endif
#else
const char cHDR_UACOMMENT_CPU[] = "68K";
#endif
#elif __dest_os == __win32_os
const char cHDR_UACOMMENT_SYSNAME[] = "Win32";
#elif __dest_os == __linux_os
#if defined(__i386__)
const char cHDR_UACOMMENT_CPU[] = "x86";
#elif defined(__x86_64__)
const char cHDR_UACOMMENT_CPU[] = "x86_64";
#elif defined(sparc)
const char cHDR_UACOMMENT_CPU[] = "SPARC";
#elif defined(__PPC__)
const char cHDR_UACOMMENT_CPU[] = "PPC";
#else
#error __dest_os
#endif
#else
#error __dest_os
#endif

// Get X-Mailer header content
cdstring CPreferences::GetMailerDetails(bool full)
{
	// Initialise OS-type string
	if (cHDR_UACOMMENT.empty())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (UEnvironment::GetOSVersion () >= 0x0A00)
			cHDR_UACOMMENT = cHDR_UACOMMENT_SYSNAMEX;
		else
		{
			cHDR_UACOMMENT = cHDR_UACOMMENT_SYSNAME;
			cHDR_UACOMMENT += "/";
			cHDR_UACOMMENT += cHDR_UACOMMENT_CPU;
		}
#elif __dest_os == __win32_os
		cHDR_UACOMMENT = cHDR_UACOMMENT_SYSNAME;
#elif __dest_os == __linux_os
#include <sys/utsname.h>

		struct utsname u_name;
		uname(&u_name);

		cHDR_UACOMMENT = u_name.sysname;
		cHDR_UACOMMENT += "/";
		cHDR_UACOMMENT += cHDR_UACOMMENT_CPU;
#endif
	}

	cdstring mail_vers = CPreferences::sPrefs->GetVersionText();

	cdstring header_vers;
	header_vers.reserve(256);
	::snprintf(header_vers, 256, CMulberryApp::sApp->IsDemo() ? cHDR_USERAGENTDEMO : cHDR_USERAGENT, mail_vers.c_str(), cHDR_UACOMMENT.c_str());

	return header_vers;
}

// Get support information
cdstring CPreferences::GetSupportDetails()
{
	std::ostrstream out;
	out << "---------- Mulberry Details ----------" << os_endl;
	out << "Version: " << GetMailerDetails(true) << os_endl;
	//out << "Serial Number: " << CRegistration::sRegistration.GetSerialNumber() << os_endl;
	out << "Plugins: " << os_endl;
	for(CPluginList::const_iterator iter = CPluginManager::sPluginManager.GetPlugins().begin(); 
			iter != CPluginManager::sPluginManager.GetPlugins().end(); iter++)
	{
		out << "  " << (*iter)->GetName() << os_endl;
		out << "    Version: " << ::GetVersionText((*iter)->GetVersion());
		if ((*iter)->IsRegistered())
			out << " (Registered)";
		else if ((*iter)->IsDemo())
			out << " (Demo)";
		out << os_endl;
	}
	out << "Preferences: " << (((CMulberryApp::sCurrentPrefsFile != NULL) && CMulberryApp::sCurrentPrefsFile->IsRemote()) ? "Remote" : "Local") << os_endl;
	out << "3-Pane: " << (mUse3Pane.GetValue() ? "On" : "Off") << os_endl;
	out << "Account Types: Email (";
	bool mail_imap = false;
	bool mail_pop3 = false;
	bool mail_local = false;
	for(CMailAccountList::const_iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
	{
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eIMAP:
			if (!mail_imap)
			{
				out << " IMAP";
				mail_imap = true;
			}
			break;
		case CINETAccount::ePOP3:
			if (!mail_pop3)
			{
				out << " POP3";
				mail_pop3 = true;
			}
			break;
		case CINETAccount::eLocal:
			if (!mail_local)
			{
				out << " LOCAL";
				mail_local = true;
			}
			break;
		default:;
		}
	}
	out << " ) Prefs (";
	bool prefs_webdav = false;
	bool prefs_imsp = false;
	bool prefs_acap = false;
	for(COptionsAccountList::const_iterator iter = mRemoteAccounts.mValue.begin(); iter != mRemoteAccounts.mValue.end(); iter++)
	{
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eWebDAVPrefs:
			if (!prefs_webdav)
			{
				out << " WebDAV";
				prefs_webdav = true;
			}
			break;
		case CINETAccount::eIMSP:
			if (!prefs_imsp)
			{
				out << " IMSP";
				prefs_imsp = true;
			}
			break;
		case CINETAccount::eACAP:
			if (!prefs_acap)
			{
				out << " ACAP";
				prefs_acap = true;
			}
			break;
		default:;
		}
	}
	out << " ) Adbk (";
	bool adbk_imsp = false;
	bool adbk_acap = false;
	bool adbk_ldap = false;
	for(CAddressAccountList::const_iterator iter = mAddressAccounts.mValue.begin(); iter != mAddressAccounts.mValue.end(); iter++)
	{
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eIMSP:
			if (!adbk_imsp)
			{
				out << " IMSP";
				adbk_imsp = true;
			}
			break;
		case CINETAccount::eACAP:
			if (!adbk_acap)
			{
				out << " ACAP";
				adbk_acap = true;
			}
			break;
		case CINETAccount::eLDAP:
			if (!adbk_ldap)
			{
				out << " LDAP";
				adbk_ldap = true;
			}
			break;
		default:;
		}
	}
	out << " ) Filters (";
	bool filter_sieve = false;
	for(CManageSIEVEAccountList::const_iterator iter = mSIEVEAccounts.mValue.begin(); iter != mSIEVEAccounts.mValue.end(); iter++)
	{
		if (!filter_sieve)
		{
			out << " ManageSIEVE";
			filter_sieve = true;
		}
	}
	out << " )" << os_endl;
	out << "--------------------------------------" << os_endl;
	out << os_endl;
	out << os_endl;
	out << std::ends;
	
	cdstring temp;
	temp.steal(out.str());
	return temp;
}


#pragma mark ____________________________Account Management

// New account
void CPreferences::NewAccount(CINETAccount* acct)
{
	// Mail accounts
	if ((typeid(*acct) == typeid(CMailAccount)))
	{
		// Check for a local account
		if ((acct->GetServerType() != CINETAccount::eLocal))
		{
			// Add new INBOX as check favourite
			cdstrpair test = cdstrpair(acct->GetName() + cMailAccountSeparator + cINBOX, static_cast<CMailAccount*>(acct)->GetDirDelim());

			// Also add INBOX to first notifier's cabinet
			CFavouriteItem& item = mFavourites.Value().at(mMailNotification.GetValue().front().GetFavouriteIndex(this));
			cdstrpairvect::iterator found = std::find(item.GetItems().begin(), item.GetItems().end(), test);
			if (found == item.GetItems().end())
			{
				item.GetItems().push_back(test);
				mFavourites.SetDirty();
			}

			// Make INBOX Open after Login if Login at Start is on and not POP3
			if (acct->GetLogonAtStart() &&
				(acct->GetServerType() != CINETAccount::ePOP3))
			{
				CFavouriteItem& item = mFavourites.Value().at(CMailAccountManager::eFavouriteOpenLogin);
				cdstrpairvect::iterator found = std::find(item.GetItems().begin(), item.GetItems().end(), test);
				if (found == item.GetItems().end())
				{
					item.GetItems().push_back(test);
					mFavourites.SetDirty();
				}
			}
		}
	}
}

// Rename account
void CPreferences::RenameAccount(CINETAccount* acct, const cdstring& new_acct)
{
	// Get old account name and set new name
	cdstring old_acct = acct->GetName();
	acct->SetName(new_acct);

	// Mail account => change anything which refers to a mailbox
	if (typeid(*acct) == typeid(CMailAccount))
	{
		// Add separator
		cdstring old_name = old_acct + cMailAccountSeparator;
		cdstring new_name = new_acct + cMailAccountSeparator;

		// clear_mailbox
		if (::strncmp(clear_mailbox.GetValue(), old_name.c_str(), old_name.length()) == 0)
		{
			cdstring copy = new_name + &clear_mailbox.GetValue().c_str()[old_name.length()];
			clear_mailbox.SetValue(copy);
		}

		// mFavourites
		for(CFavouriteItemList::iterator iter1 = mFavourites.Value().begin();
				iter1 != mFavourites.Value().end(); iter1++)
		{
			// Look at all contents of favourites
			for(cdstrpairvect::iterator iter2 = (*iter1).GetItems().begin(); iter2 != (*iter1).GetItems().end(); iter2++)
			{
				cdstring name = (*iter2).first;
				if (::strncmp(name.c_str(), old_name.c_str(), old_name.length()) == 0)
				{
					cdstring copy = new_name + &name.c_str()[old_name.length()];
					(*iter2).first = copy;
					mFavourites.SetDirty();
				}
			}
		}

		// mMRUCopyTo
		for(cdstrvect::iterator iter = mMRUCopyTo.Value().begin(); iter != mMRUCopyTo.Value().end(); iter++)
		{
			if (::strncmp((*iter).c_str(), old_name.c_str(), old_name.length()) == 0)
			{
				cdstring copy = new_name + &(*iter).c_str()[old_name.length()];
				(*iter) = copy;
				mMRUCopyTo.SetDirty();
			}
		}

		// mMRUAppendTo
		for(cdstrvect::iterator iter = mMRUAppendTo.Value().begin(); iter != mMRUAppendTo.Value().end(); iter++)
		{
			if (::strncmp((*iter).c_str(), old_name.c_str(), old_name.length()) == 0)
			{
				cdstring copy = new_name + &(*iter).c_str()[old_name.length()];
				(*iter) = copy;
				mMRUAppendTo.SetDirty();
			}
		}

		// mMailboxAliases
		if (mMailboxAliases.GetValue().size())
		{
			cdstrmap temp;
			bool dirty = false;
			for(cdstrmap::const_iterator iter = mMailboxAliases.GetValue().begin(); iter != mMailboxAliases.GetValue().end(); iter++)
			{
				if (::strncmp((*iter).first.c_str(), old_name.c_str(), old_name.length()) == 0)
				{
					// Get new key for map
					cdstring copy = new_name + &(*iter).first.c_str()[old_name.length()];
					
					// Insert new key
					temp.insert(cdstrmap::value_type(copy, (*iter).second));
					dirty = true;
				}
			}
			
			if (dirty)
			{
				mMailboxAliases.Value() = temp;
				mMailboxAliases.SetDirty();
			}
		}
	
		// mIdentities (copy to and tied mailboxes)
		if (mIdentities.Value().RenameAccount(old_name, new_name))
			mIdentities.SetDirty();

		// mServerWindowInfo
		for(CWindowStateArray::iterator iter = mServerWindowInfo.Value().begin(); iter != mServerWindowInfo.Value().end(); iter++)
		{
			if ((*iter)->GetName() == old_acct)
			{
				(*iter)->GetName() = new_acct;
				mServerWindowInfo.SetDirty();
			}
		}

		// mMailboxWindowInfo
		for(CWindowStateArray::iterator iter = mMailboxWindowInfo.Value().begin(); iter != mMailboxWindowInfo.Value().end(); iter++)
		{
			cdstring name = (*iter)->GetName();
			if (::strncmp(name.c_str(), old_name.c_str(), old_name.length()) == 0)
			{
				cdstring copy = new_name + &name.c_str()[old_name.length()];
				(*iter)->GetName() = new_acct;
				mMailboxWindowInfo.SetDirty();
			}
		}
	
		// Get filter manager to change its references
		GetFilterManager()->RenameAccount(old_acct, new_acct);
	}

	// SMTP account => change identity SMTP server
	else if (typeid(*acct) == typeid(CSMTPAccount))
	{
		// mIdentities - SMTP
		for(CIdentityList::iterator iter = mIdentities.Value().begin(); iter != mIdentities.Value().end(); iter++)
		{
			if ((*iter).GetSMTPAccount(false) == old_acct)
			{
				(*iter).SetSMTPAccount(new_acct, (*iter).UseSMTPAccount());
				mIdentities.SetDirty();
			}
		}

	}

	// Calendar account => change items that cache account name
	else if (typeid(*acct) == typeid(CCalendarAccount))
	{
		// Add separator
		cdstring old_name = old_acct + cMailAccountSeparator;
		cdstring new_name = new_acct + cMailAccountSeparator;

		// mSubscribedCalendars
		cdstrset newset;
		bool changed = false;
		for(cdstrset::const_iterator iter = mSubscribedCalendars.GetValue().begin(); iter != mSubscribedCalendars.GetValue().end(); iter++)
		{
			if ((*iter).compare_start(old_name))
			{
				cdstring copy = new_name + &(*iter).c_str()[old_name.length()];
				newset.insert(copy);
				changed = true;
			}
			else
				newset.insert(*iter);
		}
		if (changed)
		{
			mSubscribedCalendars.Value() = newset;
			mSubscribedCalendars.SetDirty();
		}

		// mExpandedCalendars
		newset.clear();
		changed = false;
		for(cdstrset::const_iterator iter = mExpandedCalendars.GetValue().begin(); iter != mExpandedCalendars.GetValue().end(); iter++)
		{
			if ((*iter).compare_start(old_name))
			{
				cdstring copy = new_name + &(*iter).c_str()[old_name.length()];
				newset.insert(copy);
				changed = true;
			}
			else
				newset.insert(*iter);
		}
		if (changed)
		{
			mExpandedCalendars.Value() = newset;
			mExpandedCalendars.SetDirty();
		}

		// mCalendarColours
		cdstruint32map newmap;
		changed = false;
		for(cdstruint32map::const_iterator iter = mCalendarColours.GetValue().begin(); iter != mCalendarColours.GetValue().end(); iter++)
		{
			if ((*iter).first.compare_start(old_name))
			{
				cdstring copy = new_name + &(*iter).first.c_str()[old_name.length()];
				newmap.insert(cdstruint32map::value_type(copy, (*iter).second));
				changed = true;
			}
			else
				newmap.insert(*iter);
		}
		if (changed)
		{
			mCalendarColours.Value() = newmap;
			mCalendarColours.SetDirty();
		}
		
		// mDefaultCalendar
		if (mDefaultCalendar.GetValue().compare_start(old_name))
		{
			cdstring copy = new_name + &mDefaultCalendar.GetValue().c_str()[old_name.length()];
			mDefaultCalendar.SetValue(copy);
		}
	}
}

// Delete account
void CPreferences::DeleteAccount(CINETAccount* acct)
{
	// Mail accounts
	if (typeid(*acct) == typeid(CMailAccount))
	{
		cdstring test = acct->GetName() + cMailAccountSeparator;

		// Remove all entries from favourites
		for(CFavouriteItemList::iterator iter1 = mFavourites.Value().begin(); iter1 != mFavourites.Value().end(); iter1++)
		{
			for(cdstrpairvect::iterator iter2 = (*iter1).GetItems().begin(); iter2 != (*iter1).GetItems().end();)
			{
				// Check starting name
				if (::strncmp((*iter2).first, test.c_str(), test.length()) == 0)
				{
					// Erase this item
					iter2 = (*iter1).GetItems().erase(iter2);
					continue;
				}
				
				iter2++;
			}
		}
	
		// Get filter manager to change its references
		GetFilterManager()->DeleteAccount(acct->GetName());
	}

	// Calendar accounts
	else if (typeid(*acct) == typeid(CCalendarAccount))
	{
		// Add separator
		cdstring test = acct->GetName() + cMailAccountSeparator;

		// mSubscribedCalendars
		cdstrset newset;
		bool changed = false;
		for(cdstrset::const_iterator iter = mSubscribedCalendars.GetValue().begin(); iter != mSubscribedCalendars.GetValue().end(); iter++)
		{
			if ((*iter).compare_start(test))
				changed = true;
			else
				newset.insert(*iter);
		}
		if (changed)
		{
			mSubscribedCalendars.Value() = newset;
			mSubscribedCalendars.SetDirty();
		}

		// mExpandedCalendars
		newset.clear();
		changed = false;
		for(cdstrset::const_iterator iter = mExpandedCalendars.GetValue().begin(); iter != mExpandedCalendars.GetValue().end(); iter++)
		{
			if ((*iter).compare_start(test))
				changed = true;
			else
				newset.insert(*iter);
		}
		if (changed)
		{
			mExpandedCalendars.Value() = newset;
			mExpandedCalendars.SetDirty();
		}

		// mCalendarColours
		cdstruint32map newmap;
		changed = false;
		for(cdstruint32map::const_iterator iter = mCalendarColours.GetValue().begin(); iter != mCalendarColours.GetValue().end(); iter++)
		{
			if ((*iter).first.compare_start(test))
				changed = true;
			else
				newmap.insert(*iter);
		}
		if (changed)
		{
			mCalendarColours.Value() = newmap;
			mCalendarColours.SetDirty();
		}
	}
}

#pragma mark ____________________________Calendar management

// Rename calendar
void CPreferences::RenameCalendar(const cdstring& old_name, const cdstring& new_name)
{
	// mSubscribedCalendars
	cdstrset::iterator found = mSubscribedCalendars.Value().find(old_name);
	if (found != mSubscribedCalendars.Value().end())
	{
		mSubscribedCalendars.Value().erase(found);
		mSubscribedCalendars.Value().insert(new_name);
		mSubscribedCalendars.SetDirty();
	}

	// mExpandedCalendars
	found = mExpandedCalendars.Value().find(old_name);
	if (found != mExpandedCalendars.Value().end())
	{
		mExpandedCalendars.Value().erase(found);
		mExpandedCalendars.Value().insert(new_name);
		mExpandedCalendars.SetDirty();
	}

	// mCalendarColours
	cdstruint32map::iterator found2 = mCalendarColours.Value().find(old_name);
	if (found2 != mCalendarColours.Value().end())
	{
		uint32_t old_colour = (*found2).second;
		mCalendarColours.Value().erase(found2);
		mCalendarColours.Value().insert(cdstruint32map::value_type(new_name, old_colour));
		mCalendarColours.SetDirty();
	}
}

// Delete calendar
void CPreferences::DeleteCalendar(const cdstring& name)
{
	// mSubscribedCalendars
	if (mSubscribedCalendars.Value().erase(name) != 0)
	{
		mSubscribedCalendars.SetDirty();
	}

	// mExpandedCalendars
	if (mExpandedCalendars.Value().erase(name) != 0)
	{
		mExpandedCalendars.SetDirty();
	}

	// mCalendarColours
	if (mCalendarColours.Value().erase(name) != 0)
	{
		mCalendarColours.SetDirty();
	}
}

#pragma mark ____________________________Identity Management

// Identity deleted
void CPreferences::DeleteIdentity(CIdentity* id)
{
	// Must exist
	CIdentityList::iterator found = std::find(mIdentities.Value().begin(), mIdentities.Value().end(), *id);
	if (found == mIdentities.Value().end())
		return;

	// Remove it from any mail accounts
	for(CMailAccountList::iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetTiedIdentity() == id->GetIdentity())
		{
			(*iter)->SetTiedIdentity(cdstring::null_str);
			(*iter)->SetTieIdentity(false);
		}
	}

	// Remove it and dirty list
	mIdentities.Value().erase(found);
	mIdentities.SetDirty();
	mTiedMailboxes.SetDirty();
	mTiedCalendars.SetDirty();
}

// Identity name change
void CPreferences::ReplaceIdentity(CIdentity& id, CIdentity& new_id)
{
	// Must exist
	CIdentityList::iterator found = std::find(mIdentities.Value().begin(), mIdentities.Value().end(), id);
	if (found == mIdentities.Value().end())
		return;

	// Remove it from any mail accounts
	for(CMailAccountList::iterator iter = mMailAccounts.mValue.begin(); iter != mMailAccounts.mValue.end(); iter++)
	{
		if ((*iter)->GetTiedIdentity() == id.GetIdentity())
			(*iter)->SetTiedIdentity(new_id.GetIdentity());
	}

	// Replace and dirty list
	id = new_id;
	mIdentities.SetDirty();
	mTiedMailboxes.SetDirty();
	mTiedCalendars.SetDirty();
}

// Identity move by indices
void CPreferences::MoveIdentity(const ulvector& movers, unsigned long dest)
{
	// Create destination list
	CIdentityList temp_dest;
	unsigned long index1 = 0;
	ulvector::const_iterator uliter1 = movers.begin();
	for(CIdentityList::const_iterator iter1 = mIdentities.GetValue().begin();
			iter1 != mIdentities.GetValue().end(); iter1++, index1++)
	{
		// Add temp list if index is drop index
		if (index1 == dest)
		{
			unsigned long index2 = 0;
			ulvector::const_iterator uliter2 = movers.begin();
			for(CIdentityList::const_iterator iter2 = mIdentities.GetValue().begin();
					(iter2 != mIdentities.GetValue().end()) && (uliter2 != movers.end()); iter2++, index2++)
			{
				if (index2 == *uliter2)
				{
					temp_dest.push_back(*iter2);
					uliter2++;
				}
			}
		}

		// Add an item that was not moved
		if ((uliter1 == movers.end()) || (index1 != *uliter1))
			temp_dest.push_back(*iter1);
		else
			uliter1++;
	}
	
	// Add temp list if index is drop index
	if (index1 == dest)
	{
		unsigned long index2 = 0;
		ulvector::const_iterator uliter2 = movers.begin();
		for(CIdentityList::const_iterator iter2 = mIdentities.GetValue().begin();
				(iter2 != mIdentities.GetValue().end()) && (uliter2 != movers.end()); iter2++, index2++)
		{
			if (index2 == *uliter2)
			{
				temp_dest.push_back(*iter2);
				uliter2++;
			}
		}
	}

	// Now copy back the list
	mIdentities.SetValue(temp_dest);
	mTiedMailboxes.SetDirty(mIdentities.IsDirty());
	mTiedCalendars.SetDirty(mIdentities.IsDirty());
}

// Get identity associated with list of messages
const CIdentity* CPreferences::GetTiedIdentity(const CMessageList* msgs)
{
	const CIdentity* id = NULL;

	// Get top message
	const CMessage* msg = msgs->front();

	// Find its mailbox
	const CMbox* mbox = msg->GetMbox();

	// See if tied to original message
	if (CPreferences::sPrefs->mMsgTied.GetValue())
	{
		// Look at each message
		for(CMessageList::const_iterator iter1 = msgs->begin(); !id && (iter1 != msgs->end()); iter1++)
		{
			// Skip if no address
			if (!(*iter1)->GetEnvelope())
				continue;
			
			// Get each To address
			CAddressList addrlist;
			if ((*iter1)->GetEnvelope()->GetTo())
			{
				for(CAddressList::const_iterator iter2 = (*iter1)->GetEnvelope()->GetTo()->begin();
						iter2 != (*iter1)->GetEnvelope()->GetTo()->end(); iter2++)
					addrlist.AddUnique(*iter2);
			}

			// Get each Cc address
			if ((*iter1)->GetEnvelope()->GetCC())
			{
				for(CAddressList::const_iterator iter2 = (*iter1)->GetEnvelope()->GetCC()->begin();
						iter2 != (*iter1)->GetEnvelope()->GetCC()->end(); iter2++)
					addrlist.AddUnique(*iter2);
			}

			// Get each From address
			if ((*iter1)->GetEnvelope()->GetFrom())
			{
				for(CAddressList::const_iterator iter2 = (*iter1)->GetEnvelope()->GetFrom()->begin();
						iter2 != (*iter1)->GetEnvelope()->GetFrom()->end(); iter2++)
					addrlist.AddUnique(*iter2);
			}

			// Look at each to/cc/from address in the message
			for(CAddressList::const_iterator iter2 = addrlist.begin(); !id && (iter2 != addrlist.end()); iter2++)
			{
				// Look at each identity
				for(CIdentityList::const_iterator iter3 = CPreferences::sPrefs->mIdentities.GetValue().begin();
						!id && (iter3 != CPreferences::sPrefs->mIdentities.GetValue().end()); iter3++)
				{
					// Get identity from addresses
					const cdstring& addrs = (*iter3).GetFrom(true);
					std::auto_ptr<CAddressList> idaddrlist(new CAddressList(addrs, addrs.length()));
					
					// Look at each from address in the identity and compare with message from address
					for(CAddressList::const_iterator iter4 = idaddrlist->begin(); !id && (iter4 != idaddrlist->end()); iter4++)
					{
						// Compare the addresses
						if (**iter4 == **iter2)
							id = &(*iter3);
					}
				}
			}
		}
	}

	// Check for mbox specific identity
	if (!id)
		id = mbox->GetTiedIdentity();

	// Get account specific id if not tied to mailbox
	if (!id)
		id = &mbox->GetProtocol()->GetMailAccount()->GetAccountIdentity();
	
	return id;
}

#pragma mark ____________________________Favourite Management

void CPreferences::RenameFavourite(unsigned long index, const cdstring& new_name)
{
	cdstring old_name = mFavourites.GetValue().at(index).GetName();
	mFavourites.Value().at(index).SetName(new_name);

	// Make pref dirty
	mFavourites.SetDirty();
	
	// Now look for matching mail notifications
	cdstring old_id = cdstring("@") + old_name;
	cdstring new_id = cdstring("@") + new_name;
	bool renamed = false;
	for(CMailNotificationList::iterator iter = mMailNotification.Value().begin();
			iter != mMailNotification.Value().end(); iter++)
	{
		if ((*iter).GetFavouriteID() == old_id)
		{
			(*iter).SetFavouriteID(new_id);
			renamed = true;
		}
	}
	if (renamed)
		mMailNotification.SetDirty();
	
	// Get filter manager to change its references
	GetFilterManager()->RenameFavourite(index, new_name);
}

void CPreferences::RemoveFavourite(unsigned long index)
{
	cdstring old_name = mFavourites.GetValue().at(index).GetName();
	mFavourites.Value().erase(mFavourites.Value().begin() + index);

	// Make pref dirty
	mFavourites.SetDirty();
	
	// Now look for matching mail notifications
	cdstring old_id = cdstring("@") + old_name;
	bool removed = false;
	for(CMailNotificationList::iterator iter = mMailNotification.Value().begin();
			iter != mMailNotification.Value().end(); iter++)
	{
		if ((*iter).GetFavouriteID() == old_id)
		{
			(*iter).SetFavouriteID(cdstring::null_str);
			removed = true;
		}
	}
	if (removed)
		mMailNotification.SetDirty();
	
	// Get filter manager to change its references
	GetFilterManager()->RemoveFavourite(index);
}

// Map mailbox name to alias
bool CPreferences::MapMailboxAlias(cdstring& name) const
{
	// Look for name in alias map
	cdstrmap::const_iterator found = mMailboxAliases.GetValue().find(name);
	if (found != mMailboxAliases.GetValue().end())
	{
		name = (*found).second;
		return true;
	}
	else
		return false;
}

#pragma mark ____________________________Toolbar management

// Toolbar has changed - sync. it
void CPreferences::ToolbarChanged()
{
#ifdef __MULBERRY
	mToolbars.SetValue(CToolbarManager::sToolbarManager.GetInfo());
#endif
}

#pragma mark ____________________________Address Book Flags

// Rename address book
void CPreferences::RenameAddressBook(const cdstring& old_name, const cdstring& new_name)
{
	// mExpandedAdbks
	cdstrset::iterator found = mExpandedAdbks.Value().find(old_name);
	if (found != mExpandedAdbks.Value().end())
	{
		mExpandedAdbks.Value().erase(found);
		mExpandedAdbks.Value().insert(new_name);
		mExpandedAdbks.SetDirty();
	}
}

// Rename address book
void CPreferences::RenameAddressBookURL(const cdstring& old_name, const cdstring& new_name)
{
	RenameAddressBookFlag(old_name, new_name, mAdbkOpenAtStartup);
	RenameAddressBookFlag(old_name, new_name, mAdbkNickName);
	RenameAddressBookFlag(old_name, new_name, mAdbkSearch);
	RenameAddressBookFlag(old_name, new_name, mAdbkAutoSync);
}

// Rename address book
void CPreferences::RenameAddressBookFlag(const cdstring& old_name, const cdstring& new_name, CPreferenceValueMap<cdstrvect>& list)
{
	cdstrvect::iterator found = std::find(list.Value().begin(), list.Value().end(), old_name);
	if (found != list.Value().end())
	{
		*found = new_name;
		list.SetDirty();
	}
}

// Delete address book
void CPreferences::DeleteAddressBook(const cdstring& name)
{
	// mExpandedAdbks
	if (mExpandedAdbks.Value().erase(name) != 0)
	{
		mExpandedAdbks.SetDirty();
	}
}

// Delete address book
void CPreferences::DeleteAddressBookURL(const cdstring& name)
{
	DeleteAddressBookFlag(name, mAdbkOpenAtStartup);
	DeleteAddressBookFlag(name, mAdbkNickName);
	DeleteAddressBookFlag(name, mAdbkSearch);
	DeleteAddressBookFlag(name, mAdbkAutoSync);
}

// Delete address book
void CPreferences::DeleteAddressBookFlag(const cdstring& name, CPreferenceValueMap<cdstrvect>& list)
{
	cdstrvect::iterator found = std::find(list.Value().begin(), list.Value().end(), name);
	if (found != list.Value().end())
	{
		list.Value().erase(found);
		list.SetDirty();
	}
}

// Changing address book flag
void CPreferences::ChangeAddressBookOpenOnStart(CAddressBook* adbk, bool set)
{
	ChangeAddressBookFlag(adbk, mAdbkOpenAtStartup, set);
}

// Changing address book flag
void CPreferences::ChangeAddressBookLookup(CAddressBook* adbk, bool set)
{
	ChangeAddressBookFlag(adbk, mAdbkNickName, set);
}

// Changing address book flag
void CPreferences::ChangeAddressBookSearch(CAddressBook* adbk, bool set)
{
	ChangeAddressBookFlag(adbk, mAdbkSearch, set);
}

// Changing address book flag
void CPreferences::ChangeAddressBookAutoSync(CAddressBook* adbk, bool set)
{
	ChangeAddressBookFlag(adbk, mAdbkAutoSync, set);
}

// Changing address book flag
void CPreferences::ChangeAddressBookFlag(CAddressBook* adbk, CPreferenceValueMap<cdstrvect>& list, bool set)
{
	// Look for name in prefs list
	cdstring adbk_url = adbk->GetURL();
	cdstrvect::iterator found = std::find(list.mValue.begin(), list.mValue.end(), adbk_url);

	// Add/remove from prefs list
	if (set)
	{
		if (found == list.mValue.end())
			list.mValue.push_back(adbk_url);
	}
	else
	{
		if (found != list.mValue.end())
			list.mValue.erase(found);
	}

	// Mark as changed
	list.SetDirty();
}
