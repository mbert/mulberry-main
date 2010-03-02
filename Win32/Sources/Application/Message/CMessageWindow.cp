/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CMessageWindow class


#include "CMessageWindow.h"

#include "CActionManager.h"
#include "CAddressBookDoc.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CBodyTable.h"
#include "CCmdUIChecker.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CDisplayFormatter.h"
#include "CFontCache.h"
#include "CMailboxToolbarPopup.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMessageDoc.h"
#include "CMessageFrame.h"
#include "CMessageHeaderView.h"
#include "CMessageToolbar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSpeechSynthesis.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CTwister.h"

#include <WIN_LDataStream.h>

#include <afxrich.h>
#include <afxwin.h>

#include <strstream>

const int cWindowWidth = 500;
const int cToolbarHeight = 56;
const int cWindowHeight = cToolbarHeight + 128;
const int cPartTitleHeight = 16;

int cHeaderHeight = 130;
int cPartsCollapsedHeight = 24;
int cPartsMinimumHeight = 84;

int cMaxTitleLength = 64;				// Maximum number of chars in window title
int cTwistMove = 40;
int cPartsTwistMove = 84;

/////////////////////////////////////////////////////////////////////////////
// CMessageWindow

IMPLEMENT_DYNCREATE(CMessageWindow, CWnd)

BEGIN_MESSAGE_MAP(CMessageWindow, CWnd)
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

	ON_UPDATE_COMMAND_UI_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnUpdateMessageCopy)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnMessageCopyPopup)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_COPY_NOW, OnUpdateMessageCopyNext)
	ON_COMMAND(IDM_MESSAGES_COPY_NOW, OnMessageCopyBtn)

	ON_COMMAND(IDC_MAILBOXCOPYCMD, OnMessageCopyCmd)
	ON_COMMAND(IDC_MAILBOXMOVECMD, OnMessageMoveCmd)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_TEXTFORMAT_FORMAT, IDM_TEXTFORMAT_RFC822, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_TEXTFORMAT_FORMAT, IDM_TEXTFORMAT_RFC822, OnTextFormatPopup)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_QUOTEDEPTH_ALL, IDM_QUOTEDEPTH_4, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_QUOTEDEPTH_ALL, IDM_QUOTEDEPTH_4, OnQuoteDepthPopup)

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

	ON_COMMAND(IDC_MESSAGEFROMTWISTER, OnMessageFromTwister)
	ON_COMMAND(IDC_MESSAGETOTWISTER, OnMessageToTwister)
	ON_COMMAND(IDC_MESSAGECCTWISTER, OnMessageCCTwister)
	ON_COMMAND(IDC_MESSAGEPARTSTWISTER, OnMessagePartsTwister)

	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_EXPAND_HEADER, OnUpdateMenuExpandHeader)
	ON_COMMAND(IDM_WINDOWS_EXPAND_HEADER, OnWindowsExpandHeader)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOW_PARTS, OnUpdateWindowsShowParts)
	ON_COMMAND(IDM_WINDOWS_SHOW_PARTS, OnMessagePartsTwister)

	ON_COMMAND(IDC_MESSAGEPARTSFLATBTN, OnMessagePartsFlat)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

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

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARWINDOWCOLLAPSEHEADERBTN, OnUpdateExpandHeader)
	ON_COMMAND(IDC_TOOLBARWINDOWCOLLAPSEHEADERBTN, OnWindowsExpandHeader)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageWindow construction/destruction

cdmutexprotect<CMessageWindow::CMessageWindowList> CMessageWindow::sMsgWindows;
CMultiDocTemplate* CMessageWindow::sMessageDocTemplate = NULL;

cdstring CMessageWindow::sNumberString;						// sprintf string for message number
cdstring CMessageWindow::sNumberStringBig;					// sprintf string for message number >= 10000
cdstring CMessageWindow::sSubMessageString;					// sprintf string for sub-message
cdstring CMessageWindow::sDigestString;						// sprintf string for digestive message

cdstring  CMessageWindow::sLastCopyTo;

// Default constructor
CMessageWindow::CMessageWindow()
{
	// Add to list
	{
		cdmutexprotect<CMessageWindowList>::lock _lock(sMsgWindows);
		sMsgWindows->push_back(this);
	}

	mItsMsg = NULL;
	mWasUnseen = false;
	mItsMsgError = false;
	mMsgs = NULL;
	mShowText = NULL;
	mCurrentPart = NULL;
	mShowHeader = CPreferences::sPrefs->showMessageHeader.GetValue();
	mShowSecure = true;
	mSecureMulti = true;
	mParsing = eViewFormatted;
	mFontScale = 0;
	mQuoteDepth = -1;

	mHeaderState.mExpanded = true;
	mHeaderState.mFromVisible = true;
	mHeaderState.mFromExpanded = false;
	mHeaderState.mToVisible = true;
	mHeaderState.mToExpanded = false;
	mHeaderState.mCcVisible = true;
	mHeaderState.mCcExpanded = false;
	mHeaderState.mSubjectVisible = true;

	mRedisplayBlock = false;
	mAllowDeleted = false;
	mDidExpandParts = false;

	mPartsTableAlign = NULL;

	// Load strings if required
	if (!sNumberString.length())
		sNumberString.FromResource("UI::Message::MessageNumber");
	if (!sNumberStringBig.length())
		sNumberStringBig.FromResource("UI::Message::MessageNumberBig");
	if (!sSubMessageString.length())
		sSubMessageString.FromResource("UI::Message::SubMessage");
	if (!sDigestString.length())
		sDigestString.FromResource("UI::Message::DigestMessage");
}

// Default destructor
CMessageWindow::~CMessageWindow()
{
	// Set status
	SetClosing();

	// Remove from list
	{
		cdmutexprotect<CMessageWindowList>::lock _lock(sMsgWindows);
		CMessageWindowList::iterator found = std::find(sMsgWindows->begin(), sMsgWindows->end(), this);
		if (found != sMsgWindows->end())
			sMsgWindows->erase(found);
	}

	// Set status
	SetClosed();
}

// Manually create document
CMessageWindow* CMessageWindow::ManualCreate(bool hidden)
{
	CDocument* aDoc = sMessageDocTemplate->OpenDocumentFile(NULL, false);
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	CMessageWindow* pWnd = &static_cast<CMessageFrameSDI*>(aFrame)->mMessageWindow;

	// Give it to document
	((CMessageDoc*) aDoc)->SetMessageWindow(pWnd);

	// Reset to default size
	if (!hidden)
		pWnd->ResetState();

	return (CMessageWindow*) pWnd;
}

// Update positions of UI elements
void CMessageWindow::UpdateUIPos()
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;

	cHeaderHeight += large_offset;
	cPartsCollapsedHeight += small_offset;

	cTwistMove += small_offset;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMessageWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Do inherited
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	const int cSmallIconBtnSize = 20;

	const int cHdrCaptionHeight = CMulberryApp::sLargeFont ? 40 : 32;
	int cHdrCaptionWidth = cWindowWidth - cSmallIconBtnSize - 8;

	// Now create sub-views

	// Create toolbar pane
	CRect rect = CRect(0, 0, cWindowWidth, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), ID_VIEW_TOOLBAR);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);
	mToolbarView.ShowDivider(true);

	// Create the main view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(&mView);
	mView.MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Header view
	rect = CRect(0, 0, cWindowWidth, 144);
	mHeader = new CMessageHeaderView;
	mHeader->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mView, IDC_STATIC);
	mView.AddAlignment(new CWndAlignment(mHeader, CWndAlignment::eAlign_TopWidth));
	CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
	if (pContext != NULL && pContext->m_pCurrentDoc != NULL)
		pContext->m_pCurrentDoc->AddView(mHeader);

	// Create splitter view
	rect = CRect(0, 144, cWindowWidth, cWindowHeight);
	mSplitterView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mView, IDC_STATIC);
	mView.AddAlignment(new CWndAlignment(&mSplitterView, CWndAlignment::eAlign_WidthHeight));

	// Parts pane
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mPartsPane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Parts table
	rect = CRect(0, cPartTitleHeight, cWindowWidth, cWindowHeight);
	mPartsTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, &mPartsPane, IDC_MESSAGEPARTSTABLE);
	mPartsTable.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	mPartsTable.ResetFont(CFontCache::GetListFont());
	mPartsTable.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_PARTS);
	mPartsTable.SetContextView(mHeader);
	mPartsTable.SetSuperCommander(this);
	mPartsTable.SetTabOrder(mText, &mHeader->mDateField, mText, mHeader);
	mHeader->mDateField.SetTabOrder(&mPartsTable, &mHeader->mSubjectField, mHeader, mHeader);
	mPartsPane.AddAlignment(mPartsTableAlign = new CWndAlignment(&mPartsTable, CWndAlignment::eAlign_WidthHeight));

	// Parts titles
	rect = CRect(0, 0, cWindowWidth, cPartTitleHeight);
	mPartsTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mPartsPane, IDC_MESSAGEPARTSTABLETITLES);
	mPartsTitles.SyncTable(&mPartsTable);
	mPartsTable.SetTitles(&mPartsTitles);
	mPartsPane.AddAlignment(new CWndAlignment(&mPartsTitles, CWndAlignment::eAlign_TopWidth));

	mPartsTitles.LoadTitles("UI::Titles::MessageParts", 5);

	// Create bottom splitter pane
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mSplitter2Pane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Create secure info pane
	rect = CRect(0, 0, cWindowWidth, cHdrCaptionHeight + 4);
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

	// Text area
	mTextArea.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, cHdrCaptionHeight + 4, cWindowWidth, cWindowHeight), &mSplitter2Pane, IDC_STATIC);
	mTextArea.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	mSplitter2Pane.AddAlignment(new CWndAlignment(&mTextArea, CWndAlignment::eAlign_WidthHeight));

	// Text view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight - cHdrCaptionHeight - 4);
	mText = new CFormattedTextDisplay;
	mText->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mTextArea, IDC_STATIC);
	if (pContext != NULL && pContext->m_pCurrentDoc != NULL)
		pContext->m_pCurrentDoc->AddView(mText);

	mText->GetRichEditCtrl().SetReadOnly(true);
	mText->GetRichEditCtrl().SetModify(false);
	mText->SetFont(CFontCache::GetDisplayFont());
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetWrap(CPreferences::sPrefs->wrap_length.GetValue());
	mText->SetHardWrap(false);
	mText->SetMessageWindow(this);
	mText->SetFindAllowed(true);
	mText->SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_TEXT);
	mText->SetTabSelectAll(false);
	mText->SetTabOrder(&mHeader->mFromField, &mPartsTable, mHeader, mHeader);
	mHeader->mFromField.SetTabOrder(&mHeader->mToField, mText, mHeader, mText);
	mPartsTable.SetTabOrder(mText, &mHeader->mDateField, mText, mHeader);
	mText->SetSuperCommander(this);
	mTextArea.AddAlignment(new CWndAlignment(mText, CWndAlignment::eAlign_WidthHeight));

	// Hide the verify/decrypt details
	ShowSecretPane(false);

	// Install the splitter items
	mSplitterView.InstallViews(&mPartsPane, &mSplitter2Pane, true);
	mSplitterView.SetMinima(84, 64);
	mSplitterView.SetPixelSplitPos(84);
	mSplitterView.ShowView(false, true);
	mSplitterView.SetLockResize(true);

	// Toggle header button
	mHeader->mHeaderBtn.SetPushed(mShowHeader);
	mParsing = eViewFormatted;
	mHeader->mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT);

	// Set status
	SetOpen();

	return 0;
}

// Tidy up when window closed
void CMessageWindow::OnDestroy()
{
	// Enable updating which may have been turned off during network op
	GetParentFrame()->UnlockWindowUpdate();

	CWnd::OnDestroy();
}

// Adjust splits
void CMessageWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	CRect rect(0, 0, cx, cy);
	mView.MoveWindow(rect);
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CWnd::OnSize(nType, cx, cy);
}

void CMessageWindow::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);
}

void CMessageWindow::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());
}

const unichar_t* CMessageWindow::GetSaveText()
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

// Add address list to caption/field:
void CMessageWindow::SetTwistList(CCmdEdit* aField,
									CTwister* aTwister,
									CWnd* aMover,
									CAddressList* list)
{

	cdstring to_addrs;

	// Add all in list
	bool first = true;
	bool got_two = false;
	for(CAddressList::iterator iter = list->begin(); iter != list->end(); iter++)
	{
		cdstring txt = (*iter)->GetFullAddress();
		if (!first)
			to_addrs += os_endl;
		to_addrs += txt;
		if (first)
			first = false;
		else
			got_two = true;
	}

	// Update field - will null if no addresses
	aField->SetText(to_addrs);

	// Disable twister if more than one line
	if (aField->GetLineCount() < 2)
		aTwister->EnableWindow(false);
	else
		aTwister->EnableWindow(true);

	// Force twist down if more than one address and preference set
	if (CPreferences::sPrefs->mExpandHeader.GetValue() &&
		((aTwister->IsPushed() && !got_two) ||
		 (!aTwister->IsPushed() && got_two)))
		DoTwist(aMover, aField, aTwister);
}

void CMessageWindow::SetMessage(CMessage* theMsg)
{
	// Disable updating
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Reset message and current part
	mItsMsg = theMsg;
	mWasUnseen = mItsMsg ? mItsMsg->IsUnseen() : false;
	mCurrentPart = NULL;

	// Reset the error flag
	mItsMsgError = false;

	// Reset any previous cached raw body
	mRawUTF16Text.reset(NULL);

	CEnvelope*	theEnv = mItsMsg ? mItsMsg->GetEnvelope() : NULL;
	cdstring	theTitle;

	// Set From: text
	SetTwistList(&mHeader->mFromField, &mHeader->mFromTwister, &mHeader->mFromMove, theEnv->GetFrom());

	// Set To: text
	SetTwistList(&mHeader->mToField, &mHeader->mToTwister, &mHeader->mToMove, theEnv->GetTo());

	// Set CC: text
	SetTwistList(&mHeader->mCCField, &mHeader->mCCTwister, &mHeader->mCCMove, theEnv->GetCC());

	// Set Subject: text
	mHeader->mSubjectField.SetText(theEnv->GetSubject());

	// Set Date: text
	mHeader->mDateField.SetText(theEnv->GetTextDate(true, true));

	// Set window title to subject (if there), else from (if there) else unknown
	if (!theEnv->GetSubject().empty())
	{
		short subj_len = theEnv->GetSubject().length();
		if (subj_len < cMaxTitleLength)
			theTitle= theEnv->GetSubject();
		else
			theTitle = cdstring(theEnv->GetSubject(), 0, cMaxTitleLength);
	}
	else if (theEnv->GetFrom()->size())
	{
		theTitle = theEnv->GetFrom()->front()->GetNamedAddress();
		theTitle.erase(cMaxTitleLength);
	}
	else
		theTitle.FromResource("UI::Message::NoSubject");

	mText->GetDocument()->SetTitle(theTitle.win_str());

	// Allow deleted messages that appear as message is read in
	mAllowDeleted = true;

	// Check for previous verify/decrypt status first (to avoid doing it again)
	// If there was a bad passphrase error, skip the cached data so user has a chance to enter
	// the correct passphrase next time
	if ((mItsMsg->GetCryptoInfo() != NULL) && !mItsMsg->GetCryptoInfo()->GetBadPassphrase())
	{
		// Show the secure info pane
		SetSecretPane(*mItsMsg->GetCryptoInfo());
		ShowSecretPane(true);
	}

	// Now check for auto verify/decrypt
	else if (CPreferences::sPrefs->mAutoVerify.GetValue() &&
		mItsMsg->GetBody()->IsVerifiable() ||
		CPreferences::sPrefs->mAutoDecrypt.GetValue() &&
		mItsMsg->GetBody()->IsDecryptable())
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
	UpdatePartsCaption();

	// Reset text
	CAttachment* attach = mItsMsg->FirstDisplayPart();
	ShowPart(attach);
	mPartsTable.SetRowShow(attach);

	// If raw mode read in raw body
	if (mParsing == eViewAsRaw)
	{
		// Handle cancel or failure of raw mode
		if (!ShowRawBody())
		{
			// Reset to formatted parse and update popup
			mParsing = eViewFormatted;
			mHeader->mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT);
		}
		
		// Always reset the text as either the text has changed or a new format display is being used
		ResetText();
	}

	// Do attachments icon
	if (mItsMsg && (mItsMsg->GetBody()->CountParts() > 1))
	{
		if (mItsMsg->GetBody()->HasUniqueTextPart())
		{
			mHeader->mAttachments.ShowWindow(SW_HIDE);
			mHeader->mAlternative.ShowWindow(SW_SHOW);
		}
		else
		{
			mHeader->mAttachments.ShowWindow(SW_SHOW);
			mHeader->mAlternative.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		mHeader->mAttachments.ShowWindow(SW_HIDE);
		mHeader->mAlternative.ShowWindow(SW_HIDE);
	}

	// Update window features
	MessageChanged();

	// No longer allow deleted
	mAllowDeleted = false;

	// Enable updating
	if (locked)
		GetParentFrame()->UnlockWindowUpdate();
	
	// Do final set message processing
	PostSetMessage();
}

// Processing after message set and displayed
void CMessageWindow::PostSetMessage()
{
	// Only do this if visible
	if (GetParentFrame()->IsWindowVisible())
	{
		// Do parts expand if no visible part or multiparts
		if (mItsMsg && (!mCurrentPart || (mItsMsg->GetBody()->CountParts() > 1) && !mItsMsg->GetBody()->HasUniquePart()))
		{
			// Do auto expansion or expand if no parts
			if ((!mCurrentPart || CPreferences::sPrefs->mExpandParts.GetValue()) &&
				!mHeader->mPartsTwister.IsPushed())
			{
				OnMessagePartsTwister();
				mDidExpandParts = true;

				// Must switch focus back to text as expanding parts sets it to the pasrt table
				GetParentFrame()->SetActiveView(mText);
				mText->SetFocus();
			}
		}
		else
		{
			// Do auto collapse
			if (CPreferences::sPrefs->mExpandParts.GetValue() &&
				mHeader->mPartsTwister.IsPushed() && mDidExpandParts)
			{
				OnMessagePartsTwister();
				mDidExpandParts = false;
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
}

void CMessageWindow::ClearMessage()
{
	// Remove the reference to it here
	mItsMsg = NULL;
	mItsMsgError = true;
	
	// Remove the reference to it in the attachment table
	mPartsTable.ClearBody();
}

void CMessageWindow::SetMessageList(CMessageList* msgs)
{
	// Disable updating
	bool locked = GetParentFrame()->LockWindowUpdate();

	mMsgs = msgs;

	CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
	cdstring	theTitle;

	// Set window title to subject (if there), else from (if there) else unknown
	if (!theEnv->GetSubject().empty())
	{
		short subj_len = theEnv->GetSubject().length();
		if (subj_len < cMaxTitleLength)
			theTitle= theEnv->GetSubject();
		else
			theTitle = CString(theEnv->GetSubject(), cMaxTitleLength);
	}
	else if (theEnv->GetFrom()->size())
	{
		theTitle = theEnv->GetFrom()->front()->GetNamedAddress();
		if (theTitle.length() > cMaxTitleLength)
			theTitle = theTitle.erase(cMaxTitleLength);
	}
	else
		theTitle.FromResource("UI::Message::NoSubject");

	mText->GetDocument()->SetTitle(theTitle.win_str());

	// Enable updating
	if (locked)
		GetParentFrame()->UnlockWindowUpdate();
}

// Someone else changed this message
void CMessageWindow::MessageChanged()
{
	// Prevent redisplay if blocked
	if (mRedisplayBlock)
		return;

	// Close window if message deleted
	if (!mAllowDeleted && mItsMsg && mItsMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
	{
		// Close window via queued task
		CCloseMessageWindowTask* task = new CCloseMessageWindowTask(this);
		task->Go();
		return;
	}
	
	// Set message number text or sub-message information
	CString newTxt;
	CString oldTxt;
	if (mItsMsg)
	{
		if (mItsMsg->IsSubMessage())
		{
			char temp[64];
			if (mItsMsg->CountDigest())
				::snprintf(temp, 64, sDigestString, mItsMsg->GetDigestNumber(), mItsMsg->CountDigest());
			else
				::snprintf(temp, 64, sSubMessageString);
			newTxt = temp;
		}
		else
		{
			char temp[64];
			::snprintf(temp, 64, mItsMsg->GetMbox()->GetNumberFound() < 10000 ? sNumberString : sNumberStringBig,
						mItsMsg->GetMessageNumber(), mItsMsg->GetMbox()->GetNumberFound());
			newTxt = temp;
		}
	}

	mHeader->mNumberField.GetWindowText(oldTxt);
	if (newTxt != oldTxt)
		mHeader->mNumberField.SetWindowText(newTxt);
	
	// Force main toolbar to update
	mView.RefreshToolbar();
}

// Reset message text
void CMessageWindow::ResetText()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

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
		else if (!mHeaderState.mExpanded)
		{
			// Get summary from envelope
			std::ostrstream hdr;
			mItsMsg->GetEnvelope()->GetSummary(hdr);
			hdr << std::ends;

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

	// Make it active
	GetParentFrame()->SetActiveView(mText);
	mText->SetFocus();
}

// Reset font scale text
void CMessageWindow::ResetFontScale()
{
	// Set text
	cdstring txt;
	txt.reserve(32);
	if (mFontScale != 0)
		::snprintf(txt.c_str_mod(), 32, "%+d", mFontScale);
	mHeader->mFontScaleField.SetWindowText(txt.win_str());

	// Enable/disable controls
	mHeader->mFontIncreaseBtn.EnableWindow(mFontScale < 7);
	mHeader->mFontDecreaseBtn.EnableWindow(mFontScale > -5);
}

// Update attachments caption
void CMessageWindow::UpdatePartsCaption()
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

	mHeader->mPartsField.SetWindowText(caption.win_str());
}

// Update list of attachments, show/hide
void CMessageWindow::UpdatePartsList()
{
	mPartsTable.SetBody(mItsMsg ? mItsMsg->GetBody() : NULL);
}

// Reset message text
void CMessageWindow::ShowSubMessage(CAttachment* attach)
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
		
		// Stagger relative to this
		CRect frame;
		GetWindowRect(frame);
		frame.OffsetRect(20, 20);
		newWindow->GetParentFrame()->SetWindowPos(NULL, frame.left, frame.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		newWindow->GetParentFrame()->ShowWindow(SW_SHOW);
		//newWindow->GetText()->UpdateMargins();
		
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
void CMessageWindow::SetSecretPane(const CMessageCryptoInfo& info)
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
				::ResizeWindowBy(&mTextArea, 0, -moveby, false);
				::MoveWindowBy(&mTextArea, 0, moveby, true);
			}
			::ResizeWindowBy(&mSecurePane, 0, moveby, true);
		}
		else
		{
			::ResizeWindowBy(&mSecurePane, 0, -moveby, true);
			if (mShowSecure)
			{
				::ResizeWindowBy(&mTextArea, 0, moveby, false);
				::MoveWindowBy(&mTextArea, 0, -moveby, true);
			}
		}

		mSecureMulti = multi_line;
	}
}

// Reset fonts
void CMessageWindow::ResetFont(CFont* list_font, CFont* display_font)
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
void CMessageWindow::AddPrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader && mHeaderState.mExpanded))
	{
		// Get summary from envelope
		std::ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());
		::FilterOutLFs(header_insert.c_str_mod());

		// Parse as header
		mText->GetFormatter()->InsertFormattedHeader(header_insert.c_str());
	}
}

// Remove temp header summary after printing
void CMessageWindow::RemovePrintSummary()
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue() &&
		(!mShowHeader && mHeaderState.mExpanded))
	{
		// Get summary from envelope
		std::ostrstream hdr;
		mItsMsg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());
		::FilterOutLFs(header_insert.c_str_mod());

		// Parse as header
		mText->SetSelectionRange(0, header_insert.length());
		mText->InsertUTF8(cdstring::null_str, true);
	}
}

#pragma mark ____________________________________Command Updaters

void CMessageWindow::OnUpdateEditSpeak(CCmdUI* pCmdUI)
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
void CMessageWindow::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CMessageWindow::OnUpdateMessageReadPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg &&
					(!mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true) ||
					 mItsMsg->IsSubMessage() && mItsMsg->GetPrevDigest()));
}

void CMessageWindow::OnUpdateMessageReadNext(CCmdUI* pCmdUI)
{
	if (mItsMsg &&
		(!mItsMsg->IsSubMessage() ||
			mItsMsg->IsSubMessage() && mItsMsg->GetNextDigest()))
	{
		pCmdUI->Enable(true);
		
		// Check for toolbar button
		if (pCmdUI->m_pOther && dynamic_cast<CToolbarButton*>(pCmdUI->m_pOther))
		{
			UINT icon;
			if (mItsMsg->IsSubMessage())
			{
				// Reset navigation buttons
				icon = mItsMsg->GetNextDigest() ? IDI_NEXTMSG : IDI_LASTNEXTMSG;
			}
			else
			{
				// Reset navigation buttons
				icon = mItsMsg->GetMbox()->GetNextMessage(mItsMsg, true) ? IDI_NEXTMSG : IDI_LASTNEXTMSG;
			}
			dynamic_cast<CToolbarButton*>(pCmdUI->m_pOther)->SetIcons(icon);
		}
	}
	else
		pCmdUI->Enable(false);
}

void CMessageWindow::OnUpdateMessageCopyNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mItsMsg != NULL);
}

void CMessageWindow::OnUpdateMessageReject(CCmdUI* pCmdUI)
{
	// Not if locked by admin
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowRejectCommand);
}

void CMessageWindow::OnUpdateMessageSendAgain(CCmdUI* pCmdUI)
{
	// Compare address with current user
	pCmdUI->Enable(mItsMsg && mItsMsg->GetEnvelope()->GetFrom()->size() &&
					CPreferences::TestSmartAddress(*mItsMsg->GetEnvelope()->GetFrom()->front()));
}

void CMessageWindow::OnUpdateMessageCopy(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	if ((pCmdUI->m_pSubMenu == NULL) && (pCmdUI->m_nID >= IDM_CopyToMailboxChoose) && (pCmdUI->m_nID <= IDM_CopyToMailboxEnd) ||
		(pCmdUI->m_nID >= IDM_CopyToPopupNone) && (pCmdUI->m_nID <= IDM_CopyToPopupEnd))
		CCopyToMenu::ResetMenuList();

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

void CMessageWindow::OnUpdateMessageDelete(CCmdUI* pCmdUI)
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

void CMessageWindow::OnUpdateMessageViewCurrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mCurrentPart != NULL) && (mParsing != eViewAsRaw));
}

void CMessageWindow::OnUpdateMessageVerifyDecrypt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity());
}

void CMessageWindow::OnUpdateMessageFlagsSeen(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eSeen);
}

void CMessageWindow::OnUpdateMessageFlagsImportant(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eFlagged);
}

void CMessageWindow::OnUpdateMessageFlagsAnswered(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eAnswered);
}

void CMessageWindow::OnUpdateMessageFlagsDeleted(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDeleted);
}

void CMessageWindow::OnUpdateMessageFlagsDraft(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, NMessage::eDraft);
}

void CMessageWindow::OnUpdateMessageFlagsLabel(CCmdUI* pCmdUI)
{
	OnUpdateMessageFlags(pCmdUI, static_cast<NMessage::EFlags>(NMessage::eLabel1 << (pCmdUI->m_nID - IDM_FLAGS_LABEL1)));
	pCmdUI->SetText(CPreferences::sPrefs->mLabels.GetValue()[pCmdUI->m_nID - IDM_FLAGS_LABEL1]->name.win_str());
}

void CMessageWindow::OnUpdateMessageFlags(CCmdUI* pCmdUI, NMessage::EFlags flag)
{
	pCmdUI->Enable(mItsMsg && !mItsMsg->IsSubMessage() && mItsMsg->GetMbox()->HasAllowedFlag(flag));
	pCmdUI->SetCheck(mItsMsg && mItsMsg->HasFlag(flag));
}

void CMessageWindow::OnUpdateMessageShowHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(!mShowHeader ? IDS_SHOWHEADER : IDS_HIDEHEADER);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CMessageWindow::OnUpdateMenuExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mHeaderState.mExpanded ? IDS_COLLAPSEHEADER : IDS_EXPANDHEADER);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CMessageWindow::OnUpdateExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
}

void CMessageWindow::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(!mHeader->mPartsTwister.IsPushed() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

#pragma mark ____________________________________Command Handlers

// Handle key down
bool CMessageWindow::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
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
	return CCommander::HandleChar(nChar, nRepCnt, nFlags);
}

void CMessageWindow::OnFileNewDraft()
{
	DoNewLetter(::GetKeyState(VK_MENU) < 0);
}

void CMessageWindow::OnFilePrint()
{
	// Print the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT);
}

void CMessageWindow::OnFilePrintPreview()
{
	// Print preview the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
}

void CMessageWindow::OnEditSpeak()
{
	if (!CSpeechSynthesis::OnEditSpeak(IDM_EDIT_SPEAK))
		SpeakMessage();
}

void CMessageWindow::OnMessageCopyRead()
{
	//bool option_key = (::GetKeyState(VK_MENU) < 0);  // Turn this off: clash between alt key and menu shortcut
	bool option_key = false;
	CopyReadNextMessage(option_key);
}

void CMessageWindow::OnMessageReply()
{
	ReplyToThisMessage(replyReplyTo, (::GetKeyState(VK_MENU) < 0));
}

void CMessageWindow::OnMessageReplySender()
{
	ReplyToThisMessage(replySender, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageWindow::OnMessageReplyFrom()
{
	ReplyToThisMessage(replyFrom, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

void CMessageWindow::OnMessageReplyAll()
{
	ReplyToThisMessage(replyAll, !CPreferences::sPrefs->optionKeyReplyDialog.GetValue());
}

// Forward this message
void CMessageWindow::OnMessageForward()
{
	ForwardThisMessage(::GetKeyState(VK_MENU) < 0);
}

// Copy the message to chosen mailbox
void CMessageWindow::OnMessageCopyBtn()
{
	TryCopyMessage(::GetKeyState(VK_MENU) < 0);
}

// Copy the message
void CMessageWindow::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMessageWindow::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	TryCopyMessage(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
}

// Copy to popup change
void CMessageWindow::OnMessageCopyPopup(UINT nID)
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);

	// Find copied to mbox
	CMbox* copy_mbox = NULL;
	if (CCopyToMenu::GetPopupMbox(true, nID - IDM_CopyToPopupChoose, copy_mbox))
	{
		if (copy_mbox && (copy_mbox != (CMbox*) -1L))
			CopyThisMessage(copy_mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

void CMessageWindow::OnTextFormatPopup(UINT nID)
{
	mHeader->mTextFormat.SetValue(nID);
	EView old_view = mParsing;
	EView new_view = static_cast<EView>(nID - IDM_TEXTFORMAT_FORMAT + eViewFormatted);
	OnViewAs(new_view);
	
	// Check for failure to change and reset popup to old value
	if (mParsing == old_view)
		mHeader->mTextFormat.SetValue(IDM_TEXTFORMAT_FORMAT + mParsing - eViewFormatted);
	
	// Turn off quote depth if not formatted mode
	mHeader->mQuoteDepthPopup.EnableWindow(mParsing == eViewFormatted);
}

void CMessageWindow::OnQuoteDepthPopup(UINT nID)
{
	mHeader->mQuoteDepthPopup.SetValue(nID);

	OnQuoteDepth(static_cast<EQuoteDepth>(nID));
}

// Copy the message to another mailbox
void CMessageWindow::OnMessageCopy(UINT nID)
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
bool CMessageWindow::TryCopyMessage(bool option_key)
{
	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (mView.GetToolbar()->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			return CopyThisMessage(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}

	return false;
}

void CMessageWindow::OnMessageFlagsSeen()
{
	OnMessageFlags(NMessage::eSeen);
}

void CMessageWindow::OnMessageFlagsAnswered()
{
	OnMessageFlags(NMessage::eAnswered);
}

void CMessageWindow::OnMessageFlagsImportant()
{
	OnMessageFlags(NMessage::eFlagged);
}

void CMessageWindow::OnMessageFlagsDraft()
{
	OnMessageFlags(NMessage::eDraft);
}

void CMessageWindow::OnMessageFlagsLabel(UINT nID)
{
	OnMessageFlags(static_cast<NMessage::EFlags>(NMessage::eLabel1 << (nID - IDM_FLAGS_LABEL1)));
}

void CMessageWindow::OnMessageShowHeader()
{
	// Toggle header display
	mShowHeader = !mShowHeader;
	mHeader->mHeaderBtn.SetPushed(mShowHeader);
	
	// Reset this message
	ResetText();
}

void CMessageWindow::OnMessageFromTwister()
{
	mHeaderState.mFromExpanded = mHeader->mFromTwister.IsPushed();
	DoTwist(&mHeader->mFromMove, &mHeader->mFromField, &mHeader->mFromTwister);
}

void CMessageWindow::OnMessageToTwister()
{
	mHeaderState.mToExpanded = mHeader->mToTwister.IsPushed();
	DoTwist(&mHeader->mToMove, &mHeader->mToField, &mHeader->mToTwister);
}

void CMessageWindow::OnMessageCCTwister()
{
	mHeaderState.mCcExpanded = mHeader->mCCTwister.IsPushed();
	DoTwist(&mHeader->mCCMove, &mHeader->mCCField, &mHeader->mCCTwister);
}

// Handle twist
void CMessageWindow::DoTwist(CWnd* aMover, CCmdEdit* aField, CTwister* aTwister)
{
	if (aTwister->IsPushed())
	{
		int moveby = -cTwistMove;
		aField->ModifyStyle(WS_VSCROLL, ES_AUTOHSCROLL);
		aField->SetSel(0, 0);
		aField->SetFocus();
		aTwister->SetPushed(false);
		::ResizeWindowBy(aMover, 0, moveby, false);
	}
	else
	{
		int moveby = cTwistMove;
		aField->ModifyStyle(0, WS_VSCROLL);
		aField->SetSel(0, 0);
		aField->SetFocus();
		aTwister->SetPushed(true);
		::ResizeWindowBy(aMover, 0, moveby, false);
	}

	// Do header layout after change
	LayoutHeader();
}

// Expand to full size header
void CMessageWindow::OnWindowsExpandHeader()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;

	LayoutHeader();
	
	// Reset text as message header summary display will also change
	ResetText();
}

// Layout header component based on state
void CMessageWindow::LayoutHeader()
{
	// Start with some initial offset
	int top = 0;
	
	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Check for From field
		if (mHeaderState.mFromVisible)
		{
			// Get current size
			CRect size;
			mHeader->mFromMove.GetWindowRect(&size);
			mHeader->mFromMove.GetParent()->ScreenToClient(&size);

			// Move to current top and increment top by height
			::MoveWindowBy(&mHeader->mFromMove, 0, top - size.top, false);
			top += size.Height();

			// Always show it
			mHeader->mFromMove.ShowWindow(SW_SHOW);
		}
		else
			// Always hide it
			mHeader->mFromMove.ShowWindow(SW_HIDE);

		// Check for To field
		if (mHeaderState.mToVisible)
		{
			// Get current size
			CRect size;
			mHeader->mToMove.GetWindowRect(&size);
			mHeader->mToMove.GetParent()->ScreenToClient(&size);

			// Move to current top and increment top by height
			::MoveWindowBy(&mHeader->mToMove, 0, top - size.top, false);
			top += size.Height();

			// Always show it
			mHeader->mToMove.ShowWindow(SW_SHOW);
		}
		else
			// Always hide it
			mHeader->mToMove.ShowWindow(SW_HIDE);

		// Check for Cc field
		if (mHeaderState.mCcVisible)
		{
			// Get current size
			CRect size;
			mHeader->mCCMove.GetWindowRect(&size);
			mHeader->mCCMove.GetParent()->ScreenToClient(&size);

			// Move to current top and increment top by height
			::MoveWindowBy(&mHeader->mCCMove, 0, top - size.top, false);
			top += size.Height();

			// Always show it
			mHeader->mCCMove.ShowWindow(SW_SHOW);
		}
		else
			// Always hide it
			mHeader->mCCMove.ShowWindow(SW_HIDE);

		// Check for Subject field
		if (mHeaderState.mSubjectVisible)
		{
			// Get current size
			CRect size;
			mHeader->mSubjectMove.GetWindowRect(&size);
			mHeader->mSubjectMove.GetParent()->ScreenToClient(&size);

			// Move to current top and increment top by height
			::MoveWindowBy(&mHeader->mSubjectMove, 0, top - size.top, false);
			top += size.Height();

			// Always show it
			mHeader->mSubjectMove.ShowWindow(SW_SHOW);
		}
		else
			// Always hide it
			mHeader->mSubjectMove.ShowWindow(SW_HIDE);
	}
	else
	{
		// Hide all addresses and subject
		mHeader->mFromMove.ShowWindow(SW_HIDE);
		mHeader->mToMove.ShowWindow(SW_HIDE);
		mHeader->mCCMove.ShowWindow(SW_HIDE);
		mHeader->mSubjectMove.ShowWindow(SW_HIDE);

		// Make sure we have something to show the message count
		top = 16;
	}
	
	// Get current size
	CRect size;
	mHeader->mPartsMove.GetWindowRect(&size);
	mHeader->mPartsMove.GetParent()->ScreenToClient(&size);

	// Move to current top and increment top by height
	::MoveWindowBy(&mHeader->mPartsMove, 0, top - size.top, false);
	top += size.Height();

	// Get enclosure area
	mView.GetClientRect(size);

	// Get current size of top header area
	CRect rectH(0, 0, size.Width(), top);
	mHeader->MoveWindow(rectH);

	// Move splitter to fill remainder of view space
	CRect rectS(0, top, size.Width(), size.Height());
	mSplitterView.MoveWindow(rectS);
	
	RedrawWindow();
}

// Show/hide parts area
void CMessageWindow::ShowSecretPane(bool show)
{
	if (!(mShowSecure ^ show))
		return;

	CRect pane_size;
	mSecurePane.GetWindowRect(pane_size);
	int moveby = pane_size.Height();

	if (show)
	{
		// Shrink/move text pane
		::ResizeWindowBy(&mTextArea, 0, -moveby, false);
		::MoveWindowBy(&mTextArea, 0, moveby, true);

		// Show parts after all other changes
		mSecurePane.ShowWindow(SW_SHOW);
	}
	else
	{
		// Hide parts before other changes
		mSecurePane.ShowWindow(SW_HIDE);

		// Expand/move splitter
		::ResizeWindowBy(&mTextArea, 0, moveby, false);
		::MoveWindowBy(&mTextArea, 0, -moveby, true);
	}
	
	mShowSecure = show;
}

void CMessageWindow::OnMessagePartsTwister()
{
	bool expanding = !mHeader->mPartsTwister.IsPushed();

	CRect parts_size;
	mPartsPane.GetWindowRect(parts_size);
	int moveby = parts_size.Height();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts before collapsing
		mPartsTableAlign->SetAlignment(true, true, true, false);

		// Hide the parts table pane in the splitter
		mSplitterView.ShowView(false, true);

		// Show/hide items
		mPartsPane.ShowWindow(SW_HIDE);
		mHeader->mFlatHierarchyBtn.ShowWindow(SW_HIDE);
		mHeader->mPartsField.ShowWindow(SW_SHOW);
		mHeader->mPartsTwister.SetPushed(false);

		// Always focus on text
		GetParentFrame()->SetActiveView(mText);
		mText->SetFocus();
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitterView.ShowView(true, true);
		
		// Show/hide items
		mPartsPane.ShowWindow(SW_SHOW);
		mHeader->mPartsField.ShowWindow(SW_HIDE);
		mHeader->mFlatHierarchyBtn.ShowWindow(SW_SHOW);
		mHeader->mPartsTwister.SetPushed(true);
		
		// Turn on resize of parts after expanding
		mPartsTableAlign->SetAlignment(true, true, true, true);
		
		// Always focus on parts
		GetParentFrame()->SetActiveView(mHeader);
		mPartsTable.SetFocus();
	}

	RedrawWindow();

}

void CMessageWindow::OnMessagePartsFlat()
{
	// Toggle flat state
	DoPartsFlat(!mPartsTable.GetFlat());
}

// Flatten parts table
void CMessageWindow::DoPartsFlat(bool flat)
{
	mPartsTable.SetFlat(flat);
	mHeader->mFlatHierarchyBtn.SetPushed(!mPartsTable.GetFlat());
	UpdatePartsCaption();
}

// Reset state from prefs
void CMessageWindow::ResetState()
{
	// Check for available state
	CMessageWindowState& state = CPreferences::sPrefs->mMessageWindowDefault.Value();

	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Do not set if empty
	CRect set_rect = state.GetBestRect(CPreferences::sPrefs->mMessageWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
										set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
	}

	// Force twist down if required
	if (state.GetPartsTwisted() ^ mHeader->mPartsTwister.IsPushed())
		OnMessagePartsTwister();
	mHeaderState.mExpanded = !state.GetCollapsed();
	DoPartsFlat(state.GetFlat());

	// Force layout
	LayoutHeader();
	mSplitterView.SetPixelSplitPos(state.GetSplitChange());

	// Do zoom
	if (state.GetState() == eWindowStateMax)
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	if (!GetParentFrame()->IsWindowVisible())
		GetParentFrame()->ActivateFrame();
	GetParentFrame()->RedrawWindow();
	
	// Redo wrap here to allow for first time update
	mText->SetHardWrap(!mText->GetHardWrap());
	mText->SetHardWrap(!mText->GetHardWrap());
}

// Save current state as default
void CMessageWindow::OnSaveDefaultState()
{
	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CMessageWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mSplitterView.GetPixelSplitPos(), mHeader->mPartsTwister.IsPushed(), !mHeaderState.mExpanded, mPartsTable.GetFlat(), false);
	if (CPreferences::sPrefs->mMessageWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMessageWindowDefault.SetDirty();

}


// Reset to default state
void CMessageWindow::OnResetDefaultState()
{
	ResetState();
}

