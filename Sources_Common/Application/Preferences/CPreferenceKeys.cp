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


// CPreferencesKeys

// Definition of keys used in prefs

#include "CPreferenceKeys.h"

const char* cVendor = "Cyrusoft";
const char* cProduct = "Mulberry";
//const char* cVendorSection = "Cyrusoft.Mulberry";
//	const char* cPrefsSetSection = "Options";
		const char* cPrefsSetListKey = "Sets";
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		const char* cDefaultPrefsSetKey_1_3 = "Default Mac OS";
#elif __dest_os == __win32_os
		const char* cDefaultPrefsSetKey_1_3 = "Default Win32";
#elif __dest_os == __linux_os
		const char* cDefaultPrefsSetKey_1_3 = "Default Unix";
#endif
		const char* cDefaultPrefsSetKey_2_0 = "Default";

const char* cValueBoolTrue = "true";
const char* cValueBoolFalse = "false";

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define OS_PART "MacOS"
#elif __dest_os == __win32_os
#define OS_PART "Win32"
#else
#define OS_PART "Unix"
#endif

const char* cOSKey = OS_PART;											// New in v2.0

const char* cAdminSection = "Admin";
	const char*	cVersionKey = "Version";			// IMPORTANT - new cVersionKey's must be sync'd with CMulberryApp::DoDefaultPrefsFile
	const char*	cVersionKey_v1_4 = "Version_v1_4";							// New in v1.4
	const char*	cVersionKey_v2_0 = "Version_v2_0";							// New in v2.0
	const char*	cUseMultiUserKey = "Use Multi-user";
	const char*	cAllowDefaultKey = "Allow Default";
	const char*	cNoRemotePrefsKey = "No Remote Prefs";
	const char*	cNoLocalPrefsKey = "No Local Prefs";
	const char*	cNoDisconnectKey = "No Disconnected Use";					// New in v2.0
	const char*	cNoSMTPQueuesKey = "No SMTP Queues";						// New in v2.0
	const char*	cNoLocalMboxKey = "No Local Mailboxes";						// New in v2.0
	const char*	cNoLocalAdbksKey = "No Local Address Books";				// New in v1.4
	const char*	cNoLocalDraftsKey = "No Local Drafts";						// New in v1.4
	const char*	cNoLocalCalendarsKey = "No Local Calendars";				// New in v4.0
	const char*	cNoAttachmentsKey = "No Attachments";						// New in v2.1.1
	const char*	cNoRulesKey = "No Rules";									// New in v2.1.1
	const char*	cLockVirtualDomainKey = "Lock Virtual Domain";				// New in v3.0.2
	const char*	cLockServerAddressKey = "Lock Server Address";
	const char*	cLockServerDomainKey = "Lock Server Domain";
	const char*	cNoLockSMTPKey = "No Lock SMTP";							// New in v2.2
	const char*	cLockedDomainKey = "Locked Domain";
	const char*	cLockSMTPSendKey = "Lock SMTP";
	const char*	cLockReturnAddressKey = "Lock Return Address";
	const char*	cLockPersonalICKey = "Lock Personal IC";
	const char*	cLockMDNKey = "Lock MDN";									// New v3.0
	const char*	cAskRealNameKey = "Prompt for Real Name";					// New in v1.4
	const char*	cGenerateAuthKey = "Generate Authentication Headers";		// Deprecated as of v1.4a7
	const char*	cGenerateSenderKey = "Generate Sender";						// New in v2.0.6
	const char*	cAllowXHeadersKey = "Allow X-Headers";
	const char*	cLockIdentityFromKey = "Lock Identity From";
	const char*	cLockIdentityReplyToKey = "Lock Identity Reply-To";
	const char*	cLockIdentitySenderKey = "Lock Identity Sender";
	const char*	cLockSavePswdKey = "Lock Save Password";
	const char*	cPromptLogoutQuitKey = "Prompt Logout Quit";				// New in v1.3.4.1
	const char*	cNoQuitKey = "No Quit";										// New in v1.4.2
	const char*	cAllowStyledCompositionKey = "Allow Styled Text Composition";
	const char*	cAllowRejectCommandKey = "Allow Reject Command";			// New v3.1
	const char*	cAutoCreateMailboxKey = "Auto Create Default Mailboxes";
	const char*	cAutoCreateMailboxesKey = "Auto Create Mailboxes";			// New v3.1
	const char*	cSubcribeAutoCreateKey = "Subscribe Auto Create";			// New v3.1

	const char* cAdminOriginatorSection = "Originator-Info";				// This section new in v1.4
		const char*	cAdminUseOriginatorKey = "Generate Header";
		const char*	cAdminOriginatorUseTokenKey = "Use login-token";
		const char*	cAdminOriginatorTokenHashKey = "Token hash";
		const char*	cAdminOriginatorUseTokenAuthorityKey = "Use token-authority";
		const char*	cAdminOriginatorTokenAuthorityKey = "token-authority";
		const char*	cAdminOriginatorTokenAuthorityEncryptKey = "Mt1uo2lk3be4en%r-^ra&yu*";	// Key used to encrypt token authority value in admin prefs

	const char* cAdminAttachmentLimitsSection = "Attachment Limits";		// This section new in v1.4
		const char*	cAdminAttachmentWarningsSection = "Warning";
		const char*	cAdminAttachmentPreventionSection = "Prevention";
			const char*	cAdminAttachmentLimitAddressKey = "Addresses";
			const char*	cAdminAttachmentLimitSizeKey = "Message Size";
			const char*	cAdminAttachmentLimitProductKey = "Addresses and Message Size";

	const char*	cAdminPreventDownloadKey = "Prevent Downloads";				// New in v1.4

	const char*	cAdminAllow3PaneChoiceKey = "Allow 3Pane Choice";			// New in v3.0
	const char*	cAdminForce3PaneKey = "Use 3Pane";							// New in v3.0

	const char*	cAdminPreventCalendarsKey = "Prevent Calendars";			// New in v4.0.1

	const char*	cAdminMulberryURLKey = "Mulberry Home URL";						// New in v3.1.1
	const char*	cAdminMulberryFAQKey = "Mulberry FAQ URL";						// New in v3.1.1
	const char*	cAdminUpdateURLKey = "Mulberry Update URL";						// New in v3.1.1
	const char*	cAdminMulberrySupportAddressKey = "Mulberry Email Support";		// New in v3.1.1

	const char*	cAdminAllowLoggingKey = "Allow Protocol Logging";				// New in v2.0b5
	const char* cAdminSSLStateDisplayKey = "Show SSL State";					// New in v3.1.6

	//const char*	cAdminUpgradeAuthenticatorKey = "Upgrade Authenticator";				// New in v1.4
	//const char*	cAdminUpgradeAuthenticatorTypeKey = "Upgrade Authenticator Type";		// New in v1.4
	const char*	cAdminUpgradeKerberosKey = "Upgrade Kerberos";		// New in v3.1.1

	const char*	cAdminUsePASSDKey = "Use PASSD";										// New in v1.4.2

const char* cPrefsSection = "Preferences";
	//const char*	cVersionKey = "Version";
	const char*	cHelpStateKey = "Help";
	const char*	cDetailedTooltipsKey = "Detailed Tooltips";					// New in v3.0a2
	const char*	cUseICKey = "Internet Config";
	const char*	cCheckDefaultMailClientKey = "Check Default Mail Client";	// New in v3.0a3
	const char*	cCheckDefaultWebcalClientKey = "Check Default Webcal Client";	// New in v4.0d1
	const char*	cWarnMailtoFilesKey = "Warn mailto files";					// New in v3.1.1
	const char*	cSimpleKey = "Simple Configuration";						// New in v1.4
	const char*	cLastPanelKey = "Last Preference Panel";					// New in v2.0b5
	const char*	cDisconnectedKey = "Disconnected";							// New in v2.0
	const char*	cSMTPQueueEnabledKey = "SMTP Queue Enabled";				// New in v2.0
	const char*	cConnectOptionsKey = "Connect Options";						// New in v2.0
	const char*	cDisconnectOptionsKey = "Disconnect Options";				// New in v2.0

	const char* cNetworkControlSection = "Network Control";						// This section new in v3.0b7
		const char* cAppIdleTimeoutKey = "Application Idle";					// New in v3.0b7
		const char* cWaitDialogStartKey = "Wait Dialog Start";					// New in v3.0b7
		const char* cWaitDialogMaxKey = "Wait Dialog Max";						// New in v3.0b7
		const char* cConnectRetryTimeoutKey = "Connect Retry";					// New in v3.0b7
		const char* cConnectRetryMaxCountKey = "Connect Retry Max Count";		// New in v3.0b7
		const char* cBackgroundTimeoutKey = "Background Timeout";				// New in v3.0b7
		const char* cTickleIntervalKey = "Tickle Interval";						// New in v3.0b7
		const char* cTickleIntervalExpireTimeoutKey = "Tickle Interval Expire";	// New in v3.0b7
		const char* cSleepReconnectKey = "Sleep Reconnect";						// New in v4.0b3

	const char* cAccountSection = "Accounts";								// This section new in v1.4
		const char* cMailAccountsKey_1_4 = "Mail Accounts";
		const char* cMailAccountsKey_2_0 = "Mail Accounts v2";				// Format change in v2.0
		const char* cSMTPAccountsKey_1_4 = "SMTP Accounts";
		const char* cSMTPAccountsKey_2_0 = "SMTP Accounts v2_0";			// Format change in v2.0
		const char* cSIEVEAccountsKey = "SIEVE Accounts";					// New in v3.0.1
		const char* cMailDomainKey = "Default Outgoing Domain";				// Also used in Network
		const char* cOSDefaultLocationKey = "OS Default Location";			// New in 2.0a6
		const char* cDisconnectedCWDKey = "Disconnected Directory";			// New in 2.0a6

	// This section no longer used, but required for backward compatibility
	const char* cNetworkSection = "Network";								// Deprecated as of v1.4a2
		const char* cIMAPServerIPKey = "IMAP Server";
		const char* cSMTPServerIPKey = "SMTP Server";
		const char* cUserNameKey = "Real Name";
		const char* cSMTPuidKey = "Email Address";
		const char* cIMAPuidKey = "IMAP User ID";
		const char* cPasswordKey = "Password";
		const char* cSaveUserKey = "Remember User ID";
		const char* cSavePswdKey = "Save Password";
		const char* cDirDelimKey = "Directory Hierarchy Character";
		const char* cWDsKey = "Places to Search";
		const char* cLogonStartupKey = "Login at Start";
		const char* cDoCheckKey = "Check for New mail";
		const char* cCheckIntervalKey = "Check Interval";
		const char* cNewMailAlertKey = "Alert for New Mail";

	const char* cRemoteSection = "Remote";
		const char* cRemoteAccountsKey_1_4 = "Remote Accounts";					// New in v1.4
		const char* cRemoteAccountsKey_2_0 = "Remote Accounts v2_0";			// New format in v2.0
		const char* cUseRemoteKey = "Use Remote";								// Deprecated as of v1.4a2
		const char* cRemoteIPKey = "Server";									// Deprecated as of v1.4a2
		const char* cRemoteUIDKey = "User ID";									// Deprecated as of v1.4a2
		const char* cRemotePswdKey = "Password";								// Deprecated as of v1.4a2
		const char* cRemoteSaveUserKey = "Save User ID";						// Deprecated as of v1.4a2
		const char* cRemoteSavePswdKey = "Save Password";						// Deprecated as of v1.4a2
		const char* cRemoteCachePswdKey = "Cache UID-Password";

	const char* cNotificationSection_1_4 = "Notification";
	const char* cNotificationSection_2_0 = "Notification v2";
		const char* cMailNotificationKey = "Mail Notifications";
		const char* cAttachmentNotificationKey = "Attachment Notifications";

	const char* cDisplaySection = "Display";
		const char* cServerOpenStyleKey = "Logged in Server Style";				// New in v1.4
		const char* cServerClosedStyleKey = "Logged out Server Style";			// New in v1.4
		const char* cServerBkgndStyleKey = "Server Background Colour";			// New in v1.4
		const char* cFavouriteBkgndStyleKey = "Favourite Background Colour";	// New in v1.4
		const char* cHierarchyBkgndStyleKey = "Hierarchy Background Colour";	// New in v1.4
		const char* cMboxRecentStyleKey = "Recent Mailbox Style";				// New in v1.4
		const char* cMboxUnseenStyleKey = "Unseen Mailbox Style";				// New in v1.4
		const char* cMboxOpenStyleKey = "Open Mailbox Style";					// New in v1.4
		const char* cMboxFavouriteStyleKey = "Favourite Mailbox Style";			// New in v1.4
		const char* cMboxClosedStyleKey = "Closed Mailbox Style";				// New in v1.4
		const char* cIgnoreRecentStyleKey = "Ignore Recent Style";				// New in v3.0a2
		const char* cUnseenStyleKey = "Unseen Style";
		const char* cSeenStyleKey = "Seen Style";
		const char* cAnsweredStyleKey = "Answered Style";
		const char* cImportantStyleKey = "Important Style";
		const char* cDeletedStyleKey = "Deleted Style";
		const char* cMultiAddressKey = "Multiple Address";
		const char* cMatchedStyleKey = "Matched Style";							// New in v1.4
		const char* cNonMatchedStyleKey = "Non-matched Style";					// New in v1.4
		const char* cUseLocalTimezoneKey = "Use Local Timezone";				// New in v3.0a2

		const char* cLabelsKey = "Labels";										// New in v3.1b1
		const char* cIMAPLabelsKey = "IMAP Labels";								// New in v4.0.1

#if __dest_os == __win32_os
		const char* cUseSDIKey = "Use SDI";										// New in v1.4
		const char* cMultiTaskbarKey = "Multiple Taskbar Items";				// New in v1.4
#endif
		const char* cUse3PaneKey = "Use 3Pane";									// New in v3.0d1
		const char* cToolbarShowKey = "Toolbar Show";							// New in v3.0a2
		const char* cToolbarSmallIconsKey = "Toolbar Small Icons";				// New in v3.0d1
		const char* cToolbarShowIconsKey = "Toolbar Show Icons";				// New in v3.0d1
		const char* cToolbarShowCaptionsKey = "Toolbar Show Captions";			// New in v3.0d1
		const char* cToolbarsKey = "Toolbars";									// New in v3.2a1

	const char *cFormattingSection = "Formatting";
		const char* cURLStyleKey = "URL Style";
		const char* cURLSeenStyleKey = "URL Seen Style";						// New in v2.0a6
		const char* cHeaderStyleKey = "Header Style";
		const char* cQuotationStyleKey = "Quotation Style";
		const char* cTagStyleKey = "Tag Style";
		const char* cUseMultipleQuotesKey = "Use Multiple Quotes";				// New in v1.4.2
		const char* cQuoteColoursKey = "Quote Colours";							// New in v1.4.2
		const char* cRecognizeQuotesKey_1_4 = "Recognized Quotes";				// New in v1.4.2; deprecated format as of v2.0b6
		const char* cRecognizeQuotesKey_2_0 = "Recognized Quotes v2_0";			// New in v2.0b6
		const char* cRecognizeURLsKey_1_4 = "Recognized URLs";					// Deprecated format as of v2.0b6
		const char* cRecognizeURLsKey_2_0 = "Recognized URLs v2_0";				// New in v2.0b6
#if __dest_os == __linux_os
		const char* cSelectionColourKey = "Selection Colour "OS_PART;			// New in v2.0.6b3
#endif

	const char *cFontsSection = "Fonts "OS_PART;
		const char* cFontKey = "Font";											// Old (removed in v1.1.1a3)
		const char* cListFontKey = "List Font";
		const char* cDisplayFontKey = "Text Font";
		const char* cPrintFontKey = "Print Font";								// New in v1.4
		const char* cCaptionFontKey = "Caption Font";							// New in v1.4
		const char* cFixedFontKey = "Fixed Text Font";
		const char* cHTMLFontKey = "HTML Text Font";
		const char* cAntiAliasFontKey = "Anti-Alias Font";
		const char* cUseStylesKey = "Use Styled Text";							// New in v3.1.3
		const char* cMinimumFontKey = "Minimum Font";							// New in v3.1.3

	const char* cMailboxSection = "Mailbox";
		const char* cOpenAtFirstKey = "Open at First";
		const char* cOpenAtLastKey = "Open at Last";
		const char* cOpenAtFirstNewKey = "Open First Unseen";
		const char* cNoOpenPreviewKey = "No Open Preview";					// New in v3.1b8
		const char* cUnseenNewKey = "Unseen is New";
		const char* cRecentNewKey = "Recent is New";
		const char* cNextIsNewestKey = "Next is newest";					// New in v3.0b7
		const char* cExpungeOnCloseKey = "Expunge on Close";
		const char* cWarnOnExpungeKey = "Warn on Expunge";
		const char* cWarnPuntUnseenKey = "Warn on Punt Unseen";				// New in v3.1b9
		const char* cDoRolloverKey = "Do Rollover";							// New in v1.4
		const char* cRolloverWarnKey = "Warn on Rollover";					// New in v1.4
		const char* cDoMailboxClearKey = "Move Read Messages";
		const char* cClearMailboxKey_1_3 = "Move to";						// Deprecated format as of v1.4
		const char* cClearMailboxKey_1_4 = "Move to v1_4";					// New format in v1.4.2
		const char* cClearWarningKey = "Warn on Move";
		const char* cPartialDisplayKey = "Limit Messages";					// Deprecated as of v1.4
		const char* cMsgBlocksKey = "Maximum Messages";						// Deprecated as of v1.4
		const char* cRLoCacheKey = "Low Cache";								// New in v1.4
		const char* cRHiCacheKey = "High Cache";								// New in v1.4
		const char* cRUseHiCacheKey = "Use High Cache";						// New in v1.4
		const char* cRCacheIncrementKey = "Cache Increment";					// New in v1.4
		const char* cRAutoCacheIncrementKey = "Auto Cache Increment";		// New in v1.4
		const char* cRSortCacheKey = "Sort Cache";							// New in v2.0.1
		const char* cLLoCacheKey = "Local Low Cache";						// New in v3.1b3
		const char* cLHiCacheKey = "Local High Cache";						// New in v3.1b3
		const char* cLUseHiCacheKey = "Local Use High Cache";				// New in v3.1b3
		const char* cLCacheIncrementKey = "Local Cache Increment";			// New in v3.1b3
		const char* cLAutoCacheIncrementKey = "Local Auto Cache Increment";	// New in v3.1b3
		const char* cLSortCacheKey = "Local Sort Cache";					// New in v3.1b3
		const char* cSmartAddressListKey_1_4 = "Email Aliases";				// Deprecated as of v2.0b6
		const char* cSmartAddressListKey_2_0 = "Email Aliases v2_0";		// New in v2.0b6
		const char* cAutoCheckMboxesKey = "Auto Check";						// Deprecated as of v1.4a2
		const char* cUseCopyToMboxesKey = "Use Copy To";					// New in v1.4
		const char* cUseAppendToMboxesKey = "Use Append To";				// New in v1.4
		const char* cMRUMaximumKey = "Maximum Recent Mailboxes";			// New in v1.4
		const char* cMboxACLStyleListKey = "ACL Styles";
		const char* cSearchStyleListKey = "Search Styles";					// New in v1.4
		const char*	cPromptDisconnectedKey = "Prompt Disconnected";			// New in v2.0
		const char*	cAutoDialKey = "Auto Dial";								// New in v2.0
		const char* cUIDValidityMismatchKey = "UIDValidity Mismatch Warn";	// New in v2.0
		const char* cQuickSearchKey = "Quick Search";						// New in v4.0
		const char* cQuickSearchVisibleKey = "Quick Search Visible";		// New in v4.0
		const char* cScrollForUnseenKey = "Scroll for Unseen";				// New in v4.0.2

	const char* cFavouritesSection = "Cabinets";							// This section new in v1.4
		const char* cFavouritesKey_1_4 = "Cabinet List";
		const char* cFavouritesKey_2_0 = "Cabinet List v2_0";				// New format in v2.0
		const char* cMRUCopyToKey_1_4 = "Recent Copy To";					// Deprecated as of v2.0b6
		const char* cMRUCopyToKey_2_0 = "Recent Copy To v2_0";				// New in v2.0b6
		const char* cMRUAppendToKey_1_4 = "Recent Append To";				// Deprecated as of v2.0b6
		const char* cMRUAppendToKey_2_0 = "Recent Append To v2_0";			// New in v2.0b6
		const char* cMailboxAliasesKey = "Mailbox Aliases";					// New in v3.0a4

	const char* cSearchSection = "Search";									// This section new in v1.4
		const char* cMultipleSearchKey = "Simultaneous Searching";
		const char* cNumberMultipleSearchKey = "Number Simultaneous";
		const char* cLoadBalanceSearchKey = "Load Balanced Searching";
		const char* cOpenFirstSearchKey = "Open First Search";

	const char* cRulesSection = "Rules";									// This section new in v2.1
		const char* cRulesLocalFiltersSection = "Local Filters";
		const char* cRulesSIEVEFiltersSection = "SIEVE Filters";
		const char* cRulesLocalTargetsSection = "Local Targets";
		const char* cRulesSIEVEScriptsSection = "SIEVE Scripts";
		const char* cRulesUseLog = "Use Log";

	const char* cMessageSection = "Message";
		const char* cShowMessageHeaderKey = "Show Header";
		const char* cSaveMessageHeaderKey = "Save Header";
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		const char* cSaveCreatorKey = "File Creator";
#elif __dest_os == __win32_os || __dest_os == __linux_os
		const char* cSaveCreatorKey = "File Extension";
#endif
		const char* cDeleteAfterCopyKey = "Delete after Copy";
		const char* cOpenDeletedKey = "Open Deleted Messages";
		const char* cCloseDeletedKey = "Close Deleted Messages";
		const char* cDoSizeWarnKey = "Warn on Message Size";
		const char* cWarnMessageSizeKey = "Warning Size";
		const char* cQuoteSelectionKey = "Quote Selection";
		const char* cAlwaysQuoteKey = "Always Quote";
		const char* cOptionReplyDialogKey = "Alt-btn Reply";
		const char* cShowStyled = "Show Styled";
		const char* cHeaderCaptionKey = "Header Caption";
		const char* cHeaderBoxKey = "Header Box";
		const char* cPrintSummaryKey = "Print Header Summary";
		const char* cFooterCaptionKey = "Footer Caption";
		const char* cFooterBoxKey = "Footer Box";
		const char* cOpenReuseKey = "Reuse Open Window";					// New as of v2.0a4
		const char* cAutoDigestKey = "Auto Digest View";					// New as of v2.0a7
		const char* cExpandHeaderKey = "Expand Header";						// New as of v2.1a5
		const char* cExpandPartsKey = "Expand Parts";						// New as of v2.0.5b3
		const char* cForwardChoiceKey = "Forward Choice";					// New as of v2.1a2
		const char* cForwardQuoteOriginalKey = "Forward Quote Original";	// New as of v2.1a2
		const char* cForwardHeadersKey = "Forward Headers";					// New as of v2.1a2
		const char* cForwardAttachmentKey = "Forward Attachment";			// New as of v2.1a3
		const char* cForwardRFC822Key = "Forward RFC822";					// New as of v2.1a2
		const char* cMDNOptionsKey = "MDN Send";							// New as of v3.0a2

	const char* cLetterSection = "Outgoing";
		const char* cHeaderKey = "X-Header Lines";							// Deprecated as of v1.4
		//const char* cGenerateSenderKey = "Generate Sender";				// Deprecated as of v2.0
		const char* cFooterKey = "Signature";								// Deprecated as of v1.4
		const char* cAutoInsertSigKey = "Auto Insert Signature";
		const char* cSigEmptyLineKey = "Line before Signature";				// New as of v1.4
		const char* cSigDashesKey = "Signature Dashes";						// New as of v3.1b1
		const char* cNoSubjectWarnKey = "Warn No Subject";					// New as of v1.4
		const char* cTabWidthKey = "Spaces per Tab";
		const char* cTabSpaceKey = "Use Space for Tab";						// New as of v1.4
		const char* cIncludeKey = "Quote Prefix";
		const char* cReplyStartKey = "Reply Start";							// New as of v1.4
		const char* cReplyEndKey = "Reply End";								// New as of v1.4
		const char* cReplyCursorTopKey = "Reply Caption Top";				// New as of v1.4
		const char* cReplyNoSignatureKey = "Reply No Signature";			// New as of v3.1b1
		const char* cForwardQuoteKey = "Forward Quote Prefix";
		const char* cForwardStartKey = "Forward Start";						// New as of v1.4
		const char* cForwardEndKey = "Forward End";							// New as of v1.4
		const char* cForwardCursorTopKey = "Forward Caption Top";			// New as of v1.4
		const char* cForwardSubjectKey = "Forward Subject";					// New as of v3.0a4
		const char* cLtrHeaderCaptionKey = "Header Caption";				// New as of v1.4
		//const char* cLtrHeaderBoxKey = "Header Box";						// Deprecated as of v3.1b1
		const char* cLtrFooterCaptionKey = "Footer Caption";				// New as of v1.4
		//const char* cLtrFooterBoxKey = "Footer Box";						// Deprecated as of v3.1b1
		const char* cSeparateBCCKey = "Separate BCC";						// New as of v2.1a3
		const char* cBCCCaptionKey = "BCC Caption";							// New as of v2.1a3
		const char* cShowCCsKey = "Show CCs";								// New as of v3.0b7
		const char* cShowBCCsKey = "Show BCCs";								// New as of v3.0b7
		const char* cExternalEditorKey = "External Editor "OS_PART;			// New as of v3.0a4
		const char* cUseExternalEditorKey = "Use External Editor";			// New as of v3.0a4
		const char* cExternalEditAutoStartKey = "Auto External Editor";		// New as of v3.0a4
		const char* cWrapLengthKey = "Wrap Length";
		const char* cWindowWrapKey = "Wrap to Window";						// New as of v1.4
		const char* cRecordOutgoingKey = "Copy Outgoing";					// Deprecated as of v1.4
		const char* cOutMailboxKey = "Copy To";								// Deprecated as of v1.4
		//const char* cRecordAttachmentsKey = "Copy Attachments";			// Deprecated as of v4.0a6
		//const char* cRememberRecordKey = "Remember Last Copy To";			// Deprecated as of v4.0a6
		const char* cDisplayAttachmentsKey = "Display Attachments";			// New as of v4.0a6
		const char* cAppendDraftKey = "Append as Draft";					// New as of v1.4
		const char* cInboxAppendKey = "Copy INBOX Originals";
		const char* cDeleteOriginalDraftKey = "Delete Original Draft";		// New as of v2.0b7
		const char* cTemplateDraftsKey = "Template Drafts";					// New as of v2.0b7
		const char* cSmartURLPasteKey = "Smart URL Paste";					// New as of v3.0.1
		const char* cWarnReplySubjectKey = "Warn Reply Subject";			// New as of v3.1b7
		const char* cSaveOptions = "Save To";								// New as of v2.1a2
		const char* cSaveMailbox = "Save To Mailbox";						// New as of v2.1a2
		const char* cComposeAs = "Compose As";
		const char* cEnrMultiAltPlain = "Enriched Alt Plain";
		const char* cHTMLMultiAltEnriched = "HTML Alt Enriched";
		const char* cEnrMultiAltHTML = "Enriched Alt HTML";
		const char* cHTMLMultiAltPlain = "HTML Alt Plain";
		const char* cFormatFlowed = "Format Flowed";						// New as of v2.0b5
		const char* cAlwaysUnicode = "Always Unicode";						// New as of v4.0b2
		const char* cDisplayIdentityFrom = "Display Identity From";			// New as of v4.0b2
		const char* cAutoSaveDraftsKey = "Auto Save Drafts";				// New as of v4.0.3
		const char* cAutoSaveDraftsIntervalKey = "Auto Save Interval";		// New as of v4.0.3

	const char* cSecuritySection = "Security";								// This section new as of v2.0
		const char* cPreferredPluginKey = "Preferred Plugin";				// New as of v3.1b3
		const char* cUseMIMESecurityKey = "Use MIME Security";
		const char* cEncryptToSelfKey = "Encrypt to Self";
		const char* cCachePassphraseKey = "Cache Passphrase";
		const char* cCachePassphraseMaxKey = "Cache Passphrase Max";		// New as of v3.1b3
		const char* cCachePassphraseIdleKey = "Cache Passphrase Idle";		// New as of v3.1b3
		const char* cAutoVerifyKey = "Auto Verify";							// New as of v2.0a6
		const char* cAutoDecryptKey = "Auto Decrypt";						// New as of v2.0a6
		const char* cWarnUnencryptedSendKey = "Warn Unencrpyted Send";		// New as of v3.1b6
		const char* cUseErrorAlertsKey = "Use Error Alerts";				// New as of v3.1b3
		const char* cCacheUserCertsKey = "Cache User Certs";				// New as of v3.1b4
		const char* cVerifyOKNotificationKey = "Verify OK Notifications";	// New in v2.0.5b3

	const char* cIdentitiesSection = "Identities";
		const char* cIdentitiesKey_1_3 = "All";								// Deprecated format as of v1.4
		const char* cIdentitiesKey_1_4 = "List";							// New format as of v1.4
		const char* cIdentitiesKey_2_0 = "List v2_0";						// New format as of v2.0
		const char* cTiedMailboxesKey = "Tied Mailboxes";					// New as of v2.0d4
		const char* cTiedCalendarsKey = "Tied Calendars";					// New as of v4.0d1
		const char* cContextTiedKey = "Context Tied";						// New as of v2.0b5
		const char* cMsgTiedKey = "Message Tied";							// New as of v3.0b7
		const char* cTiedMboxInheritKey = "Tied Mailbox Inherit";			// New as of v3.1b4

	const char* cAddressSection = "Address";
		const char* cAddressBookKey = "Default Address Books";				// Old removed in v1.3
		const char* cAddressAccountsKey_1_3 = "Address Books";				// Deprecated format as of v1.4
		const char* cAddressAccountsKey_1_4 = "Address Books v1_4";			// New format as of v1.4
		const char* cAddressAccountsKey_2_0 = "Address Books v2_0";			// New format as of v2.0
		const char* cLocalAdbkAccount = "Local Address Books";				// New format as of v2.0
		const char* cOSAdbkAccount = "OS Address Books "OS_PART;			// New format as of v3.1
		const char* cExpandedAdbksKey = "Expanded Address Books";			// New as of v4.1
		const char* cAdbkOpenAtStartup_1_3 = "Open Address Books";			// Deprecated format as of v2.0b6
		const char* cAdbkOpenAtStartup_2_0 = "Open Address Books v2_0";		// New as of v2.0b6
		const char* cAdbkNickName_1_3 = "Lookup Address Books";				// Deprecated format as of v2.0b6
		const char* cAdbkNickName_2_0 = "Lookup Address Books v2_0";		// New as of v2.0b6
		const char* cAdbkSearch_1_3 = "Search Address Books";				// Deprecated format as of v2.0b6
		const char* cAdbkSearch_2_0 = "Search Address Books v2_0";			// New as of v2.0b6
		const char* cAdbkAutoSync_1_3 = "Sync Address Books";				// New as of v2.0a2; deprecated format as of v2.0b6
		const char* cAdbkAutoSync_2_0 = "Sync Address Books v2_0";			// New as of v2.0b6
		const char* cAdbkACLStyleListKey = "Address Book ACL Styles";
		const char* cOptionAddressDialogKey = "Alt-btn Create";
		const char* cExpandNoNicknamesKey = "Expand without Nick-names";		// New as of 2.0a6
		const char* cExpandFailedNicknamesKey = "Expand failed Nick-names";		// New as of 2.0a6
		const char* cExpandFullNameKey = "Expand using Full Name";				// New as of 2.0a6
		const char* cExpandNickNameKey = "Expand using Nick-name";				// New as of 2.0a6
		const char* cExpandEmailKey = "Expand using Email";						// New as of 2.0a6
		const char* cSkipLDAPKey = "Skip LDAP";									// New as of 4.0b2
		const char* cCaptureAddressBookKey = "Capture Address Book";			// New as of 3.0b5
		const char* cCaptureAllowEditKey = "Capture Allow Edit";				// New as of 3.0b5
		const char* cCaptureAllowChoiceKey = "Capture Allow Choice";			// New as of 3.0b5
		const char* cCaptureReadKey = "Capture on Read";						// New as of 3.0b5
		const char* cCaptureRespondKey = "Capture on Response";					// New as of 3.0b5
		const char* cCaptureFromKey = "Capture From";							// New as of 3.0b5
		const char* cCaptureCcKey = "Capture Cc";								// New as of 3.0b5
		const char* cCaptureReplyToKey = "Capture Reply-to";					// New as of 3.0b5
		const char* cCaptureToKey = "Capture To";								// New as of 3.0b5

	const char* cCalendarSection = "Calendar";									// New as of 4.0d1
		const char* cCalendarAccountsKey = "Calendars";							// New as of 4.0d1
		const char* cLocalCalendarAccountKey = "Local Calendar";				// New as of 4.0d1
		const char* cWebCalendarAccountKey = "Web Calendar";					// New as of 4.0d1
		const char* cSubscribedCalendarsKey = "Subscribed Calendars";			// New as of 4.0d1
		const char* cExpandedCalendarsKey = "Expanded Calendars";				// New as of 4.0d1
		const char* cCalendarColoursKey = "Calendar Colours";					// New as of 4.0d1
		const char* cTimezoneKey = "Default Timezone";							// New as of 4.0d1
		const char* cFavouriteTimezonesKey = "Favourite Timezones"; 			// New as of 4.1a1
		const char* cWeekStartDayKey = "Week Start Day";						// New as of 4.0d1
		const char* cWorkDayMaskKey = "Workdays";								// New as of 4.0d1
		const char* cDayWeekTimeRangesKey = "Dayweek Time Range";				// New as of 4.0d1
		const char* cDisplayTimeKey = "Display Event Time";						// New as of 4.0d1
		const char* c24HourTimeKey = "Twenty-Four Hour Time";					// New as of 4.0d1
		const char* cHandleICSKey = "Handle ICS";								// New as of 4.0d1
		const char* cAutomaticIMIPKey = "Automatic iMIP";						// New as of 4.0a5
        const char* cAutomaticEDSTKey = "Automatic EDST";						// New as of 4.0.8
        const char* cShowUIDKey = "Show UID";                                   // New as of 4.1a3
		const char* cDefaultCalendarKey = "Default Calendar";					// New as of 4.0d1

	const char* cAttachmentsSection = "Attachments";
		const char* cDefaultModeKey = "Default Encoding";
		const char* cDefaultAlwaysKey = "Always use Default";
		const char* cViewDoubleClickKey = "View Double Click";					// New as of v2.1a4
		const char* cAskDownloadKey = "Ask for Download";
		const char* cDefaultDownloadKey_1_2 = "Default Download Directory";
		const char* cDefaultDownloadKey_2_0 = "Default Download "OS_PART" v2_0";
		const char* cAppLaunchKey = "Launch Application";
		const char* cLaunchTextKey = "Launch Text Parts";
		const char* cShellLaunchKey = "Shell Launch";							// New as of v1.4
		const char* cMIMEMapKey = "Mappings";
			const char* cMIMEMapFileCreatorKey = "Creator";						// Also defined in COptionsMap.cp
			const char* cMIMEMapFileTypeKey = "Type";							// Also defined in COptionsMap.cp
			const char* cMIMEMapSuffixKey = "Suffix";							// Also defined in COptionsMap.cp
			const char* cMIMEMapLaunchKey = "Launch Mode";						// Also defined in COptionsMap.cp
		const char* cExplicitMappingKey = "Always use Mappings";
		const char* cWarnMissingAttachmentsKey = "Warn Missing Attachments";		// New as of v3.1b6
		const char* cMissingAttachmentSubjectKey = "Missing Attachment Subject";	// New as of v3.1b6
		const char* cMissingAttachmentWordsKey = "Missing Attachment Words";		// New as of v3.1b6

	const char* cSpeechSection = "Speech";
		const char* cSpeakNewOpenKey = "Speak New On Open";
		const char* cSpeakNewArrivalsKey = "Speak New on Arrive";
		const char* cSpeakNewAlertKey = "Speak Alert";
		const char* cSpeakNewAlertTxtKey = "Speak Alert Text";
		const char* cSpeakMessageItemsKey = "Speak Message Items";
		const char* cSpeakLetterItemsKey = "Speak Draft Items";
		const char* cSpeakEmptyItemKey = "Speak Empty";
		const char* cSpeakMaxLengthKey = "Maximum Characters to Speak";

	const char* cMiscellaneousSection = "Miscellaneous";						// New as of v3.0
		const char* cTextMacrosKey = "Text Macros";								// New as of v3.0
		const char* cAllowKeyboardShortcutsKey = "Allow Keyboard Shortcuts";	// New as of v3.1b9

	const char* cDefaultWindowsSection = "Default Windows";
		const char* cMDIWindowKey_1_2 = "MDI";
		const char* cMDIWindowKey_2_0 = "MDI "OS_PART" v2_0";
		const char* c3PaneWindowKey = "3Pane "OS_PART;
		const char* c3PaneOptionsKey_v3_0 = "3Pane Options";
		const char* c3PaneOptionsKey_v4_0 = "3Pane Options v4_0";
		const char* c3PaneOpenMailboxesKey = "3Pane Open Mailboxes";
		const char* c3PaneOpenAdbksKey = "3Pane Open Address Books";
		const char* c1PaneOptionsKey_v3_0 = "1Pane Options";
		const char* c1PaneOptionsKey_v4_0 = "1Pane Options v4_0";
		const char* cStatusWindowKey_1_0 = "Status";
		const char* cStatusWindowKey_2_0 = "Status "OS_PART" v2_0";
		const char* cServerWindowKey_1_3 = "Server";
		const char* cServerWindowKey_1_4 = "Server v1_4";
		const char* cServerWindowKey_2_0 = "Server "OS_PART" v2_0";
		const char* cServerBrowseKey_2_0 = "Server Browse "OS_PART" v2_0";
		const char* cMailboxWindowKey_1_3 = "Mailbox";
		const char* cMailboxWindowKey_1_4 = "Mailbox v1_4";
		const char* cMailboxWindowKey_2_0 = "Mailbox "OS_PART" v2_0";
		const char* cSMTPWindowKey = "SMTP "OS_PART;
		const char* cMessageWindowKey_1_0 = "Message";
		const char* cMessageWindowKey_2_0 = "Message "OS_PART" v2_0";
		const char* cMessageView3PaneKey_2_0 = "Preview 3Pane "OS_PART;
		const char* cMessageView1PaneKey_2_0 = "Preview 1Pane "OS_PART;
		const char* cLetterWindowKey_1_0 = "Draft";
		const char* cLetterWindowKey_2_0 = "Draft "OS_PART" v2_0";
		const char* cAddressBookWindowKey_1_0 = "Address Book";
		const char* cAddressBookWindowKey_2_0 = "Address Book "OS_PART" v2_0";
		const char* cAddressBookWindowKey_2_1 = "Address Book "OS_PART" v2_1";
		const char* cAdbkManagerWindowKey_1_3 = "Address Book Manager";
		const char* cAdbkManagerWindowKey_1_4 = "Address Book Manager v1_4";
		const char* cAdbkManagerWindowKey_2_0 = "Address Book Manager "OS_PART" v2_0";
		const char* cAdbkSearchWindowKey_1_3 = "Address Search";
		const char* cAdbkSearchWindowKey_1_4 = "Address Search v1_4";
		const char* cAdbkSearchWindowKey_2_0 = "Address Search "OS_PART" v2_0";
		const char* cFindReplaceWindowKey_1_4 = "Find & Replace";
		const char* cFindReplaceWindowKey_2_0 = "Find & Replace "OS_PART" v2_0";
		const char* cSearchWindowKey_1_4 = "Message Search";
		const char* cSearchWindowKey_2_0 = "Message Search "OS_PART" v2_0";
		const char* cRulesWindowKey_2_0 = "Rules "OS_PART" v2_0";
		const char* cRulesDialogKey_2_0 = "Rules Dialog "OS_PART" v2_0";
		const char* cCalendarStoreWindowKey_4_0 = "Calendar Store "OS_PART" v4_0";
		const char* cCalendarWindowKey_4_0 = "Calendar "OS_PART" v4_0";

	const char* cServerInfoSection_1_3 = "Server Info";
	const char* cServerInfoSection_1_4 = "Server Info v1_4";
	const char* cServerInfoSection_2_0 = "Server Info "OS_PART" v2_0";
	const char* cMailboxInfoSection_1_3 = "Mailbox Info";
	const char* cMailboxInfoSection_1_4 = "Mailbox Info v1_4";
	const char* cMailboxInfoSection_2_0 = "Mailbox Info "OS_PART" v2_0";
	const char* cAddressBookInfoSection_1_0 = "Address Book Info";
	const char* cAddressBookInfoSection_2_0 = "Address Book Info "OS_PART" v2_0";
	const char* cAddressBookInfoSection_2_1 = "Address Book Info "OS_PART" v2_1";
	const char* cAdbkManagerInfoSection_1_3 = "Address Book Manager Info";
	const char* cAdbkSearchInfoSection_1_3 = "Address Search Info";
	const char* cCalendarInfoSection_4_0 = "Calendar Info "OS_PART" v4_0";

	const char* cPluginsSection = "Plugins";

		const char* cPluginAddressIO = "Address IO";

		const char* cPluginMailboxIO = "Mailbox IO";

		const char* cPluginNetworkAuthentication = "Network Authentication";

		const char* cPluginSecurity = "Security";

		const char* cPluginSpelling = "Spelling";
/* These are for Spellswell - we no longer use them
			const char* cUncapAfterPeriodKey = "Uncap After Period";
			const char* cUncapNounKey = "Uncap Noun";
			const char* cRepeatedWordsKey = "Repeated Words";
			const char* cOneSpaceAfterPeriodKey = "One Space After Period";
			const char* cSpaceBeforePunctKey = "Space Before Punct";
			const char* cTwoSpaceKey = "Two Spaces";
			const char* cAVowelKey = "A Vowel";
			const char* cSeparateHyphenKey = "Separate Hyphen";
			const char* cNumbersLettersKey = "Numbers Letters";
			const char* cViewSuggestionsKey = "View Suggestions";
			const char* cAutoPositionDialogKey = "Auto Position Dialog";
			const char* cSpellOnSendKey = "Spell On Send";
			const char* cDefaultDictionaryKey = "Default Dictionary Mac OS";
			const char* cDefaultDictionaryKey_2_0 = "Default Dictionary "OS_PART;
*/
			const char* cIgnoreCapitalisedKey = "Ignore Capitalised";
			const char* cIgnoreAllCapsKey = "Ignore AllCaps";
			const char* cIgnoreWordsNumbersKey = "Ignore Words Numbers";
			const char* cIgnoreMixedCaseKey = "Ignore Mixed Case";
			const char* cIgnoreDomainNamesKey = "Ignore Domains";
			const char* cReportDoubledWordsKey = "Report Doubles";
			const char* cCaseSensitiveKey = "Case Sensitive";
			const char* cPhoneticSuggestionsKey = "Ponetic Suggestions";
			const char* cTypoSuggestionsKey = "Typographical Suggestions";
			const char* cSuggestSplitKey = "Suggest Split";
			const char* cAutoCorrectKey = "Auto Correct";
			const char* cSuggestLevelKey = "Suggestion Level";
			const char* cDictionaryNameKey = "Dictionary Name";
			const char* cAutoPositionDialogKey = "Auto Position Dialog";
			const char* cSpellOnSendKey = "Spell On Send";
			const char* cSpellAsYouTypeKey = "Spell As You Type";
			const char* cSpellColourBackground = "Spell Colour Background";
			const char* cSpellBackgroundColour = "Spell Background Colour";

			const char* cPositionDialogKey = "Check Dialog Info";
			const char* cPositionDialogKey_2_0 = "Check Dialog Info v2_0";			// New format in v2.0

		const char* cPluginPswdChange = "Password Change";
			const char* cPswdChangeEnabledKey = "Enabled";
			const char* cPswdChangeConfigKey = "Configuration";

		const char* cPluginMailMerge = "MailMerge";
