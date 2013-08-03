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

#include "CFilterManager.h"

// Copy constructor
void CPreferences::_copy(const CPreferences &copy)
{
#define COPY(x) x = copy.x;

	// Copy from CPreferences
	COPY(vers)
	COPY(mUpdateVers)

//--------------------Prefs state
	COPY(mHelpState)
	COPY(mDetailedTooltips)
	COPY(mUse_IC)
	COPY(mCheckDefaultMailClient)
	COPY(mCheckDefaultWebcalClient)
	COPY(mWarnMailtoFiles)
	COPY(mSimple)
	COPY(mLastPanel)
	COPY(mDisconnected)
	COPY(mSMTPQueueEnabled)
	COPY(mConnectOptions)
	COPY(mDisconnectOptions)

//--------------------Network Control Prefs
	COPY(mAppIdleTimeout)
	COPY(mWaitDialogStart)
	COPY(mWaitDialogMax)
	COPY(mConnectRetryTimeout)
	COPY(mConnectRetryMaxCount)
	COPY(mBackgroundTimeout)
	COPY(mTickleInterval)
	COPY(mTickleIntervalExpireTimeout)
	COPY(mSleepReconnect)

//--------------------Account Prefs
	COPY(mMailAccounts)
	COPY(mSMTPAccounts)
	COPY(mRemoteAccounts)
	COPY(mSIEVEAccounts)
	COPY(mMailDomain)
	COPY(mRemoteCachePswd)
	COPY(mOSDefaultLocation)
	COPY(mDisconnectedCWD)

//--------------------Notification Prefs
	COPY(mMailNotification)
	COPY(mAttachmentNotification)

//--------------------Display Prefs
	COPY(mServerOpenStyle)
	COPY(mServerClosedStyle)
	COPY(mServerBkgndStyle)
	COPY(mFavouriteBkgndStyle)
	COPY(mHierarchyBkgndStyle)

	COPY(mMboxRecentStyle)
	COPY(mMboxUnseenStyle)
	COPY(mMboxOpenStyle)
	COPY(mMboxFavouriteStyle)
	COPY(mMboxClosedStyle)
	COPY(mIgnoreRecent)

	COPY(unseen)
	COPY(seen)
	COPY(answered)
	COPY(important)
	COPY(deleted)
	COPY(mMultiAddress)
	COPY(mMatch)
	COPY(mNonMatch)
	COPY(mUseLocalTimezone)

	COPY(mLabels)
	COPY(mIMAPLabels)

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	COPY(mListTextTraits)
	COPY(mDisplayTextTraits)
	COPY(mPrintTextTraits)
	COPY(mCaptionTextTraits)
	COPY(mAntiAliasFont)
#elif __dest_os == __win32_os
	COPY(mListTextFontInfo)
	COPY(mDisplayTextFontInfo)
	COPY(mPrintTextFontInfo)
	COPY(mCaptionTextFontInfo)
#elif __dest_os == __linux_os
	COPY(mListTextFontInfo)
	COPY(mDisplayTextFontInfo)
	COPY(mPrintTextFontInfo)
	COPY(mCaptionTextFontInfo)
#else
#error __dest_os
#endif

#if __dest_os == __win32_os
	COPY(mMultiTaskbar)
#endif

	COPY(mUse3Pane)
	COPY(mToolbarShow)
	COPY(mToolbarSmallIcons)
	COPY(mToolbarShowIcons)
	COPY(mToolbarShowCaptions)
	COPY(mToolbars)

//-------------------Formatting Prefs
	COPY(mURLStyle)
	COPY(mURLSeenStyle)
	COPY(mHeaderStyle)
	COPY(mQuotationStyle)
	COPY(mTagStyle)

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	COPY(mFixedTextTraits)
	COPY(mHTMLTextTraits)
#elif __dest_os == __win32_os
	COPY(mFixedTextFontInfo)
	COPY(mHTMLTextFontInfo)
#elif __dest_os == __linux_os
	COPY(mFixedTextFontInfo)
	COPY(mHTMLTextFontInfo)
#else
#error __dest_os
#endif
	COPY(mUseStyles)
	COPY(mMinimumFont)

	COPY(mUseMultipleQuotes)
	COPY(mQuoteColours)
	COPY(mRecognizeQuotes)
	COPY(mRecognizeURLs)

#if __dest_os == __linux_os
	COPY(mSelectionColour)
#endif

//--------------------Mailbox Prefs
	COPY(openAtFirst)
	COPY(openAtLast)
	COPY(openAtFirstNew)
	COPY(mNoOpenPreview)
	COPY(mUnseenNew)
	COPY(mRecentNew)
	COPY(mNextIsNewest)
	COPY(expungeOnClose)
	COPY(warnOnExpunge)
	COPY(mWarnPuntUnseen)
	COPY(mDoRollover)
	COPY(mRolloverWarn)
	COPY(mDoMailboxClear)
	COPY(clear_mailbox)
	COPY(clear_warning)
	COPY(mRLoCache)
	COPY(mRHiCache)
	COPY(mRUseHiCache)
	COPY(mRCacheIncrement)
	COPY(mRAutoCacheIncrement)
	COPY(mRSortCache)
	COPY(mLLoCache)
	COPY(mLHiCache)
	COPY(mLUseHiCache)
	COPY(mLCacheIncrement)
	COPY(mLAutoCacheIncrement)
	COPY(mLSortCache)
	COPY(mSmartAddressList)
	COPY(mUseCopyToMboxes)
	COPY(mUseAppendToMboxes)
	COPY(mMRUMaximum)
	COPY(mMboxACLStyles)
	COPY(mSearchStyles)
	COPY(mPromptDisconnected)
	COPY(mAutoDial)
	COPY(mUIDValidityMismatch)
	COPY(mQuickSearch)
	COPY(mQuickSearchVisible)
	COPY(mScrollForUnseen)

//--------------------Favourite Prefs
	COPY(mFavourites)
	COPY(mMRUCopyTo)
	COPY(mMRUAppendTo)
	COPY(mMailboxAliases)

//--------------------Search Prefs
	COPY(mMultipleSearch)
	COPY(mNumberMultipleSearch)
	COPY(mLoadBalanceSearch)
	COPY(mOpenFirstSearchResult)

//--------------------Rules Prefs
	mFilterManager = new CFilterManager(*copy.mFilterManager);
	mFilterManager->PrefsInit();

//--------------------Message Prefs
	COPY(showMessageHeader)
	COPY(saveMessageHeader)
	COPY(mSaveCreator)
	COPY(deleteAfterCopy)
	COPY(mOpenDeleted)
	COPY(mCloseDeleted)
	COPY(mDoSizeWarn)
	COPY(warnMessageSize)
	COPY(mQuoteSelection)
	COPY(mAlwaysQuote)
	COPY(optionKeyReplyDialog)
	COPY(showStyled)
	COPY(mHeaderCaption)
	COPY(mHeaderBox)
	COPY(mPrintSummary)
	COPY(mFooterCaption)
	COPY(mFooterBox)
	COPY(mOpenReuse)
	COPY(mAutoDigest)
	COPY(mExpandHeader)
	COPY(mExpandParts)
	COPY(mForwardChoice)
	COPY(mForwardQuoteOriginal)
	COPY(mForwardHeaders)
	COPY(mForwardAttachment)
	COPY(mForwardRFC822)
	COPY(mMDNOptions)

//--------------------Draft Prefs
	COPY(mAutoInsertSignature)
	COPY(mSignatureEmptyLine)
	COPY(mSigDashes)
	COPY(mNoSubjectWarn)
	COPY(spaces_per_tab)
	COPY(mTabSpace)
	COPY(mReplyQuote)
	COPY(mReplyStart)
	COPY(mReplyEnd)
	COPY(mReplyCursorTop)
	COPY(mReplyNoSignature)
	COPY(mForwardQuote)
	COPY(mForwardStart)
	COPY(mForwardEnd)
	COPY(mForwardCursorTop)
	COPY(mForwardSubject)
	COPY(mLtrHeaderCaption)
	COPY(mLtrFooterCaption)
	COPY(mSeparateBCC)
	COPY(mBCCCaption)
	COPY(mShowCCs)
	COPY(mShowBCCs)
	COPY(mExternalEditor)
	COPY(mUseExternalEditor)
	COPY(mExternalEditAutoStart)
	COPY(wrap_length)
	COPY(mWindowWrap)
	COPY(mDisplayAttachments)
	COPY(mAppendDraft)
	COPY(inbox_append)
	COPY(mDeleteOriginalDraft)
	COPY(mTemplateDrafts)
	COPY(mSmartURLPaste)
	COPY(mWarnReplySubject)
	COPY(mSaveOptions)
	COPY(mSaveMailbox)
	COPY(compose_as)
	COPY(enrMultiAltPlain)
	COPY(htmlMultiAltEnriched)
	COPY(enrMultiAltHTML)
	COPY(htmlMultiAltPlain)
	COPY(mFormatFlowed)
	COPY(mAlwaysUnicode)
	COPY(mDisplayIdentityFrom)
	COPY(mAutoSaveDrafts)
	COPY(mAutoSaveDraftsInterval)

//--------------------Security
	COPY(mPreferredPlugin)
	COPY(mUseMIMESecurity)
	COPY(mEncryptToSelf)
	COPY(mCachePassphrase)
	COPY(mCachePassphraseMax)
	COPY(mCachePassphraseIdle)
	COPY(mAutoVerify)
	COPY(mAutoDecrypt)
	COPY(mWarnUnencryptedSend)
	COPY(mUseErrorAlerts)
	COPY(mCacheUserCerts)
	COPY(mVerifyOKNotification)

//--------------------Identity
	COPY(mIdentities)
	mTiedMailboxes.mValue.SetIdentityList(&mIdentities.mValue);
	mTiedMailboxes.mValue.SetType(CIdentity::eMbox);
	COPY(mTiedMailboxes.mACL)
	mTiedCalendars.mValue.SetIdentityList(&mIdentities.mValue);
	mTiedCalendars.mValue.SetType(CIdentity::eCal);
	COPY(mTiedCalendars.mACL)
	COPY(mContextTied)
	COPY(mMsgTied)
	COPY(mTiedMboxInherit)

//--------------------Address Prefs
	COPY(mAddressAccounts)
	COPY(mLocalAdbkAccount)
	COPY(mOSAdbkAccount)
	COPY(mExpandedAdbks)
	COPY(mAdbkOpenAtStartup)
	COPY(mAdbkNickName)
	COPY(mAdbkSearch)
	COPY(mAdbkAutoSync)
	COPY(mAdbkACLStyles)
	COPY(mOptionKeyAddressDialog)
	COPY(mExpandNoNicknames)
	COPY(mExpandFailedNicknames)
	COPY(mExpandFullName)
	COPY(mExpandNickName)
	COPY(mExpandEmail)
	COPY(mSkipLDAP)
	COPY(mCaptureAddressBook)
	COPY(mCaptureAllowEdit)
	COPY(mCaptureAllowChoice)
	COPY(mCaptureRead)
	COPY(mCaptureRespond)
	COPY(mCaptureFrom)
	COPY(mCaptureCc)
	COPY(mCaptureReplyTo)
	COPY(mCaptureTo)

//--------------------Calendar Prefs
	COPY(mCalendarAccounts)
	COPY(mLocalCalendarAccount)
	COPY(mWebCalendarAccount)
	COPY(mSubscribedCalendars)
	COPY(mExpandedCalendars)
	COPY(mCalendarColours)
	COPY(mTimezone)
	COPY(mFavouriteTimezones)
	COPY(mWeekStartDay)
	COPY(mWorkDayMask)
	COPY(mDayWeekTimeRanges)
	COPY(mDisplayTime)
	COPY(m24HourTime)
	COPY(mHandleICS)
	COPY(mAutomaticIMIP)
	COPY(mAutomaticEDST)
	COPY(mShowUID)
	COPY(mDefaultCalendar)

//--------------------Attachment Prefs
	COPY(mDefault_mode)
	COPY(mDefault_Always)
	COPY(mViewDoubleClick)
	COPY(mAskDownload)
	COPY(mDefaultDownload)
	COPY(mAppLaunch)
	COPY(mLaunchText)
#if __dest_os == __win32_os
	COPY(mShellLaunch)
#endif
	COPY(mMIMEMappings)
	COPY(mExplicitMapping)
	COPY(mWarnMissingAttachments)
	COPY(mMissingAttachmentSubject)
	COPY(mMissingAttachmentWords)

#ifdef __use_speech
//--------------------Speech Prefs
	COPY(mSpeakNewOpen)
	COPY(mSpeakNewArrivals)
#ifdef __MULBERRY_CONFIGURE_V2_0
	// Special to support v2.0.x admin tool
	COPY(mSpeakNewAlert)
	COPY(mSpeakNewAlertTxt)
#endif
	COPY(mSpeakMessageItems)
	COPY(mSpeakLetterItems)
	COPY(mSpeakMessageItems)
	COPY(mSpeakLetterItems)
	COPY(mSpeakMessageEmptyItem)
	COPY(mSpeakMessageMaxLength)

#endif

//--------------------Miscellaneous Prefs
	COPY(mTextMacros)
	COPY(mAllowKeyboardShortcuts)

//--------------------Window Prefs
	// Clear existing - must delete each item
	for(CWindowStateArray::iterator iter = mServerWindowInfo.mValue.begin(); iter != mServerWindowInfo.mValue.end(); iter++)
		delete *iter;
	mServerWindowInfo.mValue.clear();
	for(CWindowStateArray::const_iterator iter = copy.mServerWindowInfo.mValue.begin();
		iter != copy.mServerWindowInfo.mValue.end(); iter++)
	{
		// Duplicate and add to list
		CServerWindowState* new_state = new CServerWindowState(*(CServerWindowState*) *iter);
		mServerWindowInfo.mValue.push_back(new_state);
	}
	// Clear existing - must delete each item
	for(CWindowStateArray::iterator iter = mMailboxWindowInfo.mValue.begin(); iter != mMailboxWindowInfo.mValue.end(); iter++)
		delete *iter;
	mMailboxWindowInfo.mValue.clear();
	for(CWindowStateArray::const_iterator iter = copy.mMailboxWindowInfo.mValue.begin();
		iter != copy.mMailboxWindowInfo.mValue.end(); iter++)
	{
		// Duplicate and add to list
		CMailboxWindowState* new_state = new CMailboxWindowState(*(CMailboxWindowState*) *iter);
		mMailboxWindowInfo.mValue.push_back(new_state);
	}
	// Clear existing - must delete each item
	for(CWindowStateArray::iterator iter = mNewAddressBookWindowInfo.mValue.begin(); iter != mNewAddressBookWindowInfo.mValue.end(); iter++)
		delete *iter;
	mNewAddressBookWindowInfo.mValue.clear();
	for(CWindowStateArray::const_iterator iter = copy.mNewAddressBookWindowInfo.mValue.begin();
		iter != copy.mNewAddressBookWindowInfo.mValue.end(); iter++)
	{
		// Duplicate and add to list
		CNewAddressBookWindowState* new_state = new CNewAddressBookWindowState(*(CNewAddressBookWindowState*) *iter);
		mNewAddressBookWindowInfo.mValue.push_back(new_state);
	}
	// Clear existing - must delete each item
	for(CWindowStateArray::iterator iter = mCalendarWindowInfo.mValue.begin(); iter != mCalendarWindowInfo.mValue.end(); iter++)
		delete *iter;
	mCalendarWindowInfo.mValue.clear();
	for(CWindowStateArray::const_iterator iter = copy.mCalendarWindowInfo.mValue.begin();
		iter != copy.mCalendarWindowInfo.mValue.end(); iter++)
	{
		// Duplicate and add to list
		CCalendarWindowState* new_state = new CCalendarWindowState(*(CCalendarWindowState*) *iter);
		mCalendarWindowInfo.mValue.push_back(new_state);
	}

	// Copy default window states
	COPY(mStatusWindowDefault)
#if __dest_os == __win32_os
	COPY(mMDIWindowDefault)
#endif
	COPY(m3PaneWindowDefault)
	COPY(m3PaneOptions)
	COPY(m3PaneOpenMailboxes)
	COPY(m3PaneOpenAdbks)
	COPY(m1PaneOptions)
	COPY(mServerWindowDefault)
	COPY(mServerBrowseDefault)
	COPY(mMailboxWindowDefault)
	COPY(mSMTPWindowDefault)
	COPY(mMessageWindowDefault)
	COPY(mMessageView3Pane)
	COPY(mMessageView1Pane)
	COPY(mLetterWindowDefault)
	COPY(mNewAddressBookWindowDefault)
	COPY(mAdbkManagerWindowDefault)
	COPY(mAdbkSearchWindowDefault)
	COPY(mFindReplaceWindowDefault)
	COPY(mSearchWindowDefault)
	COPY(mRulesWindowDefault)
	COPY(mRulesDialogDefault)
	COPY(mCalendarStoreWindowDefault)
	COPY(mCalendarWindowDefault)

}



// Compare with same type
#pragma push
#pragma global_optimizer on
int CPreferences::operator==(const CPreferences& other) const
{
#define NOTEQUAL(x) if (x != other.x) return false;

	NOTEQUAL(mHelpState)
	NOTEQUAL(mDetailedTooltips)
	NOTEQUAL(mUse_IC)
	NOTEQUAL(mCheckDefaultMailClient)
	NOTEQUAL(mCheckDefaultWebcalClient)
	NOTEQUAL(mWarnMailtoFiles)
	NOTEQUAL(mSimple)
	NOTEQUAL(mLastPanel)
	NOTEQUAL(mDisconnected)
	NOTEQUAL(mSMTPQueueEnabled)
	NOTEQUAL(mConnectOptions)
	NOTEQUAL(mDisconnectOptions)

	// Test Network Control prefs
	NOTEQUAL(mAppIdleTimeout)
	NOTEQUAL(mWaitDialogStart)
	NOTEQUAL(mWaitDialogMax)
	NOTEQUAL(mConnectRetryTimeout)
	NOTEQUAL(mConnectRetryMaxCount)
	NOTEQUAL(mBackgroundTimeout)
	NOTEQUAL(mTickleInterval)
	NOTEQUAL(mTickleIntervalExpireTimeout)
	NOTEQUAL(mSleepReconnect)

	// Test Account prefs
	NOTEQUAL(mMailAccounts)
	NOTEQUAL(mSMTPAccounts)
	NOTEQUAL(mRemoteAccounts)
	NOTEQUAL(mSIEVEAccounts)
	NOTEQUAL(mMailNotification)
	NOTEQUAL(mMailDomain)
	NOTEQUAL(mRemoteCachePswd)
	NOTEQUAL(mOSDefaultLocation)
	NOTEQUAL(mDisconnectedCWD)

	// Test Notification prefs
	NOTEQUAL(mMailNotification)
	NOTEQUAL(mAttachmentNotification)

	// Test Display prefs
	NOTEQUAL(mServerOpenStyle)
	NOTEQUAL(mServerClosedStyle)
	NOTEQUAL(mServerBkgndStyle)
	NOTEQUAL(mFavouriteBkgndStyle)
	NOTEQUAL(mHierarchyBkgndStyle)
	NOTEQUAL(mMboxRecentStyle)
	NOTEQUAL(mMboxUnseenStyle)
	NOTEQUAL(mMboxOpenStyle)
	NOTEQUAL(mMboxFavouriteStyle)
	NOTEQUAL(mMboxClosedStyle)
	NOTEQUAL(mIgnoreRecent)
	NOTEQUAL(unseen)
	NOTEQUAL(seen)
	NOTEQUAL(answered)
	NOTEQUAL(important)
	NOTEQUAL(deleted)
	NOTEQUAL(mMultiAddress)
	NOTEQUAL(mMatch)
	NOTEQUAL(mNonMatch)
	NOTEQUAL(mUseLocalTimezone)

	NOTEQUAL(mLabels)
	NOTEQUAL(mIMAPLabels)

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (::PLstrcmp(mListTextTraits.mValue.traits.fontName, other.mListTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mListTextTraits.mValue.traits.size)
	if (::PLstrcmp(mDisplayTextTraits.mValue.traits.fontName, other.mDisplayTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mDisplayTextTraits.mValue.traits.size)
	if (::PLstrcmp(mPrintTextTraits.mValue.traits.fontName, other.mPrintTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mPrintTextTraits.mValue.traits.size)
	if (::PLstrcmp(mCaptionTextTraits.mValue.traits.fontName, other.mCaptionTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mCaptionTextTraits.mValue.traits.size)
	NOTEQUAL(mAntiAliasFont)
#elif __dest_os == __win32_os
	{
		LOGFONT info1 = mListTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mListTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
	{
		LOGFONT info1 = mDisplayTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mDisplayTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
	{
		LOGFONT info1 = mPrintTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mPrintTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
	{
		LOGFONT info1 = mCaptionTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mCaptionTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
#elif __dest_os == __linux_os
	if (!((mListTextFontInfo == other.mListTextFontInfo) &&
				(mDisplayTextFontInfo == other.mDisplayTextFontInfo) &&
				(mPrintTextFontInfo == other.mPrintTextFontInfo) &&
				(mCaptionTextFontInfo == other.mCaptionTextFontInfo) &&
				(mListTextFontInfo == other.mListTextFontInfo) &&
				(mListTextFontInfo == other.mListTextFontInfo))) 
		{
			return false;
		}

#else
#error __dest_os
#endif

#if __dest_os == __win32_os
	NOTEQUAL(mMultiTaskbar)
#endif

	NOTEQUAL(mUse3Pane)
	NOTEQUAL(mToolbarShow)
	NOTEQUAL(mToolbarSmallIcons)
	NOTEQUAL(mToolbarShowIcons)
	NOTEQUAL(mToolbarShowCaptions)
	NOTEQUAL(mToolbars)

	// Test Mailbox prefs
	NOTEQUAL(openAtFirst)
	NOTEQUAL(openAtLast)
	NOTEQUAL(openAtFirstNew)
	NOTEQUAL(mNoOpenPreview)
	NOTEQUAL(mUnseenNew)
	NOTEQUAL(mRecentNew)
	NOTEQUAL(mNextIsNewest)
	NOTEQUAL(expungeOnClose)
	NOTEQUAL(warnOnExpunge)
	NOTEQUAL(mWarnPuntUnseen)
	NOTEQUAL(mDoRollover)
	NOTEQUAL(mRolloverWarn)
	NOTEQUAL(mDoMailboxClear)
	NOTEQUAL(clear_mailbox)
	NOTEQUAL(clear_warning)
	NOTEQUAL(mRLoCache)
	NOTEQUAL(mRHiCache)
	NOTEQUAL(mRUseHiCache)
	NOTEQUAL(mRCacheIncrement)
	NOTEQUAL(mRAutoCacheIncrement)
	NOTEQUAL(mRSortCache)
	NOTEQUAL(mLLoCache)
	NOTEQUAL(mLHiCache)
	NOTEQUAL(mLUseHiCache)
	NOTEQUAL(mLCacheIncrement)
	NOTEQUAL(mLAutoCacheIncrement)
	NOTEQUAL(mLSortCache)
	NOTEQUAL(mSmartAddressList)
	NOTEQUAL(mUseCopyToMboxes)
	NOTEQUAL(mUseAppendToMboxes)
	NOTEQUAL(mMRUMaximum)
	NOTEQUAL(mMboxACLStyles)
	NOTEQUAL(mSearchStyles)
	NOTEQUAL(mPromptDisconnected)
	NOTEQUAL(mAutoDial)
	NOTEQUAL(mUIDValidityMismatch)
	NOTEQUAL(mQuickSearch)
	NOTEQUAL(mQuickSearchVisible)
	NOTEQUAL(mScrollForUnseen)

	// Test Favourites prefs
	NOTEQUAL(mFavourites)
	NOTEQUAL(mMRUCopyTo)
	NOTEQUAL(mMRUAppendTo)
	NOTEQUAL(mMailboxAliases)

	// Test Search prefs
	NOTEQUAL(mMultipleSearch)
	NOTEQUAL(mNumberMultipleSearch)
	NOTEQUAL(mLoadBalanceSearch)
	NOTEQUAL(mOpenFirstSearchResult)

	// Test Message prefs
	NOTEQUAL(showMessageHeader)
	NOTEQUAL(saveMessageHeader)
	NOTEQUAL(mSaveCreator)
	NOTEQUAL(deleteAfterCopy)
	NOTEQUAL(mOpenDeleted)
	NOTEQUAL(mCloseDeleted)
	NOTEQUAL(mDoSizeWarn)
	NOTEQUAL(warnMessageSize)
	NOTEQUAL(mQuoteSelection)
	NOTEQUAL(mAlwaysQuote)
	NOTEQUAL(optionKeyReplyDialog)
	NOTEQUAL(showStyled)
	NOTEQUAL(mHeaderCaption)
	NOTEQUAL(mHeaderBox)
	NOTEQUAL(mPrintSummary)
	NOTEQUAL(mFooterCaption)
	NOTEQUAL(mFooterBox)
	NOTEQUAL(mOpenReuse)
	NOTEQUAL(mAutoDigest)
	NOTEQUAL(mExpandHeader)
	NOTEQUAL(mExpandParts)
	NOTEQUAL(mForwardChoice)
	NOTEQUAL(mForwardQuoteOriginal)
	NOTEQUAL(mForwardHeaders)
	NOTEQUAL(mForwardAttachment)
	NOTEQUAL(mForwardRFC822)
	NOTEQUAL(mMDNOptions)

	// Test Draft Prefs
	NOTEQUAL(mAutoInsertSignature)
	NOTEQUAL(mSignatureEmptyLine)
	NOTEQUAL(mSigDashes)
	NOTEQUAL(mNoSubjectWarn)
	NOTEQUAL(spaces_per_tab)
	NOTEQUAL(mTabSpace)
	NOTEQUAL(mReplyQuote)
	NOTEQUAL(mReplyStart)
	NOTEQUAL(mReplyEnd)
	NOTEQUAL(mReplyCursorTop)
	NOTEQUAL(mReplyNoSignature)
	NOTEQUAL(mForwardQuote)
	NOTEQUAL(mForwardStart)
	NOTEQUAL(mForwardEnd)
	NOTEQUAL(mForwardCursorTop)
	NOTEQUAL(mForwardSubject)
	NOTEQUAL(mLtrHeaderCaption)
	NOTEQUAL(mLtrFooterCaption)
	NOTEQUAL(mSeparateBCC)
	NOTEQUAL(mBCCCaption)
	NOTEQUAL(mShowCCs)
	NOTEQUAL(mShowBCCs)
	NOTEQUAL(mExternalEditor)
	NOTEQUAL(mUseExternalEditor)
	NOTEQUAL(mExternalEditAutoStart)
	NOTEQUAL(wrap_length)
	NOTEQUAL(mWindowWrap)
	NOTEQUAL(mDisplayAttachments)
	NOTEQUAL(mAppendDraft)
	NOTEQUAL(inbox_append)
	NOTEQUAL(mDeleteOriginalDraft)
	NOTEQUAL(mTemplateDrafts)
	NOTEQUAL(mSmartURLPaste)
	NOTEQUAL(mWarnReplySubject)
	NOTEQUAL(mSaveOptions)
	NOTEQUAL(mSaveMailbox)
	NOTEQUAL(compose_as)
	NOTEQUAL(enrMultiAltPlain)
	NOTEQUAL(htmlMultiAltEnriched)
	NOTEQUAL(enrMultiAltHTML)
	NOTEQUAL(htmlMultiAltPlain)
	NOTEQUAL(mFormatFlowed)
	NOTEQUAL(mAlwaysUnicode)
	NOTEQUAL(mDisplayIdentityFrom)
	NOTEQUAL(mAutoSaveDrafts)
	NOTEQUAL(mAutoSaveDraftsInterval)

	// Test Formatting Prefs
	NOTEQUAL(mURLStyle)
	NOTEQUAL(mURLSeenStyle)
	NOTEQUAL(mHeaderStyle)
	NOTEQUAL(mQuotationStyle)
	NOTEQUAL(mTagStyle)
	NOTEQUAL(mUseMultipleQuotes)
	if (mQuoteColours.mValue.size() != other.mQuoteColours.mValue.size())
		return false;
	{
		CColourList::const_iterator iter1 = mQuoteColours.mValue.begin();
		CColourList::const_iterator iter2 = other.mQuoteColours.mValue.begin();
		for(; iter1 != mQuoteColours.mValue.end(); iter1++, iter2++)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (((*iter1).red != (*iter2).red) ||
				((*iter1).green != (*iter2).green) ||
				((*iter1).blue != (*iter2).blue))
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (*iter1 != *iter2)
#else
#error __dest_os
#endif
				return false;
		}
	}
	NOTEQUAL(mRecognizeQuotes)
	NOTEQUAL(mRecognizeURLs)

#if __dest_os == __linux_os
	NOTEQUAL(mSelectionColour)
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (::PLstrcmp(mFixedTextTraits.mValue.traits.fontName, other.mFixedTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mFixedTextTraits.mValue.traits.size)
	if (::PLstrcmp(mHTMLTextTraits.mValue.traits.fontName, other.mHTMLTextTraits.mValue.traits.fontName) != 0)
		return false;
	NOTEQUAL(mHTMLTextTraits.mValue.traits.size)
#elif __dest_os == __win32_os
	{
		LOGFONT info1 = mFixedTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mFixedTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
	{
		LOGFONT info1 = mHTMLTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mHTMLTextFontInfo.GetValue().logfont;

		if ((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0))
		return false;
	}
#elif __dest_os == __linux_os
	if (!(mFixedTextFontInfo == other.mFixedTextFontInfo &&
				mHTMLTextFontInfo == other.mHTMLTextFontInfo)) {
		return false;
	}
#else
#error __dest_os
#endif
	NOTEQUAL(mUseStyles)
	NOTEQUAL(mMinimumFont)

	// Test Security Prefs
	NOTEQUAL(mPreferredPlugin)
	NOTEQUAL(mUseMIMESecurity)
	NOTEQUAL(mEncryptToSelf)
	NOTEQUAL(mCachePassphrase)
	NOTEQUAL(mCachePassphraseMax)
	NOTEQUAL(mCachePassphraseIdle)
	NOTEQUAL(mAutoVerify)
	NOTEQUAL(mAutoDecrypt)
	NOTEQUAL(mWarnUnencryptedSend)
	NOTEQUAL(mUseErrorAlerts)
	NOTEQUAL(mCacheUserCerts)
	NOTEQUAL(mVerifyOKNotification)

	// Test Identitiy prefs
	NOTEQUAL(mIdentities)
	// mTiedMailboxes != other.mTiedMailboxes	<- not required
	// mTiedCalendars != other.mTiedCalendars	<- not required
	NOTEQUAL(mContextTied)
	NOTEQUAL(mMsgTied)
	NOTEQUAL(mTiedMboxInherit)

	// Test Address Prefs
	NOTEQUAL(mAddressAccounts)
	NOTEQUAL(mLocalAdbkAccount)
	NOTEQUAL(mOSAdbkAccount)
	NOTEQUAL(mExpandedAdbks)
	NOTEQUAL(mAdbkOpenAtStartup)
	NOTEQUAL(mAdbkNickName)
	NOTEQUAL(mAdbkSearch)
	NOTEQUAL(mAdbkAutoSync)
	NOTEQUAL(mAdbkACLStyles)
	NOTEQUAL(mOptionKeyAddressDialog)
	NOTEQUAL(mExpandNoNicknames)
	NOTEQUAL(mExpandFailedNicknames)
	NOTEQUAL(mExpandFullName)
	NOTEQUAL(mExpandNickName)
	NOTEQUAL(mExpandEmail)
	NOTEQUAL(mSkipLDAP)
	NOTEQUAL(mCaptureAddressBook)
	NOTEQUAL(mCaptureAllowEdit)
	NOTEQUAL(mCaptureAllowChoice)
	NOTEQUAL(mCaptureRead)
	NOTEQUAL(mCaptureRespond)
	NOTEQUAL(mCaptureFrom)
	NOTEQUAL(mCaptureCc)
	NOTEQUAL(mCaptureReplyTo)
	NOTEQUAL(mCaptureTo)

	// Calendar Prefs
	NOTEQUAL(mCalendarAccounts)
	NOTEQUAL(mLocalCalendarAccount)
	NOTEQUAL(mWebCalendarAccount)
	NOTEQUAL(mSubscribedCalendars)
	NOTEQUAL(mExpandedCalendars)
	NOTEQUAL(mCalendarColours)
	NOTEQUAL(mTimezone)
	NOTEQUAL(mFavouriteTimezones)
	NOTEQUAL(mWeekStartDay)
	NOTEQUAL(mWorkDayMask)
	NOTEQUAL(mDayWeekTimeRanges)
	NOTEQUAL(mDisplayTime)
	NOTEQUAL(m24HourTime)
	NOTEQUAL(mHandleICS)
	NOTEQUAL(mAutomaticIMIP)
	NOTEQUAL(mAutomaticEDST)
	NOTEQUAL(mShowUID)
	NOTEQUAL(mDefaultCalendar)

	// Attachment Prefs
	NOTEQUAL(mDefault_mode)
	NOTEQUAL(mDefault_Always)
	NOTEQUAL(mViewDoubleClick)
	NOTEQUAL(mAskDownload)
	NOTEQUAL(mDefaultDownload)
	NOTEQUAL(mAppLaunch)
	NOTEQUAL(mLaunchText)
#if __dest_os == __win32_os
	NOTEQUAL(mShellLaunch)
#endif
	NOTEQUAL(mMIMEMappings)
	NOTEQUAL(mExplicitMapping)
	NOTEQUAL(mWarnMissingAttachments)
	NOTEQUAL(mMissingAttachmentSubject)
	NOTEQUAL(mMissingAttachmentWords)

#ifdef __use_speech
	// Speech Prefs
	NOTEQUAL(mSpeakNewOpen)
	NOTEQUAL(mSpeakNewArrivals)
#ifdef __MULBERRY_CONFIGURE_V2_0
	// Special to support v2.0.x admin tool
	NOTEQUAL(mSpeakNewAlert)
	NOTEQUAL(mSpeakNewAlertTxt)
#endif
	NOTEQUAL(mSpeakMessageItems)
	NOTEQUAL(mSpeakLetterItems)
	NOTEQUAL(mSpeakMessageEmptyItem)
	NOTEQUAL(mSpeakMessageMaxLength)
#endif

	// Miscellaneous Prefs
	NOTEQUAL(mTextMacros)
	NOTEQUAL(mAllowKeyboardShortcuts)

	return true;
}
#pragma pop

// Dirty prefs that are different from default
void CPreferences::DiffDirty()
{
#define SETDIRTY(x) x.SetDirty(x != other.x);

	CPreferences other;

	SETDIRTY(mHelpState)
	SETDIRTY(mDetailedTooltips)
	SETDIRTY(mUse_IC)
	SETDIRTY(mCheckDefaultMailClient)
	SETDIRTY(mCheckDefaultWebcalClient)
	SETDIRTY(mWarnMailtoFiles)
	SETDIRTY(mSimple)
	SETDIRTY(mLastPanel)
	SETDIRTY(mDisconnected)
	SETDIRTY(mSMTPQueueEnabled)
	SETDIRTY(mConnectOptions)
	SETDIRTY(mDisconnectOptions)

	// Test Network Control prefs
	SETDIRTY(mAppIdleTimeout)
	SETDIRTY(mWaitDialogStart)
	SETDIRTY(mWaitDialogMax)
	SETDIRTY(mConnectRetryTimeout)
	SETDIRTY(mConnectRetryMaxCount)
	SETDIRTY(mBackgroundTimeout)
	SETDIRTY(mTickleInterval)
	SETDIRTY(mTickleIntervalExpireTimeout)
	SETDIRTY(mSleepReconnect)

	// Test Account prefs
	SETDIRTY(mMailAccounts)
	SETDIRTY(mSMTPAccounts)
	SETDIRTY(mRemoteAccounts)
	SETDIRTY(mSIEVEAccounts)
	SETDIRTY(mMailNotification)
	SETDIRTY(mMailDomain)
	SETDIRTY(mRemoteCachePswd)
	SETDIRTY(mOSDefaultLocation)
	SETDIRTY(mDisconnectedCWD)

	// Test Notification prefs
	SETDIRTY(mMailNotification)
	SETDIRTY(mAttachmentNotification)

	// Test Display prefs
	SETDIRTY(mServerOpenStyle)
	SETDIRTY(mServerClosedStyle)
	SETDIRTY(mServerBkgndStyle)
	SETDIRTY(mFavouriteBkgndStyle)
	SETDIRTY(mHierarchyBkgndStyle)
	SETDIRTY(mMboxRecentStyle)
	SETDIRTY(mMboxUnseenStyle)
	SETDIRTY(mMboxOpenStyle)
	SETDIRTY(mMboxFavouriteStyle)
	SETDIRTY(mMboxClosedStyle)
	SETDIRTY(mIgnoreRecent)
	SETDIRTY(unseen)
	SETDIRTY(seen)
	SETDIRTY(answered)
	SETDIRTY(important)
	SETDIRTY(deleted)
	SETDIRTY(mMultiAddress)
	SETDIRTY(mMatch)
	SETDIRTY(mNonMatch)
	SETDIRTY(mUseLocalTimezone)

	SETDIRTY(mLabels)
	SETDIRTY(mIMAPLabels)

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mListTextTraits.SetDirty((::PLstrcmp(mListTextTraits.mValue.traits.fontName, other.mListTextTraits.mValue.traits.fontName) != 0) ||
								(mListTextTraits.mValue.traits.size != other.mListTextTraits.mValue.traits.size));
	mDisplayTextTraits.SetDirty((::PLstrcmp(mDisplayTextTraits.mValue.traits.fontName, other.mDisplayTextTraits.mValue.traits.fontName) != 0) ||
								(mDisplayTextTraits.mValue.traits.size != other.mDisplayTextTraits.mValue.traits.size));
	mPrintTextTraits.SetDirty((::PLstrcmp(mPrintTextTraits.mValue.traits.fontName, other.mPrintTextTraits.mValue.traits.fontName) != 0) ||
								(mPrintTextTraits.mValue.traits.size != other.mPrintTextTraits.mValue.traits.size));
	mCaptionTextTraits.SetDirty((::PLstrcmp(mCaptionTextTraits.mValue.traits.fontName, other.mCaptionTextTraits.mValue.traits.fontName) != 0) ||
								(mCaptionTextTraits.mValue.traits.size != other.mCaptionTextTraits.mValue.traits.size));
	SETDIRTY(mAntiAliasFont)
#elif __dest_os == __win32_os
	{
		LOGFONT info1 = mListTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mListTextFontInfo.GetValue().logfont;

		mListTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
	{
		LOGFONT info1 = mDisplayTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mDisplayTextFontInfo.GetValue().logfont;

		mDisplayTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
	{
		LOGFONT info1 = mPrintTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mPrintTextFontInfo.GetValue().logfont;

		mPrintTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
	{
		LOGFONT info1 = mCaptionTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mCaptionTextFontInfo.GetValue().logfont;

		mCaptionTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
#elif __dest_os == __linux_os
		mListTextFontInfo.SetDirty(mListTextFontInfo.GetValue() !=
															 other.mListTextFontInfo.GetValue());
		mDisplayTextFontInfo.SetDirty(mDisplayTextFontInfo.GetValue() !=
																	other.mDisplayTextFontInfo.GetValue());
		mPrintTextFontInfo.SetDirty(mPrintTextFontInfo.GetValue() !=
																	other.mPrintTextFontInfo.GetValue());
		mCaptionTextFontInfo.SetDirty(mCaptionTextFontInfo.GetValue() !=
																	other.mCaptionTextFontInfo.GetValue());
#else
#error __dest_os
#endif

#if __dest_os == __win32_os
	SETDIRTY(mMultiTaskbar)
#endif

	SETDIRTY(mUse3Pane)
	SETDIRTY(mToolbarShow)
	SETDIRTY(mToolbarSmallIcons)
	SETDIRTY(mToolbarShowIcons)
	SETDIRTY(mToolbarShowCaptions)
	SETDIRTY(mToolbars)

	// Test Mailbox prefs
	SETDIRTY(openAtFirst)
	SETDIRTY(openAtLast)
	SETDIRTY(openAtFirstNew)
	SETDIRTY(mNoOpenPreview)
	SETDIRTY(mUnseenNew)
	SETDIRTY(mRecentNew)
	SETDIRTY(mNextIsNewest)
	SETDIRTY(expungeOnClose)
	SETDIRTY(warnOnExpunge)
	SETDIRTY(mWarnPuntUnseen)
	SETDIRTY(mDoRollover)
	SETDIRTY(mRolloverWarn)
	SETDIRTY(mDoMailboxClear)
	SETDIRTY(clear_mailbox)
	SETDIRTY(clear_warning)
	SETDIRTY(mRLoCache)
	SETDIRTY(mRHiCache)
	SETDIRTY(mRUseHiCache)
	SETDIRTY(mRCacheIncrement)
	SETDIRTY(mRAutoCacheIncrement)
	SETDIRTY(mRSortCache)
	SETDIRTY(mLLoCache)
	SETDIRTY(mLHiCache)
	SETDIRTY(mLUseHiCache)
	SETDIRTY(mLCacheIncrement)
	SETDIRTY(mLAutoCacheIncrement)
	SETDIRTY(mLSortCache)
	SETDIRTY(mSmartAddressList)
	SETDIRTY(mUseCopyToMboxes)
	SETDIRTY(mUseAppendToMboxes)
	SETDIRTY(mMRUMaximum)
	SETDIRTY(mMboxACLStyles)
	SETDIRTY(mSearchStyles)
	SETDIRTY(mPromptDisconnected)
	SETDIRTY(mAutoDial)
	SETDIRTY(mUIDValidityMismatch)
	SETDIRTY(mQuickSearch)
	SETDIRTY(mQuickSearchVisible)
	SETDIRTY(mScrollForUnseen)

	// Test Favourites prefs
	SETDIRTY(mFavourites)
	SETDIRTY(mMRUCopyTo)
	SETDIRTY(mMRUAppendTo)
	SETDIRTY(mMailboxAliases)

	// Test Search prefs
	SETDIRTY(mMultipleSearch)
	SETDIRTY(mNumberMultipleSearch)
	SETDIRTY(mLoadBalanceSearch)
	SETDIRTY(mOpenFirstSearchResult)

	// Test Message prefs
	SETDIRTY(showMessageHeader)
	SETDIRTY(saveMessageHeader)
	SETDIRTY(mSaveCreator)
	SETDIRTY(deleteAfterCopy)
	SETDIRTY(mOpenDeleted)
	SETDIRTY(mCloseDeleted)
	SETDIRTY(mDoSizeWarn)
	SETDIRTY(warnMessageSize)
	SETDIRTY(mQuoteSelection)
	SETDIRTY(mAlwaysQuote)
	SETDIRTY(optionKeyReplyDialog)
	SETDIRTY(showStyled)
	SETDIRTY(mHeaderCaption)
	SETDIRTY(mHeaderBox)
	SETDIRTY(mPrintSummary)
	SETDIRTY(mFooterCaption)
	SETDIRTY(mFooterBox)
	SETDIRTY(mOpenReuse)
	SETDIRTY(mAutoDigest)
	SETDIRTY(mExpandHeader)
	SETDIRTY(mExpandParts)
	SETDIRTY(mForwardChoice)
	SETDIRTY(mForwardQuoteOriginal)
	SETDIRTY(mForwardHeaders)
	SETDIRTY(mForwardAttachment)
	SETDIRTY(mForwardRFC822)
	SETDIRTY(mMDNOptions)

	// Test Draft Prefs
	SETDIRTY(mAutoInsertSignature)
	SETDIRTY(mSignatureEmptyLine)
	SETDIRTY(mSigDashes)
	SETDIRTY(mNoSubjectWarn)
	SETDIRTY(spaces_per_tab)
	SETDIRTY(mTabSpace)
	SETDIRTY(mReplyQuote)
	SETDIRTY(mReplyStart)
	SETDIRTY(mReplyEnd)
	SETDIRTY(mReplyCursorTop)
	SETDIRTY(mReplyNoSignature)
	SETDIRTY(mForwardQuote)
	SETDIRTY(mForwardStart)
	SETDIRTY(mForwardEnd)
	SETDIRTY(mForwardCursorTop)
	SETDIRTY(mForwardSubject)
	SETDIRTY(mLtrHeaderCaption)
	SETDIRTY(mLtrFooterCaption)
	SETDIRTY(mSeparateBCC)
	SETDIRTY(mBCCCaption)
	SETDIRTY(mShowCCs)
	SETDIRTY(mShowBCCs)
	SETDIRTY(mExternalEditor)
	SETDIRTY(mUseExternalEditor)
	SETDIRTY(mExternalEditAutoStart)
	SETDIRTY(wrap_length)
	SETDIRTY(mWindowWrap)
	SETDIRTY(mDisplayAttachments)
	SETDIRTY(mAppendDraft)
	SETDIRTY(inbox_append)
	SETDIRTY(mDeleteOriginalDraft)
	SETDIRTY(mTemplateDrafts)
	SETDIRTY(mSmartURLPaste)
	SETDIRTY(mWarnReplySubject)
	SETDIRTY(mSaveOptions)
	SETDIRTY(mSaveMailbox)
	SETDIRTY(compose_as)
	SETDIRTY(enrMultiAltPlain)
	SETDIRTY(htmlMultiAltEnriched)
	SETDIRTY(enrMultiAltHTML)
	SETDIRTY(htmlMultiAltPlain)
	SETDIRTY(mFormatFlowed)
	SETDIRTY(mAlwaysUnicode)
	SETDIRTY(mDisplayIdentityFrom)
	SETDIRTY(mAutoSaveDrafts)
	SETDIRTY(mAutoSaveDraftsInterval)

	// Test Formatting Prefs
	SETDIRTY(mURLStyle)
	SETDIRTY(mURLSeenStyle)
	SETDIRTY(mHeaderStyle)
	SETDIRTY(mQuotationStyle)
	SETDIRTY(mTagStyle)
	SETDIRTY(mUseMultipleQuotes)
	if(mQuoteColours.mValue.size() == other.mQuoteColours.mValue.size())
	{
		CColourList::const_iterator iter1 = mQuoteColours.mValue.begin();
		CColourList::const_iterator iter2 = other.mQuoteColours.mValue.begin();
		bool dirty = false;
		for(; iter1 != mQuoteColours.mValue.end(); iter1++, iter2++)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (((*iter1).red != (*iter2).red) ||
				((*iter1).green != (*iter2).green) ||
				((*iter1).blue != (*iter2).blue))
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (*iter1 != *iter2)
#else
#error __dest_os
#endif
				dirty = true;
		}
		mQuoteColours.SetDirty(dirty);
	}
	else
		mQuoteColours.SetDirty(true);
	SETDIRTY(mRecognizeQuotes)
	SETDIRTY(mRecognizeURLs)

#if __dest_os == __linux_os
	SETDIRTY(mSelectionColour)
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mFixedTextTraits.SetDirty((::PLstrcmp(mFixedTextTraits.mValue.traits.fontName, other.mFixedTextTraits.mValue.traits.fontName) != 0) ||
								(mFixedTextTraits.mValue.traits.size != other.mFixedTextTraits.mValue.traits.size));
	mHTMLTextTraits.SetDirty((::PLstrcmp(mHTMLTextTraits.mValue.traits.fontName, other.mHTMLTextTraits.mValue.traits.fontName) != 0) ||
								(mHTMLTextTraits.mValue.traits.size != other.mHTMLTextTraits.mValue.traits.size));
#elif __dest_os == __win32_os
	{
		LOGFONT info1 = mFixedTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mFixedTextFontInfo.GetValue().logfont;

		mFixedTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
	{
		LOGFONT info1 = mHTMLTextFontInfo.GetValue().logfont;
		LOGFONT info2 = other.mHTMLTextFontInfo.GetValue().logfont;

		mHTMLTextFontInfo.SetDirty((info1.lfHeight != info2.lfHeight) ||
			(info1.lfWidth != info2.lfWidth) ||
			(info1.lfEscapement != info2.lfEscapement) ||
			(info1.lfOrientation != info2.lfOrientation) ||
			(info1.lfItalic != info2.lfItalic) ||
			(info1.lfStrikeOut != info2.lfStrikeOut) ||
			(info1.lfCharSet != info2.lfCharSet) ||
			(info1.lfOutPrecision != info2.lfOutPrecision) ||
			(info1.lfClipPrecision != info2.lfClipPrecision) ||
			(info1.lfQuality != info2.lfQuality) ||
			(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
			(::lstrcmp(info1.lfFaceName, info2.lfFaceName) != 0));
	}
#elif __dest_os == __linux_os
	mHTMLTextFontInfo.SetDirty(mHTMLTextFontInfo.GetValue() != 
														 other.mHTMLTextFontInfo.GetValue());
#else
#error __dest_os
#endif
	SETDIRTY(mUseStyles)
	SETDIRTY(mMinimumFont)

	// Test Security Prefs
	SETDIRTY(mPreferredPlugin)
	SETDIRTY(mUseMIMESecurity)
	SETDIRTY(mEncryptToSelf)
	SETDIRTY(mCachePassphrase)
	SETDIRTY(mCachePassphraseMax)
	SETDIRTY(mCachePassphraseIdle)
	SETDIRTY(mAutoVerify)
	SETDIRTY(mAutoDecrypt)
	SETDIRTY(mWarnUnencryptedSend)
	SETDIRTY(mUseErrorAlerts)
	SETDIRTY(mCacheUserCerts)
	SETDIRTY(mVerifyOKNotification)

	// Test Identitiy prefs
	SETDIRTY(mIdentities)
	// mTiedMailboxes != other.mTiedMailboxes	<- not required
	// mTiedCalendars != other.mTiedCalendars	<- not required
	SETDIRTY(mContextTied)
	SETDIRTY(mMsgTied)
	SETDIRTY(mTiedMboxInherit)

	// Test Address Prefs
	SETDIRTY(mAddressAccounts)
	SETDIRTY(mLocalAdbkAccount)
	SETDIRTY(mOSAdbkAccount)
	SETDIRTY(mExpandedAdbks)
	SETDIRTY(mAdbkOpenAtStartup)
	SETDIRTY(mAdbkNickName)
	SETDIRTY(mAdbkSearch)
	SETDIRTY(mAdbkAutoSync)
	SETDIRTY(mAdbkACLStyles)
	SETDIRTY(mOptionKeyAddressDialog)
	SETDIRTY(mExpandNoNicknames)
	SETDIRTY(mExpandFailedNicknames)
	SETDIRTY(mExpandFullName)
	SETDIRTY(mExpandNickName)
	SETDIRTY(mExpandEmail)
	SETDIRTY(mSkipLDAP)
	SETDIRTY(mCaptureAddressBook)
	SETDIRTY(mCaptureAllowEdit)
	SETDIRTY(mCaptureAllowChoice)
	SETDIRTY(mCaptureRead)
	SETDIRTY(mCaptureRespond)
	SETDIRTY(mCaptureFrom)
	SETDIRTY(mCaptureCc)
	SETDIRTY(mCaptureReplyTo)
	SETDIRTY(mCaptureTo)

	// Calendar Prefs
	SETDIRTY(mCalendarAccounts)
	SETDIRTY(mLocalCalendarAccount)
	SETDIRTY(mWebCalendarAccount)
	SETDIRTY(mSubscribedCalendars)
	SETDIRTY(mExpandedCalendars)
	SETDIRTY(mCalendarColours)
	SETDIRTY(mTimezone)
	SETDIRTY(mFavouriteTimezones)
	SETDIRTY(mWeekStartDay)
	SETDIRTY(mWorkDayMask)
	SETDIRTY(mDayWeekTimeRanges)
	SETDIRTY(mDisplayTime)
	SETDIRTY(m24HourTime)
	SETDIRTY(mHandleICS)
	SETDIRTY(mAutomaticIMIP)
	SETDIRTY(mAutomaticEDST)
	SETDIRTY(mShowUID)
	SETDIRTY(mDefaultCalendar)

	// Attachment Prefs
	SETDIRTY(mDefault_mode)
	SETDIRTY(mDefault_Always)
	SETDIRTY(mViewDoubleClick)
	SETDIRTY(mAskDownload)
	SETDIRTY(mDefaultDownload)
	SETDIRTY(mAppLaunch)
	SETDIRTY(mLaunchText)
#if __dest_os == __win32_os
	SETDIRTY(mShellLaunch)
#endif
	SETDIRTY(mMIMEMappings)
	SETDIRTY(mExplicitMapping)
	SETDIRTY(mWarnMissingAttachments)
	SETDIRTY(mMissingAttachmentSubject)
	SETDIRTY(mMissingAttachmentWords)

#ifdef __use_speech
	// Speech Prefs
	SETDIRTY(mSpeakNewOpen)
	SETDIRTY(mSpeakNewArrivals)
#ifdef __MULBERRY_CONFIGURE_V2_0
	// Special to support v2.0.x admin tool
	SETDIRTY(mSpeakNewAlert)
	SETDIRTY(mSpeakNewAlertTxt)
#endif
	SETDIRTY(mSpeakMessageItems)
	SETDIRTY(mSpeakLetterItems)
	SETDIRTY(mSpeakMessageEmptyItem)
	SETDIRTY(mSpeakMessageMaxLength)
#endif

	// Miscellaneous Prefs
	SETDIRTY(mTextMacros)
	SETDIRTY(mAllowKeyboardShortcuts)
}
