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


// CAdmin.h

// Class controls lockable items in prefs

#ifndef __CADMIN__MULBERRY__
#define __CADMIN__MULBERRY__

#include "cdstring.h"

class CAdmin
{
public:
	struct SOriginator
	{
		bool mGenerate;										// Set in prefs
		bool mUseToken;										// Set in prefs
		cdstring mTokenHash;								// Set in prefs
		bool mUseTokenAuthority;							// Set in prefs
		cdstring mTokenAuthority;							// Set in prefs
	};

	struct SSendLimit
	{
		long mAddress;
		long mSize;											// Size in KBytes
		long mProduct;										// Size in KBytes
	};

	// Values set in Admin section of prefs
	bool				mUseMultiUser;						// Use all multi-user prefs
	bool				mAllowDefault;						// Allow preference file to be loaded/saved to default local location
	bool				mNoRemotePrefs;						// Do not allow remote prefs
	bool				mNoLocalPrefs;						// Do not allow local prefs
	bool				mNoDisconnect;						// Do not allow disconnected ops
	bool				mNoSMTPQueues;						// Do not allow SMTP queues
	bool				mNoLocalMbox;						// Do not allow local mailboxes
	bool				mNoLocalAdbks;						// Do not allow local address books
	bool				mNoLocalDrafts;						// Do not allow local drafts
	bool				mNoLocalCalendars;					// Do not allow local calendars
	bool				mNoAttachments;						// Do not allow Attachments to be added
	bool				mNoRules;							// Do not allow rules to be used
	bool				mLockVirtualDomain;					// Substitute user id into server
	bool				mLockServerAddress;					// Do not allow changes to server
	bool				mLockServerDomain;					// Only allow servers in once domain
	bool				mNoLockSMTP;						// Don't lock SMTP server addresses
	cdstring			mLockedDomain;						// Locked domain
	bool				mLockSMTPSend;						// Do not allow SMTP until IMAP login
	bool				mLockReturnAddress;					// Lock user's return address from multi-user prefs
	bool				mLockPersonalIC;					// Prevent use of Personal Internet Config Settings
	bool				mLockMDN;							// Lock MDN to auto-response mode
	bool				mAskRealName;						// Ask for real name if not present
	//bool				mGenerateAuth;						// Generate Authentication/Licensed to header
	bool				mGenerateSender;					// Generate Sender header
	bool				mAllowXHeaders;						// Allow X-Headers
	bool				mLockIdentityFrom;					// Lock identity from field	
	bool				mLockIdentityReplyTo;				// Lock identity reply-to field						
	bool				mLockIdentitySender;				// Lock identity sender field						
	bool				mLockSavePswd;						// Allow user to save password to file from multi-user prefs
	bool				mPromptLogoutQuit;					// Prompt for quit on server logout
	bool				mNoQuit;							// Recycle on quit
	bool				mAllowStyledComposition;			// Allow user to use styled (enriched/HTML) composition
	bool				mAllowRejectCommand;				// Allow reject command
	bool				mAutoCreateMailbox;					// Auto create default mailboxes
	cdstrvect			mAutoCreateMailboxes;				// List of mailboxes to auto-create
	bool				mSubcribeAutoCreate;				// Subscribe to auto-created mailboxes
	SOriginator			mOriginator;						// Originator-Info details
	SSendLimit			mWarning;							// Warning limits
	SSendLimit			mPrevent;							// Prevention limits
	cdstrvect			mPreventDownloads;					// MIME types to prevent downloads
	bool				mAllow3PaneChoice;					// Let user choose 3/1-pane version
	bool				mForce3Pane;						// Force to 3-pane version
	bool				mPreventCalendars;					// Prevent use of (and hide) calendars
	
	cdstring			mMulberryURL;						// URL for Mulberry home page
	cdstring			mMulberryFAQ;						// URL for online FAQ
	cdstring			mUpdateURL;							// URL for update check
	cdstring			mMulberrySupportAddress;			// Email address to use for Help menu Support item

	bool				mAllowLogging;						// Allow protocol logging
	bool				mSSLStateDisplay;					// Display state of SSL in MU dialog

	//bool				mUpgradeAuthenticator;				// Force authenicator on upgrade
	//EAuthUpgrade		mUpgradeAuthenticatorType;			// Type of authenticator to use
	bool				mUpgradeKerberos;					// Upgrade Kerberos V4 auth to GSSAPI
	
	bool				mUsePASSD;							// Use password plugins

	CAdmin();
	~CAdmin() {}
	
};

#endif
