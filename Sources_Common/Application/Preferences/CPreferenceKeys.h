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


// CPreferencesKeys.h

// Definition of preference keys

#ifndef __CPREFERENCEKEYS__MULBERRY__
#define __CPREFERENCEKEYS__MULBERRY__

extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

extern const char* cVendor;
extern const char* cProduct;
//extern const char* cVendorSection;
//extern const char* cPrefsSetSection;
extern const char* cPrefsSetListKey;
extern const char* cDefaultPrefsSetKey_1_3;
extern const char* cDefaultPrefsSetKey_2_0;

extern const char* cOSKey;											// New in v2.0

extern const char* cAdminSection;
	extern const char*	cVersionKey;
	extern const char*	cVersionKey_v1_4;							// New in v1.4
	extern const char*	cVersionKey_v2_0;							// New in v2.0
	extern const char*	cUseMultiUserKey;
	extern const char*	cAllowDefaultKey;
	extern const char*	cNoRemotePrefsKey;
	extern const char*	cNoLocalPrefsKey;
	extern const char*	cNoDisconnectKey;							// New in v2.0
	extern const char*	cNoSMTPQueuesKey;							// New in v2.0
	extern const char*	cNoLocalMboxKey;							// New in v2.0
	extern const char*	cNoLocalAdbksKey;							// New in v1.4
	extern const char*	cNoLocalDraftsKey;							// New in v1.4
	extern const char*	cNoLocalCalendarsKey;						// New in v4.0
	extern const char*	cNoAttachmentsKey;							// New in v2.1.1
	extern const char*	cNoRulesKey;								// New in v2.1.1
	extern const char*	cLockVirtualDomainKey;						// New in v3.0.2
	extern const char*	cLockServerAddressKey;
	extern const char*	cLockServerDomainKey;
	extern const char*	cNoLockSMTPKey;								// New in v2.2
	extern const char*	cLockedDomainKey;
	extern const char*	cLockSMTPSendKey;
	extern const char*	cLockReturnAddressKey;
	extern const char*	cLockPersonalICKey;							// New in v1.4
	extern const char*	cLockMDNKey;								// New in v3.0
	extern const char*	cAskRealNameKey;							// New in v1.4
	extern const char*	cGenerateAuthKey;							// Deprecated as of v1.4
	extern const char*	cGenerateSenderKey;							// New in v2.0.6
	extern const char*	cAllowXHeadersKey;
	extern const char*	cLockIdentityFromKey;
	extern const char*	cLockIdentityReplyToKey;
	extern const char*	cLockIdentitySenderKey;
	extern const char*	cLockSavePswdKey;
	extern const char*	cPromptLogoutQuitKey;						// New in v1.3.4.1
	extern const char*	cNoQuitKey;									// New in v1.4.2
	extern const char*	cAllowStyledCompositionKey;
	extern const char*	cAllowRejectCommandKey;						// New in v3.1
	extern const char*	cAutoCreateMailboxKey;
	extern const char*	cAutoCreateMailboxesKey;					// New in v3.1
	extern const char*	cSubcribeAutoCreateKey;						// New in v3.1

	extern const char* cAdminOriginatorSection;						// This section new in v1.4
		extern const char*	cAdminUseOriginatorKey;
		extern const char*	cAdminOriginatorUseTokenKey;
		extern const char*	cAdminOriginatorTokenHashKey;
		extern const char*	cAdminOriginatorUseTokenAuthorityKey;
		extern const char*	cAdminOriginatorTokenAuthorityKey;
		extern const char*	cAdminOriginatorTokenAuthorityEncryptKey;	// Key used to encrypt token authority value in admin prefs

	extern const char* cAdminAttachmentLimitsSection;				// This section new in v1.4
		extern const char*	cAdminAttachmentWarningsSection;
		extern const char*	cAdminAttachmentPreventionSection;
			extern const char*	cAdminAttachmentLimitAddressKey;
			extern const char*	cAdminAttachmentLimitSizeKey;
			extern const char*	cAdminAttachmentLimitProductKey;

	extern const char*	cAdminPreventDownloadKey;					// New in v1.4

	extern const char*	cAdminAllow3PaneChoiceKey;					// New in v3.0
	extern const char*	cAdminForce3PaneKey;						// New in v3.0

	extern const char*	cAdminPreventCalendarsKey;					// New in v4.0.1

	extern const char*	cAdminMulberryURLKey;						// New in v3.1.1
	extern const char*	cAdminMulberryFAQKey;						// New in v3.1.1
	extern const char*	cAdminUpdateURLKey;							// New in v3.1.1
	extern const char*	cAdminMulberrySupportAddressKey;			// New in v3.1.1

	extern const char*	cAdminAllowLoggingKey;						// New in v2.0b5
	extern const char*	cAdminSSLStateDisplayKey;					// New in v3.1.6

	//extern const char*	cAdminUpgradeAuthenticatorKey;			// New in v1.4
	//extern const char*	cAdminUpgradeAuthenticatorTypeKey;		// New in v1.4
	extern const char*	cAdminUpgradeKerberosKey;					// New in v3.1.1

	extern const char*	cAdminUsePASSDKey;							// New in v1.4.2

extern const char* cPrefsSection;
	//extern const char*	cVersionKey;
	extern const char*	cHelpStateKey;
	extern const char*	cDetailedTooltipsKey;						// New in v3.0a2
	extern const char*	cUseICKey;
	extern const char*	cCheckDefaultMailClientKey;					// New in v3.0a3
	extern const char*	cCheckDefaultWebcalClientKey;				// New in v4.0d1
	extern const char*	cWarnMailtoFilesKey;						// New in v3.1.1
	extern const char*	cSimpleKey;									// New in v1.4
	extern const char*	cLastPanelKey;								// New in v2.0b5
	extern const char*	cDisconnectedKey;							// New in v2.0
	extern const char*	cSMTPQueueEnabledKey;						// New in v2.0
	extern const char*	cConnectOptionsKey;							// New in v2.0
	extern const char*	cDisconnectOptionsKey;						// New in v2.0

	extern const char* cNetworkControlSection;						// This section new in v3.0b7
		extern const char* cAppIdleTimeoutKey;						// New in v3.0b7
		extern const char* cWaitDialogStartKey;						// New in v3.0b7
		extern const char* cWaitDialogMaxKey;						// New in v3.0b7
		extern const char* cConnectRetryTimeoutKey;					// New in v3.0b7
		extern const char* cConnectRetryMaxCountKey;				// New in v3.0b7
		extern const char* cBackgroundTimeoutKey;					// New in v3.0b7
		extern const char* cTickleIntervalKey;						// New in v3.0b7
		extern const char* cTickleIntervalExpireTimeoutKey;			// New in v3.0b7
		extern const char* cSleepReconnectKey;						// New in v4.0b3

	extern const char* cAccountSection;								// This section new in v1.4
		extern const char* cMailAccountsKey_1_4;
		extern const char* cMailAccountsKey_2_0;					// Format change in v2.0
		extern const char* cSMTPAccountsKey_1_4;
		extern const char* cSMTPAccountsKey_2_0;					// Format change in v2.0
		extern const char* cSIEVEAccountsKey;						// New in v3.0.1
		extern const char* cMailDomainKey;
		extern const char* cOSDefaultLocationKey;					// New in 2.0a6
		extern const char* cDisconnectedCWDKey;						// New in 2.0a6

	extern const char* cNetworkSection;								// This entire section is deprecated as of v1.4a2
		extern const char* cIMAPServerIPKey;
		extern const char* cSMTPServerIPKey;
		extern const char* cUserNameKey;
		extern const char* cSMTPuidKey;
		extern const char* cIMAPuidKey;
		extern const char* cPasswordKey;
		extern const char* cSaveUserKey;
		extern const char* cSavePswdKey;
		extern const char* cDirDelimKey;
		extern const char* cWDsKey;
		extern const char* cLogonStartupKey;
		extern const char* cDoCheckKey;
		extern const char* cCheckIntervalKey;
		extern const char* cNewMailAlertKey;

	extern const char* cRemoteSection;
		extern const char* cRemoteAccountsKey_1_4;	// New in v1.4
		extern const char* cRemoteAccountsKey_2_0;	// New format in v2.0
		extern const char* cUseRemoteKey;			// Deprecated as of v1.4
		extern const char* cRemoteIPKey;			// Deprecated as of v1.4
		extern const char* cRemoteUIDKey;			// Deprecated as of v1.4
		extern const char* cRemotePswdKey;			// Deprecated as of v1.4
		extern const char* cRemoteSaveUserKey;		// Deprecated as of v1.4
		extern const char* cRemoteSavePswdKey;		// Deprecated as of v1.4
		extern const char* cRemoteCachePswdKey;

	extern const char* cNotificationSection_1_4;		// This section deprecated as of v2.0
	extern const char* cNotificationSection_2_0;		// This section new in v2.0
		extern const char* cMailNotificationKey;
		extern const char* cAttachmentNotificationKey;

	extern const char* cDisplaySection;
		extern const char* cServerOpenStyleKey;			// New in v1.4
		extern const char* cServerClosedStyleKey;		// New in v1.4
		extern const char* cServerBkgndStyleKey;		// New in v1.4
		extern const char* cFavouriteBkgndStyleKey;		// New in v1.4
		extern const char* cHierarchyBkgndStyleKey;		// New in v1.4
		extern const char* cMboxRecentStyleKey;			// New in v1.4
		extern const char* cMboxUnseenStyleKey;			// New in v1.4
		extern const char* cMboxOpenStyleKey;			// New in v1.4
		extern const char* cMboxFavouriteStyleKey;		// New in v1.4
		extern const char* cMboxClosedStyleKey;			// New in v1.4
		extern const char* cIgnoreRecentStyleKey;		// New in v3.0a2
		extern const char* cUnseenStyleKey;
		extern const char* cSeenStyleKey;
		extern const char* cAnsweredStyleKey;
		extern const char* cImportantStyleKey;
		extern const char* cDeletedStyleKey;
		extern const char* cMultiAddressKey;
		extern const char* cMatchedStyleKey;			// New in v1.4
		extern const char* cNonMatchedStyleKey;			// New in v1.4
		extern const char* cUseLocalTimezoneKey;		// New in v3.0a2
		extern const char* cLabelsKey;					// New in v3.1b1
		extern const char* cIMAPLabelsKey;				// New in v4.0.1

#if __dest_os == __win32_os
		extern const char* cUseSDIKey;					// New in v1.4
		extern const char* cMultiTaskbarKey;			// New in v1.4
#endif
		extern const char* cUse3PaneKey;				// New in v3.0d1
		extern const char* cToolbarShowKey;				// New in v3.0a2
		extern const char* cToolbarSmallIconsKey;		// New in v3.0d1
		extern const char* cToolbarShowIconsKey;		// New in v3.0d1
		extern const char* cToolbarShowCaptionsKey;		// New in v3.0d1
		extern const char* cToolbarsKey;				// New in v3.2a1

	extern const char *cFormattingSection;				// New in v1.3
		extern const char* cURLStyleKey;
		extern const char* cURLSeenStyleKey;			// New in v2.0a6
		extern const char* cHeaderStyleKey;
		extern const char* cQuotationStyleKey;
		extern const char* cTagStyleKey;
		extern const char* cUseMultipleQuotesKey;		// New in v1.4.2
		extern const char* cQuoteColoursKey;			// New in v1.4.2
		extern const char* cRecognizeQuotesKey_1_4;		// New in v1.4.2; deprecated format as of v2.0b6
		extern const char* cRecognizeQuotesKey_2_0;		// New in v2.0b6
		extern const char* cRecognizeURLsKey_1_4;		// Deprecated format as of v2.0b6
		extern const char* cRecognizeURLsKey_2_0;		// New in v2.0b6
#if __dest_os == __linux_os
		extern const char* cSelectionColourKey;			// New in v2.0.6b3
#endif

	extern const char *cFontsSection;					// This section new in v2.0
		extern const char* cFontKey;					// Old (removed in v1.1.1a3)
		extern const char* cListFontKey;
		extern const char* cDisplayFontKey;
		extern const char* cPrintFontKey;				// New in v1.4
		extern const char* cCaptionFontKey;				// New in v1.4
		extern const char* cFixedFontKey;
		extern const char* cHTMLFontKey;
		extern const char* cAntiAliasFontKey;			// New in v3.1b8
		extern const char* cUseStylesKey;				// New in v3.1.3
		extern const char* cMinimumFontKey;				// New in v3.1.3

	extern const char* cMailboxSection;
		extern const char* cOpenAtFirstKey;
		extern const char* cOpenAtLastKey;
		extern const char* cOpenAtFirstNewKey;
		extern const char* cNoOpenPreviewKey;			// New in v3.1b8
		extern const char* cUnseenNewKey;				// New in v1.4
		extern const char* cRecentNewKey;				// New in v1.4
		extern const char* cNextIsNewestKey;			// New in v3.0b7
		extern const char* cExpungeOnCloseKey;
		extern const char* cWarnOnExpungeKey;
		extern const char* cWarnPuntUnseenKey;			// New in v3.1b9
		extern const char* cDoRolloverKey;				// New in v1.4
		extern const char* cRolloverWarnKey;			// New in v1.4
		extern const char* cDoMailboxClearKey;
		extern const char* cClearMailboxKey_1_3;		// Deprecated format as of v1.4
		extern const char* cClearMailboxKey_1_4;		// New format in v1.4.2
		extern const char* cClearWarningKey;
		extern const char* cPartialDisplayKey;			// Deprecated as of v1.4
		extern const char* cMsgBlocksKey;				// Deprecated as of v1.4
		extern const char* cRLoCacheKey;				// New in v1.4
		extern const char* cRHiCacheKey;				// New in v1.4
		extern const char* cRUseHiCacheKey;				// New in v1.4
		extern const char* cRCacheIncrementKey;			// New in v1.4
		extern const char* cRAutoCacheIncrementKey;		// New in v1.4
		extern const char* cRSortCacheKey;				// New in v2.0.1
		extern const char* cLLoCacheKey;				// New in v3.1b3
		extern const char* cLHiCacheKey;				// New in v3.1b3
		extern const char* cLUseHiCacheKey;				// New in v3.1b3
		extern const char* cLCacheIncrementKey;			// New in v3.1b3
		extern const char* cLAutoCacheIncrementKey;		// New in v3.1b3
		extern const char* cLSortCacheKey;				// New in v3.1b3
		extern const char* cSmartAddressListKey_1_4;	// Deprecated as of v2.0b6
		extern const char* cSmartAddressListKey_2_0;	// New in v2.0b6
		extern const char* cAutoCheckMboxesKey;			// Deprecated as of v1.4a2
		extern const char* cUseCopyToMboxesKey;			// New in v1.4
		extern const char* cUseAppendToMboxesKey;		// New in v1.4
		extern const char* cMRUMaximumKey;				// New in v1.4
		extern const char* cMboxACLStyleListKey;
		extern const char* cSearchStyleListKey;			// New in v1.4
		extern const char* cPromptDisconnectedKey;		// New in v2.0
		extern const char* cAutoDialKey;				// New in v2.0
		extern const char* cUIDValidityMismatchKey;		// New in v2.0
		extern const char* cQuickSearchKey;				// New in v4.0
		extern const char* cQuickSearchVisibleKey;		// New in v4.0
		extern const char* cScrollForUnseenKey;			// New in v4.0.2

	extern const char* cFavouritesSection;				// This section new in v1.4
		extern const char* cFavouritesKey_1_4;
		extern const char* cFavouritesKey_2_0;			// New format in v2.0
		extern const char* cMRUCopyToKey_1_4;			// Deprecated as of v2.0b6
		extern const char* cMRUCopyToKey_2_0;			// New in v2.0b6
		extern const char* cMRUAppendToKey_1_4;			// Deprecated as of v2.0b6
		extern const char* cMRUAppendToKey_2_0;			// New in v2.0b6
		extern const char* cMailboxAliasesKey;			// New in v3.0a4

	extern const char* cSearchSection;					// This section new in v1.4
		extern const char* cMultipleSearchKey;
		extern const char* cNumberMultipleSearchKey;
		extern const char* cLoadBalanceSearchKey;
		extern const char* cOpenFirstSearchKey;

	extern const char* cRulesSection;					// This section new in v2.1
		extern const char* cRulesLocalFiltersSection;
		extern const char* cRulesSIEVEFiltersSection;
		extern const char* cRulesLocalTargetsSection;
		extern const char* cRulesSIEVEScriptsSection;
		extern const char* cRulesUseLog;

	extern const char* cMessageSection;
		extern const char* cShowMessageHeaderKey;
		extern const char* cSaveMessageHeaderKey;
		extern const char* cSaveCreatorKey;
		extern const char* cDeleteAfterCopyKey;
		extern const char* cOpenDeletedKey;
		extern const char* cCloseDeletedKey;
		extern const char* cDoSizeWarnKey;
		extern const char* cWarnMessageSizeKey;
		extern const char* cQuoteSelectionKey;
		extern const char* cAlwaysQuoteKey;
		extern const char* cOptionReplyDialogKey;
		extern const char* cShowStyled;
		extern const char* cHeaderCaptionKey;			// New as of v1.4
		extern const char* cHeaderBoxKey;				// New as of v1.4
		extern const char* cPrintSummaryKey;			// New as of v3.1b1
		extern const char* cFooterCaptionKey;			// New as of v1.4
		extern const char* cFooterBoxKey;				// New as of v1.4
		extern const char* cOpenReuseKey;				// New as of v2.0a4
		extern const char* cAutoDigestKey;				// New as of v2.0a7
		extern const char* cExpandHeaderKey;			// New as of v2.1a5
		extern const char* cExpandPartsKey;				// New as of v2.0.5b3
		extern const char* cForwardChoiceKey;			// New as of v2.1a2
		extern const char* cForwardQuoteOriginalKey;	// New as of v2.1a2
		extern const char* cForwardHeadersKey;			// New as of v2.1a2
		extern const char* cForwardAttachmentKey;		// New as of v2.1a3
		extern const char* cForwardRFC822Key;			// New as of v2.1a2
		extern const char* cMDNOptionsKey;				// New as of v3.0a2

	extern const char* cLetterSection;
		extern const char* cHeaderKey;					// Deprecated as of v1.4
		//extern const char* cGenerateSenderKey;		// Deprecated as of v2.0
		extern const char* cFooterKey;					// Deprecated as of v1.4
		extern const char* cAutoInsertSigKey;
		extern const char* cSigEmptyLineKey;			// New as of v1.4
		extern const char* cSigDashesKey;				// New as of v3.1b1
		extern const char* cNoSubjectWarnKey;			// New as of v1.4
		extern const char* cTabWidthKey;
		extern const char* cTabSpaceKey;				// New as of v1.4
		extern const char* cIncludeKey;
		extern const char* cReplyStartKey;				// New as of v1.4
		extern const char* cReplyEndKey;				// New as of v1.4
		extern const char* cReplyCursorTopKey;			// New as of v1.4
		extern const char* cReplyNoSignatureKey;		// New as of v3.1b1
		extern const char* cForwardQuoteKey;
		extern const char* cForwardStartKey;			// New as of v1.4
		extern const char* cForwardEndKey;				// New as of v1.4
		extern const char* cForwardCursorTopKey;		// New as of v1.4
		extern const char* cForwardSubjectKey;			// New as of v3.0a4
		extern const char* cLtrHeaderCaptionKey;		// New as of v1.4
		//extern const char* cLtrHeaderBoxKey;			// Deprecated as of v3.1b1
		extern const char* cLtrFooterCaptionKey;		// New as of v1.4
		//extern const char* cLtrFooterBoxKey;			// Deprecated as of v3.1b1
		extern const char* cSeparateBCCKey;				// New as of v2.1a3
		extern const char* cBCCCaptionKey;				// New as of v2.1a3
		extern const char* cShowCCsKey;					// New as of v3.0b7
		extern const char* cShowBCCsKey;				// New as of v3.0b7
		extern const char* cExternalEditorKey;			// New as of v3.0a4
		extern const char* cUseExternalEditorKey;		// New as of v3.0a4
		extern const char* cExternalEditAutoStartKey;	// New as of v3.0a4
		extern const char* cWrapLengthKey;
		extern const char* cWindowWrapKey;				// New as of v1.4
		extern const char* cRecordOutgoingKey;			// Deprecated as of v1.4
		extern const char* cOutMailboxKey;				// Deprecated as of v1.4
		//extern const char* cRecordAttachmentsKey;		// Deprecated as of v4.0a6
		//extern const char* cRememberRecordKey;		// Deprecated as of v4.0a6
		extern const char* cDisplayAttachmentsKey;		// New as of v4.0a6
		extern const char* cAppendDraftKey;				// New as of v1.4
		extern const char* cInboxAppendKey;
		extern const char* cDeleteOriginalDraftKey;		// New as of v2.0b7
		extern const char* cTemplateDraftsKey;			// New as of v2.0b7
		extern const char* cSmartURLPasteKey;			// New as of v3.0.1
		extern const char* cWarnReplySubjectKey;		// New as of v3.1b7
		extern const char* cSaveOptions;				// New as of v2.1a2
		extern const char* cSaveMailbox;				// New as of v2.1a2
		extern const char* cComposeAs;
		extern const char* cEnrMultiAltPlain;
		extern const char* cHTMLMultiAltEnriched;
		extern const char* cEnrMultiAltHTML;
		extern const char* cHTMLMultiAltPlain;		
		extern const char* cFormatFlowed;				// New as of v2.0b5
		extern const char* cAlwaysUnicode;				// New as of v4.0b2
		extern const char* cDisplayIdentityFrom;		// New as of v4.0b2
		extern const char* cAutoSaveDraftsKey;			// New as of v4.0.3
		extern const char* cAutoSaveDraftsIntervalKey;	// New as of v4.0.3
		
	extern const char* cSecuritySection;				// This section new as of v2.0
		extern const char* cPreferredPluginKey;			// New as of v3.1b3
		extern const char* cUseMIMESecurityKey;
		extern const char* cEncryptToSelfKey;
		extern const char* cCachePassphraseKey;
		extern const char* cCachePassphraseMaxKey;		// New as of v3.1b3
		extern const char* cCachePassphraseIdleKey;		// New as of v3.1b3
		extern const char* cAutoVerifyKey;				// New as of v2.0a6
		extern const char* cAutoDecryptKey;				// New as of v2.0a6
		extern const char* cWarnUnencryptedSendKey;		// New as of v3.1b6
		extern const char* cUseErrorAlertsKey;			// New as of v3.1b3
		extern const char* cCacheUserCertsKey;			// New as of v3.1b4
		extern const char* cVerifyOKNotificationKey;	// New in v2.0.5b3
		
	extern const char* cIdentitiesSection;
		extern const char* cIdentitiesKey_1_3;			// Deprecated format as of v1.4
		extern const char* cIdentitiesKey_1_4;			// New format as of v1.4
		extern const char* cIdentitiesKey_2_0;			// New format as of v2.0
		extern const char* cTiedMailboxesKey;			// New as of v2.0d4
		extern const char* cTiedCalendarsKey;			// New as of v4.0d1
		extern const char* cContextTiedKey;				// New as of v2.0b5
		extern const char* cMsgTiedKey;					// New as of v3.0b7
		extern const char* cTiedMboxInheritKey;			// New as of v3.1b4

	extern const char* cAddressSection;
		extern const char* cAddressBookKey;				// Old removed in v1.3
		extern const char* cAddressAccountsKey_1_3;		// Deprecated format as of v1.4
		extern const char* cAddressAccountsKey_1_4;		// New format as of v1.4
		extern const char* cAddressAccountsKey_2_0;		// New format as of v2.0
		extern const char* cLocalAdbkAccount;			// New as of v2.0
		extern const char* cExpandedAdbksKey;			// New as of v4.1
		extern const char* cOSAdbkAccount;				// New as of v3.1b1
		extern const char* cAdbkOpenAtStartup_1_3;		// Deprecated format as of v2.0b6
		extern const char* cAdbkOpenAtStartup_2_0;		// New as of v2.0b6
		extern const char* cAdbkNickName_1_3;			// Deprecated format as of v2.0b6
		extern const char* cAdbkNickName_2_0;			// New as of v2.0b6
		extern const char* cAdbkSearch_1_3;				// Deprecated format as of v2.0b6
		extern const char* cAdbkSearch_2_0;				// New as of v2.0b6
		extern const char* cAdbkAutoSync_1_3;			// New as of v2.0a2; deprecated format as of v2.0b6
		extern const char* cAdbkAutoSync_2_0;			// New as of v2.0b6
		extern const char* cAdbkACLStyleListKey;
		extern const char* cOptionAddressDialogKey;
		extern const char* cExpandNoNicknamesKey;		// New as of 2.0a6
		extern const char* cExpandFailedNicknamesKey;	// New as of 2.0a6
		extern const char* cExpandFullNameKey;			// New as of 2.0a6
		extern const char* cExpandNickNameKey;			// New as of 2.0a6
		extern const char* cExpandEmailKey;				// New as of 2.0a6
		extern const char* cSkipLDAPKey;				// New as of 4.0b2
		extern const char* cCaptureAddressBookKey;		// New as of 3.0b5
		extern const char* cCaptureAllowEditKey;		// New as of 3.0b5
		extern const char* cCaptureAllowChoiceKey;		// New as of 3.0b5
		extern const char* cCaptureReadKey;				// New as of 3.0b5
		extern const char* cCaptureRespondKey;			// New as of 3.0b5
		extern const char* cCaptureFromKey;				// New as of 3.0b5
		extern const char* cCaptureCcKey;				// New as of 3.0b5
		extern const char* cCaptureReplyToKey;			// New as of 3.0b5
		extern const char* cCaptureToKey;				// New as of 3.0b5

	extern const char* cCalendarSection;				// New as of 4.0d1
		extern const char* cCalendarAccountsKey;		// New as of 4.0d1
		extern const char* cLocalCalendarAccountKey;	// New as of 4.0d1
		extern const char* cWebCalendarAccountKey;		// New as of 4.0d1
		extern const char* cSubscribedCalendarsKey;		// New as of 4.0d1
		extern const char* cExpandedCalendarsKey;		// New as of 4.0d1
		extern const char* cCalendarColoursKey;			// New as of 4.0d1
		extern const char* cTimezoneKey;				// New as of 4.0d1
		extern const char* cFavouriteTimezonesKey;		// New as of 4.1a1
		extern const char* cWeekStartDayKey;			// New as of 4.0d1
		extern const char* cWorkDayMaskKey;				// New as of 4.0d1
		extern const char* cDayWeekTimeRangesKey;		// New as of 4.0d1
		extern const char* cDisplayTimeKey;				// New as of 4.0d1
		extern const char* c24HourTimeKey;				// New as of 4.0d1
		extern const char* cHandleICSKey;				// New as of 4.0d1
		extern const char* cAutomaticIMIPKey;			// New as of 4.0a5
        extern const char* cAutomaticEDSTKey;			// New as of 4.0.8
        extern const char* cShowUIDKey;                 // New as of 4.1a3
		extern const char* cDefaultCalendarKey;			// New as of 4.0d1

	extern const char* cAttachmentsSection;
		extern const char* cDefaultModeKey;
		extern const char* cDefaultAlwaysKey;
		extern const char* cViewDoubleClickKey;
		extern const char* cAskDownloadKey;
		extern const char* cDefaultDownloadKey_1_2;		// Deprecated as of 2.0
		extern const char* cDefaultDownloadKey_2_0;		// New format as of v2.0
		extern const char* cAppLaunchKey;
		extern const char* cLaunchTextKey;
		extern const char* cShellLaunchKey;				// New as of v1.4
		extern const char* cMIMEMapKey;
			extern const char* cMIMEMapFileCreatorKey;
			extern const char* cMIMEMapFileTypeKey;
			extern const char* cMIMEMapSuffixKey;
			extern const char* cMIMEMapLaunchKey;
		extern const char* cExplicitMappingKey;
		extern const char* cWarnMissingAttachmentsKey;		// New as of v3.1b6
		extern const char* cMissingAttachmentSubjectKey;	// New as of v3.1b6
		extern const char* cMissingAttachmentWordsKey;		// New as of v3.1b6

	extern const char* cSpeechSection;
		extern const char* cSpeakNewOpenKey;
		extern const char* cSpeakNewArrivalsKey;
		extern const char* cSpeakNewAlertKey;
		extern const char* cSpeakNewAlertTxtKey;
		extern const char* cSpeakMessageItemsKey;
		extern const char* cSpeakLetterItemsKey;
		extern const char* cSpeakEmptyItemKey;
		extern const char* cSpeakMaxLengthKey;

	extern const char* cMiscellaneousSection;			// New as of v3.0
		extern const char* cTextMacrosKey;				// New as of v3.0
		extern const char* cAllowKeyboardShortcutsKey;	// New as of v3.1b9

	extern const char* cDefaultWindowsSection;
		extern const char* cMDIWindowKey_1_2;			// Deprecated format as of v2.0
		extern const char* cMDIWindowKey_2_0;			// New format as of v2.0
		extern const char* c3PaneWindowKey;				// New as of v3.0
		extern const char* c3PaneOptionsKey_v3_0;		// New as of v3.0
		extern const char* c3PaneOptionsKey_v4_0;		// Modified format as of v4.0
		extern const char* c3PaneOpenMailboxesKey;		// New as of v3.0
		extern const char* c3PaneOpenAdbksKey;			// New as of v3.0
		extern const char* c1PaneOptionsKey_v3_0;		// New as of v3.0
		extern const char* c1PaneOptionsKey_v4_0;		// Modified format as of v4.0
		extern const char* cStatusWindowKey_1_0;		// Deprecated format as of v2.0
		extern const char* cStatusWindowKey_2_0;		// New format as of v2.0
		extern const char* cServerWindowKey_1_3;		// Deprecated format as of v1.4
		extern const char* cServerWindowKey_1_4;		// Deprecated format as of v2.0
		extern const char* cServerWindowKey_2_0;		// New format as of v2.0
		extern const char* cServerBrowseKey_2_0;		// New as of v2.0
		extern const char* cMailboxWindowKey_1_3;		// Deprecated format as of v1.4
		extern const char* cMailboxWindowKey_1_4;		// Deprecated format as of v2.0
		extern const char* cMailboxWindowKey_2_0;		// New format as of v2.0
		extern const char* cSMTPWindowKey;				// New as of v2.0
		extern const char* cMessageWindowKey_1_0;		// Deprecated format as of v2.0
		extern const char* cMessageWindowKey_2_0;		// New format as of v2.0
		extern const char* cMessageView3PaneKey_2_0;	// New as of v3.0b2
		extern const char* cMessageView1PaneKey_2_0;	// New as of v3.0b2
		extern const char* cLetterWindowKey_1_0;		// Deprecated format as of v2.0
		extern const char* cLetterWindowKey_2_0;		// New format as of v2.0
		extern const char* cAddressBookWindowKey_1_0;	// Deprecated format as of v2.0
		extern const char* cAddressBookWindowKey_2_0;	// New format as of v2.
		extern const char* cAddressBookWindowKey_2_1;	// New format as of v2.1 (unix)
		extern const char* cAdbkManagerWindowKey_1_3;	// Deprecated format as of v1.4
		extern const char* cAdbkManagerWindowKey_1_4;	// Deprecated format as of v2.0
		extern const char* cAdbkManagerWindowKey_2_0;	// New format as of v2.0
		extern const char* cAdbkSearchWindowKey_1_3;	// Deprecated format as of v1.4
		extern const char* cAdbkSearchWindowKey_1_4;	// Deprecated format as of v2.0
		extern const char* cAdbkSearchWindowKey_2_0;	// New format as of v2.0
		extern const char* cFindReplaceWindowKey_1_4;	// Deprecated format as of v2.0
		extern const char* cFindReplaceWindowKey_2_0;	// New format as of v2.0
		extern const char* cSearchWindowKey_1_4;		// Deprecated format as of v2.0
		extern const char* cSearchWindowKey_2_0;		// New format as of v2.0
		extern const char* cRulesWindowKey_2_0;			// New as of v2.1
		extern const char* cRulesDialogKey_2_0;			// New as of v3.1b7
		extern const char* cCalendarStoreWindowKey_4_0;	// New as of v4.0
		extern const char* cCalendarWindowKey_4_0;		// New as of v4.0

	extern const char* cServerInfoSection_1_3;			// Deprecated format as of v1.4
	extern const char* cServerInfoSection_1_4;			// Deprecated format as of v2.0
	extern const char* cServerInfoSection_2_0;			// New format as of v2.0
	extern const char* cMailboxInfoSection_1_3;			// Deprecated format as of v1.4
	extern const char* cMailboxInfoSection_1_4;			// Deprecated format as of v2.0
	extern const char* cMailboxInfoSection_2_0;			// New format as of v2.0
	extern const char* cAddressBookInfoSection_1_0;		// Deprecated format as of v2.0
	extern const char* cAddressBookInfoSection_2_0;		// New format as of v2.0
	extern const char* cAddressBookInfoSection_2_1;		// New format as of v2.1 (unix)
	extern const char* cAdbkManagerInfoSection_1_3;		// Deprecated format as of v1.4
	extern const char* cAdbkSearchInfoSection_1_3;		// Deprecated format as of v1.4
	extern const char* cCalendarInfoSection_4_0;		// New format as of v4.0

	extern const char* cPluginsSection;
		extern const char* cPluginAddressIO;
		extern const char* cPluginMailboxIO;
		extern const char* cPluginNetworkAuthentication;
		extern const char* cPluginSecurity;
		extern const char* cPluginSpelling;
/* These are for Spellswell - we no longer use them
			extern const char* cUncapAfterPeriodKey;
			extern const char* cUncapNounKey;
			extern const char* cRepeatedWordsKey;
			extern const char* cOneSpaceAfterPeriodKey;
			extern const char* cSpaceBeforePunctKey;
			extern const char* cTwoSpaceKey;
			extern const char* cAVowelKey;
			extern const char* cSeparateHyphenKey;
			extern const char* cNumbersLettersKey;
			extern const char* cViewSuggestionsKey;
			extern const char* cAutoPositionDialogKey;
			extern const char* cSpellOnSendKey;
			extern const char* cDefaultDictionaryKey;		// Deprecated format as of v2.0
			extern const char* cDefaultDictionaryKey_2_0;	// New format as of v2.0
*/
			extern const char* cIgnoreCapitalisedKey;
			extern const char* cIgnoreAllCapsKey;
			extern const char* cIgnoreWordsNumbersKey;
			extern const char* cIgnoreMixedCaseKey;
			extern const char* cIgnoreDomainNamesKey;
			extern const char* cReportDoubledWordsKey;
			extern const char* cCaseSensitiveKey;
			extern const char* cPhoneticSuggestionsKey;
			extern const char* cTypoSuggestionsKey;
			extern const char* cSuggestSplitKey;
			extern const char* cAutoCorrectKey;
			extern const char* cSuggestLevelKey;
			extern const char* cDictionaryNameKey;
			extern const char* cAutoPositionDialogKey;
			extern const char* cSpellOnSendKey;
			extern const char* cSpellAsYouTypeKey;
			extern const char* cSpellColourBackground;
			extern const char* cSpellBackgroundColour;

			extern const char* cPositionDialogKey;
			extern const char* cPositionDialogKey_2_0;		// New format in v2.0

		extern const char* cPluginPswdChange;
			extern const char* cPswdChangeEnabledKey;
			extern const char* cPswdChangeConfigKey;
		extern const char* cPluginMailMerge;

#endif
