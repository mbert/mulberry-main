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


// Header for CPreference class

#ifndef __CPREFERENCES__MULBERRY__
#define __CPREFERENCES__MULBERRY__

#include "CCalendarAccount.h"
#include "CCalendarStoreNode.h"
#include "CConnectionManager.h"
#include "CDayWeekViewTimeRange.h"
#include "CFavouriteItem.h"
#include "CICalendarDateTime.h"
#include "CIdentity.h"
#include "CMailAccount.h"
#include "CMailNotification.h"
#include "CManageSIEVEAccount.h"
#include "CMIMEMap.h"
#include "CMIMETypes.h"
#include "CNotification.h"
#include "CPreferenceEnum.h"
#include "CPreferenceValue.h"
#include "COptionsAccount.h"
#include "CSearchStyle.h"
#include "CSMTPAccount.h"
#include "CTextMacros.h"
#include "CWindowStates.h"
#include "C3PaneOptions.h"

#include "cdstring.h"
#include "templs.h"

// Types
typedef std::vector<RGBColor> CColourList;

// Consts

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const	ResIDT	Txtr_DefaultListPrefsText = 1012;
const	ResIDT	Txtr_DefaultAquaListPrefsText = 1022;
const	ResIDT	Txtr_DefaultTextPrefsText = 1000;
const	ResIDT	Txtr_DefaultAquaTextPrefsText = 1023;
const	ResIDT	Txtr_DefaultHTMLPrefsText = 1018;
#endif

// Types
class CAddressBook;
class COptionsMap;
class CFilterManager;

enum EMDNOptions
{
	eMDNAlwaysSend = 0,
	eMDNNeverSend,
	eMDNPromptSend
};

enum ESaveOptions
{
	eSaveDraftToFile = 0,
	eSaveDraftToMailbox,
	eSaveDraftChoose
};

class CPreferences
{
public:

	// Version number
	NumVersion									vers;								// Version number
	bool										mUpdateVers;						// Version needs updating

	// Prefs state
	CPreferenceValueMap<bool>					mHelpState;							// Whether to use balloon help in prefs dialog
	CPreferenceValueMap<bool>					mDetailedTooltips;					// Whether to use tooltips in tables
	CPreferenceValueMap<bool>					mUse_IC;							// Use internet config (where possible)
	CPreferenceValueMap<bool>					mCheckDefaultMailClient;			// Check whether default mailto handler
	CPreferenceValueMap<bool>					mCheckDefaultWebcalClient;			// Check whether default webcal handler
	CPreferenceValueMap<bool>					mWarnMailtoFiles;					// Warn if mailto URL attaches files
	CPreferenceValueMap<bool>					mSimple;							// Simple/advanced prefs dialog
	CPreferenceValueMap<cdstring>				mLastPanel;							// Last advanced panel used
	
	// Connection
	CPreferenceValueMap<bool>					mDisconnected;						// Disconnected mode
	CPreferenceValueMap<bool>					mSMTPQueueEnabled;					// Enable SMTP queue
	CPreferenceValueMap<CConnectionManager::CConnectOptions>	mConnectOptions;	// Connect options
	CPreferenceValueMap<CConnectionManager::CDisconnectOptions>	mDisconnectOptions;	// Disconnect options

	// Network control parameters
	CPreferenceValueMap<long>					mAppIdleTimeout;					// Timeout to force connection checks in app
	CPreferenceValueMap<long>					mWaitDialogStart;					// Start time to show wait dialog
	CPreferenceValueMap<long>					mWaitDialogMax;						// Max time before cancel of wait dialog
	CPreferenceValueMap<long>					mConnectRetryTimeout;				// Retry connect after this time
	CPreferenceValueMap<long>					mConnectRetryMaxCount;				// Max. no. of connect retries before failure
	CPreferenceValueMap<long>					mBackgroundTimeout;					// Timeout on non-responsive background checks
	CPreferenceValueMap<long>					mTickleInterval;					// Idle interval for tickles
	CPreferenceValueMap<long>					mTickleIntervalExpireTimeout;		// For failure if tickle interval is over this time
	CPreferenceValueMap<bool>					mSleepReconnect;					// Allow quick reconnect after wake from sleep

	// Account prefs
	CPreferenceValueMap<CMailAccountList>			mMailAccounts;					// List of mail accounts
	CPreferenceValueMap<CSMTPAccountList>			mSMTPAccounts;					// List of SMTP accounts
	CPreferenceValueMap<COptionsAccountList>		mRemoteAccounts;				// List of remote options accounts
	CPreferenceValueMap<CManageSIEVEAccountList>	mSIEVEAccounts;					// List of ManageSIEVE accounts
	CPreferenceValueMap<cdstring>					mMailDomain;					// Mail domain
	CPreferenceValueMap<bool>						mRemoteCachePswd;				// Cache uid/pswd pairs
	CPreferenceValueMap<bool>						mOSDefaultLocation;				// Use OS default file locations
	CPreferenceValueMap<cdstring>					mDisconnectedCWD;				// Directory for disconnected files

	// Notification prefs
	CPreferenceValueMap<CMailNotificationList>	mMailNotification;					// List of mail notification styles
	CPreferenceValueMap<CNotification>			mAttachmentNotification;			// Notification style for attachments

	// Display prefs
	CPreferenceValueMap<SStyleTraits>			mServerOpenStyle;					// Server display for open
	CPreferenceValueMap<SStyleTraits>			mServerClosedStyle;					// Server display for closed
	CPreferenceValueMap<SStyleTraits>			mServerBkgndStyle;					// Server display for closed
	CPreferenceValueMap<SStyleTraits>			mFavouriteBkgndStyle;				// Server display for closed
	CPreferenceValueMap<SStyleTraits>			mHierarchyBkgndStyle;				// Server display for closed

	CPreferenceValueMap<SStyleTraits>			mMboxRecentStyle;					// Mailbox display for recent
	CPreferenceValueMap<SStyleTraits>			mMboxUnseenStyle;					// Mailbox display for unseen
	CPreferenceValueMap<SStyleTraits>			mMboxOpenStyle;						// Mailbox display for open
	CPreferenceValueMap<SStyleTraits>			mMboxFavouriteStyle;				// Mailbox display for favourite
	CPreferenceValueMap<SStyleTraits>			mMboxClosedStyle;					// Mailbox display for closed
	CPreferenceValueMap<bool>					mIgnoreRecent;						// Ignore recent state

	CPreferenceValueMap<SStyleTraits>			unseen;								// Message display for unseen
	CPreferenceValueMap<SStyleTraits>			seen;								// Message display for seen
	CPreferenceValueMap<SStyleTraits>			answered;							// Message display for answered
	CPreferenceValueMap<SStyleTraits>			important;							// Message display for important
	CPreferenceValueMap<SStyleTraits>			deleted;							// Message display for deleted
	CPreferenceValueMap<long>					mMultiAddress;						// Style for multiple addresses
	CPreferenceValueMap<SStyleTraits>			mMatch;								// Message display for matched messages
	CPreferenceValueMap<SStyleTraits>			mNonMatch;							// Message display for non-matched messages
	CPreferenceValueMap<bool>					mUseLocalTimezone;					// Display dates/times adjusted to local timezone
	
	CPreferenceValueMap<CStyleTraits2List>		mLabels;							// List of label styles
	CPreferenceValueMap<cdstrvect>				mIMAPLabels;						// List of imap label names

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CPreferenceValueMap<STextTraitsRecord>		mListTextTraits;					// Text traits for list text
	CPreferenceValueMap<STextTraitsRecord>		mDisplayTextTraits;					// Text traits for display text
	CPreferenceValueMap<STextTraitsRecord>		mPrintTextTraits;					// Text traits for printed text
	CPreferenceValueMap<STextTraitsRecord>		mCaptionTextTraits;					// Text traits for printed captions
	CPreferenceValueMap<bool>					mAntiAliasFont;						// Use anti-aliased text
#elif __dest_os == __win32_os
	CPreferenceValueMap<SLogFont>				mListTextFontInfo;					// Font details for list text
	CPreferenceValueMap<SLogFont>				mDisplayTextFontInfo;				// Font details for display text
	CPreferenceValueMap<SLogFont>				mPrintTextFontInfo;					// Font details for print text
	CPreferenceValueMap<SLogFont>				mCaptionTextFontInfo;				// Font details for print captions
#elif __dest_os == __linux_os
	CPreferenceValueMap<SFontInfo>				mListTextFontInfo;					// Font details for list text
	CPreferenceValueMap<SFontInfo>				mDisplayTextFontInfo;				// Font details for display text
	CPreferenceValueMap<SFontInfo>				mPrintTextFontInfo;					// Font details for print text
	CPreferenceValueMap<SFontInfo>				mCaptionTextFontInfo;				// Font details for print captions
#else
#error __dest_os
#endif

#if __dest_os == __win32_os
	//CPreferenceValueMap<bool>					mUseSDI;							// Use SDI format windows - deprecated for v3
	CPreferenceValueMap<bool>					mMultiTaskbar;						// Taskbar entry for each SDI window
#endif
	CPreferenceValueMap<bool>					mUse3Pane;							// Use 3-pane format windows
	CPreferenceValueMap<bool>					mToolbarShow;						// Show toolbars
	CPreferenceValueMap<bool>					mToolbarSmallIcons;					// Toolbars have small icons
	CPreferenceValueMap<bool>					mToolbarShowIcons;					// Toolbars display icons
	CPreferenceValueMap<bool>					mToolbarShowCaptions;				// Toolbars display captions
	CPreferenceValueMap<cdstring>				mToolbars;							// Toolbar layouts

	// Formatting Prefs -- Justin
	CPreferenceValueMap<SStyleTraits>			mQuotationStyle;					// Style for excerpts and quotations
	CPreferenceValueMap<SStyleTraits> 			mURLStyle;							// Style for highlighted URLs
	CPreferenceValueMap<SStyleTraits> 			mURLSeenStyle;						// Style for seen URLs
	CPreferenceValueMap<SStyleTraits> 			mHeaderStyle;						// Style for RFC822 Field Headers
	CPreferenceValueMap<SStyleTraits>			mTagStyle;							// Style for tags
	
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CPreferenceValueMap<STextTraitsRecord>		mFixedTextTraits;					// Text traits for fixed text
	CPreferenceValueMap<STextTraitsRecord>		mHTMLTextTraits;					// Text traits for HTML text
#elif __dest_os == __win32_os
	CPreferenceValueMap<SLogFont>				mFixedTextFontInfo;					// Font details for fixed text
	CPreferenceValueMap<SLogFont>				mHTMLTextFontInfo;					// Font details for HTML text
#elif __dest_os == __linux_os
	CPreferenceValueMap<SFontInfo>				mFixedTextFontInfo;					// Font details for fixed text
	CPreferenceValueMap<SFontInfo>				mHTMLTextFontInfo;					// Font details for HTML text
#else
#error __dest_os
#endif
	CPreferenceValueMap<bool>					mUseStyles;							// Use styled text fonts
	CPreferenceValueMap<long>					mMinimumFont;						// Minimum font size

	CPreferenceValueMap<bool>					mUseMultipleQuotes;					// Use multiple colours for quotes
	CPreferenceValueMap<CColourList>			mQuoteColours;						// Multiple colours for quotes
	CPreferenceValueMap<cdstrvect>				mRecognizeQuotes;					// Quotes to recognize
	CPreferenceValueMap<cdstrvect>				mRecognizeURLs;						// URLs to recognize

#if __dest_os == __linux_os
	CPreferenceValueMap<RGBColor>				mSelectionColour;					// Colour for selections
#endif

	// Mailbox prefs
	CPreferenceValueMap<bool>					openAtFirst;						// Open mailbox window at first message
	CPreferenceValueMap<bool>					openAtLast;							// Open mailbox window at last message
	CPreferenceValueMap<bool>					openAtFirstNew;						// Open mailbox window at first new message
	CPreferenceValueMap<bool>					mNoOpenPreview;						// Don't preview first unseen message
	CPreferenceValueMap<bool>					mUnseenNew;							// Treat Unseen flag as 'new'
	CPreferenceValueMap<bool>					mRecentNew;							// Treat Recent flag as 'new'
	CPreferenceValueMap<bool>					mNextIsNewest;						// Next message is the newer one
	CPreferenceValueMap<bool>					expungeOnClose;						// Expunge mailbox when closed
	CPreferenceValueMap<bool>					warnOnExpunge;						// Warn when expunging
	CPreferenceValueMap<bool>					mWarnPuntUnseen;					// Warn when punting unseen messages
	CPreferenceValueMap<bool>					mDoRollover;						// Allow rolling over
	CPreferenceValueMap<bool>					mRolloverWarn;						// Warn when rolling over
	CPreferenceValueMap<bool>					mDoMailboxClear;					// Do the mailbox clear action
	CPreferenceValueMap<cdstring>				clear_mailbox;						// Mailbox to clear INBOX to
	CPreferenceValueMap<bool>					clear_warning;						// Always do warning when clearing
	CPreferenceValueMap<long>					mRLoCache;							// Low tide for cache
	CPreferenceValueMap<long>					mRHiCache;							// High tide for cache
	CPreferenceValueMap<bool>					mRUseHiCache;						// Use high tide for cache
	CPreferenceValueMap<long>					mRCacheIncrement;					// Cache increment on miss
	CPreferenceValueMap<bool>					mRAutoCacheIncrement;				// Cache increment on miss from window
	CPreferenceValueMap<long>					mRSortCache;							// Low tide for cache
	CPreferenceValueMap<long>					mLLoCache;							// Low tide for cache
	CPreferenceValueMap<long>					mLHiCache;							// High tide for cache
	CPreferenceValueMap<bool>					mLUseHiCache;						// Use high tide for cache
	CPreferenceValueMap<long>					mLCacheIncrement;					// Cache increment on miss
	CPreferenceValueMap<bool>					mLAutoCacheIncrement;				// Cache increment on miss from window
	CPreferenceValueMap<long>					mLSortCache;							// Low tide for cache
	//CPreferenceValueMap<bool>					mPartialDisplay;					// Display fixed number of messages
	//CPreferenceValueMap<long>					mMsgBlocks;							// Read messages in blocks
	CPreferenceValueMap<cdstrvect>				mSmartAddressList;					// List of smart addresses
	CPreferenceValueMap<bool>					mUseCopyToMboxes;					// Use mailboxes to copy to
	CPreferenceValueMap<bool>					mUseAppendToMboxes;					// Use mailboxes to append to
	CPreferenceValueMap<long>					mMRUMaximum;						// Maximum number of MRUs
	CPreferenceValueMap<SACLStyleList>			mMboxACLStyles;						// List of mailbox ACL styles
	CPreferenceValueMap<CSearchStyleList>		mSearchStyles;						// List of search styles
	CPreferenceValueMap<bool>					mPromptDisconnected;				// Prompt disconnect at start
	CPreferenceValueMap<bool>					mAutoDial;							// Automatic dial/hangup
	CPreferenceValueMap<bool>					mUIDValidityMismatch;				// Warn on UIDValidity mismatch
	CPreferenceValueMap<long>					mQuickSearch;						// Quick search popup value
	CPreferenceValueMap<bool>					mQuickSearchVisible;				// Quick search items visible
	CPreferenceValueMap<bool>					mScrollForUnseen;					// Scroll unseen message into view

	// Favourites prefs
	CPreferenceValueMap<CFavouriteItemList>		mFavourites;						// List of favourite items
	CPreferenceValueMap<cdstrvect>				mMRUCopyTo;							// List of recent copy tos
	CPreferenceValueMap<cdstrvect>				mMRUAppendTo;						// List of recent append tos
	CPreferenceValueMap<cdstrmap>				mMailboxAliases;					// Map of mailbox name aliases

	// Searching prefs
	CPreferenceValueMap<bool>					mMultipleSearch;					// Do simultaneous multiple searches
	CPreferenceValueMap<long>					mNumberMultipleSearch;				// Number of simultaneous multiple searches
	CPreferenceValueMap<bool>					mLoadBalanceSearch;					// Do load balanced multiple searching
	CPreferenceValueMap<bool>					mOpenFirstSearchResult;				// Open mbox window for first search hit	
	
	// Rules
	CFilterManager*								mFilterManager;						// Filter management

	// Message prefs
	CPreferenceValueMap<bool>					showMessageHeader;					// Show the RFC822 header in message windows
	CPreferenceValueMap<bool>					saveMessageHeader;					// Save the RFC822 header when saving messages
	CPreferenceValueMap<cdstring>				mSaveCreator;						// Creator type for saved messages
	CPreferenceValueMap<bool>					deleteAfterCopy;					// Delete messages after copying
	CPreferenceValueMap<bool>					mOpenDeleted;						// Allow deleted messages to be opened
	CPreferenceValueMap<bool>					mCloseDeleted;						// Close messages when deleted
	CPreferenceValueMap<bool>					mDoSizeWarn;						// Do size warning
	CPreferenceValueMap<long>					warnMessageSize;					// Size of message for open warning
	CPreferenceValueMap<bool>					mQuoteSelection;					// Quote selection on reply
	CPreferenceValueMap<bool>					mAlwaysQuote;						// Quote original if no selection
	CPreferenceValueMap<bool>					optionKeyReplyDialog;				// Option key shows reply dialog
	CPreferenceValueMap<bool>					showStyled;							// Show Styled text in message
	CPreferenceValueMap<cdstring>				mHeaderCaption;						// Printed header caption
	CPreferenceValueMap<bool>					mHeaderBox;							// Printed header caption box
	CPreferenceValueMap<bool>					mPrintSummary;						// Print summary headers
	CPreferenceValueMap<cdstring>				mFooterCaption;						// Printed footer caption
	CPreferenceValueMap<bool>					mFooterBox;							// Printed footer caption box
	CPreferenceValueMap<bool>					mOpenReuse;							// Reuse existing open message window
	CPreferenceValueMap<bool>					mAutoDigest;						// Automatically open multipart/digest
	CPreferenceValueMap<bool>					mExpandHeader;						// Automatically expand header for multiple addresses
	CPreferenceValueMap<bool>					mExpandParts;						// Automatically expand parts list
	CPreferenceValueMap<bool>					mForwardChoice;						// Forward using choice of options
	CPreferenceValueMap<bool>					mForwardQuoteOriginal;				// Forward using quotation
	CPreferenceValueMap<bool>					mForwardHeaders;					// Forward using headers
	CPreferenceValueMap<bool>					mForwardAttachment;					// Forward as attachment
	CPreferenceValueMap<bool>					mForwardRFC822;						// Forward using message/rfc822
	CPreferenceValueMap<CPreferenceEnum<EMDNOptions> >	mMDNOptions;				// Options for sending MDNs

	// Draft Prefs
	CPreferenceValueMap<bool>					mAutoInsertSignature;				// Automatically insert signature
	CPreferenceValueMap<bool>					mSignatureEmptyLine;				// Automatically insert empty line before signature
	CPreferenceValueMap<bool>					mSigDashes;							// Automatically insert sigdashes before signature
	CPreferenceValueMap<bool>					mNoSubjectWarn;						// Warn if no subject on send
	CPreferenceValueMap<long>					spaces_per_tab;						// Number of spaces inserted when tab typed
	CPreferenceValueMap<bool>					mTabSpace;							// Use spaces instead of Tab
	CPreferenceValueMap<cdstring>				mReplyQuote;						// Quote string for replying
	CPreferenceValueMap<cdstring>				mReplyStart;						// Start caption for replying
	CPreferenceValueMap<cdstring>				mReplyEnd;							// End caption for replying
	CPreferenceValueMap<bool>					mReplyCursorTop;					// Leave cursor at top of caption
	CPreferenceValueMap<bool>					mReplyNoSignature;					// Remove signatures from reply quotations
	CPreferenceValueMap<cdstring>				mForwardQuote;						// Quote string for forwarding
	CPreferenceValueMap<cdstring>				mForwardStart;						// Start caption for forwarding
	CPreferenceValueMap<cdstring>				mForwardEnd;						// End caption for forwarding
	CPreferenceValueMap<bool>					mForwardCursorTop;					// Leave cursor at top of caption
	CPreferenceValueMap<cdstring>				mForwardSubject;					// Forwarding subject
	CPreferenceValueMap<cdstring>				mLtrHeaderCaption;					// Printed header caption
	CPreferenceValueMap<cdstring>				mLtrFooterCaption;					// Printed footer caption
	CPreferenceValueMap<bool>					mSeparateBCC;						// Send separate BCC message
	CPreferenceValueMap<cdstring>				mBCCCaption;						// BCC caption
	CPreferenceValueMap<bool>					mShowCCs;							// Show CCs always
	CPreferenceValueMap<bool>					mShowBCCs;							// Show BCCs always
	CPreferenceValueMap<cdstring>				mExternalEditor;					// External editor command
	CPreferenceValueMap<bool>					mUseExternalEditor;					// Allow external editor use
	CPreferenceValueMap<bool>					mExternalEditAutoStart;				// Use external editor when draft created
	CPreferenceValueMap<long>					wrap_length;						// Number of chars to wrap to when sending
	CPreferenceValueMap<bool>					mWindowWrap;						// Wrap to window width
	CPreferenceValueMap<bool>					mDisplayAttachments;				// Display attachments only in parts
	CPreferenceValueMap<bool>					mAppendDraft;						// Use Draft flag on manual append
	CPreferenceValueMap<bool>					inbox_append;						// Set checkbox for INBOX
	CPreferenceValueMap<bool>					mDeleteOriginalDraft;				// Delete original drafts
	CPreferenceValueMap<bool>					mTemplateDrafts;					// Treat important-drafts as templates
	CPreferenceValueMap<bool>					mSmartURLPaste;						// Paste URLs adding <...> delimiters
	CPreferenceValueMap<bool>					mWarnReplySubject;					// Warn if reply subject is changed
	CPreferenceValueMap<CPreferenceEnum<ESaveOptions> >	mSaveOptions;				// Options for saving a draft
	CPreferenceValueMap<cdstring>				mSaveMailbox;						// Mailbox to save drafts to
	CPreferenceValueInt<EContentSubType>		compose_as;							// type to compose as
	CPreferenceValueMap<bool>					enrMultiAltPlain;					// generate multi-alt plain
	CPreferenceValueMap<bool>					htmlMultiAltEnriched;				// generate multi-alt enriched
	CPreferenceValueMap<bool>					enrMultiAltHTML;					// generate multi-alt html
	CPreferenceValueMap<bool>					htmlMultiAltPlain;					// generate multi-alt plain
	CPreferenceValueMap<bool>					mFormatFlowed;						// Use format=flowed
	CPreferenceValueMap<bool>					mAlwaysUnicode;						// Always send in unicode if required
	CPreferenceValueMap<bool>					mDisplayIdentityFrom;				// Display From address in identity popup
	CPreferenceValueMap<bool>					mAutoSaveDrafts;					// Auto-save drafts for recovery
	CPreferenceValueMap<long>					mAutoSaveDraftsInterval;			// Auto-save drafts for recovery at seconds interval
	
	// Security Prefs
	CPreferenceValueMap<cdstring>				mPreferredPlugin;					// Which plugin to use if more than one
	CPreferenceValueMap<bool>					mUseMIMESecurity;					// Use MIME parts
	CPreferenceValueMap<bool>					mEncryptToSelf;						// Always encrypt to Self
	CPreferenceValueMap<bool>					mCachePassphrase;					// Cache encryption passphrase
	CPreferenceValueMap<long>					mCachePassphraseMax;				// Cache encryption passphrase for max minutes
	CPreferenceValueMap<long>					mCachePassphraseIdle;				// Cache encryption passphrase for idle minutes
	CPreferenceValueMap<bool>					mAutoVerify;						// Auto verify when message opened
	CPreferenceValueMap<bool>					mAutoDecrypt;						// Auto decrypt when message opened
	CPreferenceValueMap<bool>					mWarnUnencryptedSend;				// Warn if not encrypting content that was originally encrypted
	CPreferenceValueMap<bool>					mUseErrorAlerts;					// Display errors as alerts
	CPreferenceValueMap<bool>					mCacheUserCerts;					// Cache all received user certs
	CPreferenceValueMap<CNotification>			mVerifyOKNotification;				// Notification style for signature verify OK
	
	// Identity Prefs
	CPreferenceValueMap<CIdentityList>			mIdentities;						// List of identities
	CPreferenceValueMap<CTiedIdentityList>		mTiedMailboxes;						// List of tied mailboxes
	CPreferenceValueMap<CTiedIdentityList>		mTiedCalendars;						// List of tied calendars
	CPreferenceValueMap<bool>					mContextTied;						// Tied identities are context sensitive
	CPreferenceValueMap<bool>					mMsgTied;							// Pick identity using original message from address
	CPreferenceValueMap<bool>					mTiedMboxInherit;					// Inherit tied mailbox identities

	// Address Prefs
	CPreferenceValueMap<CAddressAccountList>	mAddressAccounts;					// List of address accounts
	CPreferenceValueMap<CAddressAccount>		mLocalAdbkAccount;					// Local address account
	CPreferenceValueMap<CAddressAccount>		mOSAdbkAccount;						// OS address account
	CPreferenceValueMap<cdstrset>				mExpandedAdbks;						// Address books that are expanded in the display
	CPreferenceValueMap<cdstrvect>				mAdbkOpenAtStartup;					// Open at startup list
	CPreferenceValueMap<cdstrvect>				mAdbkNickName;						// Nick-name resolution
	CPreferenceValueMap<cdstrvect>				mAdbkSearch;						// Search adbks
	CPreferenceValueMap<cdstrvect>				mAdbkAutoSync;						// Sync on disconnect adbks
	CPreferenceValueMap<SACLStyleList>			mAdbkACLStyles;						// List of address book ACL styles
	CPreferenceValueMap<bool>					mOptionKeyAddressDialog;			// Option key shows address dialog
	CPreferenceValueMap<bool>					mExpandNoNicknames;					// Expansion instead of nick-name resolution
	CPreferenceValueMap<bool>					mExpandFailedNicknames;				// Expansion when nick-names fail
	CPreferenceValueMap<bool>					mExpandFullName;					// Expand using full name
	CPreferenceValueMap<bool>					mExpandNickName;					// Expand using full nick-name
	CPreferenceValueMap<bool>					mExpandEmail;						// Expand using email
	CPreferenceValueMap<bool>					mSkipLDAP;							// Skip LDAP during searches
	CPreferenceValueMap<cdstring>				mCaptureAddressBook;				// Address book to capture to
	CPreferenceValueMap<bool>					mCaptureAllowEdit;					// Allow edit during capture
	CPreferenceValueMap<bool>					mCaptureAllowChoice;				// Allow choice for multiple items
	CPreferenceValueMap<bool>					mCaptureRead;						// Capture when reading a message
	CPreferenceValueMap<bool>					mCaptureRespond;					// Capture when responding to a message
	CPreferenceValueMap<bool>					mCaptureFrom;						// Capture From address in a message
	CPreferenceValueMap<bool>					mCaptureCc;							// Capture Cc address in a message
	CPreferenceValueMap<bool>					mCaptureReplyTo;					// Capture ReplyTo address in a message
	CPreferenceValueMap<bool>					mCaptureTo;							// Capture To address in a message

	// Calendar Prefs
	CPreferenceValueMap<CCalendarAccountList>				mCalendarAccounts;		// List of calendar accounts
	CPreferenceValueMap<CCalendarAccount>					mLocalCalendarAccount;	// Local calendar account
	CPreferenceValueMap<CCalendarAccount>					mWebCalendarAccount;	// Web calendar account
	CPreferenceValueMap<cdstrset>							mSubscribedCalendars;	// Calendars that are marked for display
	CPreferenceValueMap<cdstrset>							mExpandedCalendars;		// Calendars that are expanded in the display
	CPreferenceValueMap<cdstruint32map>						mCalendarColours;		// Colours used for subscribed calendars
	CPreferenceValueMap<cdstring>							mTimezone;				// Display timezone
	CPreferenceValueMap<cdstrset>							mFavouriteTimezones;	// Timezones in popup
	CPreferenceValueInt<iCal::CICalendarDateTime::EDayOfWeek>	mWeekStartDay;		// First day of week
	CPreferenceValueMap<long>								mWorkDayMask;			// Bit mask for which days are work days
	CPreferenceValueMap<CDayWeekViewTimeRange>				mDayWeekTimeRanges;		// Time ranges to display in day-week views
	CPreferenceValueMap<bool>								mDisplayTime;			// Display time in title of timed events
	CPreferenceValueMap<bool>								m24HourTime;			// Display time using 24 hour clock
	CPreferenceValueMap<bool>								mHandleICS;				// Handle text/calendar internally
	CPreferenceValueMap<bool>								mAutomaticIMIP;			// Send iMIPs automatically
	CPreferenceValueMap<bool>								mAutomaticEDST;			// Upgrade timezones automatically
	CPreferenceValueMap<bool>								mShowUID;               // Show UID property
	CPreferenceValueMap<cdstring>							mDefaultCalendar;		// Default calendar to add new events to

	// Attachment Prefs
	CPreferenceValueInt<ETransferMode>			mDefault_mode;						// Default attachment encoding mode
	CPreferenceValueMap<bool>					mDefault_Always;					// Default attachment encoding always
	CPreferenceValueMap<bool>					mViewDoubleClick;					// View on double-click
	CPreferenceValueMap<bool>					mAskDownload;						// Ask where to download
	CPreferenceValueMap<cdstring>				mDefaultDownload;					// Default download location
	CPreferenceValueInt<EAppLaunch>				mAppLaunch;							// Method of application launch
	CPreferenceValueMap<bool>					mLaunchText;						// Launch viewable parts
#if __dest_os == __win32_os
	CPreferenceValueMap<cdstring>				mShellLaunch;						// Launch string for shell
#endif
	CPreferenceValueMap<CMIMEMapVector>			mMIMEMappings;						// List of mappings
	CPreferenceValueMap<bool>					mExplicitMapping;					// Use explicit MIME mappings
	CPreferenceValueMap<bool>					mWarnMissingAttachments;			// Warn if attachments are missing from a draft
	CPreferenceValueMap<bool>					mMissingAttachmentSubject;			// Warn if attachments are missing from a draft subject
	CPreferenceValueMap<cdstrvect>				mMissingAttachmentWords;			// Words used to imply presence of attachments

#ifdef __use_speech
	// Speech Prefs
	CPreferenceValueMap<bool>					mSpeakNewOpen;						// Speak new messages when opening INBOX
	CPreferenceValueMap<bool>					mSpeakNewArrivals;					// Speak new messages when they arrive
#ifdef __MULBERRY_CONFIGURE_V2_0
	// Special to support v2.0.x admin tool
	CPreferenceValueMap<bool>					mSpeakNewAlert;						// Speak text when new messages arrive
	CPreferenceValueMap<cdstring>				mSpeakNewAlertTxt;					// Text to speak when new messages arrive
#endif
	CPreferenceValueMap<CMessageSpeakVector>	mSpeakMessageItems;					// Items to speak in a message
	CPreferenceValueMap<CMessageSpeakVector>	mSpeakLetterItems;					// Items to speak in a message
	CPreferenceValueMap<cdstring>				mSpeakMessageEmptyItem;				// Text to use if message item has no text
	CPreferenceValueMap<long>					mSpeakMessageMaxLength;				// Maximum length of message text to speak
#endif

	// Miscellaneous prefs
	CPreferenceValueMap<CTextMacros>			mTextMacros;						// Text macros
	CPreferenceValueMap<bool>					mAllowKeyboardShortcuts;			// Allow single-key keyboard shortcuts
	

	// Lists of window states
	CPreferenceValueMap<CWindowStateArray>		mServerWindowInfo;					// Server window info
	CPreferenceValueMap<CWindowStateArray>		mMailboxWindowInfo;					// Mailbox window info
	CPreferenceValueMap<CWindowStateArray>		mNewAddressBookWindowInfo;			// New Address book window info
	CPreferenceValueMap<CWindowStateArray>		mCalendarWindowInfo;				// Calendar window info

	// List of default window states
	CPreferenceValue<CStatusWindowState>		mStatusWindowDefault;				// Default status window state
#if __dest_os == __win32_os
	CPreferenceValue<CMDIWindowState>			mMDIWindowDefault;					// Default MDI window info
#endif
	CPreferenceValue<C3PaneWindowState>			m3PaneWindowDefault;				// Default 3-pane window info
	CPreferenceValue<cdstrvect>					m3PaneOpenMailboxes;				// List of mailboxes to re-open
	CPreferenceValue<cdstrvect>					m3PaneOpenAdbks;					// List of address books to re-open
	CPreferenceValue<C3PaneOptions>				m3PaneOptions;						// Default 3-pane window options
	CPreferenceValue<C3PaneOptions>				m1PaneOptions;						// Default 1-pane window options
	CPreferenceValue<CServerWindowState>		mServerWindowDefault;				// Default server window info
	CPreferenceValue<CWindowState>				mServerBrowseDefault;				// Default server browse state
	CPreferenceValue<CMailboxWindowState>		mMailboxWindowDefault;				// Default mailbox window info
	CPreferenceValue<CMailboxWindowState>		mSMTPWindowDefault;					// Default smtp window info
	CPreferenceValue<CMessageWindowState>		mMessageWindowDefault;				// Default message window state
	CPreferenceValue<CMessageWindowState>		mMessageView3Pane;					// Default message view 3-pane state
	CPreferenceValue<CMessageWindowState>		mMessageView1Pane;					// Default message view 1-pane state
	CPreferenceValue<CLetterWindowState>		mLetterWindowDefault;				// Default letter window state
	CPreferenceValue<CNewAddressBookWindowState>	mNewAddressBookWindowDefault;			// Default address book window state
	CPreferenceValue<CAdbkManagerWindowState>	mAdbkManagerWindowDefault;			// Default adbk manager window state
	CPreferenceValue<CAdbkSearchWindowState>	mAdbkSearchWindowDefault;			// Default adbk search window state
	CPreferenceValue<CFindReplaceWindowState>	mFindReplaceWindowDefault;			// Default find & replace window state
	CPreferenceValue<CSearchWindowState>		mSearchWindowDefault;				// Default search window state
	CPreferenceValue<CRulesWindowState>			mRulesWindowDefault;				// Default rules window state
	CPreferenceValue<CWindowState>				mRulesDialogDefault;				// Default rules dialog state
	CPreferenceValue<CCalendarStoreWindowState>	mCalendarStoreWindowDefault;		// Default calendar store window state
	CPreferenceValue<CCalendarWindowState>		mCalendarWindowDefault;				// Default calendar window info

public:

	static CPreferences*	sPrefs;								// Global prefs

			CPreferences();
			CPreferences(const CPreferences &copy)				// Copy constructor
				{ _copy(copy); }
			~CPreferences();

	CPreferences& operator=(const CPreferences& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CPreferences& other) const;			// Compare with same type
	int operator!=(const CPreferences& other) const				// Compare with same type
					{ return !(*this == other); }

	void	DiffDirty();										// Dirty prefs that are different from default

	void 	SetAsDefault();										// Replace global prefs with this prefs
	void 	Changed();											// Do prefs changed action
	bool	Valid(bool showAlert);								// Check for valid prefs
	void	MultiuserConfigure(const cdstring& uid,
								const cdstring& real_name,
								bool do_return_address = true);	// Configure prefs from multi-user input
	void	LockReturnAddress(const cdstring& uid,				// Lock return address
								const cdstring& real_name);
	bool	TransferUIDs(const cdstring& uid);					// Transfer UIDs

	void	CheckIC();											// Synchronise with Internet Config if required

	cdstring	GetVersionText();								// Get version as string

	void	SetWDPrefix(char* prefix);							// Set working directory prefix

	const CFilterManager* GetFilterManager() const
		{ return mFilterManager; }
	CFilterManager* GetFilterManager()
		{ return mFilterManager; }

	CPreferenceValue<C3PaneOptions>& Get3PaneOptions();			// Get current 3pane options

	void ForwardSubject(cdstring& subj) const;					// Format subject for forwarding
	void SignatureDashes(cdstring& sig) const;					// Add sig dashes if required

	// Environment details
	cdstring GetMailerDetails(bool full);						// Get X-Mailer header content
	cdstring GetSupportDetails();								// Get details about the app for support 

	// Account management
	void	NewAccount(CINETAccount* acct);									// New account
	void	RenameAccount(CINETAccount* acct, const cdstring& new_acct);	// Rename account
	void	DeleteAccount(CINETAccount* acct);								// Delete account

	// Calendar management
	void	RenameCalendar(const cdstring& old_name, const cdstring& new_name);	// Rename calendar
	void	DeleteCalendar(const cdstring& name);								// Delete calendar

	// Identity management
	void	DeleteIdentity(CIdentity* id);									// Identity deleted
	void	ReplaceIdentity(CIdentity& id, CIdentity& new_id);				// Identity change
	void	MoveIdentity(const ulvector& movers, unsigned long dest);		// Identity move by indices
	const CIdentity*	GetTiedIdentity(const CMessageList* msgs);					// Get identity associated with list of messages

	// Favourite Management
	void	RenameFavourite(unsigned long index, const cdstring& new_name);	// Favourite renamed
	void	RemoveFavourite(unsigned long index);							// Favourite removed
	bool	MapMailboxAlias(cdstring& name) const;							// Map mailbox name to alias

	// Address book flags
	void	RenameAddressBook(const cdstring& old_name, const cdstring& new_name);	// Rename addressbook
	void	RenameAddressBookURL(const cdstring& old_name, const cdstring& new_name);	// Rename addressbook
	void	RenameAddressBookFlag(const cdstring& old_name, const cdstring& new_name, CPreferenceValueMap<cdstrvect>& list);	// Changing address book flag
	void	DeleteAddressBook(const cdstring& name);								// Delete address book
	void	DeleteAddressBookURL(const cdstring& name);								// Delete address book
	void	DeleteAddressBookFlag(const cdstring& name, CPreferenceValueMap<cdstrvect>& list);								// Delete address book
	void	ChangeAddressBookOpenOnStart(CAddressBook* adbk, bool set);	// Changing address book flag
	void	ChangeAddressBookLookup(CAddressBook* adbk, bool set);		// Changing address book flag
	void	ChangeAddressBookSearch(CAddressBook* adbk, bool set);		// Changing address book flag
	void	ChangeAddressBookAutoSync(CAddressBook* adbk, bool set);	// Changing address book flag
	void	ChangeAddressBookFlag(CAddressBook* adbk, CPreferenceValueMap<cdstrvect>& list, bool set);	// Changing address book flag

	// Toolbar management
	void	ToolbarChanged();												// Toolbar has changed - sync. it

	// Read/write
	void 	WriteToMap(COptionsMap* theMap, bool dirty_only,
						bool servers_only = false, bool not_windows = false,
						bool local = true, bool remote = true);	// Write data to a stream
	bool	ReadFromMap(COptionsMap* theMap,
							NumVersion vers_app,
							NumVersion& vers_prefs);			// Read data from a stream

	void 	WriteWindowInfoToMap(COptionsMap* theMap,
									bool dirty_only);			// Write data to a stream
	void 	ReadWindowInfoFromMap(COptionsMap* theMap,
									NumVersion vers_app,
									NumVersion vers_prefs);		// Read data from a stream

	void	AddServerWindowInfo(CServerWindowState* state);						// Add server window info
	CServerWindowState* GetServerWindowInfo(const char* name);					// Get server window info

	void	AddMailboxWindowInfo(CMailboxWindowState* state);					// Add mailbox window info
	CMailboxWindowState* GetMailboxWindowInfo(const char* name);				// Get mailbox window info

	void	AddAddressBookWindowInfo(CNewAddressBookWindowState* state);		// Add address book window info
	CNewAddressBookWindowState* GetAddressBookWindowInfo(const char* name);		// Get address book window info

	void	AddCalendarWindowInfo(CCalendarWindowState* state);					// Add calendar window info
	CCalendarWindowState* GetCalendarWindowInfo(const char* name);				// Get calendar window info

	void	ResetAllStates(bool dirty);											// Reset all state info

	static bool	TestSmartAddress(CAddress& addr);				// Test for a smart address

private:
	static cdstring sFromAddr;									// Cached from address
	
	void InitNetworkControlPrefs();
	void InitAccountPrefs();
	void InitAlertPrefs();
	void InitDisplayPrefs();
	void InitFormattingPrefs();
	void InitMailboxPrefs();
	void InitFavouritesPrefs();
	void InitSearchPrefs();
	void InitRulesPrefs();
	void InitMessagePrefs();
	void InitLetterPrefs();
	void InitSecurityPrefs();
	void InitIdentityPrefs();
	void InitAddressPrefs();
	void InitCalendarPrefs();
	void InitAttachmentPrefs();
#ifdef __use_speech
	void InitSpeechPrefs();
#endif
	void InitMiscellaneousPrefs();
	void InitWindowPrefs();

	void _copy(const CPreferences &copy);				// Copy items
	
};

#endif
