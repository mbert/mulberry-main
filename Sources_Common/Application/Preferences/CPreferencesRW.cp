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


// Source for CPreferences class

#include "CPreferences.h"

#include "CAdminLock.h"
#include "char_stream.h"
#include "CFilterManager.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#endif
#include "CINETCommon.h"
#include "CPreferenceKeys.h"
#include "CPreferenceVersions.h"
#include "CMailAccountManager.h"
#include "COptionsMap.h"
#ifdef __MULBERRY
#include "CPluginManager.h"
#endif
#include "CStringUtils.h"
#include "CToolbarManager.h"
#include "CURL.h"
#include "CUtils.h"

#include "CMIMESupport.h"

#if __dest_os != __linux_os
//#include <winsock.h>
#else
#include <netinet/in.h>
#include <stdio.h>
#endif
#include <algorithm>

#pragma push
#pragma global_optimizer off
#pragma optimization_level 0

// __________________________________________________________________________________________________
// C L A S S __ C P R E F E R E N C E S
// __________________________________________________________________________________________________

#pragma mark ____________________________Read/write

// Write data to a handle stream
void CPreferences::WriteToMap(COptionsMap* theMap, bool dirty_only,
								bool servers_only, bool not_windows, bool local, bool remote)
{
#define WRITETOMAP(x, y) x.WriteToMap(y, theMap, dirty_only);

	// Section is just prefs
	StMapSection section(theMap, cPrefsSection);

	// Write version to file (if required)
	if (!dirty_only || mUpdateVers)
	{
		char temp[256];
		NumVersionVariant tempv;
		tempv.parts = vers;
		::snprintf(temp, 256, "%#08x", tempv.whole);
		cdstring txt = temp;
		theMap->WriteValue(cVersionKey_v2_0, txt);										// >= v2.0d1

		// Reset version update flag
		mUpdateVers = false;
	}

	if (!servers_only)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Write help state to file
		WRITETOMAP(mHelpState, cHelpStateKey)							// >= v1.0
		WRITETOMAP(mUse_IC, cUseICKey)									// >= v1.0
#endif
		WRITETOMAP(mCheckDefaultMailClient, cCheckDefaultMailClientKey)	// >= v3.0a3
		WRITETOMAP(mCheckDefaultWebcalClient, cCheckDefaultWebcalClientKey)	// >= v4.0d1
		WRITETOMAP(mWarnMailtoFiles, cWarnMailtoFilesKey)				// >= v3.1.1
		WRITETOMAP(mDetailedTooltips, cDetailedTooltipsKey)				// >= v3.0a2

		WRITETOMAP(mSimple, cSimpleKey)									// >= v1.0
		WRITETOMAP(mLastPanel, cLastPanelKey)							// >= v2.0b5
		WRITETOMAP(mDisconnected, cDisconnectedKey)						// >= v2.0
	}

	// Write Network Control Prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cNetworkControlSection);
		
		WRITETOMAP(mAppIdleTimeout, cAppIdleTimeoutKey)								// v3.0b7
		WRITETOMAP(mWaitDialogStart, cWaitDialogStartKey)							// v3.0b7
		WRITETOMAP(mWaitDialogMax, cWaitDialogMaxKey)								// v3.0b7
		WRITETOMAP(mConnectRetryTimeout, cConnectRetryTimeoutKey)					// v3.0b7
		WRITETOMAP(mConnectRetryMaxCount, cConnectRetryMaxCountKey)					// v3.0b7
		WRITETOMAP(mBackgroundTimeout, cBackgroundTimeoutKey)						// v3.0b7
		WRITETOMAP(mTickleInterval, cTickleIntervalKey)								// v3.0b7
		WRITETOMAP(mTickleIntervalExpireTimeout, cTickleIntervalExpireTimeoutKey)	// v3.0b7
		WRITETOMAP(mSleepReconnect, cSleepReconnectKey)								// v4.0b3
	}

	// Write account prefs
	if (local || !remote)
	{
		StMapSection section(theMap, cAccountSection);

		WRITETOMAP(mMailAccounts, cMailAccountsKey_2_0)			// >= v1.4a2
		WRITETOMAP(mSMTPAccounts, cSMTPAccountsKey_2_0)			// >= v1.4a2
		WRITETOMAP(mSIEVEAccounts, cSIEVEAccountsKey)			// >= v3.0.1
		WRITETOMAP(mMailDomain, cMailDomainKey)					// >= v1.1
		WRITETOMAP(mOSDefaultLocation, cOSDefaultLocationKey)	// >= v2.0a6
		WRITETOMAP(mDisconnectedCWD, cDisconnectedCWDKey)		// >= v2.0a6
	}

	// Write remote prefs
	if (remote)
	{
		StMapSection section(theMap, cRemoteSection);

		WRITETOMAP(mRemoteAccounts, cRemoteAccountsKey_2_0)		// >= v1.4a2
		WRITETOMAP(mRemoteCachePswd, cRemoteCachePswdKey)		// >= v1.3d2
	}

	// Write notification prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cNotificationSection_2_0);

		WRITETOMAP(mMailNotification, cMailNotificationKey)					// >= v1.4a2
		WRITETOMAP(mAttachmentNotification, cAttachmentNotificationKey)		// >= v1.4a6
	}

	// Write display prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cDisplaySection);

		WRITETOMAP(mServerOpenStyle, cServerOpenStyleKey)					// >= v1.4a2
		WRITETOMAP(mServerClosedStyle, cServerClosedStyleKey)				// >= v1.4a2
		WRITETOMAP(mServerBkgndStyle, cServerBkgndStyleKey)					// >= v1.4a2
		WRITETOMAP(mFavouriteBkgndStyle, cFavouriteBkgndStyleKey)			// >= v1.4a2
		WRITETOMAP(mHierarchyBkgndStyle, cHierarchyBkgndStyleKey)			// >= v1.4a2

		WRITETOMAP(mMboxRecentStyle, cMboxRecentStyleKey)					// >= v1.4a2
		WRITETOMAP(mMboxUnseenStyle, cMboxUnseenStyleKey)					// >= v1.4a2
		WRITETOMAP(mMboxOpenStyle, cMboxOpenStyleKey)						// >= v1.4a2
		WRITETOMAP(mMboxFavouriteStyle, cMboxFavouriteStyleKey)				// >= v1.4a2
		WRITETOMAP(mMboxClosedStyle, cMboxClosedStyleKey)					// >= v1.4a2
		WRITETOMAP(mIgnoreRecent, cIgnoreRecentStyleKey)					// >= v3.0a2

		WRITETOMAP(unseen, cUnseenStyleKey)									// >= v1.1
		WRITETOMAP(seen, cSeenStyleKey)										// >= v1.1
		WRITETOMAP(answered, cAnsweredStyleKey)								// >= v1.1
		WRITETOMAP(important, cImportantStyleKey)							// >= v1.1
		WRITETOMAP(deleted, cDeletedStyleKey)								// >= v1.1
		WRITETOMAP(mMultiAddress, cMultiAddressKey)							// >= v1.2
		WRITETOMAP(mMatch, cMatchedStyleKey)								// >= v1.4a5
		WRITETOMAP(mNonMatch, cNonMatchedStyleKey)							// >= v1.4a5
		WRITETOMAP(mUseLocalTimezone, cUseLocalTimezoneKey)					// >= v3.0a3

		mLabels.GetValue().WriteToMapMulti(theMap, cLabelsKey, dirty_only);	// >= v3.1b1
		WRITETOMAP(mIMAPLabels, cIMAPLabelsKey)								// >= v4.0.1

#if __dest_os == __win32_os
		//WRITETOMAP(mUseSDI, cUseSDIKey)									// >= v1.4a7 < v3
		WRITETOMAP(mMultiTaskbar, cMultiTaskbarKey)							// >= v1.4a7
#endif
		WRITETOMAP(mUse3Pane, cUse3PaneKey)									// >= v3.0d1
		WRITETOMAP(mToolbarShow, cToolbarShowKey)							// >= v3.0a2
		WRITETOMAP(mToolbarSmallIcons, cToolbarSmallIconsKey)				// >= v3.0d1
		WRITETOMAP(mToolbarShowIcons, cToolbarShowIconsKey)					// >= v3.0d1
		WRITETOMAP(mToolbarShowCaptions, cToolbarShowCaptionsKey)			// >= v3.0d1
		WRITETOMAP(mToolbars, cToolbarsKey)									// >= v3.2a1
	}

	// Write formatting prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cFormattingSection);

		WRITETOMAP(mURLStyle, cURLStyleKey)
		WRITETOMAP(mURLSeenStyle, cURLSeenStyleKey)
		WRITETOMAP(mHeaderStyle, cHeaderStyleKey)
		WRITETOMAP(mQuotationStyle, cQuotationStyleKey)
		WRITETOMAP(mTagStyle, cTagStyleKey)

		WRITETOMAP(mUseMultipleQuotes, cUseMultipleQuotesKey)
		WRITETOMAP(mQuoteColours, cQuoteColoursKey)
		WRITETOMAP(mRecognizeQuotes, cRecognizeQuotesKey_2_0)
		WRITETOMAP(mRecognizeURLs, cRecognizeURLsKey_2_0)

#if __dest_os == __linux_os
		WRITETOMAP(mSelectionColour, cSelectionColourKey)
#endif
	}

	// Write font prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cFontsSection);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		WRITETOMAP(mListTextTraits, cListFontKey)					// >= v1.2
		WRITETOMAP(mDisplayTextTraits, cDisplayFontKey)				// >= v1.0
		WRITETOMAP(mPrintTextTraits, cPrintFontKey)					// >= v1.4a5
		WRITETOMAP(mCaptionTextTraits, cCaptionFontKey)				// >= v1.4a6
		WRITETOMAP(mFixedTextTraits, cFixedFontKey)
		WRITETOMAP(mHTMLTextTraits, cHTMLFontKey)
		WRITETOMAP(mAntiAliasFont, cAntiAliasFontKey)				// >= 3.1b8
#elif __dest_os == __win32_os || __dest_os == __linux_os
		WRITETOMAP(mListTextFontInfo, cListFontKey)					// >= v1.2
		WRITETOMAP(mDisplayTextFontInfo, cDisplayFontKey)			// >= v1.2
		WRITETOMAP(mPrintTextFontInfo, cPrintFontKey)				// >= v1.4a5
		WRITETOMAP(mCaptionTextFontInfo, cCaptionFontKey)			// >= v1.4a6
		WRITETOMAP(mFixedTextFontInfo, cFixedFontKey)
		WRITETOMAP(mHTMLTextFontInfo, cHTMLFontKey)
#else
#error __dest_os
#endif
		WRITETOMAP(mUseStyles, cUseStylesKey)						// >= 3.1.3
		WRITETOMAP(mMinimumFont, cMinimumFontKey)					// >= 3.1.3
	}

	// Write mailbox prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cMailboxSection);

		WRITETOMAP(openAtFirst, cOpenAtFirstKey)				// >= v1.1
		WRITETOMAP(openAtLast, cOpenAtLastKey)					// >= v1.1
		WRITETOMAP(openAtFirstNew, cOpenAtFirstNewKey)			// >= v1.1
		WRITETOMAP(mNoOpenPreview, cNoOpenPreviewKey)			// >= v3.1b8
		WRITETOMAP(mUnseenNew, cUnseenNewKey)					// >= v1.4a6
		WRITETOMAP(mRecentNew, cRecentNewKey)					// >= v1.4a6
		WRITETOMAP(mNextIsNewest, cNextIsNewestKey)				// >= v3.0b7
		WRITETOMAP(expungeOnClose, cExpungeOnCloseKey)			// >= v1.1
		WRITETOMAP(warnOnExpunge, cWarnOnExpungeKey)			// >= v1.1
		WRITETOMAP(mWarnPuntUnseen, cWarnPuntUnseenKey)			// >= v3.1b9
		WRITETOMAP(mDoRollover, cDoRolloverKey)					// >= v1.4b2
		WRITETOMAP(mRolloverWarn, cRolloverWarnKey)				// >= v1.4a7
		WRITETOMAP(mDoMailboxClear, cDoMailboxClearKey)			// >= v1.1
		WRITETOMAP(clear_mailbox, cClearMailboxKey_1_4)			// >= v1.4.2
		WRITETOMAP(clear_warning, cClearWarningKey)				// >= v1.1
		WRITETOMAP(mRLoCache, cRLoCacheKey)						// >= v1.4a5
		WRITETOMAP(mRHiCache, cRHiCacheKey)						// >= v1.4a5
		WRITETOMAP(mRUseHiCache, cRUseHiCacheKey)					// >= v1.4a5
		WRITETOMAP(mRCacheIncrement, cRCacheIncrementKey)			// >= v1.4a5
		WRITETOMAP(mRAutoCacheIncrement, cRAutoCacheIncrementKey)	// >= v1.4a5
		WRITETOMAP(mRSortCache, cRSortCacheKey)						// >= v1.4a5
		WRITETOMAP(mLLoCache, cLLoCacheKey)							// >= v3.1b3
		WRITETOMAP(mLHiCache, cLHiCacheKey)							// >= v3.1b3
		WRITETOMAP(mLUseHiCache, cLUseHiCacheKey)					// >= v3.1b3
		WRITETOMAP(mLCacheIncrement, cLCacheIncrementKey)			// >= v3.1b3
		WRITETOMAP(mLAutoCacheIncrement, cLAutoCacheIncrementKey)	// >= v3.1b3
		WRITETOMAP(mLSortCache, cLSortCacheKey)						// >= v3.1b3

		// Write smart addresses
		WRITETOMAP(mSmartAddressList, cSmartAddressListKey_2_0)		// >= v1.1

		// Write mbox menu favourites
		WRITETOMAP(mUseCopyToMboxes, cUseCopyToMboxesKey)			// >= v1.4a2
		WRITETOMAP(mUseAppendToMboxes, cUseAppendToMboxesKey)		// >= v1.4a2
		WRITETOMAP(mMRUMaximum, cMRUMaximumKey)						// >= v1.4a2

		// Write mailbox ACL styles
		WRITETOMAP(mMboxACLStyles, cMboxACLStyleListKey)			// >= v1.2.5d3

		// Write search styles
		WRITETOMAP(mSearchStyles, cSearchStyleListKey)				// >= v1.4a5

		// Disconnected stuff
		WRITETOMAP(mPromptDisconnected, cPromptDisconnectedKey)		// >= v2.0
		WRITETOMAP(mAutoDial, cAutoDialKey)							// >= v2.0
		WRITETOMAP(mUIDValidityMismatch, cUIDValidityMismatchKey)	// >= v2.0
		WRITETOMAP(mSMTPQueueEnabled, cSMTPQueueEnabledKey)			// >= v2.0
		WRITETOMAP(mConnectOptions, cConnectOptionsKey)				// >= v2.0
		WRITETOMAP(mDisconnectOptions, cDisconnectOptionsKey)		// >= v2.0

		WRITETOMAP(mQuickSearch, cQuickSearchKey)					// >= v4.0d1
		WRITETOMAP(mQuickSearchVisible, cQuickSearchVisibleKey)		// >= v4.0d1

		WRITETOMAP(mScrollForUnseen, cScrollForUnseenKey)			// >= v4.0.2
	}

	// Write favourites prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cFavouritesSection);

		// Favourites
		WRITETOMAP(mFavourites, cFavouritesKey_2_0)					// >= v1.4a2
		WRITETOMAP(mMRUCopyTo, cMRUCopyToKey_2_0)					// >= v1.4a2
		WRITETOMAP(mMRUAppendTo, cMRUAppendToKey_2_0)				// >= v1.4a2
		WRITETOMAP(mMailboxAliases, cMailboxAliasesKey)				// >= v3.0a4
	}

	// Write search prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cSearchSection);

		// Search
		WRITETOMAP(mMultipleSearch, cMultipleSearchKey)					// >= v1.4a5
		WRITETOMAP(mNumberMultipleSearch, cNumberMultipleSearchKey)		// >= v1.4a5
		WRITETOMAP(mLoadBalanceSearch, cLoadBalanceSearchKey)			// >= v1.4a5
		WRITETOMAP(mOpenFirstSearchResult, cOpenFirstSearchKey)			// >= v1.4a5
	}

	// Write rules prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cRulesSection);

		WRITETOMAP(GetFilterManager()->UseLog(), cRulesUseLog)		// >= v2.1d1
		
		// Have filter manager write itself to the map
		GetFilterManager()->WriteToMap(theMap, dirty_only);			// >= v2.1d1
	}

	// Write message prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cMessageSection);

		WRITETOMAP(showMessageHeader, cShowMessageHeaderKey)		// >= v1.1
		WRITETOMAP(saveMessageHeader, cSaveMessageHeaderKey)		// >= v1.1
		WRITETOMAP(mSaveCreator, cSaveCreatorKey)					// >= v1.3a4
		WRITETOMAP(deleteAfterCopy, cDeleteAfterCopyKey)			// >= v1.1
		WRITETOMAP(mOpenDeleted, cOpenDeletedKey)					// >= v1.1
		WRITETOMAP(mCloseDeleted, cCloseDeletedKey)					// >= v3.1b1
		WRITETOMAP(mDoSizeWarn, cDoSizeWarnKey)						// >= v1.1
		WRITETOMAP(warnMessageSize, cWarnMessageSizeKey)			// >= v1.1
		WRITETOMAP(mQuoteSelection, cQuoteSelectionKey)				// >= v2.1a3
		WRITETOMAP(mAlwaysQuote, cAlwaysQuoteKey)					// >= v1.2
		WRITETOMAP(optionKeyReplyDialog, cOptionReplyDialogKey)		// >= v1.1
		WRITETOMAP(showStyled, cShowStyled)							// >= v1.3a2
		WRITETOMAP(mHeaderCaption, cHeaderCaptionKey)				// >= v1.4a6
		WRITETOMAP(mHeaderBox, cHeaderBoxKey)						// >= v1.4a6
		WRITETOMAP(mPrintSummary, cPrintSummaryKey)					// >= v3.1b1
		WRITETOMAP(mFooterCaption, cFooterCaptionKey)				// >= v1.4a6
		WRITETOMAP(mFooterBox, cFooterBoxKey)						// >= v1.4a6
		WRITETOMAP(mOpenReuse, cOpenReuseKey)						// >= v2.0a4
		WRITETOMAP(mAutoDigest, cAutoDigestKey)						// >= v2.0a7
		WRITETOMAP(mExpandHeader, cExpandHeaderKey)					// >= v2.1a5
		WRITETOMAP(mExpandParts, cExpandPartsKey)					// >= v2.0.5b3
		WRITETOMAP(mForwardChoice, cForwardChoiceKey)				// >= v2.1a1
		WRITETOMAP(mForwardQuoteOriginal, cForwardQuoteOriginalKey)	// >= v2.1a1
		WRITETOMAP(mForwardHeaders, cForwardHeadersKey)				// >= v2.1a2
		WRITETOMAP(mForwardAttachment, cForwardAttachmentKey)		// >= v2.1a3
		WRITETOMAP(mForwardRFC822, cForwardRFC822Key)				// >= v2.1a2
		WRITETOMAP(mMDNOptions, cMDNOptionsKey)						// >= v3.0a2
	}

	// Write letter prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cLetterSection);

		WRITETOMAP(mAutoInsertSignature, cAutoInsertSigKey)			// >= v1.3a5
		WRITETOMAP(mSignatureEmptyLine, cSigEmptyLineKey)			// >= v1.4a7
		WRITETOMAP(mSigDashes, cSigDashesKey)						// >= v3.1b1
		WRITETOMAP(mNoSubjectWarn, cNoSubjectWarnKey)				// >= v1.4b2
		WRITETOMAP(spaces_per_tab, cTabWidthKey)					// >= v1.1
		WRITETOMAP(mTabSpace, cTabSpaceKey)							// >= v1.4d1
		WRITETOMAP(mReplyQuote, cIncludeKey)						// >= v1.1
		WRITETOMAP(mReplyStart, cReplyStartKey)						// >= v1.4a6
		WRITETOMAP(mReplyEnd, cReplyEndKey)							// >= v1.4a6
		WRITETOMAP(mReplyCursorTop, cReplyCursorTopKey)				// >= v1.4a7
		WRITETOMAP(mReplyNoSignature, cReplyNoSignatureKey)			// >= v3.1b1
		WRITETOMAP(mForwardQuote, cForwardQuoteKey)					// >= v1.3a3
		WRITETOMAP(mForwardStart, cForwardStartKey)					// >= v1.4a6
		WRITETOMAP(mForwardEnd, cForwardEndKey)						// >= v1.4a6
		WRITETOMAP(mForwardCursorTop, cForwardCursorTopKey)			// >= v1.4a7
		WRITETOMAP(mForwardSubject, cForwardSubjectKey)				// >= v3.0a4
		WRITETOMAP(mLtrHeaderCaption, cLtrHeaderCaptionKey)			// >= v1.4a6
		WRITETOMAP(mLtrFooterCaption, cLtrFooterCaptionKey)			// >= v1.4a6
		WRITETOMAP(mSeparateBCC, cSeparateBCCKey)					// >= v2.1a3
		WRITETOMAP(mBCCCaption, cBCCCaptionKey)						// >= v2.1a3
		WRITETOMAP(mShowCCs, cShowCCsKey)							// >= v3.0b7
		WRITETOMAP(mShowBCCs, cShowBCCsKey)							// >= v3.0b7
		WRITETOMAP(mExternalEditor, cExternalEditorKey)					// >= v3.0a4
		WRITETOMAP(mUseExternalEditor, cUseExternalEditorKey)			// >= v3.0a4
		WRITETOMAP(mExternalEditAutoStart, cExternalEditAutoStartKey)	// >= v3.0a4
		WRITETOMAP(wrap_length, cWrapLengthKey)						// >= v1.1
		WRITETOMAP(mWindowWrap, cWindowWrapKey)						// >= v1.4a7
		WRITETOMAP(mDisplayAttachments, cDisplayAttachmentsKey)		// >= v1.1
		WRITETOMAP(mAppendDraft, cAppendDraftKey)					// >= v1.4b2
		WRITETOMAP(inbox_append, cInboxAppendKey)					// >= v1.1
		WRITETOMAP(mDeleteOriginalDraft, cDeleteOriginalDraftKey)	// >= v2.0b7
		WRITETOMAP(mTemplateDrafts, cTemplateDraftsKey)				// >= v2.0b7
		WRITETOMAP(mSmartURLPaste, cSmartURLPasteKey)				// >= v3.0.1
		WRITETOMAP(mWarnReplySubject, cWarnReplySubjectKey)			// >= v3.1b7
		WRITETOMAP(mSaveOptions, cSaveOptions)						// >= v2.1a2
		WRITETOMAP(mSaveMailbox, cSaveMailbox)						// >= v2.1a2
		WRITETOMAP(compose_as, cComposeAs)							// >= v1.3a2
		WRITETOMAP(enrMultiAltPlain, cEnrMultiAltPlain)				// >= v1.3a2
		WRITETOMAP(htmlMultiAltEnriched, cHTMLMultiAltEnriched)		// >= v1.3a2
		WRITETOMAP(enrMultiAltHTML, cEnrMultiAltHTML)				// >= v1.3a2
		WRITETOMAP(htmlMultiAltPlain, cHTMLMultiAltPlain)			// >= v1.3a2
		WRITETOMAP(mFormatFlowed, cFormatFlowed)					// >= v2.0b5
		WRITETOMAP(mAlwaysUnicode, cAlwaysUnicode)					// >= v4.0b2
		WRITETOMAP(mDisplayIdentityFrom, cDisplayIdentityFrom)		// >= v4.0b2
		WRITETOMAP(mAutoSaveDrafts, cAutoSaveDraftsKey)				// >= v4.0.3
		WRITETOMAP(mAutoSaveDraftsInterval, cAutoSaveDraftsIntervalKey)				// >= v4.0.3
	}

//--------------------Security Prefs
	// Read security prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cSecuritySection);

		WRITETOMAP(mPreferredPlugin, cPreferredPluginKey)					// >= v3.1b3
		WRITETOMAP(mUseMIMESecurity, cUseMIMESecurityKey)					// >= v2.0d4
		WRITETOMAP(mEncryptToSelf, cEncryptToSelfKey)						// >= v2.0d4
		WRITETOMAP(mCachePassphrase, cCachePassphraseKey)					// >= v2.0d4
		WRITETOMAP(mCachePassphraseMax, cCachePassphraseMaxKey)				// >= v3.1b3
		WRITETOMAP(mCachePassphraseIdle, cCachePassphraseIdleKey)			// >= v3.1b3
		WRITETOMAP(mAutoVerify, cAutoVerifyKey)								// >= v2.0a6
		WRITETOMAP(mAutoDecrypt, cAutoDecryptKey)							// >= v2.0a6
		WRITETOMAP(mWarnUnencryptedSend, cWarnUnencryptedSendKey)			// >= v3.1b6
		WRITETOMAP(mUseErrorAlerts, cUseErrorAlertsKey)						// >= v3.1b3
		WRITETOMAP(mCacheUserCerts, cCacheUserCertsKey)						// >= v3.1b4
		WRITETOMAP(mVerifyOKNotification, cVerifyOKNotificationKey)			// >= v2.0.5b3
	}

	// Write identitity prefs
	if (local || !remote)
	{
		StMapSection section(theMap, cIdentitiesSection);

		//cdstring sexpression = mIdentities.mValue.CreateSExpression();
		WRITETOMAP(mIdentities, cIdentitiesKey_2_0)						// >= v2.0d1
		WRITETOMAP(mTiedMailboxes, cTiedMailboxesKey)					// >= v2.0d4
		WRITETOMAP(mTiedCalendars, cTiedCalendarsKey)					// >= v4.0d1
		WRITETOMAP(mContextTied, cContextTiedKey)						// >= v2.0b5
		WRITETOMAP(mMsgTied, cMsgTiedKey)								// >= v3.0b7
		WRITETOMAP(mTiedMboxInherit, cTiedMboxInheritKey)				// >= v3.1b4
	}

	// Write address prefs
	{
		StMapSection section(theMap, cAddressSection);

		if (local || !remote)
			WRITETOMAP(mAddressAccounts, cAddressAccountsKey_2_0)			// >= v1.3d2
		if (!servers_only)
		{
			{
				CPreferenceValueMap<CAddressAccountList> temp;
				temp.Value().push_back(new CAddressAccount(mLocalAdbkAccount.GetValue()));
				temp.SetDirty(mLocalAdbkAccount.IsDirty());
				temp.WriteToMap(cLocalAdbkAccount, theMap, dirty_only);							// >= v2.0a2
			}
			
			{
				CPreferenceValueMap<CAddressAccountList> temp;
				temp.Value().push_back(new CAddressAccount(mOSAdbkAccount.GetValue()));
				temp.SetDirty(mOSAdbkAccount.IsDirty());
				temp.WriteToMap(cOSAdbkAccount, theMap, dirty_only);							// >= v3.1b1
			}

			WRITETOMAP(mExpandedAdbks, cExpandedAdbksKey)					// >= v4.1
			WRITETOMAP(mAdbkOpenAtStartup, cAdbkOpenAtStartup_2_0)			// >= v2.0b6
			WRITETOMAP(mAdbkNickName, cAdbkNickName_2_0)					// >= v2.0b6
			WRITETOMAP(mAdbkSearch, cAdbkSearch_2_0)						// >= v2.0b6
			WRITETOMAP(mAdbkAutoSync, cAdbkAutoSync_2_0)					// >= v2.0b6
			WRITETOMAP(mAdbkACLStyles, cAdbkACLStyleListKey)				// >= v1.3d2
			WRITETOMAP(mOptionKeyAddressDialog, cOptionAddressDialogKey)	// >= v1.1
			WRITETOMAP(mExpandNoNicknames, cExpandNoNicknamesKey)			// >= v2.0a6
			WRITETOMAP(mExpandFailedNicknames, cExpandFailedNicknamesKey)	// >= v2.0a6
			WRITETOMAP(mExpandFullName, cExpandFullNameKey)					// >= v2.0a6
			WRITETOMAP(mExpandNickName, cExpandNickNameKey)					// >= v2.0a6
			WRITETOMAP(mExpandEmail, cExpandEmailKey)						// >= v2.0a6
			WRITETOMAP(mSkipLDAP, cSkipLDAPKey)								// >= v4.0b2
			WRITETOMAP(mCaptureAddressBook, cCaptureAddressBookKey)			// >= v3.0b5
			WRITETOMAP(mCaptureAllowEdit, cCaptureAllowEditKey)				// >= v3.0b5
			WRITETOMAP(mCaptureAllowChoice, cCaptureAllowChoiceKey)			// >= v3.0b5
			WRITETOMAP(mCaptureRead, cCaptureReadKey)						// >= v3.0b5
			WRITETOMAP(mCaptureRespond, cCaptureRespondKey)					// >= v3.0b5
			WRITETOMAP(mCaptureFrom, cCaptureFromKey)						// >= v3.0b5
			WRITETOMAP(mCaptureCc, cCaptureCcKey)							// >= v3.0b5
			WRITETOMAP(mCaptureReplyTo, cCaptureReplyToKey)					// >= v3.0b5
			WRITETOMAP(mCaptureTo, cCaptureToKey)							// >= v3.0b5
		}
	}

	// Write calendar prefs
	{
		StMapSection section(theMap, cCalendarSection);

		if (!servers_only)
		{
			WRITETOMAP(mCalendarAccounts, cCalendarAccountsKey)				// >= v4.0d1
			{
				CPreferenceValueMap<CCalendarAccountList> temp;
				temp.Value().push_back(new CCalendarAccount(mLocalCalendarAccount.GetValue()));
				temp.SetDirty(mLocalCalendarAccount.IsDirty());
				temp.WriteToMap(cLocalCalendarAccountKey, theMap, dirty_only);	// >= v4.0d1
			}
			{
				CPreferenceValueMap<CCalendarAccountList> temp;
				temp.Value().push_back(new CCalendarAccount(mWebCalendarAccount.GetValue()));
				temp.SetDirty(mWebCalendarAccount.IsDirty());
				temp.WriteToMap(cWebCalendarAccountKey, theMap, dirty_only);	// >= v4.0d1
			}
			
			WRITETOMAP(mSubscribedCalendars, cSubscribedCalendarsKey)		// >= v4.0d1
			WRITETOMAP(mExpandedCalendars, cExpandedCalendarsKey)			// >= v4.0d1
			WRITETOMAP(mCalendarColours, cCalendarColoursKey)				// >= v4.0d1
			WRITETOMAP(mTimezone, cTimezoneKey)								// >= v4.0d1
			WRITETOMAP(mFavouriteTimezones, cFavouriteTimezonesKey)			// >= v4.1a1
			WRITETOMAP(mWeekStartDay, cWeekStartDayKey)						// >= v4.0d1
			WRITETOMAP(mWorkDayMask, cWorkDayMaskKey)						// >= v4.0d1
			WRITETOMAP(mDayWeekTimeRanges, cDayWeekTimeRangesKey)			// >= v4.0d1
			WRITETOMAP(mDisplayTime, cDisplayTimeKey)						// >= v4.0d1
			WRITETOMAP(m24HourTime, c24HourTimeKey)							// >= v4.0d1
			WRITETOMAP(mHandleICS, cHandleICSKey)							// >= v4.0d1
			WRITETOMAP(mAutomaticIMIP, cAutomaticIMIPKey)					// >= v4.0a5
			WRITETOMAP(mAutomaticEDST, cAutomaticEDSTKey)					// >= v4.0.8
			WRITETOMAP(mShowUID, cShowUIDKey)                               // >= v4.1a3
			WRITETOMAP(mDefaultCalendar, cDefaultCalendarKey)				// >= v4.0d1
		}
	}

	// Write attachment prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cAttachmentsSection);

		WRITETOMAP(mDefault_mode, cDefaultModeKey)					// >= v1.1
		WRITETOMAP(mDefault_Always, cDefaultAlwaysKey)				// >= v1.1
		WRITETOMAP(mViewDoubleClick, cViewDoubleClickKey)			// >= v2.1a4
		WRITETOMAP(mAskDownload, cAskDownloadKey)					// >= v1.2
		WRITETOMAP(mDefaultDownload, cDefaultDownloadKey_2_0)		// >= v2.0
		WRITETOMAP(mAppLaunch, cAppLaunchKey)						// >= v1.2
		WRITETOMAP(mLaunchText, cLaunchTextKey)						// >= v1.2
#if __dest_os == __win32_os
		WRITETOMAP(mShellLaunch, cShellLaunchKey)					// >= v1.4b5
#endif
		WRITETOMAP(mMIMEMappings, cMIMEMapKey)						// >= v1.1
		WRITETOMAP(mExplicitMapping, cExplicitMappingKey)			// >= v1.1

		WRITETOMAP(mWarnMissingAttachments, cWarnMissingAttachmentsKey)				// >= v3.1b6
		WRITETOMAP(mMissingAttachmentSubject, cMissingAttachmentSubjectKey)			// >= v3.1b6
		WRITETOMAP(mMissingAttachmentWords, cMissingAttachmentWordsKey)				// >= v3.1b6
	}

#ifdef __use_speech
	// Write speech prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cSpeechSection);

		WRITETOMAP(mSpeakNewOpen, cSpeakNewOpenKey)					// >= v1.1.0
		WRITETOMAP(mSpeakNewArrivals, cSpeakNewArrivalsKey)			// >= v1.1.0
#ifdef __MULBERRY_CONFIGURE_V2_0
		// Special to support v2.0.x admin tool
		WRITETOMAP(mSpeakNewAlert, cSpeakNewAlertKey)				// >= v1.1.0 < 2.1a2
		WRITETOMAP(mSpeakNewAlertTxt, cSpeakNewAlertTxtKey)			// >= v1.1.0 < 2.1a2
#endif
		WRITETOMAP(mSpeakMessageItems, cSpeakMessageItemsKey)		// >= v1.1.0
		WRITETOMAP(mSpeakLetterItems, cSpeakLetterItemsKey)			// >= v1.2
		WRITETOMAP(mSpeakMessageEmptyItem, cSpeakEmptyItemKey)		// >= v1.1.0
		WRITETOMAP(mSpeakMessageMaxLength, cSpeakMaxLengthKey)		// >= v1.1.0
	}
#endif

	// Write miscellaneous prefs
	if (!servers_only)
	{
		StMapSection section(theMap, cMiscellaneousSection);

		WRITETOMAP(mTextMacros, cTextMacrosKey)								// >= v3.0
		WRITETOMAP(mAllowKeyboardShortcuts, cAllowKeyboardShortcutsKey)		// >= v3.1b9
	}

	// Now do window
	if (!servers_only && !not_windows)
		WriteWindowInfoToMap(theMap, dirty_only);

#ifdef __MULBERRY
	// Now do plugins
	CPluginManager::sPluginManager.WriteToMap(theMap, dirty_only);
#endif
}

#pragma dont_inline on
// Read data from a handle stream
bool CPreferences::ReadFromMap(COptionsMap* theMap,
									NumVersion vers_app,
									NumVersion& vers_prefs)
{
#define READFROMMAP(x, y) x.ReadFromMap(y, theMap, vers_prefs);

	CIdentity* default_id = nil;	// Needed for v1.3 -> v1.4 update
	cdstring firstWD;				// Needed for v1.2 -> v1.4 update

	// Section is just prefs
	StMapSection section(theMap, cPrefsSection);

	// Try to read appropriate version key
	cdstring txt;
	if (theMap->ReadValue(cVersionKey_v2_0, txt, vers_app))
	{
		// Got new key
	}

	// Try to read v1.4 version key
	else if (theMap->ReadValue(cVersionKey_v1_4, txt, vers_app))
	{
		// Got new key
	}

	// Try to read <= v1.3 version key
	else
		// Read prefs version
		theMap->ReadValue(cVersionKey, txt, vers_app);

	// Convert text to vers
	NumVersionVariant tempv;
	tempv.whole = ::strtol(txt, nil, 0);
	vers = tempv.parts;
#if 0
	// Hack for alphas
	if ((VersionTest(vers, VERS_1_4_0_A_1) >= 0) &&
		(VersionTest(vers, VERS_1_4_0_A_6) <= 0))
	{
		// Reset to apps value to ensure its correct when it gets written back
		vers = vers_app;
		return false;
	}
#endif

	// Check version
	mUpdateVers = (*(long*) &vers != *(long*) &vers_app);

	// Special if no vers present then assume this is the current version
	if (*(long*) &vers == 0)
		vers = vers_app;

	// Copy for conversion
	vers_prefs = vers;

	// Now set to apps
	vers = vers_app;

	// Needed for conversion
	NumVersionVariant temp_1_3_0;
	temp_1_3_0.whole = VERS_1_3_0;
	NumVersionVariant temp_1_4_0;
	temp_1_4_0.whole = VERS_1_4_0;

//--------------------General Prefs

	READFROMMAP(mHelpState, cHelpStateKey)
	READFROMMAP(mDetailedTooltips, cDetailedTooltipsKey)
	READFROMMAP(mUse_IC, cUseICKey)
	READFROMMAP(mCheckDefaultMailClient, cCheckDefaultMailClientKey)
	READFROMMAP(mCheckDefaultWebcalClient, cCheckDefaultWebcalClientKey)
	READFROMMAP(mWarnMailtoFiles, cWarnMailtoFilesKey)
	READFROMMAP(mSimple, cSimpleKey)
	READFROMMAP(mLastPanel, cLastPanelKey)
	READFROMMAP(mDisconnected, cDisconnectedKey)

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure IC is available
#ifdef __MULBERRY
	if (!CICSupport::ICInstalled())
#endif
	{
		mUse_IC.mValue = false;
		mCheckDefaultMailClient.mValue = false;
		mCheckDefaultWebcalClient.mValue = false;
	}
#else
	// Win32 never has IC
	mUse_IC.mValue = false;
#endif

//--------------------Network Control Prefs
	{
		StMapSection section(theMap, cNetworkControlSection);
		
		READFROMMAP(mAppIdleTimeout, cAppIdleTimeoutKey)
		READFROMMAP(mWaitDialogStart, cWaitDialogStartKey)
		READFROMMAP(mWaitDialogMax, cWaitDialogMaxKey)
		READFROMMAP(mConnectRetryTimeout, cConnectRetryTimeoutKey)
		READFROMMAP(mConnectRetryMaxCount, cConnectRetryMaxCountKey)
		READFROMMAP(mBackgroundTimeout, cBackgroundTimeoutKey)
		READFROMMAP(mTickleInterval, cTickleIntervalKey)
		READFROMMAP(mTickleIntervalExpireTimeout, cTickleIntervalExpireTimeoutKey)
		READFROMMAP(mSleepReconnect, cSleepReconnectKey)
	}

//--------------------Network Prefs
	// >= v1.4
	if (VersionTest(vers_prefs, VERS_1_4_0) >= 0)
	{
		StMapSection section(theMap, cAccountSection);

		// Try each different version in turn
		if (!mMailAccounts.ReadFromMap(cMailAccountsKey_2_0, theMap, vers_prefs))
		{
			// >= v1.4
			mMailAccounts.ReadFromMap(cMailAccountsKey_1_4, theMap, temp_1_4_0.parts);
			mMailAccounts.SetDirty();
		}
		if (!mSMTPAccounts.ReadFromMap(cSMTPAccountsKey_2_0, theMap, vers_prefs))
		{
			// >= v1.4
			mSMTPAccounts.ReadFromMap(cSMTPAccountsKey_1_4, theMap, temp_1_4_0.parts);
			mSMTPAccounts.SetDirty();
		}
		else if (VersionTest(vers_prefs, VERS_2_0_0_B_6) < 0)
		{
			// This is needed bacuse prior to v2.0b6 the hold option was not inialised properly

			// Turn off hold option on each account
			for(CSMTPAccountList::iterator iter = mSMTPAccounts.Value().begin(); iter != mSMTPAccounts.Value().end(); iter++)
				(*iter)->SetHoldMessages(false);
		}

		READFROMMAP(mSIEVEAccounts, cSIEVEAccountsKey)
		READFROMMAP(mMailDomain, cMailDomainKey)
		READFROMMAP(mOSDefaultLocation, cOSDefaultLocationKey)
		READFROMMAP(mDisconnectedCWD, cDisconnectedCWDKey)
	}
	else
	{
		StMapSection section(theMap, cNetworkSection);

		// Read in old set of Network prefs
		CPreferenceValueMap<cdstring>				IMAPServerIP;						// IMAP server IP address
		CPreferenceValueMap<cdstring>				SMTPServerIP;						// SMTP server IP address
		CPreferenceValueMap<cdstring>				mailDomain;							// Mail domain
		CPreferenceValueMap<cdstring>				userName;							// User name
		CPreferenceValueMap<cdstring>				SMTPuid;							// User email return address
		CPreferenceValueMap<cdstring>				IMAPuid;							// User logon id
		CPreferenceValueMap<cdstring>				userPswd;							// User password
		CPreferenceValueMap<bool>					saveUser(false);					// User ids auto saved?
		CPreferenceValueMap<bool>					savePswd(false);					// User pswd auto saved?
		CPreferenceValueMap<char>					mDirDelim('\0');					// Character delimiting directory hierarchies in server file system
		CPreferenceValueMap<cdstrvect>				mWDs;								// List of working directories for account
		CPreferenceValueMap<bool>					logon_startup(false);				// Logon at startup
		CPreferenceValueMap<bool>					mDoCheck(true);						// Do mail check?
		CPreferenceValueMap<long>					checkInterval(5);					// New mail check interval (mins)
		CPreferenceValueMap<bool>					newMailAlert(true);					// Show new mail alert

		READFROMMAP(IMAPServerIP, cIMAPServerIPKey)
		READFROMMAP(SMTPServerIP, cSMTPServerIPKey)
		READFROMMAP(mailDomain, cMailDomainKey)
		READFROMMAP(userName, cUserNameKey)
#ifdef __MULBERRY
		if (!CAdminLock::sAdminLock.mLockReturnAddress || CAdminLock::sAdminLock.mLockedReturnDomain.empty())
#endif
			READFROMMAP(SMTPuid, cSMTPuidKey)
		READFROMMAP(IMAPuid, cIMAPuidKey)
		READFROMMAP(userPswd, cPasswordKey)
		// >= v1.3.2
		if (VersionTest(vers_prefs, VERS_1_3_2) >= 0)
			userPswd.mValue.Decrypt(cdstring::eEncryptSimplemUTF7);
		else
		{
			// Use old encoding and force dirty for update
			userPswd.mValue.Decrypt(cdstring::eEncryptSimple);
			userPswd.SetDirty();
		}
		READFROMMAP(saveUser, cSaveUserKey)
		READFROMMAP(savePswd, cSavePswdKey)
		if (!savePswd.mValue)
			userPswd.mValue = cdstring::null_str;
		READFROMMAP(mDirDelim, cDirDelimKey)

		// Read working directories addresses
		READFROMMAP(mWDs, cWDsKey)
		if ((mWDs.mValue.size() > 0) &&
			(mWDs.mValue.front() != cWILDCARD) &&
			(mWDs.mValue.front() != cWILDCARD_NODIR))
			firstWD = mWDs.mValue.front();

		READFROMMAP(logon_startup, cLogonStartupKey)
		READFROMMAP(mDoCheck, cDoCheckKey)
		READFROMMAP(checkInterval, cCheckIntervalKey)
		READFROMMAP(newMailAlert, cNewMailAlertKey)

		// Now create new style accounts

		// Create IMAP account
		{
			CMailAccount* mail = new CMailAccount;

			// Make name first part of IP address
			cdstring acct_name = IMAPServerIP.mValue;
			if (acct_name.empty())
				acct_name = "IMAP";
			else
			{
				char* p = ::strchr(acct_name.c_str_mod(), '.');
				if (p)
					*p = 0;
			}
			mail->SetName(acct_name);

			mail->SetServerType(CINETAccount::eIMAP);
			mail->SetServerIP(IMAPServerIP.mValue);
			mail->SetDirDelim(mDirDelim.mValue);
			{
				CDisplayItemList wds;
				for(cdstrvect::const_iterator iter = mWDs.mValue.begin(); iter != mWDs.mValue.end(); iter++)
					wds.push_back(CDisplayItem(*iter));
				mail->SetWDs(wds);
			}
			mail->SetLoginAtStart(logon_startup.mValue);

			CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) mail->GetAuthenticatorUserPswd();
			auth->SetUID(IMAPuid.mValue);
			auth->SetPswd(userPswd.mValue);
			auth->SetSaveUID(saveUser.mValue);
			auth->SetSavePswd(savePswd.mValue);

			// Add to list (clear it first)
			mMailAccounts.mValue.clear();
			mMailAccounts.mValue.push_back(mail);

			// Process this as a new account
			NewAccount(mail);
		}

		// Create SMTP account
		{
			CSMTPAccount* smtp = new CSMTPAccount;

			// Make name first part of IP address
			cdstring acct_name = SMTPServerIP.mValue;
			if (acct_name.empty())
				acct_name = "SMTP";
			else
			{
				char* p = ::strchr(acct_name.c_str_mod(), '.');
				if (p)
					*p = 0;
			}
			smtp->SetName(acct_name);

			smtp->SetServerType(CINETAccount::eSMTP);
			smtp->SetServerIP(SMTPServerIP.mValue);
			smtp->SetLoginAtStart(false);

			// Add to list (clear it first)
			mSMTPAccounts.mValue.clear();
			mSMTPAccounts.mValue.push_back(smtp);
		}

		// Create default identity
		{
			default_id = new CIdentity;
			default_id->SetIdentity(userName.mValue.empty() ? cdstring("Default") : userName.mValue);
			default_id->SetSMTPAccount(mSMTPAccounts.mValue.front()->GetName(), true);
			{
				CAddress addr(SMTPuid.mValue, userName.mValue);
				default_id->SetFrom(addr.GetFullAddress(), true);
			}
		}

		// Create default mail notification
		{
			CMailNotification notify;
			notify.SetName("Default");
			notify.Enable(mDoCheck.mValue);
			notify.SetCheckInterval(checkInterval.mValue);
			notify.SetShowAlertForeground(newMailAlert.mValue);
			notify.SetPlaySound(true);
			mMailNotification.mValue.clear();
			mMailNotification.mValue.push_back(notify);
		}

		// Mail domain - same
		mMailDomain.mValue = mailDomain.mValue;

		// Force refresh of new
		mMailAccounts.SetDirty();
		mSMTPAccounts.SetDirty();
		mMailNotification.SetDirty();
		mMailDomain.SetDirty();
	}

//--------------------Remote Prefs
	// Read remote prefs
	{
		StMapSection section(theMap, cRemoteSection);

		// >= v1.4
		if (VersionTest(vers_prefs, VERS_1_4_0) >= 0)
		{
			if (!mRemoteAccounts.ReadFromMap(cRemoteAccountsKey_2_0, theMap, vers_prefs))
			{
				// >= v1.4
				mRemoteAccounts.ReadFromMap(cRemoteAccountsKey_1_4, theMap, temp_1_4_0.parts);
				mRemoteAccounts.SetDirty();
			}
			READFROMMAP(mRemoteCachePswd, cRemoteCachePswdKey)
		}
		else
		{
			// Read in old set of remote prefs
			CPreferenceValueMap<bool>					mUseRemote(false);					// Use remote prefs on startup
			CPreferenceValueMap<cdstring>				mRemoteIP;							// Remote prefs server IP address
			CPreferenceValueMap<cdstring>				mRemoteUID;							// Remote server UID
			CPreferenceValueMap<cdstring>				mRemotePswd;						// Remote server password
			CPreferenceValueMap<bool>					mRemoteSaveUser(false);				// Save remote user id
			CPreferenceValueMap<bool>					mRemoteSavePswd(false);				// Save remote pswd

			READFROMMAP(mUseRemote, cUseRemoteKey)
			READFROMMAP(mRemoteIP, cRemoteIPKey)
			READFROMMAP(mRemoteUID, cRemoteUIDKey)
			READFROMMAP(mRemotePswd, cRemotePswdKey)
			// >= v1.3.2
			if (VersionTest(vers_prefs, VERS_1_3_2) >= 0)
				mRemotePswd.mValue.Decrypt(cdstring::eEncryptSimplemUTF7);
			else
			{
				// Use old encoding and force dirty for update
				mRemotePswd.mValue.Decrypt(cdstring::eEncryptSimple);
				mRemotePswd.SetDirty();
			}
			READFROMMAP(mRemoteSaveUser, cRemoteSaveUserKey)
			READFROMMAP(mRemoteSavePswd, cRemoteSavePswdKey)
			if (!mRemoteSavePswd.mValue)
				mRemotePswd.mValue = cdstring::null_str;
			READFROMMAP(mRemoteCachePswd, cRemoteCachePswdKey)

			// Now create real account info
			if (!mRemoteIP.mValue.empty())
			{
				COptionsAccount* remote = new COptionsAccount;

				// Make name first part of IP address
				cdstring acct_name = mRemoteIP.mValue;
				if (acct_name.empty())
					acct_name = "IMSP";
				else
				{
					char* p = ::strchr(acct_name.c_str_mod(), '.');
					if (p)
						*p = 0;
				}
				remote->SetName(acct_name);

				remote->SetServerType(CINETAccount::eIMSP);
				remote->SetServerIP(mRemoteIP.mValue);
				remote->SetLoginAtStart(mUseRemote.mValue);

				CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) remote->GetAuthenticatorUserPswd();
				auth->SetUID(mRemoteUID.mValue);
				auth->SetPswd(mRemotePswd.mValue);
				auth->SetSaveUID(mRemoteSaveUser.mValue);
				auth->SetSavePswd(mRemoteSavePswd.mValue);

				// Add to list
				mRemoteAccounts.mValue.clear();
				mRemoteAccounts.mValue.push_back(remote);
			}
			else
				mRemoteAccounts.mValue.clear();
		}

		// Force refresh of new
		mRemoteAccounts.SetDirty();
	}

//--------------------Notification Prefs
	// Read display prefs
	{
		bool got_mail = false;
		bool got_attach = false;

		// Try 2.0 format first
		{
			StMapSection section(theMap, cNotificationSection_2_0);

			got_mail = READFROMMAP(mMailNotification, cMailNotificationKey)
			got_attach = READFROMMAP(mAttachmentNotification, cAttachmentNotificationKey)
		}

		// If some missing try as v1.4 format
		if (!got_mail || !got_attach)
		{
			StMapSection section(theMap, cNotificationSection_1_4);

			if (!got_mail)
			{
				got_mail = READFROMMAP(mMailNotification, cMailNotificationKey)
				mMailNotification.SetDirty();
			}
			if (!got_attach)
			{
				got_attach = READFROMMAP(mAttachmentNotification, cAttachmentNotificationKey)
				mAttachmentNotification.SetDirty();
			}
		}

		// Must have default notifier
		if (!got_mail && !mMailNotification.mValue.size())
		{
			CMailNotification notify;
			notify.SetName("Default");
			mMailNotification.mValue.push_back(notify);
		}
	}

//--------------------Display Prefs
	// Read display prefs
	{
		StMapSection section(theMap, cDisplaySection);

		READFROMMAP(mServerOpenStyle, cServerOpenStyleKey)
		READFROMMAP(mServerClosedStyle, cServerClosedStyleKey)
		READFROMMAP(mServerBkgndStyle, cServerBkgndStyleKey)
		READFROMMAP(mFavouriteBkgndStyle, cFavouriteBkgndStyleKey)
		READFROMMAP(mHierarchyBkgndStyle, cHierarchyBkgndStyleKey)

		READFROMMAP(mMboxRecentStyle, cMboxRecentStyleKey)
		READFROMMAP(mMboxUnseenStyle, cMboxUnseenStyleKey)
		READFROMMAP(mMboxOpenStyle, cMboxOpenStyleKey)
		READFROMMAP(mMboxFavouriteStyle, cMboxFavouriteStyleKey)
		READFROMMAP(mMboxClosedStyle, cMboxClosedStyleKey)
		READFROMMAP(mIgnoreRecent, cIgnoreRecentStyleKey)

		READFROMMAP(unseen, cUnseenStyleKey)
		READFROMMAP(seen, cSeenStyleKey)
		READFROMMAP(answered, cAnsweredStyleKey)
		READFROMMAP(important, cImportantStyleKey)
		READFROMMAP(deleted, cDeletedStyleKey)

		READFROMMAP(mMultiAddress, cMultiAddressKey)
		READFROMMAP(mMatch, cMatchedStyleKey)
		READFROMMAP(mNonMatch, cNonMatchedStyleKey)
		READFROMMAP(mUseLocalTimezone, cUseLocalTimezoneKey)

		mLabels.Value().ReadFromMapMulti(theMap, cLabelsKey, vers_prefs, false);
		READFROMMAP(mIMAPLabels, cIMAPLabelsKey)

#if __dest_os == __win32_os
		//READFROMMAP(mUseSDI, cUseSDIKey)			Deprecated for v3
		READFROMMAP(mMultiTaskbar, cMultiTaskbarKey)
#endif
		READFROMMAP(mUse3Pane, cUse3PaneKey)
		READFROMMAP(mToolbarShow, cToolbarShowKey)
		READFROMMAP(mToolbarSmallIcons, cToolbarSmallIconsKey)
		READFROMMAP(mToolbarShowIcons, cToolbarShowIconsKey)
		READFROMMAP(mToolbarShowCaptions, cToolbarShowCaptionsKey)
		READFROMMAP(mToolbars, cToolbarsKey)
	}

//--------------------Formatting Prefs
	// Read formatting prefs
	{
		StMapSection section(theMap, cFormattingSection);

		READFROMMAP(mURLStyle, cURLStyleKey)
		READFROMMAP(mURLSeenStyle, cURLSeenStyleKey)
		READFROMMAP(mHeaderStyle, cHeaderStyleKey)
		READFROMMAP(mQuotationStyle, cQuotationStyleKey)
		READFROMMAP(mTagStyle, cTagStyleKey)

		READFROMMAP(mUseMultipleQuotes, cUseMultipleQuotesKey)
		READFROMMAP(mQuoteColours, cQuoteColoursKey)
		if (!mRecognizeQuotes.ReadFromMap(cRecognizeQuotesKey_2_0, theMap, vers_prefs))
		{
			mRecognizeQuotes.ReadFromMap(cRecognizeQuotesKey_1_4, theMap, vers_prefs);
			mRecognizeQuotes.SetDirty();
		}
		if (!mRecognizeURLs.ReadFromMap(cRecognizeURLsKey_2_0, theMap, vers_prefs))
		{
			mRecognizeURLs.ReadFromMap(cRecognizeURLsKey_1_4, theMap, vers_prefs);
			mRecognizeURLs.SetDirty();
		}

#if __dest_os == __linux_os
		READFROMMAP(mSelectionColour, cSelectionColourKey)
#endif
	}

//--------------------Font Prefs
	// Read formatting prefs
	{
		bool got_list = false;
		bool got_display = false;
		bool got_print = false;
		bool got_caption = false;
		bool got_fixed = false;
		bool got_html = false;

		// Try 2.0 bits first
		{
			StMapSection section(theMap, cFontsSection);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			got_list = READFROMMAP(mListTextTraits, cListFontKey)
			got_display = READFROMMAP(mDisplayTextTraits, cDisplayFontKey)
			got_print = READFROMMAP(mPrintTextTraits, cPrintFontKey)
			got_caption = READFROMMAP(mCaptionTextTraits, cCaptionFontKey)
			got_fixed = READFROMMAP(mFixedTextTraits, cFixedFontKey)
			got_html = READFROMMAP(mHTMLTextTraits, cHTMLFontKey)
			READFROMMAP(mAntiAliasFont, cAntiAliasFontKey)
#elif __dest_os == __win32_os || __dest_os == __linux_os
			got_list = READFROMMAP(mListTextFontInfo, cListFontKey)
			got_display = READFROMMAP(mDisplayTextFontInfo, cDisplayFontKey)
			got_print = READFROMMAP(mPrintTextFontInfo, cPrintFontKey)
			got_caption = READFROMMAP(mCaptionTextFontInfo, cCaptionFontKey)
			got_fixed = READFROMMAP(mFixedTextFontInfo, cFixedFontKey)
			got_html = READFROMMAP(mHTMLTextFontInfo, cHTMLFontKey)
#else
#error __dest_os
#endif
			READFROMMAP(mUseStyles, cUseStylesKey)
			READFROMMAP(mMinimumFont, cMinimumFontKey)
		}

		// Now try old sections
		if (!got_list || !got_display || !got_print || !got_caption)
		{
			StMapSection section(theMap, cDisplaySection);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (!got_list)
			{
				READFROMMAP(mListTextTraits, cListFontKey)
				mListTextTraits.SetDirty();
			}
			if (!got_display)
			{
				READFROMMAP(mDisplayTextTraits, cDisplayFontKey)
				mDisplayTextTraits.SetDirty();
			}
			if (!got_print)
			{
				READFROMMAP(mPrintTextTraits, cPrintFontKey)
				mPrintTextTraits.SetDirty();
			}
			if (!got_caption)
			{
				READFROMMAP(mCaptionTextTraits, cCaptionFontKey)
				mCaptionTextTraits.SetDirty();
			}
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (!got_list)
			{
				READFROMMAP(mListTextFontInfo, cListFontKey)
				mListTextFontInfo.SetDirty();
			}
			if (!got_display)
			{
				READFROMMAP(mDisplayTextFontInfo, cDisplayFontKey)
				mDisplayTextFontInfo.SetDirty();
			}
			if (!got_print)
			{
				READFROMMAP(mPrintTextFontInfo, cPrintFontKey)
				mPrintTextFontInfo.SetDirty();
			}
			if (!got_caption)
			{
				READFROMMAP(mCaptionTextFontInfo, cCaptionFontKey)
				mCaptionTextFontInfo.SetDirty();
			}
#else
#error __dest_os
#endif
		}
		if (!got_fixed || !got_html)
		{
			StMapSection section(theMap, cFormattingSection);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (!got_fixed)
			{
				READFROMMAP(mFixedTextTraits, cFixedFontKey)
				mFixedTextTraits.SetDirty();
			}
			if (!got_html)
			{
				READFROMMAP(mHTMLTextTraits, cHTMLFontKey)
				mHTMLTextTraits.SetDirty();
			}
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (!got_fixed)
			{
				READFROMMAP(mFixedTextFontInfo, cFixedFontKey)
				mFixedTextFontInfo.SetDirty();
			}
			if (!got_html)
			{
				READFROMMAP(mHTMLTextFontInfo, cHTMLFontKey)
				mHTMLTextFontInfo.SetDirty();
			}
#else
#error __dest_os
#endif
		}
	}

//--------------------Mailbox Prefs
	// Read mailbox prefs
	{
		StMapSection section(theMap, cMailboxSection);

		READFROMMAP(openAtFirst, cOpenAtFirstKey)
		READFROMMAP(openAtLast, cOpenAtLastKey)
		READFROMMAP(openAtFirstNew, cOpenAtFirstNewKey)
		READFROMMAP(mNoOpenPreview, cNoOpenPreviewKey)
		READFROMMAP(mUnseenNew, cUnseenNewKey)
		READFROMMAP(mRecentNew, cRecentNewKey)
		READFROMMAP(mNextIsNewest, cNextIsNewestKey)
		READFROMMAP(expungeOnClose, cExpungeOnCloseKey)
		READFROMMAP(warnOnExpunge, cWarnOnExpungeKey)
		READFROMMAP(mWarnPuntUnseen, cWarnPuntUnseenKey)
		READFROMMAP(mDoRollover, cDoRolloverKey)
		READFROMMAP(mRolloverWarn, cRolloverWarnKey)
		READFROMMAP(mDoMailboxClear, cDoMailboxClearKey)

		// Try to read new format
		if (!clear_mailbox.ReadFromMap(cClearMailboxKey_1_4, theMap, vers_prefs))
		{
			clear_mailbox.ReadFromMap(cClearMailboxKey_1_3, theMap, vers_prefs);

			// < v1.3d1
			if (VersionTest(vers_prefs, VERS_1_3_0_D_1) < 0)
			{
				// Prepend first places to search
				cdstring name = firstWD;
				name += clear_mailbox.mValue;
				clear_mailbox.mValue = name;
			}
			// < v1.4
			if (VersionTest(vers_prefs, VERS_1_4_0) < 0)
			{
				// Prepend account name
				cdstring acct_name = mMailAccounts.mValue.front()->GetName();
				acct_name += cMailAccountSeparator;
				acct_name += clear_mailbox.mValue;
				clear_mailbox.mValue = acct_name;
			}

			// Force write
			clear_mailbox.SetDirty();
		}
		READFROMMAP(clear_warning, cClearWarningKey)

		// < v1.4
		if (VersionTest(vers_prefs, VERS_1_4_0) < 0)
		{
			CPreferenceValueMap<bool> mPartialDisplay(false);			// Display fixed number of messages
			CPreferenceValueMap<long> mMsgBlocks(100);					// Read messages in blocks

			READFROMMAP(mPartialDisplay, cPartialDisplayKey)
			READFROMMAP(mMsgBlocks, cMsgBlocksKey)

			// Now use these as hi-cache values
			mRUseHiCache.mValue = mPartialDisplay.GetValue();
			mRHiCache.mValue = mMsgBlocks.GetValue();
			mLUseHiCache.mValue = mPartialDisplay.GetValue();
			mLHiCache.mValue = mMsgBlocks.GetValue();
		}
		else
		{
			READFROMMAP(mRLoCache, cRLoCacheKey)
			READFROMMAP(mRHiCache, cRHiCacheKey)
			READFROMMAP(mRUseHiCache, cRUseHiCacheKey)
			READFROMMAP(mRCacheIncrement, cRCacheIncrementKey)
			READFROMMAP(mRAutoCacheIncrement, cRAutoCacheIncrementKey)
			READFROMMAP(mRSortCache, cRSortCacheKey)
			READFROMMAP(mLLoCache, cLLoCacheKey)
			READFROMMAP(mLHiCache, cLHiCacheKey)
			READFROMMAP(mLUseHiCache, cLUseHiCacheKey)
			READFROMMAP(mLCacheIncrement, cLCacheIncrementKey)
			READFROMMAP(mLAutoCacheIncrement, cLAutoCacheIncrementKey)
			READFROMMAP(mLSortCache, cLSortCacheKey)
		}

		// Read smart addresses
		if (!mSmartAddressList.ReadFromMap(cSmartAddressListKey_2_0, theMap, vers_prefs))
		{
			mSmartAddressList.ReadFromMap(cSmartAddressListKey_1_4, theMap, vers_prefs);
			mSmartAddressList.SetDirty();
		}

		// < v1.4
		if (VersionTest(vers_prefs, VERS_1_4_0) < 0)
		{
			CPreferenceValueMap<cdstrvect> mAutoCheckMboxes;					// List of mailboxes to autocheck

			// Read mailbox specific prefs
			READFROMMAP(mAutoCheckMboxes, cAutoCheckMboxesKey)

			// Must convert mbox names to urls
			cdstring acct_name = mMailAccounts.mValue.front()->GetName();
			acct_name += cMailAccountSeparator;
			char dir_delim = mMailAccounts.mValue.front()->GetDirDelim();

			// Convert each entry in auto check and add to first notifiers cabinet
			for(cdstrvect::iterator iter = mAutoCheckMboxes.mValue.begin(); iter != mAutoCheckMboxes.mValue.end(); iter++)
			{
				*iter = acct_name + *iter;
				mFavourites.mValue.at(mMailNotification.GetValue().front().GetFavouriteIndex(this)).GetItems().push_back(cdstrpair(*iter, dir_delim));
			}
		}


		// Read mailbox menu favourites
		READFROMMAP(mUseCopyToMboxes, cUseCopyToMboxesKey)
		READFROMMAP(mUseAppendToMboxes, cUseAppendToMboxesKey)
		READFROMMAP(mMRUMaximum, cMRUMaximumKey)

		// Mailbox ACL styles
		READFROMMAP(mMboxACLStyles, cMboxACLStyleListKey)

		// Search styles
		READFROMMAP(mSearchStyles, cSearchStyleListKey)

		// Disconnected stuff
		READFROMMAP(mPromptDisconnected, cPromptDisconnectedKey)
		READFROMMAP(mAutoDial, cAutoDialKey)
		READFROMMAP(mUIDValidityMismatch, cUIDValidityMismatchKey)
		READFROMMAP(mSMTPQueueEnabled, cSMTPQueueEnabledKey)
		READFROMMAP(mConnectOptions, cConnectOptionsKey)
		READFROMMAP(mDisconnectOptions, cDisconnectOptionsKey)

		READFROMMAP(mQuickSearch, cQuickSearchKey)
		READFROMMAP(mQuickSearchVisible, cQuickSearchVisibleKey)

		READFROMMAP(mScrollForUnseen, cScrollForUnseenKey)
	}

//--------------------Favourite Prefs
	// Read favourite prefs
	{
		StMapSection section(theMap, cFavouritesSection);

		// Favourites
		if (!mFavourites.ReadFromMap(cFavouritesKey_2_0, theMap, vers_prefs))
		{
			// >= v1.4
			if (mFavourites.ReadFromMap(cFavouritesKey_1_4, theMap, temp_1_4_0.parts))
			{
				// Add new cabinet items
				mFavourites.Value().insert(mFavourites.Value().begin() + CMailAccountManager::eFavouriteAutoSync, CFavouriteItem());
				mFavourites.Value().at(CMailAccountManager::eFavouriteAutoSync).SetVisible(false);
				mFavourites.SetDirty();
			}
		}

		if (!mMRUCopyTo.ReadFromMap(cMRUCopyToKey_2_0, theMap, vers_prefs))
		{
			mMRUCopyTo.ReadFromMap(cMRUCopyToKey_1_4, theMap, vers_prefs);
			mMRUCopyTo.SetDirty();
		}
		if (!mMRUAppendTo.ReadFromMap(cMRUAppendToKey_2_0, theMap, vers_prefs))
		{
			mMRUAppendTo.ReadFromMap(cMRUAppendToKey_1_4, theMap, vers_prefs);
			mMRUAppendTo.SetDirty();
		}
		READFROMMAP(mMailboxAliases, cMailboxAliasesKey)
	}

//--------------------Search Prefs
	// Read search prefs
	{
		StMapSection section(theMap, cSearchSection);

		// Favourites
		READFROMMAP(mMultipleSearch, cMultipleSearchKey)
		READFROMMAP(mNumberMultipleSearch, cNumberMultipleSearchKey)
		READFROMMAP(mLoadBalanceSearch, cLoadBalanceSearchKey)
		READFROMMAP(mOpenFirstSearchResult, cOpenFirstSearchKey)
	}

//--------------------Rules Prefs
	// Read search prefs
	{
		StMapSection section(theMap, cRulesSection);

		READFROMMAP(GetFilterManager()->UseLog(), cRulesUseLog)

		// Let filter manager read its items in
		GetFilterManager()->ReadFromMap(theMap, vers_prefs);
	}

//--------------------Message Prefs
	// Read message prefs
	{
		StMapSection section(theMap, cMessageSection);

		READFROMMAP(showMessageHeader, cShowMessageHeaderKey)
		READFROMMAP(showStyled, cShowStyled)
		READFROMMAP(saveMessageHeader, cSaveMessageHeaderKey)
		// < v1.3d1
		if (VersionTest(vers_prefs, VERS_1_3_0_D_1) < 0)
		{
			long temp = *reinterpret_cast<const unsigned long*>(mSaveCreator.mValue.c_str());
			theMap->ReadValue(cSaveCreatorKey, temp, vers_prefs);
			if (temp)
			{
				mSaveCreator.mValue.reserve(4);
				*reinterpret_cast<unsigned long*>(mSaveCreator.mValue.c_str_mod()) = temp;
			}
		}
		else
			READFROMMAP(mSaveCreator, cSaveCreatorKey)
		READFROMMAP(deleteAfterCopy, cDeleteAfterCopyKey)
		READFROMMAP(mOpenDeleted, cOpenDeletedKey)
		READFROMMAP(mCloseDeleted, cCloseDeletedKey)
		READFROMMAP(mDoSizeWarn, cDoSizeWarnKey)
		READFROMMAP(warnMessageSize, cWarnMessageSizeKey)
		READFROMMAP(mQuoteSelection, cQuoteSelectionKey)
		READFROMMAP(mAlwaysQuote, cAlwaysQuoteKey)
		READFROMMAP(optionKeyReplyDialog, cOptionReplyDialogKey)

		READFROMMAP(mHeaderCaption, cHeaderCaptionKey)
		READFROMMAP(mHeaderBox, cHeaderBoxKey)
		READFROMMAP(mPrintSummary, cPrintSummaryKey)
		READFROMMAP(mFooterCaption, cFooterCaptionKey)
		READFROMMAP(mFooterBox, cFooterBoxKey)
		READFROMMAP(mOpenReuse, cOpenReuseKey)
		READFROMMAP(mAutoDigest, cAutoDigestKey)
		READFROMMAP(mExpandHeader, cExpandHeaderKey)
		READFROMMAP(mExpandParts, cExpandPartsKey)
		READFROMMAP(mForwardChoice, cForwardChoiceKey)
		READFROMMAP(mForwardQuoteOriginal, cForwardQuoteOriginalKey)
		READFROMMAP(mForwardHeaders, cForwardHeadersKey)
		READFROMMAP(mForwardAttachment, cForwardAttachmentKey)
		READFROMMAP(mForwardRFC822, cForwardRFC822Key)
		READFROMMAP(mMDNOptions, cMDNOptionsKey)
	}

//--------------------New Letter Prefs
	// Read letter prefs
	{
		StMapSection section(theMap, cLetterSection);

		READFROMMAP(mAutoInsertSignature, cAutoInsertSigKey)
		READFROMMAP(mSignatureEmptyLine, cSigEmptyLineKey)
		READFROMMAP(mSigDashes, cSigDashesKey)
		READFROMMAP(mNoSubjectWarn, cNoSubjectWarnKey)
		READFROMMAP(spaces_per_tab, cTabWidthKey)
		READFROMMAP(mTabSpace, cTabSpaceKey)
		READFROMMAP(mReplyQuote, cIncludeKey)
		READFROMMAP(mReplyStart, cReplyStartKey)
		READFROMMAP(mReplyEnd, cReplyEndKey)
		READFROMMAP(mReplyCursorTop, cReplyCursorTopKey)
		READFROMMAP(mReplyNoSignature, cReplyNoSignatureKey)
		READFROMMAP(mForwardQuote, cForwardQuoteKey)
		READFROMMAP(mForwardStart, cForwardStartKey)
		READFROMMAP(mForwardEnd, cForwardEndKey)
		READFROMMAP(mForwardCursorTop, cForwardCursorTopKey)
		READFROMMAP(mForwardSubject, cForwardSubjectKey)
		READFROMMAP(mLtrHeaderCaption, cLtrHeaderCaptionKey)
		READFROMMAP(mLtrFooterCaption, cLtrFooterCaptionKey)
		READFROMMAP(mSeparateBCC, cSeparateBCCKey)
		READFROMMAP(mBCCCaption, cBCCCaptionKey)
		READFROMMAP(mShowCCs, cShowCCsKey)
		READFROMMAP(mShowBCCs, cShowBCCsKey)
		READFROMMAP(mExternalEditor, cExternalEditorKey)
		READFROMMAP(mUseExternalEditor, cUseExternalEditorKey)
		READFROMMAP(mExternalEditAutoStart, cExternalEditAutoStartKey)
		READFROMMAP(wrap_length, cWrapLengthKey)
		READFROMMAP(mWindowWrap, cWindowWrapKey)
		READFROMMAP(mDisplayAttachments, cDisplayAttachmentsKey)
		READFROMMAP(mAppendDraft, cAppendDraftKey)
		READFROMMAP(inbox_append, cInboxAppendKey)
		READFROMMAP(mDeleteOriginalDraft, cDeleteOriginalDraftKey)
		READFROMMAP(mTemplateDrafts, cTemplateDraftsKey)
		READFROMMAP(mSmartURLPaste, cSmartURLPasteKey)
		READFROMMAP(mWarnReplySubject, cWarnReplySubjectKey)
		READFROMMAP(mSaveOptions, cSaveOptions)
		READFROMMAP(mSaveMailbox, cSaveMailbox)

		READFROMMAP(compose_as, cComposeAs)
		READFROMMAP(enrMultiAltPlain, cEnrMultiAltPlain)
		READFROMMAP(htmlMultiAltEnriched, cHTMLMultiAltEnriched)
		READFROMMAP(enrMultiAltHTML, cEnrMultiAltHTML)
		READFROMMAP(htmlMultiAltPlain, cHTMLMultiAltPlain)
		READFROMMAP(mFormatFlowed, cFormatFlowed)
		READFROMMAP(mAlwaysUnicode, cAlwaysUnicode)
		READFROMMAP(mDisplayIdentityFrom, cDisplayIdentityFrom)
		READFROMMAP(mAutoSaveDrafts, cAutoSaveDraftsKey)
		READFROMMAP(mAutoSaveDraftsInterval, cAutoSaveDraftsIntervalKey)

		// < v1.4 - convert old fields into new default identity
		if (VersionTest(vers_prefs, VERS_1_4_0) < 0)
		{
			cdstring txt;
			theMap->ReadValue(cHeaderKey, txt, vers_prefs);
			txt.FilterOutEscapeChars();
			default_id->SetHeader(txt, !txt.empty());

			theMap->ReadValue(cFooterKey, txt, vers_prefs);
			txt.FilterOutEscapeChars();
			default_id->SetSignature(txt, !txt.empty());

			bool record_outgoing = false;
			theMap->ReadValue(cRecordOutgoingKey, record_outgoing, vers_prefs);
			theMap->ReadValue(cOutMailboxKey, txt, vers_prefs);
			// < v1.3d1
			if (VersionTest(vers_prefs, VERS_1_3_0_D_1) < 0)
			{
				// Prepend first places to search
				cdstring name = firstWD;
				name += txt;
				txt = name;
			}
			// < v1.4
			if (VersionTest(vers_prefs, VERS_1_4_0) < 0)
			{
				// Prepend account name
				cdstring acct_name = mMailAccounts.mValue.front()->GetName();
				acct_name += cMailAccountSeparator;
				acct_name += txt;
				txt = acct_name;
			}
			default_id->SetCopyTo(txt, record_outgoing);
		}
	}

//--------------------Security Prefs
	// Read security prefs
	{
		StMapSection section(theMap, cSecuritySection);

		READFROMMAP(mPreferredPlugin, cPreferredPluginKey)
		READFROMMAP(mUseMIMESecurity, cUseMIMESecurityKey)
		READFROMMAP(mEncryptToSelf, cEncryptToSelfKey)
		READFROMMAP(mCachePassphrase, cCachePassphraseKey)
		READFROMMAP(mCachePassphraseMax, cCachePassphraseMaxKey)
		READFROMMAP(mCachePassphraseIdle, cCachePassphraseIdleKey)
		READFROMMAP(mAutoVerify, cAutoVerifyKey)
		READFROMMAP(mAutoDecrypt, cAutoDecryptKey)
		READFROMMAP(mWarnUnencryptedSend, cWarnUnencryptedSendKey)
		READFROMMAP(mUseErrorAlerts, cUseErrorAlertsKey)
		READFROMMAP(mCacheUserCerts, cCacheUserCertsKey)
		READFROMMAP(mVerifyOKNotification, cVerifyOKNotificationKey)
	}

//--------------------Identities Prefs
	// Read identities prefs
	{
		StMapSection section(theMap, cIdentitiesSection);

		// Try each different version in turn
		// >= v2.0d1
		if (!mIdentities.ReadFromMap(cIdentitiesKey_2_0, theMap, vers_prefs))
		{
			// >= v1.4
			if (!mIdentities.ReadFromMap(cIdentitiesKey_1_4, theMap, temp_1_4_0.parts))
			{
				// < v1.4
				mIdentities.ReadFromMap(cIdentitiesKey_1_3, theMap, temp_1_3_0.parts);

				// Prepend account name to each copy to
				cdstring acct_name = mMailAccounts.mValue.front()->GetName();
				acct_name += cMailAccountSeparator;
				for(CIdentityList::iterator iter = mIdentities.mValue.begin(); iter != mIdentities.mValue.end(); iter++)
				{
					// Add SMTP server account to each identity
					(*iter).SetSMTPAccount(mSMTPAccounts.mValue.front()->GetName(), true);

					// Change copy to
					cdstring txt = (*iter).GetCopyTo();
					if (txt.length())
						txt = acct_name + txt;
					(*iter).SetCopyTo(txt, (*iter).UseCopyTo());
				}

				// prepend default identity
				if (default_id)
				{
					mIdentities.mValue.insert(mIdentities.mValue.begin(), *default_id);
					delete default_id;
				}
			}
			mIdentities.SetDirty();
		}
		READFROMMAP(mTiedMailboxes, cTiedMailboxesKey)
		READFROMMAP(mTiedCalendars, cTiedCalendarsKey)
		READFROMMAP(mContextTied, cContextTiedKey)
		READFROMMAP(mMsgTied, cMsgTiedKey)
		READFROMMAP(mTiedMboxInherit, cTiedMboxInheritKey)
	}

//--------------------Address Book Prefs
	// Read address prefs
	{
		StMapSection section(theMap, cAddressSection);

		// < v1.3d1
		if (VersionTest(vers_prefs, VERS_1_3_0_D_1) < 0)
		{
			// Read old local address books to open
			READFROMMAP(mAdbkOpenAtStartup, cAddressBookKey)

			//Convert to URL scheme
			cdstring local_url = cFileURLScheme;
			local_url += cURLLocalhost;
			for(cdstrvect::iterator iter = mAdbkOpenAtStartup.mValue.begin(); iter != mAdbkOpenAtStartup.mValue.end(); iter++)
				*iter = local_url + *iter;

			// Do nick-name resolution on these as well
			mAdbkNickName = mAdbkOpenAtStartup;

			// Remove old key
			theMap->RemoveValue(cAddressBookKey);
		}
		
		// Try each format in turn
		if (!mAddressAccounts.ReadFromMap(cAddressAccountsKey_2_0, theMap, vers_prefs))
		{
			// >= v1.4
			if (!mAddressAccounts.ReadFromMap(cAddressAccountsKey_1_4, theMap, temp_1_4_0.parts))
				mAddressAccounts.ReadFromMap(cAddressAccountsKey_1_3, theMap, temp_1_3_0.parts);
			mAddressAccounts.SetDirty();
		}

		{
			CPreferenceValueMap<CAddressAccountList> temp;
			READFROMMAP(temp, cLocalAdbkAccount)
			if (temp.GetValue().size())
				mLocalAdbkAccount.Value() = *temp.GetValue().front();
		}

		{
			CPreferenceValueMap<CAddressAccountList> temp;
			READFROMMAP(temp, cOSAdbkAccount)
			if (temp.GetValue().size())
				mOSAdbkAccount.Value() = *temp.GetValue().front();
		}

		READFROMMAP(mExpandedAdbks, cExpandedAdbksKey)

		if (!mAdbkOpenAtStartup.ReadFromMap(cAdbkOpenAtStartup_2_0, theMap, vers_prefs))
		{
			mAdbkOpenAtStartup.ReadFromMap(cAdbkOpenAtStartup_1_3, theMap, vers_prefs);
			mAdbkOpenAtStartup.SetDirty();
		}
		if (!mAdbkNickName.ReadFromMap(cAdbkNickName_2_0, theMap, vers_prefs))
		{
			mAdbkNickName.ReadFromMap(cAdbkNickName_1_3, theMap, vers_prefs);
			mAdbkNickName.SetDirty();
		}
		if (!mAdbkSearch.ReadFromMap(cAdbkSearch_2_0, theMap, vers_prefs))
		{
			mAdbkSearch.ReadFromMap(cAdbkSearch_1_3, theMap, vers_prefs);
			mAdbkSearch.SetDirty();
		}
		if (!mAdbkAutoSync.ReadFromMap(cAdbkAutoSync_2_0, theMap, vers_prefs))
		{
			mAdbkAutoSync.ReadFromMap(cAdbkAutoSync_1_3, theMap, vers_prefs);
			mAdbkAutoSync.SetDirty();
		}

		READFROMMAP(mAdbkACLStyles, cAdbkACLStyleListKey)
		READFROMMAP(mOptionKeyAddressDialog, cOptionAddressDialogKey)
		READFROMMAP(mExpandNoNicknames, cExpandNoNicknamesKey)
		READFROMMAP(mExpandFailedNicknames, cExpandFailedNicknamesKey)
		READFROMMAP(mExpandFullName, cExpandFullNameKey)
		READFROMMAP(mExpandNickName, cExpandNickNameKey)
		READFROMMAP(mExpandEmail, cExpandEmailKey)
		READFROMMAP(mSkipLDAP, cSkipLDAPKey)
		READFROMMAP(mCaptureAddressBook, cCaptureAddressBookKey)
		READFROMMAP(mCaptureAllowEdit, cCaptureAllowEditKey)
		READFROMMAP(mCaptureAllowChoice, cCaptureAllowChoiceKey)
		READFROMMAP(mCaptureRead, cCaptureReadKey)
		READFROMMAP(mCaptureRespond, cCaptureRespondKey)
		READFROMMAP(mCaptureFrom, cCaptureFromKey)
		READFROMMAP(mCaptureCc, cCaptureCcKey)
		READFROMMAP(mCaptureReplyTo, cCaptureReplyToKey)
		READFROMMAP(mCaptureTo, cCaptureToKey)
	}

//--------------------Calendar Prefs
	// Write calendar prefs
	{
		StMapSection section(theMap, cCalendarSection);

		READFROMMAP(mCalendarAccounts, cCalendarAccountsKey)
		{
			CPreferenceValueMap<CCalendarAccountList> temp;
			READFROMMAP(temp, cLocalCalendarAccountKey)
			if (temp.GetValue().size())
				mLocalCalendarAccount.Value() = *temp.GetValue().front();
		}
		{
			CPreferenceValueMap<CCalendarAccountList> temp;
			READFROMMAP(temp, cWebCalendarAccountKey)
			if (temp.GetValue().size())
				mWebCalendarAccount.Value() = *temp.GetValue().front();
		}

		READFROMMAP(mSubscribedCalendars, cSubscribedCalendarsKey)
		READFROMMAP(mExpandedCalendars, cExpandedCalendarsKey)
		READFROMMAP(mCalendarColours, cCalendarColoursKey)
		READFROMMAP(mTimezone, cTimezoneKey)
		READFROMMAP(mFavouriteTimezones, cFavouriteTimezonesKey)
		READFROMMAP(mWeekStartDay, cWeekStartDayKey)
		READFROMMAP(mWorkDayMask, cWorkDayMaskKey)
		READFROMMAP(mDayWeekTimeRanges, cDayWeekTimeRangesKey)
		READFROMMAP(mDisplayTime, cDisplayTimeKey)
		READFROMMAP(m24HourTime, c24HourTimeKey)
		READFROMMAP(mHandleICS, cHandleICSKey)
		READFROMMAP(mAutomaticIMIP, cAutomaticIMIPKey)
		READFROMMAP(mAutomaticEDST, cAutomaticEDSTKey)
		READFROMMAP(mShowUID, cShowUIDKey)
		READFROMMAP(mDefaultCalendar, cDefaultCalendarKey)
	}

//--------------------Attachment Prefs
	// Read attachment prefs
	{
		StMapSection section(theMap, cAttachmentsSection);

		READFROMMAP(mDefault_mode, cDefaultModeKey)
		READFROMMAP(mDefault_Always, cDefaultAlwaysKey)
		READFROMMAP(mViewDoubleClick, cViewDoubleClickKey)
		READFROMMAP(mAskDownload, cAskDownloadKey)
		if (!mDefaultDownload.ReadFromMap(cDefaultDownloadKey_2_0, theMap, vers_prefs))
		{
			mDefaultDownload.ReadFromMap(cDefaultDownloadKey_1_2, theMap, vers_prefs);
			mDefaultDownload.SetDirty();
		}
		READFROMMAP(mAppLaunch, cAppLaunchKey)
		READFROMMAP(mLaunchText, cLaunchTextKey)
#if __dest_os == __win32_os
		READFROMMAP(mShellLaunch, cShellLaunchKey)
#endif
		READFROMMAP(mMIMEMappings, cMIMEMapKey)
		READFROMMAP(mExplicitMapping, cExplicitMappingKey)

		READFROMMAP(mWarnMissingAttachments, cWarnMissingAttachmentsKey)
		READFROMMAP(mMissingAttachmentSubject, cMissingAttachmentSubjectKey)
		READFROMMAP(mMissingAttachmentWords, cMissingAttachmentWordsKey)
	}

#ifdef __use_speech
//--------------------Speech Prefs
	{
		StMapSection section(theMap, cSpeechSection);

		READFROMMAP(mSpeakNewOpen, cSpeakNewOpenKey)
		READFROMMAP(mSpeakNewArrivals, cSpeakNewArrivalsKey)
#ifdef __MULBERRY_CONFIGURE_V2_0
		// Special to support v2.0.x admin tool
		READFROMMAP(mSpeakNewAlert, cSpeakNewAlertKey)
		READFROMMAP(mSpeakNewAlertTxt, cSpeakNewAlertTxtKey)
#endif
		READFROMMAP(mSpeakMessageItems, cSpeakMessageItemsKey)
		READFROMMAP(mSpeakLetterItems, cSpeakLetterItemsKey)
		READFROMMAP(mSpeakMessageEmptyItem, cSpeakEmptyItemKey)
		READFROMMAP(mSpeakMessageMaxLength, cSpeakMaxLengthKey)
	}
#endif

//--------------------Miscellaneous Prefs
	{
		StMapSection section(theMap, cMiscellaneousSection);

		READFROMMAP(mTextMacros, cTextMacrosKey)
		READFROMMAP(mAllowKeyboardShortcuts, cAllowKeyboardShortcutsKey)
	}

#ifdef __MULBERRY
	// Sync with InternetConfig if required
	CheckIC();
#endif

	// Reset all window states first then read them in
	ResetAllStates(false);
	ReadWindowInfoFromMap(theMap, vers_app, vers_prefs);

#ifdef __MULBERRY
	// Now do plugins
	mUpdateVers = CPluginManager::sPluginManager.ReadFromMap(theMap, vers_app, vers_prefs) | mUpdateVers;
#endif

#ifdef __MULBERRY
	// Force Admin locks
	CAdminLock::sAdminLock.LockPrefs(this);
#endif

#ifdef __MULBERRY
	// Do change if this is the default one
	if (sPrefs == this)
		Changed();
#endif

	return mUpdateVers;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#pragma dont_inline off
#endif

// Write window state data to map
void CPreferences::WriteWindowInfoToMap(COptionsMap* theMap, bool dirty_only)
{
	// Write all infos to map
	WRITETOMAP(mServerWindowInfo, cServerInfoSection_2_0)
	WRITETOMAP(mMailboxWindowInfo, cMailboxInfoSection_2_0)
	WRITETOMAP(mNewAddressBookWindowInfo, cAddressBookInfoSection_2_1)
	WRITETOMAP(mCalendarWindowInfo, cCalendarInfoSection_4_0)
	{
		StMapSection section(theMap, cDefaultWindowsSection);

		// Write each default window type to map
		// For v1.3 compatibility MUST NOT QUOTE
#if __dest_os == __win32_os
		if (!dirty_only || mMDIWindowDefault.IsDirty())
			theMap->WriteValue(cMDIWindowKey_2_0, mMDIWindowDefault.mValue);
#endif
		if (!dirty_only || m3PaneWindowDefault.IsDirty())
			theMap->WriteValue(c3PaneWindowKey, m3PaneWindowDefault.mValue);
		if (!dirty_only || m3PaneOptions.IsDirty())
			theMap->WriteValue(c3PaneOptionsKey_v4_0, m3PaneOptions.mValue);
		if (!dirty_only || m3PaneOpenMailboxes.IsDirty())
			theMap->WriteValue(c3PaneOpenMailboxesKey, m3PaneOpenMailboxes.mValue);
		if (!dirty_only || m3PaneOpenAdbks.IsDirty())
			theMap->WriteValue(c3PaneOpenAdbksKey, m3PaneOpenAdbks.mValue);
		if (!dirty_only || m1PaneOptions.IsDirty())
			theMap->WriteValue(c1PaneOptionsKey_v4_0, m1PaneOptions.mValue);
		if (!dirty_only || mStatusWindowDefault.IsDirty())
			theMap->WriteValue(cStatusWindowKey_2_0, mStatusWindowDefault.mValue);
		if (!dirty_only || mServerWindowDefault.IsDirty())
			theMap->WriteValue(cServerWindowKey_2_0, mServerWindowDefault.mValue);
		if (!dirty_only || mServerBrowseDefault.IsDirty())
			theMap->WriteValue(cServerBrowseKey_2_0, mServerBrowseDefault.mValue);
		if (!dirty_only || mMailboxWindowDefault.IsDirty())
			theMap->WriteValue(cMailboxWindowKey_2_0, mMailboxWindowDefault.mValue);
		if (!dirty_only || mSMTPWindowDefault.IsDirty())
			theMap->WriteValue(cSMTPWindowKey, mSMTPWindowDefault.mValue);
		if (!dirty_only || mMessageWindowDefault.IsDirty())
			theMap->WriteValue(cMessageWindowKey_2_0, mMessageWindowDefault.mValue);
		if (!dirty_only || mMessageView3Pane.IsDirty())
			theMap->WriteValue(cMessageView3PaneKey_2_0, mMessageView3Pane.mValue);
		if (!dirty_only || mMessageView1Pane.IsDirty())
			theMap->WriteValue(cMessageView1PaneKey_2_0, mMessageView1Pane.mValue);
		if (!dirty_only || mLetterWindowDefault.IsDirty())
			theMap->WriteValue(cLetterWindowKey_2_0, mLetterWindowDefault.mValue);
		if (!dirty_only || mNewAddressBookWindowDefault.IsDirty())
			theMap->WriteValue(cAddressBookWindowKey_2_1, mNewAddressBookWindowDefault.mValue);
		if (!dirty_only || mAdbkManagerWindowDefault.IsDirty())
			theMap->WriteValue(cAdbkManagerWindowKey_2_0, mAdbkManagerWindowDefault.mValue);
		if (!dirty_only || mAdbkSearchWindowDefault.IsDirty())
			theMap->WriteValue(cAdbkSearchWindowKey_2_0, mAdbkSearchWindowDefault.mValue);
		if (!dirty_only || mFindReplaceWindowDefault.IsDirty())
			theMap->WriteValue(cFindReplaceWindowKey_2_0, mFindReplaceWindowDefault.mValue);
		if (!dirty_only || mSearchWindowDefault.IsDirty())
			theMap->WriteValue(cSearchWindowKey_2_0, mSearchWindowDefault.mValue);
		if (!dirty_only || mRulesWindowDefault.IsDirty())
			theMap->WriteValue(cRulesWindowKey_2_0, mRulesWindowDefault.mValue);
		if (!dirty_only || mRulesDialogDefault.IsDirty())
			theMap->WriteValue(cRulesDialogKey_2_0, mRulesDialogDefault.mValue);
		if (!dirty_only || mCalendarStoreWindowDefault.IsDirty())
			theMap->WriteValue(cCalendarStoreWindowKey_4_0, mCalendarStoreWindowDefault.mValue);
		if (!dirty_only || mCalendarWindowDefault.IsDirty())
			theMap->WriteValue(cCalendarWindowKey_4_0, mCalendarWindowDefault.mValue);
	}
}

// Read data from map
void CPreferences::ReadWindowInfoFromMap(COptionsMap* theMap, NumVersion vers_app, NumVersion vers_prefs)
{
	// < v1.3d1
	if (VersionTest(vers_prefs, VERS_1_3_0_D_1) < 0)
	{
		{
			StMapSection section(theMap, cServerInfoSection_1_3);

			// Read all infos from map
			int i = 0;
			cdstring key;
			while(theMap->EnumValue(i++, key))
			{
				CServerWindowState* info = new CServerWindowState;
				info->GetName() = key;
				cdstring value;
				theMap->ReadValue(key, value, vers_prefs);
				info->SetInfo_Old(value, vers_prefs);
				theMap->RemoveValue(key);

				// Add to list
				mServerWindowInfo.mValue.push_back(info);
			}

			theMap->ClearSection();
		}
		{
			StMapSection section(theMap, cMailboxInfoSection_1_3);

			// Read all infos from map
			int i = 0;
			cdstring key;
			while(theMap->EnumValue(i++, key))
			{
				CMailboxWindowState* info = new CMailboxWindowState;
				info->GetName() = key;
				cdstring value;
				theMap->ReadValue(key, value, vers_prefs);
				info->SetInfo_Old(value, vers_prefs);
				theMap->RemoveValue(key);

				// Add to list
				mMailboxWindowInfo.mValue.push_back(info);
			}

			theMap->ClearSection();
		}
		{
			StMapSection section(theMap, cAddressBookInfoSection_1_0);

			// Read all infos from map
			int i = 0;
			cdstring key;
			while(theMap->EnumValue(i++, key))
			{
				CAddressBookWindowState* info = new CAddressBookWindowState;
				info->GetName() = key;
				cdstring value;
				theMap->ReadValue(key, value, vers_prefs);
				info->SetInfo_Old(value, vers_prefs);
				theMap->RemoveValue(key);

				// Add to list
#if 0
				mAddressBookWindowInfo.mValue.push_back(info);
#endif
			}

			theMap->ClearSection();
		}
	}
	else
	{
		// Read all infos from map
		NumVersionVariant vers2_0;
		vers2_0.parts = vers_prefs;
		NumVersionVariant vers1_4;
		vers1_4.whole = VERS_1_4_0;
		NumVersionVariant vers1_3;
		vers1_3.whole = VERS_1_3_0;
		NumVersionVariant vers3_0;
		vers3_0.whole = VERS_3_0_0;
		if (!mServerWindowInfo.ReadFromMap(cServerInfoSection_2_0, theMap, vers2_0.parts))
		{
			if (mServerWindowInfo.ReadFromMap(cServerInfoSection_1_4, theMap, vers1_4.parts) ||
				mServerWindowInfo.ReadFromMap(cServerInfoSection_1_3, theMap, vers1_3.parts))
				mServerWindowInfo.SetDirty();
		}
		if (!mMailboxWindowInfo.ReadFromMap(cMailboxInfoSection_2_0, theMap, vers2_0.parts))
		{
			if (mMailboxWindowInfo.ReadFromMap(cMailboxInfoSection_1_4, theMap, vers1_4.parts) ||
				mMailboxWindowInfo.ReadFromMap(cMailboxInfoSection_1_3, theMap, vers1_3.parts))
				mMailboxWindowInfo.SetDirty();
		}
		mNewAddressBookWindowInfo.ReadFromMap(cAddressBookInfoSection_2_1, theMap, vers2_0.parts);
		mCalendarWindowInfo.ReadFromMap(cCalendarInfoSection_4_0, theMap, vers2_0.parts);

		{
			StMapSection section(theMap, cDefaultWindowsSection);

			// Read each default window type from map
			cdstring info;
			NumVersion vers = vers2_0.parts;
#if __dest_os == __win32_os
			if ((vers = vers2_0.parts, theMap->ReadValue(cMDIWindowKey_2_0, mMDIWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cMDIWindowKey_1_2, mMDIWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mMDIWindowDefault.SetDirty();
			}
#endif

			theMap->ReadValue(c3PaneWindowKey, m3PaneWindowDefault.mValue, vers_prefs);
			if (!theMap->ReadValue(c3PaneOptionsKey_v4_0, m3PaneOptions.mValue, vers_prefs))
				theMap->ReadValue(c3PaneOptionsKey_v3_0, m3PaneOptions.mValue, vers3_0.parts);
			theMap->ReadValue(c3PaneOpenMailboxesKey, m3PaneOpenMailboxes.mValue, vers_prefs);
			theMap->ReadValue(c3PaneOpenAdbksKey, m3PaneOpenAdbks.mValue, vers_prefs);
			if (!theMap->ReadValue(c1PaneOptionsKey_v4_0, m1PaneOptions.mValue, vers_prefs))
				theMap->ReadValue(c1PaneOptionsKey_v3_0, m1PaneOptions.mValue, vers3_0.parts);

			if ((vers = vers2_0.parts, theMap->ReadValue(cStatusWindowKey_2_0, mStatusWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cStatusWindowKey_1_0, mStatusWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mStatusWindowDefault.SetDirty();
			}

			if ((vers = vers2_0.parts, theMap->ReadValue(cServerWindowKey_2_0, mServerWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cServerWindowKey_1_4, mServerWindowDefault.mValue, vers)) ||
				(vers = vers1_3.parts, theMap->ReadValue(cServerWindowKey_1_3, mServerWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mServerWindowDefault.SetDirty();
			}

			if (theMap->ReadValue(cServerBrowseKey_2_0, mServerBrowseDefault.mValue, vers))
				mServerBrowseDefault.SetDirty();

			if ((vers = vers2_0.parts, theMap->ReadValue(cMailboxWindowKey_2_0, mMailboxWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cMailboxWindowKey_1_4, mMailboxWindowDefault.mValue, vers)) ||
				(vers = vers1_3.parts, theMap->ReadValue(cMailboxWindowKey_1_3, mMailboxWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mMailboxWindowDefault.SetDirty();
			}

			theMap->ReadValue(cSMTPWindowKey, mSMTPWindowDefault.mValue, vers_prefs);

			if ((vers = vers2_0.parts, theMap->ReadValue(cMessageWindowKey_2_0, mMessageWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cMessageWindowKey_1_0, mMessageWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mMessageWindowDefault.SetDirty();
			}

			theMap->ReadValue(cMessageView3PaneKey_2_0, mMessageView3Pane.mValue, vers_prefs);

			theMap->ReadValue(cMessageView1PaneKey_2_0, mMessageView1Pane.mValue, vers_prefs);

			if ((vers = vers2_0.parts, theMap->ReadValue(cLetterWindowKey_2_0, mLetterWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cLetterWindowKey_1_0, mLetterWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mLetterWindowDefault.SetDirty();
			}

			theMap->ReadValue(cAddressBookWindowKey_2_1, mNewAddressBookWindowDefault.mValue, vers_prefs);

			if ((vers = vers2_0.parts, theMap->ReadValue(cAdbkManagerWindowKey_2_0, mAdbkManagerWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cAdbkManagerWindowKey_1_4, mAdbkManagerWindowDefault.mValue, vers)) ||
				(vers = vers1_3.parts, theMap->ReadValue(cAdbkManagerWindowKey_1_3, mAdbkManagerWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mAdbkManagerWindowDefault.SetDirty();
			}

			if ((vers = vers2_0.parts, theMap->ReadValue(cAdbkSearchWindowKey_2_0, mAdbkSearchWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cAdbkSearchWindowKey_1_4, mAdbkSearchWindowDefault.mValue, vers)) ||
				(vers = vers1_3.parts, theMap->ReadValue(cAdbkSearchWindowKey_1_3, mAdbkSearchWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mAdbkSearchWindowDefault.SetDirty();
			}

			if ((vers = vers2_0.parts, theMap->ReadValue(cFindReplaceWindowKey_2_0, mFindReplaceWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cFindReplaceWindowKey_1_4, mFindReplaceWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mFindReplaceWindowDefault.SetDirty();
			}

			if ((vers = vers2_0.parts, theMap->ReadValue(cSearchWindowKey_2_0, mSearchWindowDefault.mValue, vers)) ||
				(vers = vers1_4.parts, theMap->ReadValue(cSearchWindowKey_1_4, mSearchWindowDefault.mValue, vers)))
			{
				if (VersionTest(vers, vers2_0.whole) < 0)
					mSearchWindowDefault.SetDirty();
			}

			theMap->ReadValue(cRulesWindowKey_2_0, mRulesWindowDefault.mValue, vers_prefs);
			theMap->ReadValue(cRulesDialogKey_2_0, mRulesDialogDefault.mValue, vers_prefs);
			theMap->ReadValue(cCalendarStoreWindowKey_4_0, mCalendarStoreWindowDefault.mValue, vers_prefs);
			theMap->ReadValue(cCalendarWindowKey_4_0, mCalendarWindowDefault.mValue, vers_prefs);
		}
	}
}

#pragma mark ____________________________Window Info

// Add server window info
void CPreferences::AddServerWindowInfo(CServerWindowState* state)
{
	bool list_changed;
	bool added = state->MergeToList(mServerWindowInfo.mValue, mServerWindowDefault.mValue, list_changed);

	if (!added)
		delete state;

	// Mark as changed
	if (list_changed)
		mServerWindowInfo.SetDirty();
}

// Get server window info
CServerWindowState* CPreferences::GetServerWindowInfo(const char* name)
{
	// Check for duplicate
	for(CWindowStateArray::const_iterator iter = mServerWindowInfo.mValue.begin(); iter != mServerWindowInfo.mValue.end(); iter++)
	{
		// Check for same
		if (**iter == name)
			return (CServerWindowState*) *iter;
	}

	return nil;
}

// Add mailbox window info
void CPreferences::AddMailboxWindowInfo(CMailboxWindowState* state)
{
	bool list_changed;
	bool added = state->MergeToList(mMailboxWindowInfo.mValue, mMailboxWindowDefault.mValue, list_changed);

	if (!added)
		delete state;

	// Mark as changed
	if (list_changed)
		mMailboxWindowInfo.SetDirty();
}

// Get mailbox window info
CMailboxWindowState* CPreferences::GetMailboxWindowInfo(const char* name)
{
	// Check for duplicate
	for(CWindowStateArray::const_iterator iter = mMailboxWindowInfo.mValue.begin(); iter != mMailboxWindowInfo.mValue.end(); iter++)
	{
		// Check for same
		if (**iter == name)
			return (CMailboxWindowState*) *iter;
	}

	return nil;
}

// Add address book window info
void CPreferences::AddAddressBookWindowInfo(CNewAddressBookWindowState* state)
{
	bool list_changed;
	bool added = state->MergeToList(mNewAddressBookWindowInfo.mValue, mNewAddressBookWindowDefault.mValue, list_changed);

	if (!added)
		delete state;

	// Mark as changed
	if (list_changed)
		mNewAddressBookWindowInfo.SetDirty();
}

// Get address book window info
CNewAddressBookWindowState* CPreferences::GetAddressBookWindowInfo(const char* name)
{
	// Check for duplicate
	for(CWindowStateArray::const_iterator iter = mNewAddressBookWindowInfo.mValue.begin(); iter != mNewAddressBookWindowInfo.mValue.end(); iter++)
	{
		// Check for same
		if (**iter == name)
			return (CNewAddressBookWindowState*) *iter;
	}

	return nil;
}

// Add calendar window info
void CPreferences::AddCalendarWindowInfo(CCalendarWindowState* state)
{
	bool list_changed;
	bool added = state->MergeToList(mCalendarWindowInfo.mValue, mCalendarWindowDefault.mValue, list_changed);

	if (!added)
		delete state;

	// Mark as changed
	if (list_changed)
		mCalendarWindowInfo.SetDirty();
}

// Get address book window info
CCalendarWindowState* CPreferences::GetCalendarWindowInfo(const char* name)
{
	// Check for duplicate
	for(CWindowStateArray::const_iterator iter = mCalendarWindowInfo.mValue.begin(); iter != mCalendarWindowInfo.mValue.end(); iter++)
	{
		// Check for same
		if (**iter == name)
			return (CCalendarWindowState*) *iter;
	}

	return nil;
}

// Reset all state info
void CPreferences::ResetAllStates(bool dirty)
{
	// Removed named entries
	for(CWindowStateArray::iterator iter = mServerWindowInfo.mValue.begin(); iter != mServerWindowInfo.mValue.end(); iter++)
		delete *iter;
	mServerWindowInfo.mValue.clear();
	mServerWindowInfo.SetDirty(dirty);

	for(CWindowStateArray::iterator iter = mMailboxWindowInfo.mValue.begin(); iter != mMailboxWindowInfo.mValue.end(); iter++)
		delete *iter;
	mMailboxWindowInfo.mValue.clear();
	mMailboxWindowInfo.SetDirty(dirty);

	for(CWindowStateArray::iterator iter = mNewAddressBookWindowInfo.mValue.begin(); iter != mNewAddressBookWindowInfo.mValue.end(); iter++)
		delete *iter;
	mNewAddressBookWindowInfo.mValue.clear();
	mNewAddressBookWindowInfo.SetDirty(dirty);

	for(CWindowStateArray::iterator iter = mCalendarWindowInfo.mValue.begin(); iter != mCalendarWindowInfo.mValue.end(); iter++)
		delete *iter;
	mCalendarWindowInfo.mValue.clear();
	mCalendarWindowInfo.SetDirty(dirty);

	// Reset defaults
	InitWindowPrefs();
	mStatusWindowDefault.SetDirty(dirty);
#if __dest_os == __win32_os
	mMDIWindowDefault.SetDirty(dirty);
#endif
	m3PaneWindowDefault.SetDirty(dirty);
	m3PaneOptions.SetDirty(dirty);
	m1PaneOptions.SetDirty(dirty);
	mServerWindowDefault.SetDirty(dirty);
	mServerBrowseDefault.SetDirty(dirty);
	mMailboxWindowDefault.SetDirty(dirty);
	mSMTPWindowDefault.SetDirty(dirty);
	mMessageWindowDefault.SetDirty(dirty);
	mMessageView3Pane.SetDirty(dirty);
	mMessageView1Pane.SetDirty(dirty);
	mLetterWindowDefault.SetDirty(dirty);
#if 0
	mAddressBookWindowDefault.SetDirty(dirty);
#else
	mNewAddressBookWindowDefault.SetDirty(dirty);
#endif
	mAdbkManagerWindowDefault.SetDirty(dirty);
	mAdbkSearchWindowDefault.SetDirty(dirty);
	mFindReplaceWindowDefault.SetDirty(dirty);
	mSearchWindowDefault.SetDirty(dirty);
	mRulesWindowDefault.SetDirty(dirty);
	mRulesDialogDefault.SetDirty(dirty);
	mCalendarStoreWindowDefault.SetDirty(dirty);
	mCalendarWindowDefault.SetDirty(dirty);
}

// Test for a smart address
bool CPreferences::TestSmartAddress(CAddress& addr)
{
	// Compare with SMTP return address first (first identity)
	cdstring addr_email = addr.GetMailAddress();
	if (!::strcmpnocase(addr_email, sFromAddr))
		return true;

	// Compare against smart address list
	for(cdstrvect::const_iterator iter = sPrefs->mSmartAddressList.mValue.begin(); iter != sPrefs->mSmartAddressList.mValue.end(); iter++)
	{
		if (::strpmatch(addr_email, *iter))
			return true;
	}

	return false;
}
