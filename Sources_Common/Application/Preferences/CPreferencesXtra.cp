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


// CPreferencesInit.cp

#include "CPreferences.h"

#include "CAdbkACL.h"
#include "CAdminLock.h"
#include "CFilterManager.h"
#include "CIMAPCommon.h"
#include "CMailAccountManager.h"
#include "CMboxACL.h"
#include "CMulberryCommon.h"
#ifdef __MULBERRY
#include "CToolbarManager.h"
#endif
#include "CXStringResources.h"

#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif

#if __dest_os == __win32_os
#include "CFontMenu.h"
#endif

#if __dest_os == __linux_os
#include <jGlobals.h>
#endif

// Default colours
#if __dest_os == __mac_os || __dest_os == __mac_os_x
const	RGBColor	kServerOpenColor = {0x0000,0x8000,0x0000};
const	RGBColor	kServerClosedColor = {0x0000,0x0000,0x0000};
const	RGBColor	kServerBkgndColor = {0xBBBB,0xBBBB,0xBBBB};
const	RGBColor	kFavouriteBkgndColor = {0xDDDD,0xDDDD,0xDDDD};
const	RGBColor	kHierarchyBkgndColor = {0xDDDD,0xDDDD,0xDDDD};

const	RGBColor	kMboxRecentColor = {0xDDDD,0x0000,0x0000};
const	RGBColor	kMboxUnseenColor = {0xFFFF,0x0000,0xCCCC};
const	RGBColor	kMboxOpenColor = {0x0000,0x8000,0x0000};
const	RGBColor	kMboxFavouriteColor = {0x0000,0x0000,0xDDDD};
const	RGBColor	kMboxClosedColor = {0x0000,0x0000,0x0000};

const	RGBColor	kMsgUnseenColor = {0x0000,0x0000,0xDDDD};
const	RGBColor	kMsgSeenColor = {0x0000,0x0000,0x0000};
const	RGBColor	kMsgAnsweredColor = {0x0000,0x8000,0x0000};
const	RGBColor	kMsgImportantColor = {0xFFFF,0x0000,0xCCCC};
const	RGBColor	kMsgDeletedColor = {0xDDDD,0x0000,0x0000};
const	RGBColor	kMsgMatchColor = {0xA000,0xA000,0xA000};
const	RGBColor	kMsgNonMatchColor = {0xA000,0x4000,0x4000};

const	RGBColor	kLabelColor[] = { {0xFFFF, 0x6666, 0x0000},
										{0xCCCC, 0x0000, 0x0000},
										{0xFFFF, 0x0000, 0x9999},
										{0x0000, 0x9999, 0xFFFF},
										{0x0000, 0x0000, 0xCCCC},
										{0x0000, 0x6666, 0x0000},
										{0x6666, 0x3333, 0x0000},
										{0xCCCC, 0x6666, 0xCCCC} };
const	RGBColor	kLabelBkgColor = {0xFFFF, 0xFFFF, 0xFFFF};

const	RGBColor	kQuotation1Color = {0x9999, 0x3333, 0x6666};
const	RGBColor	kQuotation2Color = {0x9999, 0x3333, 0x9999};
const	RGBColor	kQuotation3Color = {0x9999, 0x6666, 0x9999};
const	RGBColor	kQuotation4Color = {0x9999, 0x9999, 0x9999};
const	RGBColor	kQuotation5Color = {0xCCCC, 0xCCCC, 0xCCCC};

const 	RGBColor	kURLColor = {0x0000, 0x0000, 0xFFFF};
const 	RGBColor	kURLSeenColor = {0x6666, 0x0000, 0x6666};
const	RGBColor	kHeaderColor = {0x0000, 0x0000, 0x0000};
const	RGBColor	kTagColor = {0x0000, 0x8000, 0x0000};
#elif __dest_os == __win32_os
const	RGBColor	kServerOpenColor = RGB(0x00,0x80,0x00);
const	RGBColor	kServerClosedColor = RGB(0x00,0x00,0x00);
const	RGBColor	kServerBkgndColor = RGB(0xBB,0xBB,0xBB);
const	RGBColor	kFavouriteBkgndColor = RGB(0xDD,0xDD,0xDD);
const	RGBColor	kHierarchyBkgndColor = RGB(0xDD,0xDD,0xDD);

const	RGBColor	kMboxRecentColor = RGB(0xDD,0x00,0x00);
const	RGBColor	kMboxUnseenColor = RGB(0xFF,0x00,0xCC);
const	RGBColor	kMboxOpenColor = RGB(0x00,0x80,0x00);
const	RGBColor	kMboxFavouriteColor = RGB(0x00,0x00,0xDD);
const	RGBColor	kMboxClosedColor = RGB(0x00,0x00,0x00);

const	RGBColor	kLabelColor[] = { RGB(0xFF, 0x66, 0x00),
										RGB(0xCC, 0x00, 0x00),
										RGB(0xFF, 0x00, 0x99),
										RGB(0x00, 0x99, 0xFF),
										RGB(0x00, 0x00, 0xCC),
										RGB(0x00, 0x66, 0x00),
										RGB(0x66, 0x33, 0x00),
										RGB(0xCC, 0x66, 0xCC) };
const	RGBColor	kLabelBkgColor = RGB(0xFF, 0xFF, 0xFF);

const	RGBColor	kMsgUnseenColor = RGB(0x00,0x00,0xDD);
const	RGBColor	kMsgSeenColor = RGB(0x00,0x00,0x00);
const	RGBColor	kMsgAnsweredColor = RGB(0x00,0x80,0x00);
const	RGBColor	kMsgImportantColor = RGB(0xFF,0x00,0xCC);
const	RGBColor	kMsgDeletedColor = RGB(0xDD,0x00,0x00);
const	RGBColor	kMsgMatchColor = RGB(0xAA,0xAA,0xAA);
const	RGBColor	kMsgNonMatchColor = RGB(0xAA,0x44,0x44);

const	RGBColor	kQuotation1Color = RGB(0x99, 0x33, 0x66);
const	RGBColor	kQuotation2Color = RGB(0x99, 0x33, 0x99);
const	RGBColor	kQuotation3Color = RGB(0x99, 0x66, 0x99);
const	RGBColor	kQuotation4Color = RGB(0x99, 0x99, 0x99);
const	RGBColor	kQuotation5Color = RGB(0xCC, 0xCC, 0xCC);

const 	RGBColor	kURLColor = RGB(0x00, 0x00, 0xFF);
const 	RGBColor	kURLSeenColor = RGB(0x66, 0x00, 0x66);
const	RGBColor	kHeaderColor = RGB(0x00, 0x00, 0x00);
const	RGBColor	kTagColor = RGB(0x00, 0x80, 0x00);
#elif __dest_os == __linux_os
const	RGBColor	kServerOpenColor = JRGB(0x0000,0x8000,0x0000);
const	RGBColor	kServerClosedColor = JRGB(0x0000,0x0000,0x0000);
const	RGBColor	kServerBkgndColor = JRGB(0xBBBB,0xBBBB,0xBBBB);
const	RGBColor	kFavouriteBkgndColor = JRGB(0xDDDD,0xDDDD,0xDDDD);
const	RGBColor	kHierarchyBkgndColor = JRGB(0xDDDD,0xDDDD,0xDDDD);

const	RGBColor	kMboxRecentColor = JRGB(0xDDDD,0x0000,0x0000);
const	RGBColor	kMboxUnseenColor = JRGB(0xFFFF,0x0000,0xCCCC);
const	RGBColor	kMboxOpenColor = JRGB(0x0000,0x8000,0x0000);
const	RGBColor	kMboxFavouriteColor = JRGB(0x0000,0x0000,0xDDDD);
const	RGBColor	kMboxClosedColor = JRGB(0x0000,0x0000,0x0000);

const	RGBColor	kMsgUnseenColor = JRGB(0x0000,0x0000,0xDDDD);
const	RGBColor	kMsgSeenColor = JRGB(0x0000,0x0000,0x0000);
const	RGBColor	kMsgAnsweredColor = JRGB(0x0000,0x8000,0x0000);
const	RGBColor	kMsgImportantColor = JRGB(0xFFFF,0x0000,0xCCCC);
const	RGBColor	kMsgDeletedColor = JRGB(0xDDDD,0x0000,0x0000);
const	RGBColor	kMsgMatchColor = JRGB(0xA000,0xA000,0xA000);
const	RGBColor	kMsgNonMatchColor = JRGB(0xA000,0x4000,0x4000);

const	RGBColor	kLabelColor[] = { JRGB(0xFFFF, 0x6666, 0x0000),
										JRGB(0xCCCC, 0x0000, 0x0000),
										JRGB(0xFFFF, 0x0000, 0x9999),
										JRGB(0x0000, 0x9999, 0xFFFF),
										JRGB(0x0000, 0x0000, 0xCCCC),
										JRGB(0x0000, 0x6666, 0x0000),
										JRGB(0x6666, 0x3333, 0x0000),
										JRGB(0xCCCC, 0x6666, 0xCCCC) };
const	RGBColor	kLabelBkgColor = JRGB(0xFFFF, 0xFFFF, 0xFFFF);

const	RGBColor	kQuotation1Color = JRGB(0x9999, 0x3333, 0x6666);
const	RGBColor	kQuotation2Color = JRGB(0x9999, 0x3333, 0x9999);
const	RGBColor	kQuotation3Color = JRGB(0x9999, 0x6666, 0x9999);
const	RGBColor	kQuotation4Color = JRGB(0x9999, 0x9999, 0x9999);
const	RGBColor	kQuotation5Color = JRGB(0xCCCC, 0xCCCC, 0xCCCC);

const 	RGBColor	kURLColor = JRGB(0x0000, 0x0000, 0xFFFF);
const	RGBColor	kURLSeenColor = JRGB(0x6666, 0x0000, 0x6666);
const	RGBColor	kHeaderColor = JRGB(0x0000, 0x0000, 0x0000);
const	RGBColor	kTagColor = JRGB(0x0000, 0x8000, 0x0000);

const RGBColor		kSelectionColour = JRGB(0x9999, 0xFFFF, 0xFFFF);
#endif

void CPreferences::InitNetworkControlPrefs()
{
	mAppIdleTimeout.mValue = 5 * 60;				// 5 minutes
	mWaitDialogStart.mValue = 5;					// 5 secs
	mWaitDialogMax.mValue = 5 * 60;					// 5 minutes
	mConnectRetryTimeout.mValue = 15;				// 15 secs
	mConnectRetryMaxCount.mValue = 3;				// 3 times
	mBackgroundTimeout.mValue = 30;					// 30 secs
	mTickleInterval.mValue = 25 * 60;				// 25 minutes
	mTickleIntervalExpireTimeout.mValue = 5 * 60;	// 5 minutes
	mSleepReconnect.mValue = true;					// Default to quick reconnect after sleep
}

void CPreferences::InitAccountPrefs()
{
	{
		CMailAccount* mail_acct = new CMailAccount;
		mail_acct->SetServerType(CINETAccount::eIMAP);
		mail_acct->SetName(cdstring("IMAP"));
		mail_acct->NewAccount();
		mMailAccounts.mValue.push_back(mail_acct);
	}
	{
		CSMTPAccount* smtp_acct = new CSMTPAccount;
		smtp_acct->SetServerType(CINETAccount::eSMTP);
		smtp_acct->SetName(cdstring("SMTP"));
		mSMTPAccounts.mValue.push_back(smtp_acct);
	}

	mRemoteCachePswd.mValue = true;
	mOSDefaultLocation.mValue = true;
}

void CPreferences::InitAlertPrefs()
{
	// Add default notifier
	CMailNotification notify;
	notify.SetName("Default");
	mMailNotification.mValue.push_back(notify);

	mAttachmentNotification.mValue.SetShowAlert(false);
#ifdef __use_speech
	mAttachmentNotification.mValue.SetPlaySound(!CSpeechSynthesis::Available());
	mAttachmentNotification.mValue.SetSpeakText(CSpeechSynthesis::Available());
#else
	mAttachmentNotification.mValue.SetPlaySound(true);
	mAttachmentNotification.mValue.SetSpeakText(false);
#endif
	mAttachmentNotification.mValue.SetTextToSpeak("This message has an attachment");
}

void CPreferences::InitDisplayPrefs()
{
	mServerOpenStyle.mValue.style = bold;
	mServerOpenStyle.mValue.color = kServerOpenColor;
	mServerClosedStyle.mValue.style = bold;
	mServerClosedStyle.mValue.color = kServerClosedColor;
	mServerBkgndStyle.mValue.style = 1;
	mServerBkgndStyle.mValue.color = kServerBkgndColor;
	mFavouriteBkgndStyle.mValue.style = 1;
	mFavouriteBkgndStyle.mValue.color = kFavouriteBkgndColor;
	mHierarchyBkgndStyle.mValue.style = 1;
	mHierarchyBkgndStyle.mValue.color = kHierarchyBkgndColor;

	mMboxRecentStyle.mValue.style = bold;
	mMboxRecentStyle.mValue.color = kMboxRecentColor;
	mMboxUnseenStyle.mValue.style = normal;
	mMboxUnseenStyle.mValue.color = kMboxUnseenColor;
	mMboxOpenStyle.mValue.style = normal;
	mMboxOpenStyle.mValue.color = kMboxOpenColor;
	mMboxFavouriteStyle.mValue.style = bold;
	mMboxFavouriteStyle.mValue.color = kMboxFavouriteColor;
	mMboxClosedStyle.mValue.style = normal;
	mMboxClosedStyle.mValue.color = kMboxClosedColor;
	mIgnoreRecent.mValue = false;

	unseen.mValue.style = bold;
	unseen.mValue.color = kMsgUnseenColor;
	seen.mValue.style = normal;
	seen.mValue.color = kMsgSeenColor;
	answered.mValue.style = normal;
	answered.mValue.color = kMsgAnsweredColor;
	important.mValue.style = bold;
	important.mValue.color = kMsgImportantColor;
	deleted.mValue.style = strike_through;
	deleted.mValue.color = kMsgDeletedColor;
	mMultiAddress.mValue = underline;
	mMatch.mValue.color = kMsgMatchColor;
	mMatch.mValue.style = 1;
	mNonMatch.mValue.color = kMsgNonMatchColor;
	mNonMatch.mValue.style = 0;
	mUseLocalTimezone.mValue = false;

	// Labels
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		SStyleTraits2 traits2;
		traits2.style = normal;
		traits2.color = kLabelColor[i];
		traits2.usecolor = true;;
		traits2.bkgcolor = kLabelBkgColor;
		traits2.usebkgcolor = false;
		traits2.name = "Label";
		traits2.name += cdstring(i + 1);
		
		mLabels.mValue.push_back(new SStyleTraits2(traits2));
		mIMAPLabels.mValue.push_back(cFLAGLABELS[i]);
	}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	{
		// Lucida Grande 10pt
		UTextTraits::LoadTextTraits(Txtr_DefaultAquaListPrefsText, mListTextTraits.mValue.traits);

		// Monaco 10pt
		UTextTraits::LoadTextTraits(Txtr_DefaultAquaTextPrefsText, mDisplayTextTraits.mValue.traits);

		// Monaco 9pt
		UTextTraits::LoadTextTraits(Txtr_DefaultTextPrefsText, mPrintTextTraits.mValue.traits);

		// Monaco 9pt
		UTextTraits::LoadTextTraits(Txtr_DefaultTextPrefsText, mCaptionTextTraits.mValue.traits);

		mAntiAliasFont.mValue = true;
	}
#elif __dest_os == __win32_os
	// Determine name of system font
	CFont* sysfont = CFont::FromHandle((HFONT) ::GetStockObject(DEFAULT_GUI_FONT));
	LOGFONT lf;
	sysfont->GetLogFont(&lf);

	const TCHAR* cDefaultFont1 = _T("Courier New");
	const TCHAR* cDefaultFont2 = lf.lfFaceName;
	int cDefaultFontSize1 = 100;
	int cDefaultFontSize2 = 80;
	CFontPopup::InitFontList();
	bool has_1 = CFontPopup::FontExists(cdstring(cDefaultFont1));
	
	CFont temp;
	temp.CreatePointFont(80, cDefaultFont2);
	temp.GetLogFont(&mListTextFontInfo.mValue.logfont);
	temp.DeleteObject();

	temp.CreatePointFont(has_1 ? cDefaultFontSize1 : cDefaultFontSize2, has_1 ? cDefaultFont1 : cDefaultFont2);
	temp.GetLogFont(&mDisplayTextFontInfo.mValue.logfont);
	temp.DeleteObject();

	temp.CreatePointFont(has_1 ? cDefaultFontSize1 : cDefaultFontSize2, has_1 ? cDefaultFont1 : cDefaultFont2);
	temp.GetLogFont(&mPrintTextFontInfo.mValue.logfont);
	temp.DeleteObject();

	temp.CreatePointFont(cDefaultFontSize1, has_1 ? cDefaultFont1 : cDefaultFont2);
	temp.GetLogFont(&mCaptionTextFontInfo.mValue.logfont);
	temp.DeleteObject();
#elif __dest_os == __linux_os
	mListTextFontInfo.mValue.fontname = 
		JGetDefaultFontName();
	mListTextFontInfo.mValue.size = 12;

	mPrintTextFontInfo.mValue.fontname = mDisplayTextFontInfo.mValue.fontname = 
		mCaptionTextFontInfo.mValue.fontname = JGetMonospaceFontName();   
	mPrintTextFontInfo.mValue.size = mDisplayTextFontInfo.mValue.size =
		mCaptionTextFontInfo.mValue.size = 10;
	
#else
#error __dest_os
#endif

#if __dest_os == __win32_os
	mMultiTaskbar.mValue = true;
#endif

	mUse3Pane.mValue = CAdminLock::sAdminLock.mForce3Pane;	// Use the admin lock value as the default for new preferences
	mToolbarShow.mValue = true;
	mToolbarSmallIcons.mValue = true;
	mToolbarShowIcons.mValue = true;
	mToolbarShowCaptions.mValue = true;
#ifdef __MULBERRY
	mToolbars.mValue = CToolbarManager::sToolbarManager.GetInfo();
#endif
}

void CPreferences::InitFormattingPrefs()
{
	mURLStyle.mValue.style = underline;
	mURLStyle.mValue.color = kURLColor;

	mURLSeenStyle.mValue.style = underline;
	mURLSeenStyle.mValue.color = kURLSeenColor;

	mHeaderStyle.mValue.style = bold;
	mHeaderStyle.mValue.color = kHeaderColor;

	mQuotationStyle.mValue.style = normal;
	mQuotationStyle.mValue.color = kQuotation1Color;

	mTagStyle.mValue.style = normal;
	mTagStyle.mValue.color = kTagColor;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	{
		// Monaco 10pt
		UTextTraits::LoadTextTraits(Txtr_DefaultAquaTextPrefsText, mFixedTextTraits.mValue.traits);

		// Times 12pt
		UTextTraits::LoadTextTraits(Txtr_DefaultHTMLPrefsText, mHTMLTextTraits.mValue.traits);
	}
#elif __dest_os == __win32_os
	// Determine name of system font
	CFont* sysfont = CFont::FromHandle((HFONT) ::GetStockObject(DEFAULT_GUI_FONT));
	LOGFONT lf;
	sysfont->GetLogFont(&lf);

	const TCHAR* cDefaultFont1 = _T("Courier New");
	const TCHAR* cDefaultFont2 = lf.lfFaceName;
	int cDefaultFontSize1 = 100;
	int cDefaultFontSize2 = 80;
	bool has_1 = CFontPopup::FontExists(cdstring(cDefaultFont1));
	
	CFont temp;
	temp.CreatePointFont(has_1 ? cDefaultFontSize1 : cDefaultFontSize2, has_1 ? cDefaultFont1 : cDefaultFont2);
	temp.GetLogFont(&mFixedTextFontInfo.mValue.logfont);
	temp.DeleteObject();

	temp.CreatePointFont(100, _T("Times New Roman"));
	temp.GetLogFont(&mHTMLTextFontInfo.mValue.logfont);
	temp.DeleteObject();
#elif __dest_os == __linux_os
	mFixedTextFontInfo.mValue.fontname = JGetMonospaceFontName();
	mFixedTextFontInfo.mValue.size = 10;

	mHTMLTextFontInfo.mValue.fontname = "Times";
	mHTMLTextFontInfo.mValue.size = 12;
	
#else
#error __dest_os
#endif
	mUseStyles.mValue = true;
	mMinimumFont.mValue = 8;

	mUseMultipleQuotes.mValue = true;
	mQuoteColours.mValue.push_back(kQuotation2Color);
	mQuoteColours.mValue.push_back(kQuotation3Color);
	mQuoteColours.mValue.push_back(kQuotation4Color);
	mQuoteColours.mValue.push_back(kQuotation5Color);

	mRecognizeQuotes.mValue.push_back(">");
	mRecognizeQuotes.mValue.push_back("#");
	mRecognizeQuotes.mValue.push_back("|");

	mRecognizeURLs.mValue.push_back("http://");
	mRecognizeURLs.mValue.push_back("https://");
	mRecognizeURLs.mValue.push_back("mailto:");
	mRecognizeURLs.mValue.push_back("ftp://");
	mRecognizeURLs.mValue.push_back("file://");
	mRecognizeURLs.mValue.push_back("imap://");
	mRecognizeURLs.mValue.push_back("acap://");
	mRecognizeURLs.mValue.push_back("ldap://");
	mRecognizeURLs.mValue.push_back("news:");
	mRecognizeURLs.mValue.push_back("nntp://");
	mRecognizeURLs.mValue.push_back("telnet://");
	mRecognizeURLs.mValue.push_back("gopher://");
	mRecognizeURLs.mValue.push_back("mid:");
	mRecognizeURLs.mValue.push_back("cid:");

#if __dest_os == __linux_os
	mSelectionColour.mValue = kSelectionColour;
#endif
}

void CPreferences::InitMailboxPrefs()
{
	openAtFirst.mValue = false;
	openAtLast.mValue = false;
	openAtFirstNew.mValue = true;
	mNoOpenPreview.mValue = false;
	mUnseenNew.mValue = true;
	mRecentNew.mValue = false;
	mNextIsNewest.mValue = true;
	expungeOnClose.mValue = false;
	warnOnExpunge.mValue = true;
	mWarnPuntUnseen.mValue = true;
	mDoRollover.mValue = true;
	mRolloverWarn.mValue = true;
	mDoMailboxClear.mValue = false;
	clear_warning.mValue = true;
	mRLoCache.mValue = 20;
	mRHiCache.mValue = 100;
	mRUseHiCache.mValue = false;
	mRCacheIncrement.mValue = 20;
	mRAutoCacheIncrement.mValue = true;
	mRSortCache.mValue = 100;
	mLLoCache.mValue = 50;
	mLHiCache.mValue = 100;
	mLUseHiCache.mValue = false;
	mLCacheIncrement.mValue = 50;
	mLAutoCacheIncrement.mValue = true;
	mLSortCache.mValue = 1000;
	mUseCopyToMboxes.mValue = false;
	mUseAppendToMboxes.mValue = false;
	mMRUMaximum.mValue = 10;
	{
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("All"), SACLRight(SACLRight::eACL_AllRights)));
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("Shared"), SACLRight(CMboxACL::eMboxACL_Lookup | CMboxACL::eMboxACL_Read | CMboxACL::eMboxACL_Seen | CMboxACL::eMboxACL_Write | CMboxACL::eMboxACL_Insert | CMboxACL::eMboxACL_Post | CMboxACL::eMboxACL_Delete)));
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("BBoard"), SACLRight(CMboxACL::eMboxACL_Lookup | CMboxACL::eMboxACL_Read | CMboxACL::eMboxACL_Seen | CMboxACL::eMboxACL_Insert | CMboxACL::eMboxACL_Post)));
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("Post-only BBoard"), SACLRight(CMboxACL::eMboxACL_Lookup | CMboxACL::eMboxACL_Read | CMboxACL::eMboxACL_Seen | CMboxACL::eMboxACL_Post)));
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("Read-only"), SACLRight(CMboxACL::eMboxACL_Lookup | CMboxACL::eMboxACL_Read | CMboxACL::eMboxACL_Seen)));
		mMboxACLStyles.mValue.push_back(std::make_pair(cdstring("Delivery Only"), SACLRight(CMboxACL::eMboxACL_Post)));
	}
	mPromptDisconnected.mValue = true;
	mAutoDial.mValue = true;
	mUIDValidityMismatch.mValue = true;
	mQuickSearch.mValue = 0;
	mQuickSearchVisible.mValue = true;
	mScrollForUnseen.mValue = true;
}

void CPreferences::InitFavouritesPrefs()
{
	// Make sure default set of favourites exists
	for(int index = CMailAccountManager::eFavouriteStart; index < CMailAccountManager::eFavouriteOthers; index++)
	{
		CFavouriteItem empty_item;

		mFavourites.mValue.push_back(empty_item);

		// Make sure only New Messages is initially visible
		if (index != CMailAccountManager::eFavouriteNew)
			mFavourites.mValue.at(index).SetVisible(false);
	}
}

void CPreferences::InitSearchPrefs()
{
	mMultipleSearch.mValue = true;
	mNumberMultipleSearch.mValue = 5;
	mLoadBalanceSearch.mValue = false;
	mOpenFirstSearchResult.mValue = true;
}

void CPreferences::InitRulesPrefs()
{
	mFilterManager = new CFilterManager;
	mFilterManager->PrefsInit();
}

void CPreferences::InitMessagePrefs()
{
	showMessageHeader.mValue = false;
	showStyled.mValue = true;
	saveMessageHeader.mValue = true;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSaveCreator.mValue = "ttxt";
#else
	mSaveCreator.mValue = "txt";
#endif
	deleteAfterCopy.mValue = true;
	mOpenDeleted.mValue = false;
	mCloseDeleted.mValue = false;
	mDoSizeWarn.mValue = true;
	warnMessageSize.mValue = 20L;
	mQuoteSelection.mValue = true;
	mAlwaysQuote.mValue = true;
	optionKeyReplyDialog.mValue = false;

	mHeaderCaption.mValue = "Page %page.  %smart-full  on %sent-short";
	mHeaderBox.mValue = true;
	mPrintSummary.mValue = true;
	mFooterCaption.mValue = "Printed for: %me-name  on  %now";
	mFooterBox.mValue = true;

	mOpenReuse.mValue = false;
	mAutoDigest.mValue = true;
	mExpandHeader.mValue = true;
	mExpandParts.mValue = true;

	mForwardChoice.mValue = true;
	mForwardQuoteOriginal.mValue = true;
	mForwardHeaders.mValue = false;
	mForwardAttachment.mValue = false;
	mForwardRFC822.mValue = true;
	mMDNOptions.mValue = eMDNPromptSend;
}

void CPreferences::InitLetterPrefs()
{
	mAutoInsertSignature.mValue = true;
	mSignatureEmptyLine.mValue = true;
	mSigDashes.mValue = false;
	mNoSubjectWarn.mValue = true;
	spaces_per_tab.mValue = 8;
	mTabSpace.mValue = false;
	mReplyQuote.mValue = ">";
	mReplyStart.mValue = "--On %sent-long %from-full wrote:";
	mReplyStart.mValue += os_endl;
	mReplyEnd.mValue = "";
	mReplyCursorTop.mValue = true;
	mReplyNoSignature.mValue = false;
	mForwardQuote.mValue = "";
	mForwardStart.mValue = "------------ Forwarded Message ------------";
	mForwardStart.mValue += os_endl;
	mForwardStart.mValue += "Date: %sent-long";
	mForwardStart.mValue += os_endl;
	mForwardStart.mValue += "From: %from-full";
	mForwardStart.mValue += os_endl;
	mForwardStart.mValue += "To: %to-full";
	mForwardStart.mValue += os_endl;
	mForwardStart.mValue += "Cc: %cc-full";
	mForwardStart.mValue += os_endl;
	mForwardStart.mValue += "Subject: %subject";
	mForwardStart.mValue += os_endl;
	mForwardEnd.mValue = "---------- End Forwarded Message ----------";
	mForwardEnd.mValue += os_endl;
	mForwardCursorTop.mValue = true;
	mForwardSubject.mValue = "* (fwd)";
	mLtrHeaderCaption.mValue = "Page %page. To: %to-full  about  %subject";
	mLtrFooterCaption.mValue = "Printed for: %me-name  on  %now";
	mSeparateBCC.mValue = true;
	mBCCCaption.mValue = "IMPORTANT! This message has been blind-carbon-copied to you.";
	mBCCCaption.mValue += os_endl;
	mBCCCaption.mValue += "Do not reply-to-all or forward it without the author's permission.";
	mBCCCaption.mValue += os_endl2;
	mShowCCs.mValue = false;
	mShowBCCs.mValue = false;
	mExternalEditor.mValue = "%s";
	mUseExternalEditor.mValue = false;
	mExternalEditAutoStart.mValue = false;
	wrap_length.mValue = 76;
	mWindowWrap.mValue = true;
	mDisplayAttachments.mValue = true;
	mAppendDraft.mValue = true;
	inbox_append.mValue = false;
	mDeleteOriginalDraft.mValue = false;
	mTemplateDrafts.mValue = true;
	mSmartURLPaste.mValue = true;
	mWarnReplySubject.mValue = true;
	mSaveOptions.mValue = eSaveDraftChoose;
	compose_as.mValue = eContentSubPlain;
	enrMultiAltPlain.mValue = true;
	enrMultiAltHTML.mValue = false;
	htmlMultiAltPlain.mValue = true;
	htmlMultiAltEnriched.mValue = false;
	mFormatFlowed.mValue = true;
	mAlwaysUnicode.mValue = true;
	mDisplayIdentityFrom.mValue = true;
	mAutoSaveDrafts.mValue = true;
	mAutoSaveDraftsInterval.mValue = 60;
}

void CPreferences::InitSecurityPrefs()
{
	mUseMIMESecurity.mValue = true;
	mEncryptToSelf.mValue = true;
	mCachePassphrase.mValue = false;
	mCachePassphraseMax.mValue = 60;
	mCachePassphraseIdle.mValue = 20;
	mAutoVerify.mValue = true;
	mAutoDecrypt.mValue = true;
	mWarnUnencryptedSend.mValue = true;
	mUseErrorAlerts.mValue = false;
	mCacheUserCerts.mValue = true;

	mVerifyOKNotification.mValue.SetShowAlert(true);
	mVerifyOKNotification.mValue.SetPlaySound(false);
	mVerifyOKNotification.mValue.SetSpeakText(false);
	mVerifyOKNotification.mValue.SetTextToSpeak("");
}

void CPreferences::InitIdentityPrefs()
{
	// Create default identity
	CIdentity id;
	id.SetIdentity(cdstring("Default"));
	id.SetSMTPAccount(cdstring("SMTP"), true);

	// Initial identity should have copy to turned on and it should be set to Choose.
	id.SetCopyTo(cdstring::null_str, true);
	id.SetCopyToChoose(true);
	
	// Add to list
	mIdentities.mValue.push_back(id);

	mTiedMailboxes.mValue.SetIdentityList(&mIdentities.mValue);
	mTiedMailboxes.mValue.SetType(CIdentity::eMbox);
	mTiedCalendars.mValue.SetIdentityList(&mIdentities.mValue);
	mTiedCalendars.mValue.SetType(CIdentity::eCal);
	mContextTied.mValue = false;
	mMsgTied.mValue = false;
	mTiedMboxInherit.mValue = false;
}

void CPreferences::InitAddressPrefs()
{
	{
		CAddressAccount temp;
		temp.SetServerType(CAddressAccount::eLocalAdbk);
		cdstring acct_name;
		acct_name.FromResource("UI::AdbkMgr::AddressBooks");
		temp.SetName(acct_name);
		mLocalAdbkAccount.Value() = temp;
	}

	{
		CAddressAccount temp;
		temp.SetServerType(CAddressAccount::eOSAdbk);
		cdstring acct_name;
		acct_name.FromResource("UI::AdbkMgr::OSAddressBooks");
		temp.SetName(acct_name);
		mOSAdbkAccount.Value() = temp;
	}

	mAdbkACLStyles.mValue.push_back(std::make_pair(cdstring("All"), SACLRight(SACLRight::eACL_AllRights)));
	mAdbkACLStyles.mValue.push_back(std::make_pair(cdstring("Shared"), SACLRight(CAdbkACL::eAdbkACL_Lookup | CAdbkACL::eAdbkACL_Read | CAdbkACL::eAdbkACL_Write | CAdbkACL::eAdbkACL_Delete)));
	mAdbkACLStyles.mValue.push_back(std::make_pair(cdstring("Read-only"), SACLRight(CAdbkACL::eAdbkACL_Lookup | CAdbkACL::eAdbkACL_Read)));

	mOptionKeyAddressDialog.mValue = false;

	mExpandNoNicknames.mValue = false;
	mExpandFailedNicknames.mValue = true;
	mExpandFullName.mValue = true;
	mExpandNickName.mValue = false;
	mExpandEmail.mValue = false;
	mSkipLDAP.mValue = false;
	mCaptureAllowEdit.mValue = false;
	mCaptureAllowChoice.mValue = true;
	mCaptureRead.mValue = false;
	mCaptureRespond.mValue = false;
	mCaptureFrom.mValue = true;
	mCaptureCc.mValue = false;
	mCaptureReplyTo.mValue = false;
	mCaptureTo.mValue = false;
}

void CPreferences::InitCalendarPrefs()
{
	{
		CCalendarAccount temp;
		temp.SetServerType(CCalendarAccount::eLocalCalendar);
		cdstring acct_name(rsrc::GetString("Preferences::LocalCalendarAccount"));
		temp.SetName(acct_name);
		mLocalCalendarAccount.Value() = temp;
	}

	{
		CCalendarAccount temp;
		temp.SetServerType(CCalendarAccount::eHTTPCalendar);
		cdstring acct_name(rsrc::GetString("Preferences::WebCalendarAccount"));
		temp.SetName(acct_name);
		mWebCalendarAccount.Value() = temp;
	}

	mWeekStartDay.mValue = iCal::CICalendarDateTime::eSunday;
	mWorkDayMask.mValue = (1L << iCal::CICalendarDateTime::eMonday) |
							(1L << iCal::CICalendarDateTime::eTuesday) |
							(1L << iCal::CICalendarDateTime::eWednesday) |
							(1L << iCal::CICalendarDateTime::eThursday) |
							(1L << iCal::CICalendarDateTime::eFriday);
	mDisplayTime.mValue = true;
	m24HourTime.mValue = false;
	mHandleICS.mValue = true;
	mAutomaticIMIP.mValue = false;
	mAutomaticEDST.mValue = true;
    mShowUID.mValue = false;
}

void CPreferences::InitAttachmentPrefs()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mDefault_mode.mValue = eAppleDoubleMode;
#else
	mDefault_mode.mValue = eUUMode;
#endif
	mDefault_Always.mValue = false;
	mViewDoubleClick.mValue = true;
	mAskDownload.mValue = true;
	mAppLaunch.mValue = eAppLaunchAsk;
	mLaunchText.mValue = true;
#if __dest_os == __win32_os
	mShellLaunch.mValue = cdstring::null_str;
#endif
	mExplicitMapping.mValue = true;
	
	mWarnMissingAttachments.mValue = true;
	mMissingAttachmentSubject.mValue = true;
	mMissingAttachmentWords.mValue.push_back("attachment");
	mMissingAttachmentWords.mValue.push_back("attached");
}

#ifdef __use_speech
void CPreferences::InitSpeechPrefs()
{
	cdstring temp;
	mSpeakNewOpen.mValue = false;
	mSpeakNewArrivals.mValue = false;
#ifdef __MULBERRY_CONFIGURE_V2_0
	// Special to support v2.0.x admin tool
	mSpeakNewAlert.mValue = true;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	temp.FromResource(STRx_Speech, str_DefaultAlert);
#else
	temp.FromResource(IDS_SPEAK_DEFAULTALERT);
#endif
	mSpeakNewAlertTxt.mValue = temp;
#endif
	{
		CMessageSpeak speaker;
		speaker.mItem = eMessageSpeakFrom1;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultFrom);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTFROM);
#endif
		speaker.mItemText = temp;
		mSpeakMessageItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakSubject;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultSubject);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTSUBJECT);
#endif
		speaker.mItemText = temp;
		mSpeakMessageItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakBodyNoHdr;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultBody);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTBODY);
#endif
		speaker.mItemText = temp;
		mSpeakMessageItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakNone;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultEnd);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTEND);
#endif
		speaker.mItemText = temp;
		mSpeakMessageItems.mValue.push_back(speaker);
	}
	{
		CMessageSpeak speaker;
		speaker.mItem = eMessageSpeakTo1;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultTo);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTTO);
#endif
		speaker.mItemText = temp;
		mSpeakLetterItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakSubject;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultSubject);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTSUBJECT);
#endif
		speaker.mItemText = temp;
		mSpeakLetterItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakBodyNoHdr;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultBody);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTBODY);
#endif
		speaker.mItemText = temp;
		mSpeakLetterItems.mValue.push_back(speaker);
		speaker.mItem = eMessageSpeakNone;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		temp.FromResource(STRx_Speech, str_DefaultEnd);
#else
		temp.FromResource(IDS_SPEAK_DEFAULTEND);
#endif
		speaker.mItemText = temp;
		mSpeakLetterItems.mValue.push_back(speaker);
	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	temp.FromResource(STRx_Speech, str_EmptyItem);
#else
	temp.FromResource(IDS_SPEAK_DEFAULTEMPTY);
#endif
	mSpeakMessageEmptyItem.mValue = temp;
	mSpeakMessageMaxLength.mValue = 4096;
}
#endif

void CPreferences::InitMiscellaneousPrefs()
{
	mTextMacros.mValue.Add("mulberry", "email is my life");
	mAllowKeyboardShortcuts.mValue = true;
}

void CPreferences::InitWindowPrefs()
{
	mStatusWindowDefault.mValue = CStatusWindowState();
#if __dest_os == __win32_os
	mMDIWindowDefault.mValue = CMDIWindowState();
#endif
	m3PaneWindowDefault.mValue = C3PaneWindowState();
	m3PaneOptions.mValue = C3PaneOptions();
	m1PaneOptions.mValue = C3PaneOptions();
	mServerWindowDefault.mValue = CServerWindowState();
	mServerBrowseDefault.mValue = CWindowState();
	mMailboxWindowDefault.mValue = CMailboxWindowState();
	mSMTPWindowDefault.mValue = CMailboxWindowState();
	mMessageWindowDefault.mValue = CMessageWindowState();
	mMessageView3Pane.mValue = CMessageWindowState();
	mMessageView1Pane.mValue = CMessageWindowState();
	mLetterWindowDefault.mValue = CLetterWindowState();
#if 0
	mAddressBookWindowDefault.mValue = CAddressBookWindowState();
#else
	mNewAddressBookWindowDefault.mValue = CNewAddressBookWindowState();
#endif
	mAdbkManagerWindowDefault.mValue = CAdbkManagerWindowState();
	mAdbkSearchWindowDefault.mValue = CAdbkSearchWindowState();
	mFindReplaceWindowDefault.mValue = CFindReplaceWindowState();
	mSearchWindowDefault.mValue = CSearchWindowState();
	mRulesWindowDefault.mValue = CRulesWindowState();
	mRulesDialogDefault.mValue = CWindowState();
	mCalendarStoreWindowDefault.mValue = CCalendarStoreWindowState();
	mCalendarWindowDefault.mValue = CCalendarWindowState();
}
