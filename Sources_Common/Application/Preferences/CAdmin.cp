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


// CAdmin.cp

// Class controls lockable items in prefs

#include "CAdmin.h"

const char* cMulberryURL = "http://www.mulberrymail.com";
const char* cMulberryFAQ = "http://www.mulberrymail.com/faq/";
const char* cUpdatesURL = "http://www.mulberrymail.com/mulberry/checkversion.php";
const char* cMulberrySupportAddress = "mulberry-support@mulberrymail.com";

CAdmin::CAdmin()
{
	mUseMultiUser = false;						// Use all multi-user prefs
	mAllowDefault = true;						// Allow preference file to be loaded/saved to default local location
	mNoRemotePrefs = false;						// Do not allow remote prefs
	mNoLocalPrefs = false;						// Do not allow local prefs
	mNoDisconnect = false;						// Do not allow disconnected ops
	mNoSMTPQueues = false;						// Do not allow SMTP Queues
	mNoLocalMbox = false;						// Do not allow local mailboxes
	mNoLocalAdbks = false;						// Do not allow local address books
	mNoLocalDrafts = false;						// Do not allow local drafts
	mNoLocalCalendars = false;					// Do not allow local calendars
	mNoAttachments = false;						// Do not allow Attachments to be added
	mNoRules = false;							// Do not allow rules to be used
	mLockVirtualDomain = false;					// Substitute user id into server
	mLockServerAddress = false;					// Do not allow changes to server
	mLockServerDomain = false;					// Only allow servers in once domain
	mNoLockSMTP = false;						// Don't lock SMTP servers
	mLockedDomain = cdstring::null_str;			// Locked domain
	mLockSMTPSend = false;						// Do not allow SMTP until IMAP login
	mLockReturnAddress = false;					// Lock user's return address from multi-user prefs
	mLockPersonalIC = false;					// Prevent use of Personal Internet Config Settings
	mLockMDN = false;							// Lock MDN to auto-response mode
	mAskRealName = false;						// Ask for real name if not present
	//mGenerateAuth = false;					// Generate Authentication/Licensed to header
	mGenerateSender = false;					// Generate Sender header
	mAllowXHeaders = true;						// Allow X-Headers
	mLockIdentityFrom = false;					// Lock identity from field
	mLockIdentityReplyTo = false;				// Lock identity reply-to field
	mLockIdentitySender = false;				// Lock identity sender field
	mLockSavePswd = false;						// Allow user to save password to file from multi-user prefs
	mPromptLogoutQuit = false;					// Prompt for quit on server logout
	mNoQuit = false;							// Recycle on quit
	mAllowStyledComposition = true;				// Allow user to use styled (enriched/HTML) composition
	mAllowRejectCommand = true;					// Allow reject command
	mAutoCreateMailbox = false;					// Auto create default mailboxes
	mSubcribeAutoCreate = false;				// Subscribe to auto-created mailboxes

	mOriginator.mGenerate = false;				// Generate Originator-Info header
	mOriginator.mUseToken = false;				// Use login-token attribute
	mOriginator.mTokenHash = "1tM2ou3kl4eb%ne^-r&ar*uy";	// Login hash value
	mOriginator.mUseTokenAuthority = false;		// Use token_authority attribute
	mOriginator.mTokenAuthority = cdstring::null_str;	// Token authority

	mWarning.mAddress = 0;								// Warning limits
	mWarning.mSize = 0;									// Warning limits
	mWarning.mProduct = 0;								// Warning limits
	mPrevent.mAddress = 0;								// Prevention limits
	mPrevent.mSize = 0;									// Prevention limits
	mPrevent.mProduct = 0;								// Prevention limits

	//mPreventDownloads.push_back("image/*");
	//mPreventDownloads.push_back("application/octet-stream");

	mAllow3PaneChoice = true;							// Let user choose 3/1-pane version
	mForce3Pane = true;									// Force/default to 3-pane version
	
	mPreventCalendars = false;							// Prevent use of (and hide) calendars

	mMulberryURL = cMulberryURL;						// URL for Mulberry home page
	mMulberryFAQ = cMulberryFAQ;						// URL for online FAQ
	mUpdateURL = cUpdatesURL;							// URL for update check
	mMulberrySupportAddress = cMulberrySupportAddress;	// Email address to use for Help menu Support item

	mAllowLogging = true;								// Allow protocol logging
	mSSLStateDisplay = true;							// Display state of SSL in MU dialog

	//mUpgradeAuthenticator = false;						// Force authenicator on upgrade
	//mUpgradeAuthenticatorType = eUpgradeNone;			// Type of authenticator to use
	mUpgradeKerberos = false;							// Force Kerberos v4 -> GSSAPI upgrade

	mUsePASSD = true;									// Use password plugins
}
