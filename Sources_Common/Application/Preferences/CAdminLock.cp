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


// CAdminLock.cp

// Class controls lockable items in prefs

#include "CAdminLock.h"

#include "CAddressList.h"
#include "CErrorHandler.h"
#include "CMessage.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#endif
#include "COptionsMap.h"
#include "CPluginManager.h"
#include "CPreferenceKeys.h"
#include "CPreferences.h"

#if __dest_os == __linux_os
#include <JRect.h>
#endif

CAdminLock CAdminLock::sAdminLock;

CAdminLock::CAdminLock()
{
	mLockedReturnDomain = cdstring::null_str;					// Locked user's return address domain from multi-user prefs
	mLockedHeader = cdstring::null_str;							// Header lines that always appear
	mLoginID = cdstring::null_str;								// Login ID
	mServerID = cdstring::null_str;								// Server ID
}

void CAdminLock::ReadFromMap(COptionsMap* theMap)
{
	StMapSection section(theMap, cAdminSection);

	// Read prefs version
	NumVersion vers_prefs;
	*(long*) &vers_prefs = 0L;
	cdstring txt;
	theMap->ReadValue(cVersionKey, txt, vers_prefs);
	*(long*) &vers_prefs = ::strtol(txt, nil, 0);

//--------------------General Prefs
	// Preference usage
	theMap->ReadValue(cAllowDefaultKey, mAllowDefault, vers_prefs);
	theMap->ReadValue(cUseMultiUserKey, mUseMultiUser, vers_prefs);
	theMap->ReadValue(cNoRemotePrefsKey, mNoRemotePrefs, vers_prefs);
	theMap->ReadValue(cNoLocalPrefsKey, mNoLocalPrefs, vers_prefs);
	theMap->ReadValue(cNoDisconnectKey, mNoDisconnect, vers_prefs);
	theMap->ReadValue(cNoSMTPQueuesKey, mNoSMTPQueues, vers_prefs);
	theMap->ReadValue(cNoLocalMboxKey, mNoLocalMbox, vers_prefs);
	theMap->ReadValue(cNoLocalAdbksKey, mNoLocalAdbks, vers_prefs);
	theMap->ReadValue(cNoLocalDraftsKey, mNoLocalDrafts, vers_prefs);
	theMap->ReadValue(cNoLocalCalendarsKey, mNoLocalCalendars, vers_prefs);
	theMap->ReadValue(cNoAttachmentsKey, mNoAttachments, vers_prefs);
	theMap->ReadValue(cNoRulesKey, mNoRules, vers_prefs);
	theMap->ReadValue(cLockVirtualDomainKey, mLockVirtualDomain, vers_prefs);
	theMap->ReadValue(cLockServerAddressKey, mLockServerAddress, vers_prefs);
	theMap->ReadValue(cLockServerDomainKey, mLockServerDomain, vers_prefs);
	theMap->ReadValue(cNoLockSMTPKey, mNoLockSMTP, vers_prefs);
	theMap->ReadValue(cLockedDomainKey, mLockedDomain, vers_prefs);
	theMap->ReadValue(cLockSMTPSendKey, mLockSMTPSend, vers_prefs);
	theMap->ReadValue(cLockReturnAddressKey, mLockReturnAddress, vers_prefs);
	theMap->ReadValue(cLockPersonalICKey, mLockPersonalIC, vers_prefs);
	theMap->ReadValue(cLockMDNKey, mLockMDN, vers_prefs);
	theMap->ReadValue(cAskRealNameKey, mAskRealName, vers_prefs);
	//theMap->ReadValue(cGenerateAuthKey, mGenerateAuth, vers_prefs);
	theMap->ReadValue(cGenerateSenderKey, mGenerateSender, vers_prefs);
	theMap->ReadValue(cAllowXHeadersKey, mAllowXHeaders, vers_prefs);
	theMap->ReadValue(cLockIdentityFromKey, mLockIdentityFrom, vers_prefs);
	theMap->ReadValue(cLockIdentityReplyToKey, mLockIdentityReplyTo, vers_prefs);
	theMap->ReadValue(cLockIdentitySenderKey, mLockIdentitySender, vers_prefs);
	theMap->ReadValue(cLockSavePswdKey, mLockSavePswd, vers_prefs);
	theMap->ReadValue(cPromptLogoutQuitKey, mPromptLogoutQuit, vers_prefs);
	theMap->ReadValue(cNoQuitKey, mNoQuit, vers_prefs);
	theMap->ReadValue(cAllowStyledCompositionKey, mAllowStyledComposition, vers_prefs);
	theMap->ReadValue(cAllowRejectCommandKey, mAllowRejectCommand, vers_prefs);
	theMap->ReadValue(cAutoCreateMailboxKey, mAutoCreateMailbox, vers_prefs);
	theMap->ReadValue(cAutoCreateMailboxesKey, mAutoCreateMailboxes, vers_prefs);
	theMap->ReadValue(cSubcribeAutoCreateKey, mSubcribeAutoCreate, vers_prefs);

	// Originator info
	{
		StMapSection section(theMap, cAdminOriginatorSection);
		theMap->ReadValue(cAdminUseOriginatorKey, mOriginator.mGenerate, vers_prefs);
		theMap->ReadValue(cAdminOriginatorUseTokenKey, mOriginator.mUseToken, vers_prefs);
		theMap->ReadValue(cAdminOriginatorTokenHashKey, mOriginator.mTokenHash, vers_prefs);
		mOriginator.mTokenHash.Decrypt(cdstring::eEncryptCipher, cAdminOriginatorTokenAuthorityEncryptKey);
		theMap->ReadValue(cAdminOriginatorUseTokenAuthorityKey, mOriginator.mUseTokenAuthority, vers_prefs);
		theMap->ReadValue(cAdminOriginatorTokenAuthorityKey, mOriginator.mTokenAuthority, vers_prefs);
	}

	// Attachment limits info
	{
		StMapSection section(theMap, cAdminAttachmentLimitsSection);
		{
			StMapSection section(theMap, cAdminAttachmentWarningsSection);
			theMap->ReadValue(cAdminAttachmentLimitAddressKey, mWarning.mAddress, vers_prefs);
			theMap->ReadValue(cAdminAttachmentLimitSizeKey, mWarning.mSize, vers_prefs);
			theMap->ReadValue(cAdminAttachmentLimitProductKey, mWarning.mProduct, vers_prefs);
		}
		{
			StMapSection section(theMap, cAdminAttachmentPreventionSection);
			theMap->ReadValue(cAdminAttachmentLimitAddressKey, mPrevent.mAddress, vers_prefs);
			theMap->ReadValue(cAdminAttachmentLimitSizeKey, mPrevent.mSize, vers_prefs);
			theMap->ReadValue(cAdminAttachmentLimitProductKey, mPrevent.mProduct, vers_prefs);
		}
	}

	theMap->ReadValue(cAdminPreventDownloadKey, mPreventDownloads, vers_prefs);

	theMap->ReadValue(cAdminAllow3PaneChoiceKey, mAllow3PaneChoice, vers_prefs);
	theMap->ReadValue(cAdminForce3PaneKey, mForce3Pane, vers_prefs);

	theMap->ReadValue(cAdminPreventCalendarsKey, mPreventCalendars, vers_prefs);

	theMap->ReadValue(cAdminMulberryURLKey, mMulberryURL, vers_prefs);
	theMap->ReadValue(cAdminMulberryFAQKey, mMulberryFAQ, vers_prefs);
	theMap->ReadValue(cAdminUpdateURLKey, mUpdateURL, vers_prefs);
	theMap->ReadValue(cAdminMulberrySupportAddressKey, mMulberrySupportAddress, vers_prefs);

	theMap->ReadValue(cAdminAllowLoggingKey, mAllowLogging, vers_prefs);
	theMap->ReadValue(cAdminSSLStateDisplayKey, mSSLStateDisplay, vers_prefs);

	//theMap->ReadValue(cAdminUpgradeAuthenticatorKey, mUpgradeAuthenticator, vers_prefs);
	//long temp = 0;
	//theMap->ReadValue(cAdminUpgradeAuthenticatorTypeKey, temp, vers_prefs);
	//mUpgradeAuthenticatorType = static_cast<EAuthUpgrade>(temp);

	theMap->ReadValue(cAdminUpgradeKerberosKey, mUpgradeKerberos, vers_prefs);

	theMap->ReadValue(cAdminUsePASSDKey, mUsePASSD, vers_prefs);
}

// Reset preferences locks
void CAdminLock::ResetLock()
{
	mLockedMailboxServerAddr.clear();
	mLockedPrefsServerAddr.clear();
	mLockedSMTPServerAddr.clear();
	mLockedAdbkServerAddr.clear();
	mLockedManageSIEVEServerAddr.clear();
	mLockedReturnDomain = cdstring::null_str;
	mLockedHeader = cdstring::null_str;
}

// Read special values from prefs
void CAdminLock::ProcessPrefs(const CPreferences* prefs)
{
	// Get locked server address info

	// IMAP server
	for(CMailAccountList::const_iterator iter = prefs->mMailAccounts.GetValue().begin(); iter != prefs->mMailAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->IsLocal() || !(*iter)->GetServerIP().empty())
			mLockedMailboxServerAddr.push_back((*iter)->GetServerIP());
		
		// Possible authenticator upgrade
		UpgradeAuthenticator(*iter);
	}

	// Remote server
	for(COptionsAccountList::const_iterator iter = prefs->mRemoteAccounts.GetValue().begin(); iter != prefs->mRemoteAccounts.GetValue().end(); iter++)
	{
		if (!(*iter)->GetServerIP().empty())
			mLockedPrefsServerAddr.push_back((*iter)->GetServerIP());
		
		// Possible authenticator upgrade
		UpgradeAuthenticator(*iter);
	}

	// SMTP server
	for(CSMTPAccountList::const_iterator iter = prefs->mSMTPAccounts.GetValue().begin(); iter != prefs->mSMTPAccounts.GetValue().end(); iter++)
	{
		if (!(*iter)->GetServerIP().empty())
			mLockedSMTPServerAddr.push_back((*iter)->GetServerIP());
		
		// Possible authenticator upgrade
		UpgradeAuthenticator(*iter);
	}

	if (!prefs->mIdentities.GetValue().empty())
	{
		// Get locked return address info
		CAddress addr(prefs->mIdentities.GetValue().front().GetFrom());
		ProcessLockedReturnDomain(mLockedReturnDomain, addr.GetMailAddress());

		// Copy locked header
		mLockedHeader = prefs->mIdentities.GetValue().front().GetHeader();
		const_cast<CPreferences*>(prefs)->mIdentities.Value().front().SetHeader(cdstring::null_str, false);
	}

	// Adbk server
	for(CAddressAccountList::const_iterator iter = prefs->mAddressAccounts.GetValue().begin(); iter != prefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if (!(*iter)->GetServerIP().empty())
			mLockedAdbkServerAddr.push_back((*iter)->GetServerIP());
		
		// Possible authenticator upgrade
		UpgradeAuthenticator(*iter);
	}

	// ManageSIEVE server
	for(CManageSIEVEAccountList::const_iterator iter = prefs->mSIEVEAccounts.GetValue().begin(); iter != prefs->mSIEVEAccounts.GetValue().end(); iter++)
	{
		if (!(*iter)->GetServerIP().empty())
			mLockedManageSIEVEServerAddr.push_back((*iter)->GetServerIP());
		
		// Possible authenticator upgrade
		UpgradeAuthenticator(*iter);
	}
}

// Do lock on preferences for virtual domain support
void CAdminLock::VirtualDomainPrefs(CPreferences* prefs, const cdstring& uid)
{
	// Get the domain part from uid
	if (!::strchr(uid.c_str(), '@'))
		return;
	cdstring vdomain(::strchr(uid.c_str(), '@') + 1);
	if (vdomain.empty())
		return;

	// Do each server address

	// IMAP server
	for(CMailAccountList::iterator iter = prefs->mMailAccounts.Value().begin(); iter != prefs->mMailAccounts.Value().end(); iter++)
	{
		cdstring serverip = (*iter)->GetServerIP();
		const char* p = serverip.c_str();
		const char* q = ::strchr(serverip.c_str(), '*');
		if (q)
		{
			cdstring newip(p, q - p);
			newip += vdomain;
			newip += q + 1;
			(*iter)->SetServerIP(newip);
		}
	}

	// Remote server
	for(COptionsAccountList::iterator iter = prefs->mRemoteAccounts.Value().begin(); iter != prefs->mRemoteAccounts.Value().end(); iter++)
	{
		cdstring serverip = (*iter)->GetServerIP();
		const char* p = serverip.c_str();
		const char* q = ::strchr(serverip.c_str(), '*');
		if (q)
		{
			cdstring newip(p, q - p);
			newip += vdomain;
			newip += q + 1;
			(*iter)->SetServerIP(newip);
		}
	}

	// SMTP server
	for(CSMTPAccountList::iterator iter = prefs->mSMTPAccounts.Value().begin(); iter != prefs->mSMTPAccounts.Value().end(); iter++)
	{
		cdstring serverip = (*iter)->GetServerIP();
		const char* p = serverip.c_str();
		const char* q = ::strchr(serverip.c_str(), '*');
		if (q)
		{
			cdstring newip(p, q - p);
			newip += vdomain;
			newip += q + 1;
			(*iter)->SetServerIP(newip);
		}
	}

	// Adbk server
	for(CAddressAccountList::iterator iter = prefs->mAddressAccounts.Value().begin(); iter != prefs->mAddressAccounts.Value().end(); iter++)
	{
		cdstring serverip = (*iter)->GetServerIP();
		const char* p = serverip.c_str();
		const char* q = ::strchr(serverip.c_str(), '*');
		if (q)
		{
			cdstring newip(p, q - p);
			newip += vdomain;
			newip += q + 1;
			(*iter)->SetServerIP(newip);
		}
	}

	// ManageSIEVE server
	for(CManageSIEVEAccountList::iterator iter = prefs->mSIEVEAccounts.Value().begin(); iter != prefs->mSIEVEAccounts.Value().end(); iter++)
	{
		cdstring serverip = (*iter)->GetServerIP();
		const char* p = serverip.c_str();
		const char* q = ::strchr(serverip.c_str(), '*');
		if (q)
		{
			cdstring newip(p, q - p);
			newip += vdomain;
			newip += q + 1;
			(*iter)->SetServerIP(newip);
		}
	}
	
	// Do each identity From address
	for(CIdentityList::iterator iter = prefs->mIdentities.Value().begin(); iter != prefs->mIdentities.Value().end(); iter++)
	{
		if ((*iter).UseFrom())
		{
			cdstring fromaddr = (*iter).GetFrom();
			const char* p = fromaddr.c_str();
			const char* q = ::strchr(fromaddr.c_str(), '*');
			if (q)
			{
				cdstring newfrom(p, q - p);
				newfrom += vdomain;
				newfrom += q + 1;
				(*iter).SetFrom(newfrom, true);
			}
		}
	}
	
	// Process prefs againg to get new locked values
	ResetLock();
	ProcessPrefs(prefs);
}


// Do lock on preferences
void CAdminLock::LockPrefs(CPreferences* prefs)
{
	// Check locked servers
	if (mLockServerAddress)
	{
		// IMAP server
		bool remote_set = mLockedMailboxServerAddr.empty();
		unsigned long pos = 0;
		for(CMailAccountList::iterator iter = prefs->mMailAccounts.Value().begin(); iter != prefs->mMailAccounts.Value().end(); iter++, pos++)
		{
			if (remote_set)
			{
				if ((*iter)->IsLocal() || !(*iter)->GetServerIP().empty())
					mLockedMailboxServerAddr.push_back((*iter)->GetServerIP());
			}
			else
			{
				if (pos >= mLockedMailboxServerAddr.size())
					pos = mLockedMailboxServerAddr.size() - 1;
				(*iter)->SetServerIP(mLockedMailboxServerAddr.at(pos));
			}
		}

		// Remote server
		remote_set = mLockedPrefsServerAddr.empty();
		pos = 0;
		for(COptionsAccountList::iterator iter = prefs->mRemoteAccounts.Value().begin(); iter != prefs->mRemoteAccounts.Value().end(); iter++, pos++)
		{
			if (remote_set)
			{
				if (!(*iter)->GetServerIP().empty())
					mLockedPrefsServerAddr.push_back((*iter)->GetServerIP());
			}
			else
			{
				if (pos >= mLockedPrefsServerAddr.size())
					pos = mLockedPrefsServerAddr.size() - 1;
				(*iter)->SetServerIP(mLockedPrefsServerAddr.at(pos));
			}
		}

		// SMTP server - only if lock required
		if (!mNoLockSMTP)
		{
			remote_set = mLockedSMTPServerAddr.empty();
			pos = 0;
			for(CSMTPAccountList::iterator iter = prefs->mSMTPAccounts.Value().begin(); iter != prefs->mSMTPAccounts.Value().end(); iter++, pos++)
			{
				if (remote_set)
				{
					if (!(*iter)->GetServerIP().empty())
						mLockedSMTPServerAddr.push_back((*iter)->GetServerIP());
				}
				else
				{
					if (pos >= mLockedSMTPServerAddr.size())
						pos = mLockedSMTPServerAddr.size() - 1;
					(*iter)->SetServerIP(mLockedSMTPServerAddr.at(pos));
				}
			}
		}

		// Adbk server
		remote_set = mLockedAdbkServerAddr.empty();
		pos = 0;
		for(CAddressAccountList::iterator iter = prefs->mAddressAccounts.Value().begin(); iter != prefs->mAddressAccounts.Value().end(); iter++, pos++)
		{
			if (remote_set)
			{
				if (!(*iter)->GetServerIP().empty())
					mLockedAdbkServerAddr.push_back((*iter)->GetServerIP());
			}
			else
			{
				if (pos >= mLockedAdbkServerAddr.size())
					pos = mLockedAdbkServerAddr.size() - 1;
				(*iter)->SetServerIP(mLockedAdbkServerAddr.at(pos));
			}
		}

		// ManageSIEVE server
		remote_set = mLockedManageSIEVEServerAddr.empty();
		pos = 0;
		for(CManageSIEVEAccountList::iterator iter = prefs->mSIEVEAccounts.Value().begin(); iter != prefs->mSIEVEAccounts.Value().end(); iter++, pos++)
		{
			if (remote_set)
			{
				if (!(*iter)->GetServerIP().empty())
					mLockedManageSIEVEServerAddr.push_back((*iter)->GetServerIP());
			}
			else
			{
				if (pos >= mLockedManageSIEVEServerAddr.size())
					pos = mLockedManageSIEVEServerAddr.size() - 1;
				(*iter)->SetServerIP(mLockedManageSIEVEServerAddr.at(pos));
			}
		}
	}

	// Check locked domain
	else if (mLockServerDomain)
	{
		// IMAP server
		for(CMailAccountList::iterator iter = prefs->mMailAccounts.Value().begin(); iter != prefs->mMailAccounts.Value().end(); iter++)
		{
			// Remove port number which shuld not be considered part of the domain
			cdstring server_ip = (*iter)->GetServerIP();
			if (::strchr(server_ip.c_str(), ':'))
				*::strchr(server_ip.c_str_mod(), ':') = 0;
			if (!server_ip.compare_end(mLockedDomain) && server_ip.length() && (server_ip[server_ip.length() - 1] != ','))
				(*iter)->SetServerIP(cdstring::null_str);
		}

		// Remote server
		for(COptionsAccountList::iterator iter = prefs->mRemoteAccounts.Value().begin(); iter != prefs->mRemoteAccounts.Value().end(); iter++)
		{
			// Remove port number which shuld not be considered part of the domain
			cdstring server_ip = (*iter)->GetServerIP();
			if (::strchr(server_ip.c_str(), ':'))
				*::strchr(server_ip.c_str_mod(), ':') = 0;
			if (!server_ip.compare_end(mLockedDomain) && server_ip.length() && (server_ip[server_ip.length() - 1] != ','))
				(*iter)->SetServerIP(cdstring::null_str);
		}

		// SMTP server - only if lock required
		if (!mNoLockSMTP)
		{
			for(CSMTPAccountList::iterator iter = prefs->mSMTPAccounts.Value().begin(); iter != prefs->mSMTPAccounts.Value().end(); iter++)
			{
				// Remove port number which shuld not be considered part of the domain
				cdstring server_ip = (*iter)->GetServerIP();
				if (::strchr(server_ip.c_str(), ':'))
					*::strchr(server_ip.c_str_mod(), ':') = 0;
				if (!server_ip.compare_end(mLockedDomain))
					(*iter)->SetServerIP(cdstring::null_str);
			}
		}

		// Address server
		for(CAddressAccountList::iterator iter = prefs->mAddressAccounts.Value().begin(); iter != prefs->mAddressAccounts.Value().end(); iter++)
		{
			// Remove port number which shuld not be considered part of the domain
			cdstring server_ip = (*iter)->GetServerIP();
			if (::strchr(server_ip.c_str(), ':'))
				*::strchr(server_ip.c_str_mod(), ':') = 0;
			if (!server_ip.compare_end(mLockedDomain))
				(*iter)->SetServerIP(cdstring::null_str);
		}
	}

	// Return address lock - empty => use remote
	if (mLockReturnAddress && mLockedReturnDomain.empty() && !prefs->mIdentities.GetValue().empty())
	{
		CAddress addr(prefs->mIdentities.GetValue().front().GetFrom());
		cdstring smtp_uid = addr.GetMailAddress();

		// Strip uid from return address
		ProcessLockedReturnDomain(mLockedReturnDomain, addr.GetMailAddress());
	}

	// Password saving
	if (mLockSavePswd)
	{
		// Change each plain text authenticator
		for(CMailAccountList::const_iterator iter = prefs->mMailAccounts.GetValue().begin(); iter != prefs->mMailAccounts.GetValue().end(); iter++)
		{
			if ((*iter)->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) (*iter)->GetAuthenticatorUserPswd();
				auth->SetSavePswd(false);
				//auth->SetPswd(cdstring::null_str);	<-- Must not delete cached password which may be in use in an open account
			}
		}

		// Change each plain text authenticator
		for(COptionsAccountList::const_iterator iter = prefs->mRemoteAccounts.GetValue().begin(); iter != prefs->mRemoteAccounts.GetValue().end(); iter++)
		{
			if ((*iter)->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) (*iter)->GetAuthenticatorUserPswd();
				auth->SetSavePswd(false);
				//auth->SetPswd(cdstring::null_str);	<-- Must not delete cached password which may be in use in an open account
			}
		}

		// Change each plain text authenticator
		for(CAddressAccountList::const_iterator iter = prefs->mAddressAccounts.GetValue().begin(); iter != prefs->mAddressAccounts.GetValue().end(); iter++)
		{
			if ((*iter)->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) (*iter)->GetAuthenticatorUserPswd();
				auth->SetSavePswd(false);
				//auth->SetPswd(cdstring::null_str);	<-- Must not delete cached password which may be in use in an open account
			}
		}
	}

	// Styled composition
	if (!mAllowStyledComposition)
		prefs->compose_as.SetValue(eContentSubPlain, false);

	// 3/1-pane choice
	if (!mAllow3PaneChoice)
		prefs->mUse3Pane.SetValue(mForce3Pane, false);
	
	// MDN lock
	if (mLockMDN)
		prefs->mMDNOptions.SetValue(eMDNAlwaysSend, false);

	// Do authenticator upgrade
	UpgradeAuthenticators(prefs);
}

// Process email address into locked domain part
bool CAdminLock::ProcessLockedReturnDomain(cdstring& locked, const cdstring& email)
{
	bool valid = true;

	// Copy existing
	locked = email;

	// Look for an '@' anywhere inside it
	const char* p = ::strrchr(locked.c_str(), '@');

	// Process '@'
	if (p)
	{
		// Look for '@' or '+' not at start and remove prefix
		if ((p != locked.c_str()) &&
			((p != locked.c_str() + 1) || (*(p-1) != '+')))
			locked = cdstring(p);
		else
			valid = false;
	}
	else
	{
		// No '@' so add it at start
		if (!locked.empty())
			locked = cdstring("@") + cdstring(locked);
		valid = false;
	}

	return valid;
}

// Upgrade all authenticators if required
void CAdminLock::UpgradeAuthenticators(CPreferences* prefs)
{
	// May need to convert
	if (CAdminLock::sAdminLock.mUpgradeKerberos)
	{
		// IMAP server
		for(CMailAccountList::const_iterator iter = prefs->mMailAccounts.GetValue().begin(); iter != prefs->mMailAccounts.GetValue().end(); iter++)
			UpgradeAuthenticator(*iter);

		// Remote server
		for(COptionsAccountList::const_iterator iter = prefs->mRemoteAccounts.GetValue().begin(); iter != prefs->mRemoteAccounts.GetValue().end(); iter++)
			UpgradeAuthenticator(*iter);

		// SMTP server
		for(CSMTPAccountList::const_iterator iter = prefs->mSMTPAccounts.GetValue().begin(); iter != prefs->mSMTPAccounts.GetValue().end(); iter++)
			UpgradeAuthenticator(*iter);

		// Adbk server
		for(CAddressAccountList::const_iterator iter = prefs->mAddressAccounts.GetValue().begin(); iter != prefs->mAddressAccounts.GetValue().end(); iter++)
			UpgradeAuthenticator(*iter);

		// ManageSIEVE server
		for(CManageSIEVEAccountList::const_iterator iter = prefs->mSIEVEAccounts.GetValue().begin(); iter != prefs->mSIEVEAccounts.GetValue().end(); iter++)
			UpgradeAuthenticator(*iter);
	}
}


// Upgrade an authenticator if required
void CAdminLock::UpgradeAuthenticator(CINETAccount* acct)
{
#ifdef __MULBERRY
	// Must have GSSAPI plugin available
	if (CPluginManager::sPluginManager.GetAuthPlugin("GSSAPI") != NULL)
	{
		if (acct->GetAuthenticator().GetDescriptor() == "Kerberos V4")
			acct->GetAuthenticator().SetDescriptor("GSSAPI");
	}
#endif
}
// Switching to use remote prefs rather than local
void CAdminLock::PrepareRemote()
{
	mLockedMailboxServerAddr.clear();
	mLockedPrefsServerAddr.clear();
	mLockedSMTPServerAddr.clear();
	mLockedAdbkServerAddr.clear();
	mLockedManageSIEVEServerAddr.clear();
}

void CAdminLock::SetServerID(const cdstring& server)
{
	// Copy server spec
	mServerID = server;
	
	// Strip off any port number
	char* p = ::strchr(mServerID.c_str_mod(), ':');
	if (p)
		*p = 0;
}

cdstring CAdminLock::GetLoginToken() const
{
	cdstring result = "Mulberry:";
	result += "01";					// method id for version control
	cdstring encrypt = GetLoginID();
	encrypt += "@";
	encrypt += GetServerID();
	encrypt.Encrypt(cdstring::eEncryptCipher, mOriginator.mTokenHash);
	result += encrypt;

	return result;
}

bool CAdminLock::CanSend(const CMessage* msg) const
{
	// Get address sizes
	long addr_size = 0;
	const CEnvelope* env = msg->GetEnvelope();
	addr_size += env->GetTo()->size();
	addr_size += env->GetCC()->size();
	addr_size += env->GetBcc()->size();

	// Check prevention
	if (CAdminLock::sAdminLock.mPrevent.mAddress &&
		(addr_size >= CAdminLock::sAdminLock.mPrevent.mAddress))
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Admin::PreventAddress");
		return false;
	}

	// Get attachment sizes
	long atch_size = msg->GetBody()->GetTotalSize() / 1024L;

	// Check prevention
	if (CAdminLock::sAdminLock.mPrevent.mSize &&
		(atch_size >= CAdminLock::sAdminLock.mPrevent.mSize))
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Admin::PreventSize");
		return false;
	}

	// Check prevention
	if (CAdminLock::sAdminLock.mPrevent.mProduct &&
		(addr_size * atch_size >= CAdminLock::sAdminLock.mPrevent.mProduct))
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Admin::PreventProduct");
		return false;
	}

	// Check warning
	if (CAdminLock::sAdminLock.mWarning.mAddress &&
		(addr_size >= CAdminLock::sAdminLock.mWarning.mAddress))
	{
		return CErrorHandler::PutCautionAlertRsrc(false, "Alerts::Admin::WarningAddress") == CErrorHandler::Ok;
	}

	// Check warning
	if (CAdminLock::sAdminLock.mWarning.mSize &&
		(atch_size >= CAdminLock::sAdminLock.mWarning.mSize))
	{
		return CErrorHandler::PutCautionAlertRsrc(false, "Alerts::Admin::WarningSize") == CErrorHandler::Ok;
	}

	// Check warning
	if (CAdminLock::sAdminLock.mWarning.mProduct &&
		(addr_size * atch_size >= CAdminLock::sAdminLock.mWarning.mProduct))
	{
		return CErrorHandler::PutCautionAlertRsrc(false, "Alerts::Admin::WarningProduct") == CErrorHandler::Ok;
	}

	return true;
}

bool CAdminLock::CanDownload(const CMIMEContent& content) const
{
	cdstring compare = content.GetContentTypeText();
	compare += "/";
	compare += content.GetContentSubtypeText();

	for(cdstrvect::const_iterator iter = mPreventDownloads.begin(); iter != mPreventDownloads.end(); iter++)
	{
		if (compare.PatternMatch(*iter))
			return false;
	}

	return true;
}
