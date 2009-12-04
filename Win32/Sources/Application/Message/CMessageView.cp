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


// Source for CMessageView class


#include "CMessageView.h"

#include "CActionManager.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CBodyTable.h"
#include "CCmdUIChecker.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CDisplayFormatter.h"
#include "CFontCache.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxView.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CSpeechSynthesis.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CUnicodeUtils.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include "StValueChanger.h"

#include <strstream.h>

/////////////////////////////////////////////////////////////////////////////
// CMessageView

BEGIN_MESSAGE_MAP(CMessageView, CBaseView)
	ON_UPDATE_COMMAND_UI(IDM_FILE_NEW_DRAFT, OnUpdateAlways)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnFileNewDraft)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_SPEAK, OnUpdateEditSpeak)
	ON_COMMAND(IDM_EDIT_SPEAK, OnEditSpeak)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_SPELLCHECK, OnUpdateAlways)
	ON_COMMAND(IDM_EDIT_SPELLCHECK, OnEditSpellCheck)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_READ_PREV, OnUpdateMessageReadPrev)
	ON_COMMAND(IDM_MESSAGES_READ_PREV, OnMessageReadPrev)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_READ_NEXT, OnUpdateMessageReadNext)
	ON_COMMAND(IDM_MESSAGES_READ_NEXT, OnMessageReadNext)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_DELETE_READ, OnUpdateMessageDelete)
	ON_COMMAND(IDM_MESSAGES_DELETE_READ, OnMessageDeleteRead)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_COPY_READ, OnUpdateMessageCopyNext)
	ON_COMMAND(IDM_MESSAGES_COPY_READ, OnMessageCopyRead)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_REPLY, OnMessageReply)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_SENDER, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_REPLY_SENDER, OnMessageReplySender)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_FROM, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_REPLY_FROM, OnMessageReplyFrom)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REPLY_ALL, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_REPLY_ALL, OnMessageReplyAll)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_FORWARD, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_FORWARD, OnMessageForward)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_BOUNCE, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_BOUNCE, OnMessageBounce)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_REJECT, OnUpdateMessageReject)
	ON_COMMAND(IDM_MESSAGES_REJECT, OnMessageReject)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_SEND_AGAIN, OnUpdateMessageSendAgain)
	ON_COMMAND(IDM_MESSAGES_SEND_AGAIN, OnMessageSendAgain)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_CopyToMailboxChoose, IDM_CopyToMailboxEnd, OnUpdateMessageCopy)
	ON_COMMAND_RANGE(IDM_CopyToMailboxChoose, IDM_CopyToMailboxEnd, OnMessageCopy)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_COPY_NOW, OnUpdateMessageCopy)
	ON_COMMAND(IDM_MESSAGES_COPY_NOW, OnMessageCopyBtn)

	ON_COMMAND(IDC_MAILBOXCOPYCMD, OnMessageCopyCmd)
	ON_COMMAND(IDC_MAILBOXMOVECMD, OnMessageMoveCmd)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_TEXTFORMAT_FORMAT, IDM_TEXTFORMAT_RFC822, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_TEXTFORMAT_FORMAT, IDM_TEXTFORMAT_RFC822, OnTextFormatPopup)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_QUOTEDEPTH_ALL, IDM_QUOTEDEPTH_4, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_QUOTEDEPTH_ALL, IDM_QUOTEDEPTH_4, OnQuoteDepthPopup)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_AddressCopyStart, IDM_AddressCopyEnd, OnUpdateAlways)

	ON_UPDATE_COMMAND_UI(IDM_FLAGS_SEEN, OnUpdateMessageFlagsSeen)
	ON_COMMAND(IDM_FLAGS_SEEN, OnMessageFlagsSeen)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_IMPORTANT, OnUpdateMessageFlagsImportant)
	ON_COMMAND(IDM_FLAGS_IMPORTANT, OnMessageFlagsImportant)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_ANSWERED, OnUpdateMessageFlagsAnswered)
	ON_COMMAND(IDM_FLAGS_ANSWERED, OnMessageFlagsAnswered)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_DELETED, OnUpdateMessageFlagsDeleted)
	ON_COMMAND(IDM_FLAGS_DELETED, OnMessageDelete)
	ON_UPDATE_COMMAND_UI(IDM_FLAGS_DRAFT, OnUpdateMessageFlagsDraft)
	ON_COMMAND(IDM_FLAGS_DRAFT, OnMessageFlagsDraft)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FLAGS_LABEL1, IDM_FLAGS_LABEL8, OnUpdateMessageFlagsLabel)
	ON_COMMAND_RANGE(IDM_FLAGS_LABEL1, IDM_FLAGS_LABEL8, OnMessageFlagsLabel)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_VIEW_PARTS, OnUpdateMessageViewCurrent)
	ON_COMMAND(IDM_MESSAGES_VIEW_PARTS, OnMessageViewCurrent)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_DECODE_BINHEX, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_DECODE_BINHEX, OnMessageDecodeBinHex)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_DECODE_UU, OnUpdateAlways)
	ON_COMMAND(IDM_MESSAGES_DECODE_UU, OnMessageDecodeUU)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_VERIFY_DECRYPT, OnUpdateMessageVerifyDecrypt)
	ON_COMMAND(IDM_MESSAGES_VERIFY_DECRYPT, OnMessageVerifyDecrypt)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_DELETE, OnUpdateMessageDelete)
	ON_COMMAND(IDM_MESSAGES_DELETE, OnMessageDelete)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_SHOW_HEADER, OnUpdateMessageShowHeader)
	ON_COMMAND(IDM_MESSAGES_SHOW_HEADER, OnMessageShowHeader)

	ON_COMMAND(IDC_MESSAGES_FONTINCREASE, OnIncreaseFont)
	ON_COMMAND(IDC_MESSAGES_FONTDECREASE, OnDecreaseFont)

	ON_COMMAND(IDC_MESSAGEPARTSTWISTER, OnMessagePartsTwister)

	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOW_PARTS, OnUpdateWindowsShowParts)
	ON_COMMAND(IDM_WINDOWS_SHOW_PARTS, OnMessagePartsTwister)

	ON_COMMAND(IDC_MESSAGEPARTSFLATBTN, OnMessagePartsFlat)

	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnZoomPane)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnFileNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnFileNewDraft)

	//ON_COMMAND(IDC_TOOLBARFILESAVEBTN, OnFileSave)

	ON_COMMAND(IDC_TOOLBARMESSAGEPRINTBTN, OnFilePrint)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEPREVIOUSBTN, OnUpdateMessageReadPrev)
	ON_COMMAND(IDC_TOOLBARMESSAGEPREVIOUSBTN, OnMessageReadPrev)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGENEXTBTN, OnUpdateMessageReadNext)
	ON_COMMAND(IDC_TOOLBARMESSAGENEXTBTN, OnMessageReadNext)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEDELETENEXTBTN, OnUpdateMessageDelete)
	ON_COMMAND(IDC_TOOLBARMESSAGEDELETENEXTBTN, OnMessageDeleteRead)

	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYBTN, OnMessageReply)
	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYOPTIONBTN, OnMessageReply)

	ON_COMMAND(IDC_TOOLBARMESSAGEREPLYALLBTN, OnMessageReplyAll)

	ON_COMMAND(IDC_TOOLBARMESSAGEFORWARDBTN, OnMessageForward)
	ON_COMMAND(IDC_TOOLBARMESSAGEFORWARDOPTIONBTN, OnMessageForward)

	ON_COMMAND(IDC_TOOLBARMESSAGEBOUNCEBTN, OnMessageBounce)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEREJECTBTN, OnUpdateMessageReject)
	ON_COMMAND(IDC_TOOLBARMESSAGEREJECTBTN, OnMessageReject)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGESENDAGAINBTN, OnUpdateMessageSendAgain)
	ON_COMMAND(IDC_TOOLBARMESSAGESENDAGAINBTN, OnMessageSendAgain)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGECOPYBTN, OnUpdateMessageCopy)
	ON_COMMAND(IDC_TOOLBARMESSAGECOPYBTN, OnMessageCopyBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEDELETEBTN, OnUpdateMessageDelete)
	ON_COMMAND(IDC_TOOLBARMESSAGEDELETEBTN, OnMessageDelete)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARMESSAGEFLAGSBTN, OnUpdateAlways)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageView construction/destruction

cdmutexprotect<CMessageView::CMessageViewList> CMessageView::sMsgViews;
cdstring  CMessageView::sLastCopyTo;

// Default constructor
CMessageView::CMessageView()
{
	// Add to list
	{
		cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
		sMsgViews->push_back(this);
	}

	mItsMsg = NULL;
	mMailboxView = NULL;
	mWasUnseen = false;
	mItsMsgError = false;
	mShowText = NULL;
	mCurrentPart = NULL;
	mShowHeader = CPreferences::sPrefs->showMessageHeader.GetValue();
	mShowAddressCaption = true;
	mShowParts = true;
	mShowSecure = true;
	mSecureMulti = true;
	mParsing = eViewFormatted;
	mFontScale = 0;
	mQuoteDepth = -1;

	mRedisplayBlock = false;
	mResetTextBlock = false;
	mAllowDeleted = false;
	mDidExpandParts = false;
	
	mMailboxView = NULL;
	mText = NULL;
	mPartsTableAlign = NULL;
	
	mTimerID = 0;
}

// Default destructor
CMessageView::~CMessageView()
{
	// Always remove timer
	StopSeenTimer();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMessageView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	const int cSmallIconBtnSize = 20;

	const int cHdrCaptionHeight = CMulberryApp::sLargeFont ? 48 : 40;
	int cHdrCaptionWidth = width - cSmallIconBtnSize - 8;

	const int cPartsHeaderTop = cHdrCaptionHeight;
	const int cPartsHeaderHeight = 25;

	const int cHeaderHeight = cHdrCaptionHeight + cPartsHeaderHeight + 2;

	const int cSplitterTop = cHeaderHeight;
	const int cSplitterMin1 = 84;
	const int cSplitterMin2 = 32;

	const int cCaptionHeight = 16;
	const int cEditHeight = 22;

	const int cFromTitleTop = 8;

	const int cTitleLeft = 12;
	const int cTitleWidth = 46;
	const int cTwisterLeft = cTitleLeft + cTitleWidth;
	const int cTwisterTopOffset = 0;
	const int cTwistOffset = 32;
	const int cEditLeft = cTwisterLeft + 16;
	const int cEditTopOffset = -5;
	const int cEditWidth = 310;

	const int cPartsTwisterLeft = 0;
	const int cPartsTitleLeft = 15;
	const int cPartsLeft = cPartsTitleLeft + 46;
	const int cPartsWidth = 48;
	const int cAttachmentsLeft = cPartsLeft + cPartsWidth + 8;
	const int cHeaderBtnLeft = cAttachmentsLeft + 24;
	const int cFormatPopupLeft = cHeaderBtnLeft + 32;
	const int cFormatPopupWidth = 96;

	const int cFontIncreaseLeft = cFormatPopupLeft + cFormatPopupWidth + 24;
	const int cFontIncreaseWidth = 24;
	const int cFontDecreaseLeft = cFontIncreaseLeft + cFontIncreaseWidth;
	const int cFontDecreaseWidth = cFontIncreaseWidth;
	const int cFontScaleLeft = cFontDecreaseLeft + cFontDecreaseWidth + 8;
	const int cFontScaleWidth = 24;

	const int cQuoteDepthPopupLeft = cFontScaleLeft + cFontScaleWidth + 16;
	const int cQuoteDepthPopupWidth = 96;

	const int cCopyToTitleLeft = cFormatPopupLeft + cFormatPopupWidth + 8;
	const int cCopyToTitleWidth = 56;
	const int cCopyToPopupLeft = cCopyToTitleLeft + cCopyToTitleWidth;

	const int cCopyToBtnWidth = 24;
	const int cCopyToBtnOffset = 8;
	const int cCopyToBtnLeft = width - cCopyToBtnOffset - cCopyToBtnWidth;
	const int cCopyToPopupRight = cCopyToBtnLeft - cCopyToBtnOffset;

	const int cPartTitleHeight = 16;

	// Set commander (do before changing is-3pane)
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	// Treat as 3-pane if in 3-pane window or is a preview in mailbox window
	CMailboxWindow* mbox_preview = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
	mIs3Pane = mbox_preview || (mOwnerWindow == C3PaneWindow::s3PaneWindow);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Create main header view
	CRect rect = CRect(0, 0, width, cHeaderHeight);
	mHeader.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_TopWidth));

	// Create header display section
	rect = CRect(0, 0, cHdrCaptionWidth / 2, cHdrCaptionHeight - 6);
	mCaption1.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, rect, &mHeader, IDC_MESSAGEPREVIEWCAPTION1);
	mCaption1.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	//mHeader.AddAlignment(new CWndAlignment(&mCaption, CWndAlignment::eAlign_TopWidth));
	mCaption1.SetBackgroundColor(false, ::GetSysColor(COLOR_BTNFACE));
	mCaption1.SetFont(CMulberryApp::sAppFont);
	mCaption1.SetReadOnly(true);

	rect = CRect(cHdrCaptionWidth / 2, 0, cHdrCaptionWidth / 2, cHdrCaptionHeight - 6);
	mCaption2.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, rect, &mHeader, IDC_MESSAGEPREVIEWCAPTION2);
	mCaption2.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	//mHeader.AddAlignment(new CWndAlignment(&mCaption, CWndAlignment::eAlign_TopWidth));
	mCaption2.SetBackgroundColor(false, ::GetSysColor(COLOR_BTNFACE));
	mCaption2.SetFont(CMulberryApp::sAppFont);
	mCaption2.SetReadOnly(true);

	CString s;
	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), &mHeader, IDC_3PANETOOLBAR_ZOOMPREVIEW, 0, IDI_3PANE_ZOOM);
	mHeader.AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);
	if (mOwnerWindow != C3PaneWindow::s3PaneWindow)
		mZoom.ShowWindow(SW_HIDE);
	
	// Create parts view
	rect = CRect(0, cPartsHeaderTop, width, cPartsHeaderTop + cPartsHeaderHeight);
	mPartsMove.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mPartsMove, CWndAlignment::eAlign_BottomWidth));

	// Divider
	rect = CRect(0, 0, width, 2);
	mDivider.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, rect, &mPartsMove, IDC_STATIC);
	mPartsMove.AddAlignment(new CWndAlignment(&mDivider, CWndAlignment::eAlign_TopWidth));

	// Create parts section
	CRect r = CRect(cPartsTwisterLeft, cFromTitleTop + cTwisterTopOffset, cPartsTwisterLeft + 16, cFromTitleTop + cTwisterTopOffset + 16);
	mPartsTwister.Create(r, &mPartsMove, IDC_MESSAGEPARTSTWISTER);
	s.LoadString(IDS_MESSAGEPARTSTITLE);
	r = CRect(cPartsTitleLeft, cFromTitleTop, cPartsTitleLeft + cTitleWidth, cFromTitleTop + cCaptionHeight);
	mPartsTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_STATIC);
	mPartsTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cPartsLeft, cFromTitleTop, cPartsLeft + cPartsWidth, cFromTitleTop + cCaptionHeight);
	s = "";
	mPartsField.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_MESSAGEPARTSFIELD);
	mPartsField.SetFont(CMulberryApp::sAppFont);

	mFlatHierarchyBtn.Create(_T(""), CRect(cPartsLeft, cFromTitleTop, cPartsLeft + 16, cFromTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSFLATBTN, IDC_STATIC, IDI_SERVERFLAT, IDI_SERVERHIERARCHIC);
	mFlatHierarchyBtn.ShowWindow(SW_HIDE);

	mAttachments.Create(NULL, WS_CHILD, CRect(cAttachmentsLeft, cFromTitleTop, cAttachmentsLeft + 16, cFromTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSATTACHMENTS);
	mAttachments.SetIconID(IDI_MSG_ATTACHMENT);
	mAlternative.Create(NULL, WS_CHILD, CRect(cAttachmentsLeft, cFromTitleTop, cAttachmentsLeft + 16, cFromTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSALTERNATIVE);
	mAlternative.SetIconID(IDI_MSG_ALTERNATIVE);

	// Extra parts list bits
	mHeaderBtn.Create(_T(""), CRect(cHeaderBtnLeft, cFromTitleTop + cEditTopOffset, cHeaderBtnLeft + 24, cFromTitleTop + cEditTopOffset + 22), &mPartsMove, IDM_MESSAGES_SHOW_HEADER, IDC_STATIC, IDI_HEADERMSG);

	r = CRect(cFormatPopupLeft, cFromTitleTop + cEditTopOffset, cFormatPopupLeft + cFormatPopupWidth, cFromTitleTop + cEditTopOffset + cEditHeight);
	mTextFormat.Create(_T(""), r, &mPartsMove, IDM_MESSAGES_TEXT_FORMAT, IDC_STATIC, IDI_POPUPBTN);
	mTextFormat.SetMenu(IDR_POPUP_TEXTFORMAT);
	mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT);
	mTextFormat.SetFont(CMulberryApp::sAppFont);

	mFontIncreaseBtn.Create(_T(""), CRect(cFontIncreaseLeft, cFromTitleTop + cEditTopOffset, cFontIncreaseLeft + cFontIncreaseWidth, cFromTitleTop + cEditTopOffset + 22), &mPartsMove, IDC_MESSAGES_FONTINCREASE, IDC_STATIC, IDI_FONTINCREASE);
	mFontDecreaseBtn.Create(_T(""), CRect(cFontDecreaseLeft, cFromTitleTop + cEditTopOffset, cFontDecreaseLeft + cFontDecreaseWidth, cFromTitleTop + cEditTopOffset + 22), &mPartsMove, IDC_MESSAGES_FONTDECREASE, IDC_STATIC, IDI_FONTDECREASE);
	mFontScaleField.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(cFontScaleLeft, cFromTitleTop, cFontScaleLeft + cFontScaleWidth, cFromTitleTop + cCaptionHeight), &mPartsMove, IDC_MESSAGES_FONTSCALE);
	mFontScaleField.SetFont(CMulberryApp::sAppFont);

	r = CRect(cQuoteDepthPopupLeft, cFromTitleTop + cEditTopOffset, cQuoteDepthPopupLeft + cQuoteDepthPopupWidth, cFromTitleTop + cEditTopOffset + cEditHeight);
	mQuoteDepthPopup.Create(_T(""), r, &mPartsMove, IDM_QUOTEDEPTH_ALL, IDC_STATIC, IDI_POPUPBTN);
	mQuoteDepthPopup.SetMenu(IDR_POPUP_QUOTEDEPTH);
	mQuoteDepthPopup.SetValue(IDM_QUOTEDEPTH_ALL);
	mQuoteDepthPopup.SetFont(CMulberryApp::sAppFont);

	// Create splitter view
	rect = CRect(0, cSplitterTop, width, height);
	mSplitter.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mSplitter, CWndAlignment::eAlign_WidthHeight));

	// Parts focus - only in 3-pane window - not mailbox preview or standalone
	mPartsFocus.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width, height), this, IDC_STATIC);
	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
		mPartsFocus.SetFocusBorder();
	UINT focus_indent = (mOwnerWindow == C3PaneWindow::s3PaneWindow) ? 3 : 0;

	// Parts table
	mPartsTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
							CRect(focus_indent, focus_indent + cPartTitleHeight, width - focus_indent, height - focus_indent), &mPartsFocus, IDC_MESSAGEPARTSTABLE);
	mPartsTable.SetSuperCommander(this);
	mPartsTable.ModifyStyleEx(0, (mOwnerWindow == C3PaneWindow::s3PaneWindow ? 0 : WS_EX_CLIENTEDGE) | WS_EX_NOPARENTNOTIFY, 0);
	mPartsTable.ResetFont(CFontCache::GetListFont());
	mPartsTable.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_PARTS);
	mPartsTable.SetContextView(static_cast<CView*>(GetOwningWindow()));
	//mPartsTable.SetTabOrder(((CMessageWindow*) GetParent())->GetText(), &mDateField, ((CMessageWindow*) GetParent())->GetText(), this);
	mPartsFocus.AddAlignment(mPartsTableAlign = new CWndAlignment(&mPartsTable, CWndAlignment::eAlign_WidthHeight));

	// Parts titles
	mPartsTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent, width - focus_indent, focus_indent + cPartTitleHeight), &mPartsFocus, IDC_MESSAGEPARTSTABLETITLES);
	mPartsTitles.SyncTable(&mPartsTable);
	mPartsTable.SetTitles(&mPartsTitles);
	mPartsFocus.AddAlignment(new CWndAlignment(&mPartsTitles, CWndAlignment::eAlign_TopWidth));

	mPartsTitles.LoadTitles("UI::Titles::MessageParts", 5);

	// Create bottom splitter pane
	rect = CRect(0, 0, width, height);
	mSplitter2Pane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Create secure info pane
	rect = CRect(0, 0, width, cHdrCaptionHeight + 4);
	mSecurePane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mSplitter2Pane, IDC_STATIC);
	mSecurePane.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	mSplitter2Pane.AddAlignment(new CWndAlignment(&mSecurePane, CWndAlignment::eAlign_TopWidth));

	// Create header display section
	rect = CRect(0, 0, cHdrCaptionWidth, cHdrCaptionHeight - 2);
	mSecureInfo.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, rect, &mSecurePane, IDC_STATIC);
	mSecureInfo.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	mSecurePane.AddAlignment(new CWndAlignment(&mSecureInfo, CWndAlignment::eAlign_TopWidth));
	mSecureInfo.SetBackgroundColor(false, ::GetSysColor(COLOR_BTNFACE));
	mSecureInfo.SetFont(CMulberryApp::sAppFont);
	mSecureInfo.SetReadOnly(true);

	// Text focus - only in 3-pane window - not mailbox preview or standalone
	mTextFocus.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, cHdrCaptionHeight + 4, width, height), &mSplitter2Pane, IDC_STATIC);
	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
		mTextFocus.SetFocusBorder();
	mSplitter2Pane.AddAlignment(new CWndAlignment(&mTextFocus, CWndAlignment::eAlign_WidthHeight));

	// Create message text view
	rect = CRect(focus_indent, focus_indent, width - focus_indent, height - cHdrCaptionHeight - 4 - focus_indent);
	mText = new CFormattedTextDisplay;
	mText->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mTextFocus, IDC_STATIC);
	mText->SetSuperCommander(this);
	mText->ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mText->GetRichEditCtrl().SetReadOnly(true);
	mText->GetRichEditCtrl().SetModify(false);
	mText->SetFont(CFontCache::GetDisplayFont());
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetWrap(CPreferences::sPrefs->wrap_length.GetValue());
	mText->SetHardWrap(false);
	mText->SetMessageView(this);
	mText->SetFindAllowed(true);
	mText->SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_TEXT);
	mText->SetTabSelectAll(false);
	mText->Add_Listener(this);
	mTextFocus.AddAlignment(new CWndAlignment(mText, CWndAlignment::eAlign_WidthHeight));

	// Hide the verify/decrypt details
	ShowSecretPane(false);

	// Install the splitter items
	mSplitter.InstallViews(&mPartsFocus, &mSplitter2Pane, true);
	mSplitter.ShowView(false, true);
	mSplitter.SetMinima(cSplitterMin1, cSplitterMin2);
	mSplitter.SetPixelSplitPos(cSplitterMin1);
	mSplitter.SetLockResize(true);
	
	// Reset to default size
	ResetState();

	// Reset based on current view options
	ResetOptions();

	// Do this to disable parts toolbar items
	MessageChanged();
	SetOpen();

	return 0;
}

// Tidy up when window closed
void CMessageView::OnDestroy()
{
	// Set status
	SetClosing();

	// Remove from list
	{
		cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
		CMessageViewList::iterator found = ::find(sMsgViews->begin(), sMsgViews->end(), this);
		if (found != sMsgViews->end())
			sMsgViews->erase(found);
	}

	// Set status
	SetClosed();
}

// Tidy up when window closed
void CMessageView::OnSize(UINT nType, int cx, int cy)
{
	const int cSmallIconBtnSize = 20;
	const int cHdrCaptionHeight = CMulberryApp::sLargeFont ? 48 : 40;

	CBaseView::OnSize(nType, cx, cy);
	
	// Adjust caption halves
	CRect rect;
	mHeader.GetWindowRect(rect);
	
	const int caption_width = (rect.Width() - cSmallIconBtnSize) / 2;
	::ResizeWindowTo(&mCaption1, caption_width - 8, cHdrCaptionHeight);
	::ResizeWindowTo(&mCaption2, caption_width - 8, cHdrCaptionHeight);
	::MoveWindowTo(&mCaption2, caption_width, 0);
}

void CMessageView::MakeToolbars(CToolbarView* parent)
{
	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
	{
		mCopyToPopup = C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->GetCopyBtn();

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(mText);
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(&mCaption1);
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(&mCaption2);
	}
	else
	{
		CMailboxWindow* mbox_window = dynamic_cast<CMailboxWindow*>(mOwnerWindow);
		CMailboxView* mbox_view = mbox_window->GetMailboxView();

		mCopyToPopup = static_cast<CMailboxInfoToolbar*>(mbox_view->GetToolbar())->GetCopyBtn();

		Add_Listener(mbox_view->GetToolbar());
		mbox_view->GetToolbar()->AddCommander(mText);
		mbox_view->GetToolbar()->AddCommander(&mCaption1);
		mbox_view->GetToolbar()->AddCommander(&mCaption2);
	}
}

void CMessageView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTextDisplay::eBroadcast_Activate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewActivate, this);
		break;
	case CTextDisplay::eBroadcast_Deactivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewDeactivate, this);
		break;
	case CTextDisplay::eBroadcast_SelectionChanged:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewSelectionChanged, this);
		break;
	}
}		

bool CMessageView::HasFocus() const
{
	// Check whether text is focussed
	return mText->IsTarget();
}

void CMessageView::Focus()
{
	if (IsWindowVisible())
		mText->SetFocus();
}

const unichar_t* CMessageView::GetSaveText()
{
	// Reset the formatter
	mText->Reset(true);

	// Get appropriate content typt to interpret, checking for raw view
	bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
	EContentSubType actual_content = use_part ? mCurrentPart->GetContent().GetContentSubtype() : eContentSubPlain;

	// Parse data via formatter and return
	if ((mParsing == eViewAsRaw) && mRawUTF16Text.get())
		return mText->GetFormatter()->ParseBody(mRawUTF16Text.get(), actual_content, mParsing, mQuoteDepth);
	else if (mCurrentPart && mUTF16Text.get())
		return mText->GetFormatter()->ParseBody(mUTF16Text.get(), actual_content, mParsing, mQuoteDepth);
	
	return NULL;
}

void CMessageView::SetMessage(CMessage* theMsg, bool restore)
{
	// Only if different and no error
	if ((mItsMsg == theMsg) && !mItsMsgError)
		return;
		
	// If its not cached (and also missing envelope or body), we cannot open it
	if (theMsg && !theMsg->IsFullyCached())
		theMsg = NULL;

	// Always stop any seen timer
	StopSeenTimer();

	// Disable updating
	//bool locked = GetParentFrame()->LockWindowUpdate();

	// Reset message and current part
	mItsMsg = theMsg;
	mWasUnseen = mItsMsg ? mItsMsg->IsUnseen() : false;
	mCurrentPart = NULL;

	// Reset the error flag
	mItsMsgError = false;

	// Reset any previous cached raw body
	mRawUTF16Text.reset(NULL);

	// Fill out address/subject/date fields
	ResetCaption();

	// Allow deleted messages that appear as message is read in
	mAllowDeleted = true;

	// Check for previous verify/decrypt status first (to avoid doing it again)
	// If there was a bad passphrase error, skip the cached data so user has a chance to enter
	// the correct passphrase next time
	if (mItsMsg && (mItsMsg->GetCryptoInfo() != NULL) && !mItsMsg->GetCryptoInfo()->GetBadPassphrase())
	{
		// Show the secure info pane
		SetSecretPane(*mItsMsg->GetCryptoInfo());
		ShowSecretPane(true);
	}

	// Now check for auto verify/decrypt - but not when restoring
	else if (mItsMsg && !restore &&
		(CPreferences::sPrefs->mAutoVerify.GetValue() && mItsMsg->GetBody()->IsVerifiable() ||
		CPreferences::sPrefs->mAutoDecrypt.GetValue() && mItsMsg->GetBody()->IsDecryptable()))
	{
		// NULL out current part as its used in VerifyDecrypt
		mCurrentPart = NULL;
		OnMessageVerifyDecrypt();
	}

	// Not verify/decrypt
	else
		// Turn of secure info pane
		ShowSecretPane(false);

	// Reset attachment list
	UpdatePartsList();

	// Reset text to first text part and reset table display
	if (mItsMsg)
	{
		// Reset text
		CAttachment* attach = mItsMsg->FirstDisplayPart();
		ShowPart(attach, restore);
		mPartsTable.SetRowShow(attach);

		// If raw mode read in raw body
		if (mParsing == eViewAsRaw)
		{
			// Handle cancel or failure of raw mode
			if (!ShowRawBody())
			{
				// Reset to formatted parse and update popup
				mParsing = eViewFormatted;
				mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT);
			}
			else
				// Always reset the text as either the text has changed or a new format display is being used
				ResetText();
		}
	}
	else
	{
		// Wipe out the text
		mText->WipeText(CFontCache::GetDisplayFont(), 0);
	}

	// Do attachments icon
	if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1))
	{
		if (mItsMsg->GetBody()->HasUniqueTextPart())
		{
			mAttachments.ShowWindow(SW_HIDE);
			mAlternative.ShowWindow(SW_SHOW);
		}
		else
		{
			mAttachments.ShowWindow(SW_SHOW);
			mAlternative.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		mAttachments.ShowWindow(SW_HIDE);
		mAlternative.ShowWindow(SW_HIDE);
	}

	// Update window features
	MessageChanged();

	// No longer allow deleted
	mAllowDeleted = false;

	// Enable updating
	//if (locked)
	//{
	//	GetParentFrame()->UnlockWindowUpdate();
	//	RedrawWindow(NULL, NULL, RDW_INVALIDATE);
	//}

	// Do final set message processing
	PostSetMessage(restore);
}

// Restore message text and state
void CMessageView::RestoreMessage(CMessage* theMsg, const SMessageViewState& state)
{
	// Don't allow drawing until complete
	StNoRedraw _noredraw(this);

	// Block changes to text whilst we change the state to avoid multple
	// calls to ResetText which is expensive if styled text parsing required
	{
		StValueChanger<bool> _block(mResetTextBlock, true);

		if (state.mShowHeader ^ mShowHeader)
			OnMessageShowHeader();
		if (state.mPartsExpanded ^ mPartsTwister.IsPushed())
			OnMessagePartsTwister();
		if (state.mParsing ^ mParsing)
			OnTextFormatPopup(state.mParsing - eViewFormatted + IDM_TEXTFORMAT_FORMAT);
		if (state.mFontScale != mFontScale)
		{
			mFontScale = state.mFontScale;
			ResetFontScale();
		}
	}
	
	// Now reset the message itself (force the current to NULL to force a proper reset)
	mItsMsg = NULL;
	SetMessage(theMsg, true);

	// Restore scroll and selection state - do outside of no redraw as scroll
	// does not occur otherwise
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
}

// Processing after message set and displayed
void CMessageView::PostSetMessage(bool restore)
{
	// Only do this if visible and not restoring
	if (!restore && GetParentFrame()->IsWindowVisible())
	{
		// Do parts expand if no visible part or multiparts and the parts area is visible
		if (mShowParts)
		{
			if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
			{
				// Do auto expansion or expand if no parts
				if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
					!mPartsTwister.IsPushed())
				{
					OnMessagePartsTwister();
					mDidExpandParts = true;

					// Must switch focus back to text as expanding parts sets it to the parts table
					// Only do this when in 1-pane mode
					if (!Is3Pane())
						mText->SetFocus();
				}
			}
			else
			{
				// Do auto collapse
				if (CPreferences::sPrefs->mExpandParts.GetValue() &&
					mPartsTwister.IsPushed() && mDidExpandParts)
				{
					OnMessagePartsTwister();
					mDidExpandParts = false;
				}

			}
		}
		
		// Set seen flag if no attachments
		if (!mCurrentPart && mItsMsg && mItsMsg->IsUnseen())
			mItsMsg->ChangeFlags(NMessage::eSeen, true);

		// Do attachments announcement
		if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniqueTextPart())
		{
			// Do attachment notification
			const CNotification& notify = CPreferences::sPrefs->mAttachmentNotification.GetValue();
			if (notify.DoPlaySound())
				::PlayNamedSound(notify.GetSoundID());
			if (notify.DoSpeakText())
				CSpeechSynthesis::SpeakString(notify.GetTextToSpeak());
			if (notify.DoShowAlert())
				CErrorHandler::PutNoteAlertRsrc("Alerts::Message::MessageHasAttachments", !notify.DoPlaySound());
		}
		
		// Look for top-level multipart/digest
		if (CPreferences::sPrefs->mAutoDigest.GetValue() && mItsMsg)
		{
			CAttachment* digest = mItsMsg->GetBody()->GetFirstDigest();
			if (digest)
				ShowSubMessage(digest);
		}
		
		// If seen state has changed do actions associated with that
		if (mItsMsg && mWasUnseen && !mItsMsg->IsUnseen())
			CActionManager::MessageSeenChange(mItsMsg);
	}

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CMessageView::ClearMessage()
{
	// Remove the reference to it here
	mItsMsg = NULL;
	mItsMsgError = true;
	
	// Remove the reference to it in the attachment table
	mPartsTable.ClearBody();
}

// Someone else changed this message
void CMessageView::MessageChanged()
{
	// Prevent redisplay if blocked
	if (mRedisplayBlock)
		return;

	// Clear it out if message deleted
	if (!mAllowDeleted && mItsMsg && mItsMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
		SetMessage(NULL);

	// Enable if message exists and not fake
	bool enable = mItsMsg && !mItsMsg->IsFake();

	// Disable the zoom/close buttons
	mZoom.EnableWindow(enable);

	// Disable parts area controls if no suitable message
	mPartsTwister.EnableWindow(enable);
	mHeaderBtn.EnableWindow(enable);
	mTextFormat.EnableWindow(enable);
	mFontIncreaseBtn.EnableWindow(enable);
	mFontDecreaseBtn.EnableWindow(enable);
	mQuoteDepthPopup.EnableWindow(enable);

	// Disable parts area controls if no suitable message
	if (!enable)
	{
		// If currently active, force focus to owner view
		if (mText->IsTarget() || mPartsTable.IsTarget() || mCaption1.IsTarget() || mCaption2.IsTarget())
			mMailboxView->Focus();
	}

}

// Reset message text
void CMessageView::ResetText()
{
	// Only do if message exists and not blocked
	if (!mItsMsg || mResetTextBlock) return;

	// Start cursor for busy operation
	CWaitCursor wait;

	// Determine actual view type and content
	bool use_part = (mCurrentPart != NULL) && (mParsing != eViewAsRaw);
	EView actual_view = mParsing;
	bool actual_styles = CPreferences::sPrefs->mUseStyles.GetValue();
	EContentSubType actual_content = use_part ? mCurrentPart->GetContent().GetContentSubtype() : eContentSubPlain;
	i18n::ECharsetCode charset = use_part ? mCurrentPart->GetContent().GetCharset() : i18n::eUSASCII;
	CFont* actual_font = NULL;
	switch(mParsing)
	{
	case eViewFormatted:
	{
		bool html = ((actual_content == eContentSubEnriched) || (actual_content == eContentSubHTML)) && actual_styles;

		actual_font = html ? CFontCache::GetHTMLFont() : CFontCache::GetDisplayFont();
		break;
	}
	case eViewPlain:
	case eViewRaw:
	case eViewAsRaw:
		actual_font = CFontCache::GetDisplayFont();
		break;
	case eViewAsHTML:
		actual_view = eViewFormatted;
		actual_styles = true;				// Force style rendering on
		actual_content = eContentSubHTML;
		actual_font = CFontCache::GetHTMLFont();
		break;
	case eViewAsEnriched:
		actual_view = eViewFormatted;
		actual_styles = true;				// Force style rendering on
		actual_content = eContentSubEnriched;
		actual_font = CFontCache::GetHTMLFont();
		break;
	case eViewAsFixedFont:
		actual_view = eViewFormatted;
		actual_font = CFontCache::GetFixedFont();
		break;
	}

	{
		StStopRedraw redraw(mText);

		// Reset font
		mText->WipeText(actual_font, mFontScale);

		// Reset the parser's data
		mText->Reset(true, mFontScale);
		
		// Copy in header if required
		if (mShowHeader)
			mText->GetFormatter()->ParseHeader(mItsMsg->GetHeader(), actual_view);
		else if (GetViewOptions().GetShowSummary())
		{
			// Get summary from envelope
			ostrstream hdr;
			mItsMsg->GetEnvelope()->GetSummary(hdr);
			hdr << ends;

			mText->GetFormatter()->ParseHeader(hdr.str(), actual_view);
			hdr.freeze(false);
		}

		// Only do if message exists
		if (!mItsMsg)
			return;

		// Copy in text
		if ((mParsing == eViewAsRaw) && mRawUTF16Text.get())
			mText->GetFormatter()->ParseBody(mRawUTF16Text.get(), actual_content, actual_view, mQuoteDepth);
		else if (mCurrentPart && mUTF16Text.get())
			mText->GetFormatter()->ParseBody(mUTF16Text.get(), actual_content, actual_view, mQuoteDepth, actual_styles);
		else if (!mCurrentPart)
		{
			// Add special text if there are no displayable parts
			cdstring no_displayable;
			no_displayable.FromResource("UI::Message::NoDisplayablePart");
			cdustring no_displayable_utf16(no_displayable);
			mText->GetFormatter()->ParseBody(no_displayable_utf16, eContentSubPlain, actual_view, mQuoteDepth);
		}

		// Give text to control
		mText->GetFormatter()->InsertFormatted(actual_view);

		// Set selection at start
		mText->SetSelectionRange(0, 0);
	}
	mText->GetRichEditCtrl().Invalidate();

	mText->UpdateMargins();
	{
		CRect wrect;
		mText->GetRichEditCtrl().GetWindowRect(wrect);
		mText->GetRichEditCtrl().PostMessage(WM_SIZE, SIZE_RESTORED, (wrect.Height() << 16) + wrect.Width());
	}

	// Prevent saves
	mText->GetRichEditCtrl().SetModify(false);

	// Make it active in 1-pane mode
	if (!Is3Pane())
		mText->SetFocus();
}

// Reset font scale text
void CMessageView::ResetFontScale()
{
	// Set text
	cdstring txt;
	txt.reserve(32);
	if (mFontScale != 0)
		::snprintf(txt.c_str_mod(), 32, "%+d", mFontScale);
	mFontScaleField.SetWindowText(txt.win_str());

	// Enable/disable controls
	mFontIncreaseBtn.EnableWindow(mFontScale < 7);
	mFontDecreaseBtn.EnableWindow(mFontScale > -5);
}

// Reset message text
void CMessageView::ResetCaption()
{
	// Temporarily stop screen drawing
	StNoRedraw _noredraw1(&mCaption1);
	StNoRedraw _noredraw2(&mCaption2);

	// Delete any previous text
	mCaption1.SetText(cdstring::null_str);
	mCaption1.ResetFont(CMulberryApp::sAppFont);
	mCaption2.SetText(cdstring::null_str);
	mCaption2.ResetFont(CMulberryApp::sAppFont);

	CEnvelope* env = (mItsMsg && mItsMsg->GetEnvelope()) ? mItsMsg->GetEnvelope() : NULL;

	CHARFORMAT format_bold;
	format_bold.dwMask = CFM_BOLD;
	format_bold.dwEffects = CFE_BOLD;
	CHARFORMAT format_plain;
	format_plain.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_ITALIC;
	format_plain.dwEffects = 0;

	// From
	if (env && env->GetFrom()->size())
	{
		mCaption1.SetSelectionCharFormat(format_bold);
		mCaption1.InsertUTF8(cHDR_FROM);
		mCaption1.SetSelectionCharFormat(format_plain);
		cdstring addr = env->GetFrom()->front()->GetFullAddress();
		mCaption1.InsertUTF8(addr);
	}

	// To
	if (env && env->GetTo()->size())
	{
		mCaption2.SetSelectionCharFormat(format_bold);
		mCaption2.InsertUTF8(cHDR_TO);
		mCaption2.SetSelectionCharFormat(format_plain);
		cdstring addr = env->GetTo()->front()->GetFullAddress();
		if (env->GetTo()->size() > 1)
			addr += ", ...";
		mCaption2.InsertUTF8(addr);
	}

	// CC
	if (env && env->GetCC()->size())
	{
		cdstring space = "    ";
		mCaption2.InsertUTF8(space);

		mCaption2.SetSelectionCharFormat(format_bold);
		mCaption2.InsertUTF8(cHDR_CC);
		mCaption2.SetSelectionCharFormat(format_plain);
		cdstring addr = env->GetCC()->front()->GetFullAddress();
		if (env->GetCC()->size() > 1)
			addr += ", ...";
		mCaption2.InsertUTF8(addr);
	}

	// Next line
	mCaption1.InsertUTF8(os_endl, os_endl_len);
	mCaption2.InsertUTF8(os_endl, os_endl_len);

	// Subject
	if (env)
	{
		mCaption1.SetSelectionCharFormat(format_bold);
		mCaption1.InsertUTF8(cHDR_SUBJECT);
		mCaption1.SetSelectionCharFormat(format_plain);
		cdstring subj = env->GetSubject();
		mCaption1.InsertUTF8(subj);
	}

	// Date
	if (env)
	{
		mCaption2.SetSelectionCharFormat(format_bold);
		mCaption2.InsertUTF8(cHDR_DATE);
		mCaption2.SetSelectionCharFormat(format_plain);
		cdstring date = env->GetTextDate(true, false);
		mCaption2.InsertUTF8(date);
	}

	CDC* sDC = mCaption1.GetDC();
	int screen_dpi = sDC->GetDeviceCaps(LOGPIXELSY);
	int pixel_to_twips = 1440 / screen_dpi;
	ReleaseDC(sDC);

	// Set line spacing for tall lines
	const int cHdrCaptionHeight = CMulberryApp::sLargeFont ? 48 : 40;

	mCaption1.SetSel(0, -1);
	PARAFORMAT2 pf;
	pf.dwMask = PFM_LINESPACING;
	pf.bLineSpacingRule = 3;
	pf.dyLineSpacing = pixel_to_twips * ((cHdrCaptionHeight - 6) / 2);
	mCaption1.SetParaFormat(pf);

	mCaption2.SetSel(0, -1);
	pf.dwMask = PFM_LINESPACING;
	pf.bLineSpacingRule = 3;
	pf.dyLineSpacing = pixel_to_twips * ((cHdrCaptionHeight - 6) / 2);
	mCaption2.SetParaFormat(pf);

	mCaption1.SetSel(0, 0);
	mCaption2.SetSel(0, 0);
}

// Mail view options changed
void CMessageView::ResetOptions()
{
	// Reset any message text to ensure summary info is shown/hidden
	ResetText();
	
	// Show hide the address caption
	if (mShowAddressCaption ^ GetViewOptions().GetShowAddressPane())
	{
		mShowAddressCaption = GetViewOptions().GetShowAddressPane();
		ShowCaption(mShowAddressCaption);
	}
	
	// Show/hide the parts
	if (mShowParts ^ GetViewOptions().GetShowParts())
	{
		mShowParts = GetViewOptions().GetShowParts();
		ShowParts(mShowParts);
	}

	// Check zoom & close visible state - only when in 3-pane
	if ((mOwnerWindow == C3PaneWindow::s3PaneWindow) && (mShowAddressCaption || mShowParts))
		mZoom.ShowWindow(SW_SHOW);
	else
		mZoom.ShowWindow(SW_HIDE);
}

// Update attachments caption
void CMessageView::UpdatePartsCaption()
{
	long count = 0;
	if (mPartsTable.GetFlat())
	{
		TableIndexT rows;
		TableIndexT cols;
		mPartsTable.GetTableSize(rows, cols);
		count = rows;
	}
	else
		count = mItsMsg ? mItsMsg->GetBody()->CountParts() : 0;

	cdstring caption;
	if ((count >= 1) || !mItsMsg || !mItsMsg->GetBody()->CanDisplay())
		caption = count;
	else
		caption.FromResource("Alerts::Letter::NoAttachments");

	CUnicodeUtils::SetWindowTextUTF8(&mPartsField, caption);
}

// Update list of attachments, show/hide
void CMessageView::UpdatePartsList()
{
	mPartsTable.SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
	UpdatePartsCaption();
}

// Reset message text
void CMessageView::ShowSubMessage(CAttachment* attach)
{
	// Check that its a message part
	if (!attach->IsMessage())
		return;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Read the message
		// attach->GetMessage()->ReadPart(); <- Not required - done in new message window

		// Create the message window
		newWindow = CMessageWindow::ManualCreate();
		newWindow->SetMessage(attach->GetMessage());
		newWindow->GetParentFrame()->ShowWindow(SW_SHOW);
		
		attach->SetSeen(true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Window failed to be created
		if (newWindow)
			FRAMEWORK_DELETE_WINDOW(newWindow)
	}
}

// Set details for decrypt/verify
void CMessageView::SetSecretPane(const CMessageCryptoInfo& info)
{
	// Temporarily stop screen drawing
	StNoRedraw _noredraw(&mSecureInfo);

	// Delete any previous text
	mSecureInfo.SetText(cdstring::null_str);
	mSecureInfo.ResetFont(CMulberryApp::sAppFont);

	bool multi_line = false;

	if (info.GetSuccess())
	{
		CHARFORMAT format_bold;
		format_bold.dwMask = CFM_BOLD | CFM_COLOR;
		format_bold.dwEffects = CFE_BOLD;
		format_bold.crTextColor = RGB(0x00,0x00,0x00);

		CHARFORMAT format_redbold;
		format_redbold.dwMask = CFM_BOLD | CFM_COLOR;
		format_redbold.dwEffects = CFE_BOLD;
		format_redbold.crTextColor = RGB(0x80,0x00,0x00);

		CHARFORMAT format_greenbold;
		format_greenbold.dwMask = CFM_BOLD | CFM_COLOR;
		format_greenbold.dwEffects = CFE_BOLD;
		format_greenbold.crTextColor = RGB(0x00,0x80,0x00);

		CHARFORMAT format_plain;
		format_plain.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_ITALIC | CFM_COLOR;
		format_plain.dwEffects = 0;
		format_plain.crTextColor = RGB(0x00,0x00,0x00);

		CHARFORMAT format_redplain;
		format_redplain.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_ITALIC | CFM_COLOR;
		format_redplain.dwEffects = 0;
		format_redplain.crTextColor = RGB(0x80,0x00,0x00);

		if (info.GetDidSignature())
		{
			cdstring txt;
			if (info.GetSignatureOK())
			{
				txt += "Signature: OK";
				mSecureInfo.SetSelectionCharFormat(format_greenbold);
			}
			else
			{
				txt += "Signature: Bad";
				mSecureInfo.SetSelectionCharFormat(format_redbold);
			}
			mSecureInfo.InsertUTF8(txt);

			mSecureInfo.SetSelectionCharFormat(format_bold);
			txt = "    Signed By: ";
			mSecureInfo.InsertUTF8(txt);

			mSecureInfo.SetSelectionCharFormat(format_plain);

			cdstring addr;
			bool matched_from = false;
			for(cdstrvect::const_iterator iter = info.GetSignedBy().begin(); iter != info.GetSignedBy().end(); iter++)
			{
				// Add text
				if (iter != info.GetSignedBy().begin())
					addr += ", ";
				addr += *iter;
				
				// Determine whether item matches from address
				if (mItsMsg && mItsMsg->GetEnvelope() &&
					(mItsMsg->GetEnvelope()->GetFrom()->size() > 0) &&
					mItsMsg->GetEnvelope()->GetFrom()->front()->StrictCompareEmail(CAddress(*iter)))
				{
					// Only show the address of the one that matches - ignore others
					matched_from = true;
					addr = *iter;
					break;
				}
			}

			// Change colour if n matching from address
			if (!matched_from)
			{
				mSecureInfo.SetSelectionCharFormat(format_redplain);
				
				addr += " WARNING: Does not match From address";
			}

			// Insert address data
			mSecureInfo.InsertUTF8(addr);

			// Next line
			if (info.GetDidDecrypt())
			{
				mSecureInfo.InsertUTF8(os_endl);
				multi_line = true;
			}
		}

		if (info.GetDidDecrypt())
		{
			mSecureInfo.SetSelectionCharFormat(format_bold);
			
			cdstring txt;
			txt += "Decrypted: OK";
			mSecureInfo.SetSelectionCharFormat(format_greenbold);
			mSecureInfo.InsertUTF8(txt);

			mSecureInfo.SetSelectionCharFormat(format_bold);
			txt = "    Encrypted To: ";
			mSecureInfo.InsertUTF8(txt);

			mSecureInfo.SetSelectionCharFormat(format_plain);

			cdstring addr;
			for(cdstrvect::const_iterator iter = info.GetEncryptedTo().begin(); iter != info.GetEncryptedTo().end(); iter++)
			{
				if (iter != info.GetEncryptedTo().begin())
					addr += ", ";
				addr += *iter;
			}
			mSecureInfo.InsertUTF8(addr);
		}
	}
	else
	{
		CHARFORMAT format_redbold;
		format_redbold.dwMask = CFM_BOLD | CFM_COLOR;
		format_redbold.dwEffects = CFE_BOLD;
		format_redbold.crTextColor = RGB(0x80,0x00,0x00);

		mSecureInfo.SetSelectionCharFormat(format_redbold);

		cdstring txt;
		txt += "Failed to Verify/Decrypt";
		if (!info.GetError().empty())
		{
			txt+= ":   ";
			txt += info.GetError();
		}
		mSecureInfo.InsertUTF8(txt);
	}

	// Check multi-line state
	if (mSecureMulti ^ multi_line)
	{
		const int moveby = 14;

		if (multi_line)
		{
			if (mShowSecure)
			{
				::ResizeWindowBy(&mTextFocus, 0, -moveby, false);
				::MoveWindowBy(&mTextFocus, 0, moveby, true);
			}
			::ResizeWindowBy(&mSecurePane, 0, moveby, true);
		}
		else
		{
			::ResizeWindowBy(&mSecurePane, 0, -moveby, true);
			if (mShowSecure)
			{
				::ResizeWindowBy(&mTextFocus, 0, moveby, false);
				::MoveWindowBy(&mTextFocus, 0, -moveby, true);
			}
		}

		mSecureMulti = multi_line;
	}
}

// Reset fonts
void CMessageView::ResetFont(CFont* list_font, CFont* display_font)
{
	mPartsTable.ResetFont(list_font);
	UInt16 old_height = mPartsTitles.GetRowHeight(1);
	mPartsTitles.ResetFont(list_font);
	SInt16 delta = mPartsTitles.GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(&mPartsTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(&mPartsTable, 0, -delta, false);
		::MoveWindowBy(&mPartsTable, 0, delta, false);
		
		mPartsTable.RedrawWindow();
		mPartsTitles.RedrawWindow();
	}

	mText->ResetFont(display_font);
}

// Temporarily add header summary for printing
void CMessageView::AddPrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader || !GetViewOptions().GetShowSummary()))
	{
		// Get summary from envelope
		ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());
		::FilterOutLFs(header_insert.c_str_mod());

		// Parse as header
		mText->GetFormatter()->InsertFormattedHeader(header_insert.c_str());
	}
}

// Remove temp header summary after printing
void CMessageView::RemovePrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader || !GetViewOptions().GetShowSummary()))
	{
		// Get summary from envelope
		ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());
		::FilterOutLFs(header_insert.c_str_mod());

		// Parse as header
		mText->SetSelectionRange(0, header_insert.length());
		mText->InsertUTF8(cdstring::null_str);
	}
}

#pragma mark ____________________________Timer

// Called during idle
void CMessageView::OnTimer(UINT nIDEvent)
{
	// See if we are equal or greater than trigger
	if (nIDEvent == mTimerID)
	{
		// Change the seen flag and stop the timer
		if (mItsMsg && mItsMsg->IsUnseen())
		{
			mItsMsg->ChangeFlags(NMessage::eSeen, true);
			
			// If seen state has changed do actions associated with that
			if (mItsMsg && mWasUnseen)
				CActionManager::MessageSeenChange(mItsMsg);
		}

		StopSeenTimer();
	}
	else
		CBaseView::OnTimer(nIDEvent);
}

// Start timer to trigger seen flag
void CMessageView::StartSeenTimer(unsigned long secs)
{
	// Start timer
	mTimerID = SetTimer(12345, 1000 * secs, NULL);
}

// Stop timer to trigger seen flag
void CMessageView::StopSeenTimer()
{
	// Just stop idle time processing
	if (mTimerID != 0)
		KillTimer(mTimerID);
	mTimerID = 0;
}

#pragma mark ____________________________________Command Updaters

void CMessageView::OnUpdateEditSpeak(CCmdUI* pCmdUI)
{
	// Pass to speech synthesis
	pCmdUI->Enable(true);	// Always
	if (!CSpeechSynthesis::OnUpdateEditSpeak(IDM_EDIT_SPEAK, pCmdUI))
	{
		CString txt;
		txt.LoadString(IDS_SPEAK_SPEAKMESSAGE);
		OnUpdateMenuTitle(pCmdUI, txt);
		pCmdUI->Enable(true);	// Always
	}
}

// Common updaters
void CMessageView::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CMessageView::OnUpdateMessageReadPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true) ||
					 mItsMsg->IsSubMessage() && mItsMsg->GetPrevDigest()));
}

void CMessageView::OnUpdateMessageReadNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() ||
						mItsMsg->IsSubMessage() && mItsMsg->GetNextDigest()));
}

void CMessageView::OnUpdateMessageCopyNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg != NULL);
}

void CMessageView::OnUpdateMessageReject(CCmdUI* pCmdUI)
{
	// Not if locked by admin
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowRejectCommand);
}

void CMessageView::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(mItsMsg && mItsMsg->GetEnvelope()->GetFrom()->size() &&
					CPreferences::TestSmartAddress(*mItsMsg->GetEnvelope()->GetFrom()->front()));
}

void CMessageView::OnUpdateMessageCopy(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	if ((pCmdUI->m_pSubMenu == NULL) &&
		(pCmdUI->m_nID >= IDM_CopyToMailboxChoose) &&
		(pCmdUI->m_nID <= IDM_CopyToMailboxEnd))
		CCopyToMenu::ResetMenuList();

	// Do update for non-deleted selection
	bool enable = mItsMsg;
	if (pCmdUI->m_pSubMenu)
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | (enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
 	else
		pCmdUI->Enable(enable);

	// Adjust menu title move/copy
	CString txt;
	switch(pCmdUI->m_nID)
	{
	case IDM_MESSAGES_COPY_NOW:
		txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVENOW_CMD_TEXT : IDS_COPYNOW_CMD_TEXT);
		break;
	case IDC_TOOLBARMESSAGECOPYBTN:
		txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETO_CMD_TEXT : IDS_COPYTO_CMD_TEXT);
		break;
	default:
		if ((pCmdUI->m_pSubMenu != NULL) && (pCmdUI->m_pSubMenu->m_hMenu == CCopyToMenu::GetMenu(true)->m_hMenu))
			txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
		break;
	}

	if (!txt.IsEmpty())
	{
		if (pCmdUI->m_pSubMenu)
			pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) pCmdUI->m_pMenu->GetSubMenu(pCmdUI->m_nIndex)->m_hMenu, txt);
		else
			OnUpdateMenuTitle(pCmdUI, txt);
	}

}

void CMessageView::OnUpdateMessageDelete(CCmdUI* pCmdUI)
{
	// Only if not sub-message and not read only
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eDeleted));
	
	// Change title of Delete menu item but not Delete & Next
	if ((pCmdUI->m_nID == IDM_MESSAGES_DELETE) || (pCmdUI->m_nID == IDC_TOOLBARMESSAGEDELETEBTN))
	{
		CString txt;
		txt.LoadString(mItsMsg && mItsMsg->IsDeleted() ? IDS_UNDELETE_CMD_TEXT : IDS_DELETE_CMD_TEXT);
		
		OnUpdateMenuTitle(pCmdUI, txt);

		if (!pCmdUI->m_pMenu)
			pCmdUI->SetCheck(mItsMsg && mItsMsg->IsDeleted());
	}
}

void CMessageView::OnUpdateMessageViewCurrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mCurrentPart != NULL) && (mParsing != eViewAsRaw));
}

void CMessageView::OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity());
}

void CMessageView::OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eSeen);
}

void CMessageView::OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eFlagged);
}

void CMessageView::OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eAnswered);
}

void CMessageView::OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDeleted);
}

void CMessageView::OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDraft);
}

void CMessageView::OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->m_nID - IDM_FLAGS_LABEL1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->m_nID - IDM_FLAGS_LABEL1]->name.win_str());
}

void CMessageView::OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag)
{
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(flag));
	pCmdUI->SetCheck(mItsMsg && mItsMsg->HasFlag(flag));
}

void CMessageView::OnUpdateMessageShowHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(!mShowHeader ? IDS_SHOWHEADER : IDS_HIDEHEADER);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CMessageView::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(!mPartsTwister.IsPushed() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

#pragma mark ____________________________________Command Handlers

BOOL CMessageView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try to let this handle it
	if (CBaseView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// Try super commander if any
	else if (GetSuperCommander() && GetSuperCommander()->HandleCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// Pass message up to owning window if not handled here
	return GetParent() ? GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : false;
}

// Handle key down
bool CMessageView::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	UINT cmd = 0;
	switch(nChar)
	{
	// Key shortcuts
	case 'd':
	case 'D':
		cmd = IDM_MESSAGES_DELETE;
		break;

	case 's':
	case 'S':
		cmd = IDM_FLAGS_SEEN;
		break;

	case 'i':
	case 'I':
		cmd = IDM_FLAGS_IMPORTANT;
		break;

	case 'r':
	case 'R':
		cmd = IDM_MESSAGES_REPLY;
		break;

	case 'f':
	case 'F':
		cmd = IDM_MESSAGES_FORWARD;
		break;

	case 'b':
	case 'B':
		cmd = IDM_MESSAGES_BOUNCE;
		break;

	case 'c':
	case 'C':
		cmd = IDC_MAILBOXCOPYCMD;
		break;

	case 'm':
	case 'M':
		cmd = IDC_MAILBOXMOVECMD;
		break;

	case 'n':
	case 'N':
		cmd = IDM_MESSAGES_READ_NEXT;
		break;

	case 'p':
	case 'P':
		cmd = IDM_MESSAGES_READ_PREV;
		break;

	case 'h':
	case 'H':
		cmd = IDM_MESSAGES_SHOW_HEADER;
		break;

	case 'v':
	case 'V':
		cmd = IDM_WINDOWS_SHOW_PARTS;
		break;

	case 'z':
	case 'Z':
		cmd = IDC_3PANETOOLBAR_ZOOMPREVIEW;
		break;

	default:;
	}

	// Check for command execute
	if (cmd != 0)
	{
		// Check whether command is valid right now
		CCmdUIChecker cmdui;
		cmdui.m_nID = cmd;
		
		// Only if commander available
		cmdui.DoUpdate(this, true);
		
		// Execute if enabled
		if (cmdui.GetEnabled())
			SendMessage(WM_COMMAND, cmd);

		return true;
	}

	// Did not handle key
	// Pass up to super commander
	return CCommander::HandleChar(nChar, nRepCnt, nFlags);
}

void CMessageView::OnFileNewDraft()
{
	DoNewLetter(::GetKeyState(VK_MENU) < 0);
}

void CMessageView::OnFilePrint()
{
	// Print the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT);
}

void CMessageView::OnFilePrintPreview()
{
	// Print preview the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
}

void CMessageView::OnEditSpeak()
{
	if (!CSpeechSynthesis::OnEditSpeak(IDM_EDIT_SPEAK))
		SpeakMessage();
}

void CMessageView::OnMessageCopyRead()
{
	//bool option_key = (::GetKeyState(VK_MENU) < 0);  // Turn this off: clash between alt key and menu shortcut
	bool option_key = false;
	CopyReadNextMessage(option_key);
}

void CMessageView::OnMessageReply()
{
	ReplyToThisMessage(replyReplyTo, (::GetKeyState(VK_MENU) < 0));
}

void CMessageView::OnMessageReplySender()
{
	ReplyToThisMessage(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageView::OnMessageReplyFrom()
{
	ReplyToThisMessage(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageView::OnMessageReplyAll()
{
	ReplyToThisMessage(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

// Forward this message
void CMessageView::OnMessageForward()
{
	ForwardThisMessage(::GetKeyState(VK_MENU) < 0);
}

// Copy the message to chosen mailbox
void CMessageView::OnMessageCopyBtn()
{
	TryCopyMessage(::GetKeyState(VK_MENU) < 0);
}

// Copy the message
void CMessageView::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMessageView::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
}

void CMessageView::OnTextFormatPopup(UINT nID)
{
	mTextFormat.SetValue(nID);
	EView old_view = mParsing;
	EView new_view = static_cast<EView>(nID - IDM_TEXTFORMAT_FORMAT + eViewFormatted);
	OnViewAs(new_view);
	
	// Check for failure to change and reset popup to old value
	if (mParsing == old_view)
		mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT + mParsing - eViewFormatted);
	
	// Turn off quote depth if not formatted mode
	mQuoteDepthPopup.EnableWindow(mParsing == eViewFormatted);
}


void CMessageView::OnQuoteDepthPopup(UINT nID)
{
	mQuoteDepthPopup.SetValue(nID);

	OnQuoteDepth(static_cast<EQuoteDepth>(nID));
}

// Copy the message to another mailbox
void CMessageView::OnMessageCopy(UINT nID)
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);

	// Get mbox corresponding to menu selection
	CMbox* mbox = NULL;
	if (CCopyToMenu::GetMbox(true, nID - IDM_CopyToMailboxChoose, mbox))
	{
		if (mbox)
			CopyThisMessage(mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

// Try to copy a message
bool CMessageView::TryCopyMessage(bool option_key)
{
	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (mCopyToPopup->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			return CopyThisMessage(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}

	return false;
}

void CMessageView::OnMessageFlagsSeen()
{
	OnMessageFlags(NMessage::eSeen);
}

void CMessageView::OnMessageFlagsAnswered()
{
	OnMessageFlags(NMessage::eAnswered);
}

void CMessageView::OnMessageFlagsImportant()
{
	OnMessageFlags(NMessage::eFlagged);
}

void CMessageView::OnMessageFlagsDraft()
{
	OnMessageFlags(NMessage::eDraft);
}

void CMessageView::OnMessageFlagsLabel(UINT nID)
{
	OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (nID - IDM_FLAGS_LABEL1)));
}

void CMessageView::OnMessageShowHeader()
{
	// Toggle header display
	mShowHeader = !mShowHeader;
	mHeaderBtn.SetPushed(mShowHeader);
	
	// Reset this message
	ResetText();
}

void CMessageView::OnMessagePartsTwister()
{
	bool expanding = !mPartsTwister.IsPushed();

	CRect parts_size;
	mPartsFocus.GetWindowRect(parts_size);
	int moveby = parts_size.Height();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts before collapsing
		mPartsTableAlign->SetAlignment(true, true, true, false);

		// Collapsing splitter
		mSplitter.ShowView(false, true);
				
		// Show/hide items
		mPartsFocus.ShowWindow(SW_HIDE);
		mFlatHierarchyBtn.ShowWindow(SW_HIDE);
		mPartsField.ShowWindow(SW_SHOW);
		mPartsTwister.SetPushed(false);

		// Only do this when in 1-pane mode
		if (!Is3Pane() || mPartsTable.IsTarget())
			mText->SetFocus();
	}
	else
	{
		// Expanding splitter
		mSplitter.ShowView(true, true);
		
		// Show/hide items
		mPartsFocus.ShowWindow(SW_SHOW);
		mFlatHierarchyBtn.ShowWindow(SW_SHOW);
		mPartsField.ShowWindow(SW_HIDE);
		mPartsTwister.SetPushed(true);
		
		// Turn on resize of parts after expanding
		mPartsTableAlign->SetAlignment(true, true, true, true);

		// Only do this when in 1-pane mode
		if (!Is3Pane())
			mPartsTable.SetFocus();
	}
}

void CMessageView::OnMessagePartsFlat()
{
	// Toggle flat state
	DoPartsFlat(!mPartsTable.GetFlat());
}

// Flatten parts table
void CMessageView::DoPartsFlat(bool flat)
{
	mPartsTable.SetFlat(flat);
	mFlatHierarchyBtn.SetPushed(!mPartsTable.GetFlat());
	UpdatePartsCaption();
}

// Scroll the text if possible
bool CMessageView::SpacebarScroll(bool shift_key)
{
	// Look for maximum pos of scroll bar
	SCROLLINFO info;
	bool has_scroll = (mText->GetRichEditCtrl().GetStyle() & WS_VSCROLL) &&
						mText->GetRichEditCtrl().GetScrollInfo(SB_VERT, &info);
	if (!has_scroll || (info.nPage == 0))
		return false;
	else if (shift_key)
	{
		if (info.nPos == info.nMin)
			return false;
		else
		{
			// Do page up
			mText->GetRichEditCtrl().SendMessage(WM_VSCROLL, SB_PAGEUP, 0L);
			return true;
		}
	}
	else
	{
		if (info.nPos + info.nPage >= info.nMax)
			return false;
		else
		{
			// Do page down
			mText->GetRichEditCtrl().SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L);
			return true;
		}
	}
}

void CMessageView::OnZoomPane()
{
	C3PaneWindow::s3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMPREVIEW);
}

#pragma mark ____________________________Window state

// Get current state of the view
void CMessageView::GetViewState(SMessageViewState& state) const
{
	mText->GetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->GetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
	state.mShowHeader = mShowHeader;
	state.mPartsExpanded = mPartsTwister.IsPushed();
	state.mParsing = mParsing;
	state.mFontScale = mFontScale;
}

// Change view state to one saved earlier
void CMessageView::SetViewState(const SMessageViewState& state)
{
	bool changed = false;
	if (state.mShowHeader ^ mShowHeader)
	{
		OnMessageShowHeader();
		changed = true;
	}
	if (state.mPartsExpanded ^ mPartsTwister.IsPushed())
	{
		OnMessagePartsTwister();
		changed = true;
	}
	if (state.mParsing ^ mParsing)
	{
		OnTextFormatPopup(state.mParsing - eViewFormatted + IDM_TEXTFORMAT_FORMAT);
		changed = true;
	}
	if (state.mFontScale != mFontScale)
	{
		mFontScale = state.mFontScale;
		ResetFontScale();
		changed = true;
	}

	// Nowe reset text if changed
	if (changed)
		ResetText();

	// Do these after all other changes
	mText->SetScrollPos(state.mScrollHpos, state.mScrollVpos);
	mText->SetSelectionRange(state.mTextSelectionStart, state.mTextSelectionEnd);
}

// Show/hide address caption area
void CMessageView::ShowCaption(bool show)
{
	CRect caption_size;
	mCaption1.GetWindowRect(caption_size);
	int moveby = caption_size.Height();

	if (show)
	{
		// Shrink/move splitter
		::ResizeWindowBy(&mSplitter, 0, -moveby, false);
		::MoveWindowBy(&mSplitter, 0, moveby, true);

		// Increase header height
		::ResizeWindowBy(&mHeader, 0, moveby, true);

		// Show caption after all other changes
		mCaption1.ShowWindow(SW_SHOW);
		mCaption2.ShowWindow(SW_SHOW);
		mDivider.ShowWindow(SW_SHOW);
	}
	else
	{
		// Hide caption before other changes
		mCaption1.ShowWindow(SW_HIDE);
		mCaption2.ShowWindow(SW_HIDE);
		mDivider.ShowWindow(SW_HIDE);

		// Decrease header height
		::ResizeWindowBy(&mHeader, 0, -moveby, true);

		// Expand/move splitter
		::ResizeWindowBy(&mSplitter, 0, moveby, false);
		::MoveWindowBy(&mSplitter, 0, -moveby, true);
	}
}

// Show/hide parts area
void CMessageView::ShowParts(bool show)
{
	// Collapse parts if needed
	if (!show && mPartsTwister.IsPushed())
		OnMessagePartsTwister();

	CRect parts_size;
	mPartsMove.GetWindowRect(parts_size);
	int moveby = parts_size.Height();

	if (show)
	{
		// Shrink/move splitter
		::ResizeWindowBy(&mSplitter, 0, -moveby, false);
		::MoveWindowBy(&mSplitter, 0, moveby, true);

		// Increase header height
		::ResizeWindowBy(&mHeader, 0, moveby, true);

		// Show parts after all other changes
		mPartsMove.ShowWindow(SW_SHOW);
	}
	else
	{
		// Hide parts before all other changes
		mPartsMove.ShowWindow(SW_HIDE);

		// Decrease header height
		::ResizeWindowBy(&mHeader, 0, -moveby, true);

		// Expand/move splitter
		::ResizeWindowBy(&mSplitter, 0, moveby, false);
		::MoveWindowBy(&mSplitter, 0, -moveby, true);
	}
}

// Show/hide parts area
void CMessageView::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	CRect pane_size;
	mSecurePane.GetWindowRect(pane_size);
	int moveby = pane_size.Height();

	if (show)
	{
		// Shrink/move text pane
		::ResizeWindowBy(&mTextFocus, 0, -moveby, false);
		::MoveWindowBy(&mTextFocus, 0, moveby, true);

		// Show parts after all other changes
		mSecurePane.ShowWindow(SW_SHOW);
	}
	else
	{
		// Hide parts before other changes
		mSecurePane.ShowWindow(SW_HIDE);

		// Expand/move splitter
		::ResizeWindowBy(&mTextFocus, 0, moveby, false);
		::MoveWindowBy(&mTextFocus, 0, -moveby, true);
	}
	
	mShowSecure = show;
}

// Reset state from prefs
void CMessageView::ResetState(bool force)
{
	CMessageWindowState& state = (mOwnerWindow == C3PaneWindow::s3PaneWindow) ?
										CPreferences::sPrefs->mMessageView3Pane.Value() :
										CPreferences::sPrefs->mMessageView1Pane.Value();

	// Force twist down if required
	if (state.GetPartsTwisted() ^ mPartsTwister.IsPushed())
		OnMessagePartsTwister();
	mSplitter.SetPixelSplitPos(state.GetSplitChange());
	DoPartsFlat(state.GetFlat());
}

// Save current state in prefs
void CMessageView::SaveState()
{
	SaveDefaultState();
}

// Save current state as default
void CMessageView::SaveDefaultState()
{
	// Get bounds
	CRect bounds;

	// Add info to prefs
	CMessageWindowState state(NULL, &bounds, eWindowStateNormal, mSplitter.GetPixelSplitPos(), mPartsTwister.IsPushed(), false, mPartsTable.GetFlat(), false);

	if (mOwnerWindow == C3PaneWindow::s3PaneWindow)
	{
		if (CPreferences::sPrefs->mMessageView3Pane.Value().Merge(state))
			CPreferences::sPrefs->mMessageView3Pane.SetDirty();
	}
	else
	{
		if (CPreferences::sPrefs->mMessageView1Pane.Value().Merge(state))
			CPreferences::sPrefs->mMessageView1Pane.SetDirty();
	}
}

// Reset to default state
void CMessageView::OnResetDefaultState()
{
	ResetState();
}

// Save current state as default
void CMessageView::OnSaveDefaultState()
{
	SaveDefaultState();
}

