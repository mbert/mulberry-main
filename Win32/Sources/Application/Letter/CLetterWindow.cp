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


// Source for CLetterWindow class


#include "CLetterWindow.h"

#include "CAddressBookManager.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCaptionParser.h"
#include "CCharsetManager.h"
#include "CCopyToMenu.h"
#include "CDataAttachment.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CIMAPClient.h"
#include "CLetterDoc.h"
#include "CLetterFrame.h"
#include "CLetterHeaderView.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CServerWindow.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSecurityPlugin.h"
#include "CSpeechSynthesis.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CTextEngine.h"
#include "CTwister.h"
#include "CUnicodeUtils.h"
#include "CEnrichedTransformer.h"
#include "CHTMLTransformer.h"

#include <afxrich.h>

const int cWindowWidth = 500;
const int cToolbarHeight = 56;
const int cWindowHeight = cToolbarHeight + 128;
const int cPartTitleHeight = 16;

int cLtrWndHeaderHeight = 124;
int cLtrWndPartsCollapsedHeight = 24;
int cLtrWndPartsMinimumHeight = 84;

int cLtrWndMaxTitleLength = 64;				// Maximum number of chars in window title
int cLtrWndTwistMove = 32;
int cLtrWndPartsTwistMove = 84;

/////////////////////////////////////////////////////////////////////////////
// CLetterWindow

IMPLEMENT_DYNCREATE(CLetterWindow, CWnd)

BEGIN_MESSAGE_MAP(CLetterWindow, CWnd)

	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnFileNewDraft)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)

	ON_UPDATE_COMMAND_UI(IDM_FILE_IMPORT, OnUpdateFileImport)
	ON_COMMAND(IDM_FILE_IMPORT, OnFileImport)

	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)

	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PASTE_INDENTED, OnUpdatePasteIndented)
	ON_COMMAND(IDM_EDIT_PASTE_INDENTED, OnPasteIndented)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_SPEAK, OnUpdateEditSpeak)
	ON_COMMAND(IDM_EDIT_SPEAK, OnEditSpeak)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_SPELLCHECK, OnUpdateEditSpellCheck)
	ON_COMMAND(IDM_EDIT_SPELLCHECK, OnEditSpellCheck)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_INCLUDE, OnUpdateDraftInclude)
	ON_COMMAND(IDM_DRAFT_INCLUDE, OnDraftInclude)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_INSERT_SIGNATURE, OnUpdateDraftNotReject)
	ON_COMMAND(IDM_DRAFT_INSERT_SIGNATURE, OnDraftInsertSignature)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_ADD_CC, OnUpdateDraftAddCc)
	ON_COMMAND(IDM_DRAFT_ADD_CC, OnDraftAddCc)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_ADD_BCC, OnUpdateDraftAddBcc)
	ON_COMMAND(IDM_DRAFT_ADD_BCC, OnDraftAddBcc)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_ATTACH_FILE, OnUpdateDraftAttachFile)
	ON_COMMAND(IDM_DRAFT_ATTACH_FILE, OnDraftAttachFile)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_NEW_PLAIN_PART, OnUpdateNotReject)
	ON_COMMAND(IDM_DRAFT_NEW_PLAIN_PART, OnDraftNewPlainPart)
	
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_NEW_ENRICHED_PART, OnUpdateAllowStyled)
	ON_COMMAND(IDM_DRAFT_NEW_ENRICHED_PART, OnDraftNewEnrichedPart)
	
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_NEW_HTML_PART, OnUpdateAllowStyled)
	ON_COMMAND(IDM_DRAFT_NEW_HTML_PART, OnDraftNewHTMLPart)

	ON_UPDATE_COMMAND_UI(IDM_MULTIPART_MIXED, OnUpdateNotReject)
	ON_COMMAND(IDM_MULTIPART_MIXED, OnDraftMultipartMixed)

	ON_UPDATE_COMMAND_UI(IDM_MULTIPART_PARALLEL, OnUpdateNotReject)
	ON_COMMAND(IDM_MULTIPART_PARALLEL, OnDraftMultipartParallel)

	ON_UPDATE_COMMAND_UI(IDM_MULTIPART_DIGEST, OnUpdateNotReject)
	ON_COMMAND(IDM_MULTIPART_DIGEST, OnDraftMultipartDigest)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_DRAFT_CURRENT_PLAIN, IDM_DRAFT_CURRENT_HTML, OnUpdateTextStyle)
	ON_COMMAND_RANGE(IDM_DRAFT_CURRENT_PLAIN, IDM_DRAFT_CURRENT_HTML, OnChangeTextStyle)

	ON_UPDATE_COMMAND_UI(IDM_MULTIPART_ALTERNATIVE, OnUpdateNotReject)
	ON_COMMAND(IDM_MULTIPART_ALTERNATIVE, OnDraftMultipartAlternative)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_SEND, OnUpdateDraftSendMail)
	ON_COMMAND(IDM_DRAFT_SEND, OnDraftSendMail)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_AppendToMailboxChoose, IDM_AppendToMailboxEnd, OnUpdateNotReject)
	ON_COMMAND_RANGE(IDM_AppendToMailboxChoose, IDM_AppendToMailboxEnd, OnDraftAppend)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_APPEND_NOW, OnUpdateNotReject)
	ON_COMMAND(IDM_DRAFT_APPEND_NOW, OnDraftAppendNow)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_COPY_ORIGINAL, OnUpdateDraftCopyOriginal)
	ON_COMMAND(IDM_DRAFT_COPY_ORIGINAL, OnDraftCopyOriginal)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_SIGN, OnUpdateDraftSign)
	ON_COMMAND(IDM_DRAFT_SIGN, OnDraftSign)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_ENCRYPT, OnUpdateDraftEncrypt)
	ON_COMMAND(IDM_DRAFT_ENCRYPT, OnDraftEncrypt)

	ON_UPDATE_COMMAND_UI(IDM_DRAFT_MDNRECEIPT, OnUpdateDraftMDN)
	ON_COMMAND(IDM_DRAFT_MDNRECEIPT, OnDraftMDN)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_DSNSUCCESS, OnUpdateDraftDSNSuccess)
	ON_COMMAND(IDM_DRAFT_DSNSUCCESS, OnDraftDSNSuccess)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_DSNFAILURE, OnUpdateDraftDSNFailure)
	ON_COMMAND(IDM_DRAFT_DSNFAILURE, OnDraftDSNFailure)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_DSNDELAY, OnUpdateDraftDSNDelay)
	ON_COMMAND(IDM_DRAFT_DSNDELAY, OnDraftDSNDelay)
	ON_UPDATE_COMMAND_UI(IDM_DRAFT_DSNFULL, OnUpdateDraftDSNFull)
	ON_COMMAND(IDM_DRAFT_DSNFULL, OnDraftDSNFull)

	ON_COMMAND(IDC_LETTERTOTWISTER, OnLetterToTwister)
	ON_COMMAND(IDC_LETTERCCTWISTER, OnLetterCCTwister)
	ON_COMMAND(IDC_LETTERBCCTWISTER, OnLetterBCCTwister)
	ON_COMMAND(IDC_LETTERPARTSTWISTER, OnLetterPartsTwister)

	ON_COMMAND(IDC_LETTERSUBJECTFIELD, OnChangeSubject)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnUpdateDraftAppend)
	ON_COMMAND_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnLetterCopyToPopup)

	ON_COMMAND(IDC_LETTER_IDENTITYEDIT, OnEditCustom)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnChangeIdentity)

	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_EXPAND_HEADER, OnUpdateMenuExpandHeader)
	ON_COMMAND(IDM_WINDOWS_EXPAND_HEADER, OnLetterHeaderTwister)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOW_PARTS, OnUpdateWindowsShowParts)
	ON_COMMAND(IDM_WINDOWS_SHOW_PARTS, OnLetterPartsTwister)

	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	ON_COMMAND_RANGE(IDM_STYLE_PLAIN, IDM_STYLE_UNDERLINE, OnStyle)
	ON_COMMAND_RANGE(IDM_ALIGN_LEFT, IDM_ALIGN_RIGHT, OnAlignment)
	ON_COMMAND_RANGE(IDM_FONTStart, IDM_FONTEnd, OnFont)
	ON_COMMAND_RANGE(IDM_SIZE_8, IDM_SIZE_24, OnFontSize)
	ON_COMMAND_RANGE(IDM_COLOUR_BLACK, IDM_COLOUR_OTHER, OnColor)
	
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STYLE_PLAIN, IDM_STYLE_UNDERLINE, OnUpdateStyle)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ALIGN_LEFT, IDM_ALIGN_RIGHT, OnUpdateAlignment)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FONTStart, IDM_FONTEnd, OnUpdateFont)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SIZE_8, IDM_SIZE_24, OnUpdateFontSize)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_COLOUR_BLACK, IDM_COLOUR_OTHER, OnUpdateColor)
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnFileNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnFileNewDraft)

	ON_COMMAND(IDC_TOOLBARFILESAVEBTN, OnFileSave)

	ON_COMMAND(IDC_TOOLBARMESSAGEPRINTBTN, OnFilePrint)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERSPELLBTN, OnUpdateEditSpellCheck)
	ON_COMMAND(IDC_TOOLBARLETTERSPELLBTN, OnEditSpellCheck)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERSIGNATUREBTN, OnUpdateDraftNotReject)
	ON_COMMAND(IDC_TOOLBARLETTERSIGNATUREBTN, OnDraftInsertSignature)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERADDCCBTN, OnUpdateDraftAddCc)
	ON_COMMAND(IDC_TOOLBARLETTERADDCCBTN, OnDraftAddCc)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERADDBCCBTN, OnUpdateDraftAddBcc)
	ON_COMMAND(IDC_TOOLBARLETTERADDBCCBTN, OnDraftAddBcc)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERATTACHBTN, OnUpdateDraftAttachFile)
	ON_COMMAND(IDC_TOOLBARLETTERATTACHBTN, OnDraftAttachFile)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERSENDBTN, OnUpdateDraftSendMail)
	ON_COMMAND(IDC_TOOLBARLETTERSENDBTN, OnDraftSendMail)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERAPPENDNOWBTN, OnUpdateNotReject)
	ON_COMMAND(IDC_TOOLBARLETTERAPPENDNOWBTN, OnDraftAppendNowBtn)
	ON_COMMAND(IDC_TOOLBARLETTERAPPENDNOWOPTIONBTN, OnDraftAppendNowOptionBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERCOPYORIGINALBTN, OnUpdateDraftCopyOriginal)
	ON_COMMAND(IDC_TOOLBARLETTERCOPYORIGINALBTN, OnDraftCopyOriginal)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERSIGNPOPUP, OnUpdateDraftSign)
	ON_COMMAND(IDC_TOOLBARLETTERSIGNPOPUP, OnDraftSign)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERENCRYPTPOPUP, OnUpdateDraftEncrypt)
	ON_COMMAND(IDC_TOOLBARLETTERENCRYPTPOPUP, OnDraftEncrypt)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERRECEIPTBTN, OnUpdateDraftMDN)
	ON_COMMAND(IDC_TOOLBARLETTERRECEIPTBTN, OnDraftMDN)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARLETTERDSNBTN, OnUpdateDraftDSNSuccess)
	ON_COMMAND(IDC_TOOLBARLETTERDSNBTN, OnDraftDSNSuccess)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARWINDOWCOLLAPSEHEADERBTN, OnUpdateExpandHeader)
	ON_COMMAND(IDC_TOOLBARWINDOWCOLLAPSEHEADERBTN, OnLetterHeaderTwister)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterWindow construction/destruction

cdmutexprotect<CLetterWindow::CLetterWindowList> CLetterWindow::sLetterWindows;
CMultiDocTemplate* CLetterWindow::sLetterDocTemplate = NULL;
cdstring  CLetterWindow::sLastAppendTo;

const int IDT_TIMER_ID = 12347;

// Default constructor
CLetterWindow::CLetterWindow()
{
	// Add to list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		sLetterWindows->push_back(this);
	}

	// NULL out others
	mMsgs = NULL;
	mIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	mCustomIdentity = false;
	mDoSign = false;
	mDoEncrypt = false;
	mSignatureLength = 0;
	mCopyOriginalAllowed = false;
	mCopyOriginal = false;
	mCurrentPart = NULL;
	mBody = NULL;

	mHeaderState.mExpanded = true;
	mHeaderState.mToVisible = true;
	mHeaderState.mToExpanded = false;
	mHeaderState.mCcVisible = false;
	mHeaderState.mCcExpanded = false;
	mHeaderState.mBccVisible = false;
	mHeaderState.mBccExpanded = false;
	mHeaderState.mSubjectVisible = true;

	mSentOnce = false;
	mDirty = false;
	mReply = false;
	mForward = false;
	mBounce = false;
	mReject = false;
	mSendAgain = false;
	mOriginalEncrypted = false;
	mMarkSaved = false;
	mBounceHeader = NULL;
	mPartsTableAlign = NULL;
	
	mTimerID = 0;
}

// Default destructor
CLetterWindow::~CLetterWindow()
{
	// Set status
	SetClosing();

	// Delete all parts
	delete mBody;
	mBody = NULL;

	// Delete any stored bounce header
	delete mBounceHeader;
	mBounceHeader = NULL;

	// Delete any message lists
	delete mMsgs;
	mMsgs = NULL;

	// Remove from list
	{
		cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
		CLetterWindowList::iterator found = std::find(sLetterWindows->begin(), sLetterWindows->end(), this);
		if (found != sLetterWindows->end())
			sLetterWindows->erase(found);
	}

	// Set status
	SetClosed();
}

// Manually create document
CLetterWindow* CLetterWindow::ManualCreate()
{
	CDocument* aDoc = sLetterDocTemplate->OpenDocumentFile(NULL, false);
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	CLetterWindow* pWnd = static_cast<CLetterWindow*>(aFrame->GetTopWindow());

	// Give it to document
	((CLetterDoc*) aDoc)->SetLetterWindow(pWnd);

	// Reset to default size
	pWnd->ResetState();

	// Do after reset state, once its visible
	if (pWnd->mHeaderState.mExpanded)
		pWnd->mHeader->mToField.SetFocus();
	else
		pWnd->mText->SetFocus();

	return (CLetterWindow*) pWnd;
}

// Update positions of UI elements
void CLetterWindow::UpdateUIPos()
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;

	cLtrWndHeaderHeight += large_offset;
	cLtrWndPartsCollapsedHeight += small_offset;

	cLtrWndTwistMove += small_offset;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CLetterWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Do inherited
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

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
	rect = CRect(0, 0, cWindowWidth, 122);
	mHeader = new CLetterHeaderView;
	mHeader->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mView, IDC_STATIC);
	mView.AddAlignment(new CWndAlignment(mHeader, CWndAlignment::eAlign_TopWidth));
	CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
	if (pContext != NULL && pContext->m_pCurrentDoc != NULL)
		pContext->m_pCurrentDoc->AddView(mHeader);

	// Init twisters
	mHeader->mToField.SetItems(this, &mHeader->mToTwister, &mHeader->mToMove);
	mHeader->mCCField.SetItems(this, &mHeader->mCCTwister, &mHeader->mCCMove);
	mHeader->mBCCField.SetItems(this, &mHeader->mBCCTwister, &mHeader->mBCCMove);

	// Create splitter view
	rect = CRect(0, 122, cWindowWidth, cWindowHeight);
	mSplitterView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mView, IDC_STATIC);
	mView.AddAlignment(new CWndAlignment(&mSplitterView, CWndAlignment::eAlign_WidthHeight));

	// Parts pane
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mPartsPane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Parts table
	rect = CRect(0, cPartTitleHeight, cWindowWidth, cWindowHeight);
	mPartsTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, &mPartsPane, IDC_LETTERPARTSTABLE);
	mPartsTable.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	mPartsTable.SetContextMenuID(IDR_POPUP_CONTEXT_LETTER_PARTS);
	mPartsTable.SetContextView(mHeader);
	mPartsTable.SetSuperCommander(this);
	mPartsTable.SetTabOrder(mText, &mHeader->mSubjectField, mText, mHeader);
	mHeader->mSubjectField.SetTabOrder(&mPartsTable, &mHeader->mBCCField, mHeader, mHeader);
	mPartsTable.ResetFont(CFontCache::GetListFont());
	mPartsPane.AddAlignment(mPartsTableAlign = new CWndAlignment(&mPartsTable, CWndAlignment::eAlign_WidthHeight));

	// Parts titles
	rect = CRect(0, 0, cWindowWidth, cPartTitleHeight);
	mPartsTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mPartsPane, IDC_MESSAGEPARTSTABLETITLES);
	mPartsTitles.SyncTable(&mPartsTable);
	mPartsTable.SetTitles(&mPartsTitles);
	mPartsPane.AddAlignment(new CWndAlignment(&mPartsTitles, CWndAlignment::eAlign_TopWidth));

	mPartsTitles.LoadTitles("UI::Titles::LetterParts", 6);
	mPartsTable.SetAttachmentsOnly(CPreferences::sPrefs->mDisplayAttachments.GetValue());

	// Create dummy text part
	mBody = (mCurrentPart = new CDataAttachment((char*) NULL));
	mCurrentPart->SetContent(eContentText, CPreferences::sPrefs->compose_as.GetValue());
	mPartsTable.SetBody(mBody);
	mPartsTable.SetRowShow(mCurrentPart);
	UpdatePartsCaption();

	// Text area
	mTextArea.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, cWindowWidth, cWindowHeight), this, IDC_STATIC);
	mTextArea.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	// Text view (do first as header view requires this for tabbing)
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mText = new CLetterTextEditView;
	mText->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, &mTextArea, IDC_STATIC);
	if (pContext != NULL && pContext->m_pCurrentDoc != NULL)
		pContext->m_pCurrentDoc->AddView(mText);

	mText->GetRichEditCtrl().SetModify(false);
	bool html = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML);
	mText->SetFont(html ? CFontCache::GetHTMLFont() : CFontCache::GetDisplayFont());
	mText->SetContextMenuID(IDR_POPUP_CONTEXT_LETTER_EDIT);
	mText->SetSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	mText->SetWrap(CPreferences::sPrefs->wrap_length.GetValue());
	mText->SetHardWrap(!CPreferences::sPrefs->mWindowWrap.GetValue());
	mText->SetTabSelectAll(false);
	mText->SetTabOrder(&mHeader->mToField, &mPartsTable, mHeader, mHeader);
	mHeader->mToField.SetTabOrder(&mHeader->mCCField, mText, mHeader, mText);
	mPartsTable.SetTabOrder(mText, &mHeader->mSubjectField, mText, mHeader);
	mText->SetLetterWindow(this);
	mText->SetToolbar(CPreferences::sPrefs->compose_as.GetValue());
	mText->SetFindAllowed(true);
	mText->SetQuotation(CPreferences::sPrefs->mReplyQuote.GetValue());
	mText->SetSelectionRange(0, 0);
	mText->SetSuperCommander(this);
	mTextArea.AddAlignment(new CWndAlignment(mText, CWndAlignment::eAlign_WidthHeight));
	mTextArea.AddAlignment(new CWndAlignment(mText->GetToolbar(), CWndAlignment::eAlign_TopWidth));

	// Install the splitter items
	mSplitterView.InstallViews(&mPartsPane, &mTextArea, true);
	mSplitterView.SetMinima(84, 64);
	mSplitterView.SetPixelSplitPos(84);
	mSplitterView.ShowView(false, true);
	mSplitterView.SetLockResize(true);

	// Do CC/Bcc show if required
	if (CPreferences::sPrefs->mShowCCs.GetValue())
		OnDraftAddCc();
	if (CPreferences::sPrefs->mShowBCCs.GetValue())
		OnDraftAddBcc();

#if 0
	// Set to default identity
	SetIdentity(NULL);

	// Set to cached copy to if prefs set
	if (CPreferences::sPrefs->remember_record.GetValue() && !sLastAppendTo.empty())
		mHeader->mCopyTo.SetSelectedMbox(sLastAppendTo, sLastAppendTo.empty(), (sLastAppendTo == "\1"));

	// Check for missing Copy To
	if (mHeader->mCopyTo.IsMissingMailbox())
		CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::MissingCopyTo", mHeader->mCopyTo.GetMissingName());
#endif

	// Make to address focus
	if (mHeaderState.mExpanded)
		mHeader->mToField.SetFocus();
	else
		mText->SetFocus();

	// Set status
	GetParentFrame()->SetActiveView(mHeader);
	SetOpen();

	// Start safety save timer if required
	mAutoSaveTime = ::time(NULL);
	mTimerID = SetTimer(IDT_TIMER_ID, 1000, NULL);

	return 0;
}

// Adjust splits
void CLetterWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	if (mView.m_hWnd != NULL)
	{
		CRect rect(0, 0, cx, cy);
		mView.MoveWindow(rect);
	}
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CWnd::OnSize(nType, cx, cy);
}

void CLetterWindow::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);
}

void CLetterWindow::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());
}

void CLetterWindow::OnTimer(UINT nIDEvent)
{
	// Do periodic chore
	if (nIDEvent == mTimerID)
	{
		// See if we are equal or greater than trigger
		if (::time(NULL) >= mAutoSaveTime + CPreferences::sPrefs->mAutoSaveDraftsInterval.GetValue())
		{
			// Do auto save
			static_cast<CLetterDoc*>(GetDocument())->SaveTemporary();
		}
	}
}

void CLetterWindow::ResetAutoSaveTimer()
{
	mAutoSaveTime = ::time(NULL);
}

void CLetterWindow::PauseAutoSaveTimer(bool pause)
{
	if (pause)
	{
		KillTimer(mTimerID);
		mTimerID = 0;
	}
	else
		mTimerID = SetTimer(IDT_TIMER_ID, 1000, NULL);
}

// SafetySave
//   Save this draft into the safety save directory when Mulberry crashes.
//
// path   -> path to save to
// result <- total number of recovered drafts
//
// Notes:
// This function is called after during exception handling. We must do our best to save the
// draft without generate further exceptions (ie test validity of all pointers we use here).
// This may not be possible if the memory subsystem generated the crash, but we have to try anyway.
//
// History:
// 11-Sep-2002 cd: created initial implementation
//

bool CLetterWindow::SafetySave(const cdstring& path)
{
	bool result = false;
	bool was_dirty = IsDirty();
	CFile* pFile = NULL;
	try
	{
		// Get document object
		CDocument* doc = mText ? GetDocument() : NULL;
		if (doc == NULL)
			return false;
		
		// Create file object
		pFile = new CFile;
		if (pFile == NULL)
			return false;

		// Try to create the file for saving to
		CFileException fe;
		if (!pFile->Open(path.win_str(), CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, &fe))
		{
			delete pFile;
			return false;
		}

		// Create the archive for saving
		CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
		saveArchive.m_pDocument = doc;
		saveArchive.m_bForceFlat = FALSE;
		
		// Do the save
		Serialize(saveArchive);
		
		// Close the archive and file
		saveArchive.Close();
		pFile->Close();
		delete pFile;

		result = true;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		if (pFile)
		{
			pFile->Abort(); // will not throw an exception
			delete pFile;
		}

		result = false;
	}

	// Make sure it is still marked as dirty	
	if (was_dirty)
		SetDirty(true);

	return result;
}

#pragma mark ____________________________Initialise

// Add address list to caption/field:
void CLetterWindow::SetTwistList(CCmdEdit* aField,
									CTwister* aTwister,
									CWnd* aMover,
									const CAddressList* list)
{
	
	cdstring addrs;

	// Add all in list
	bool got_two = false;
	for(CAddressList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		if (!addrs.empty())
		{
			addrs += os_endl;
			got_two = true;
		}
		addrs += (*iter)->GetFullAddress();
	}
	
	// Update field - will null if no addresses
	aField->SetText(addrs);

	// Force twist down if one or more
	if ((aTwister->IsPushed() && !got_two) ||
		(!aTwister->IsPushed() && got_two))
		DoTwist(aMover, aField, aTwister);

}

// Add address lists
void CLetterWindow::SetAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc)
{

	// Add all tos
	if (to)
		SetTwistList(&mHeader->mToField, &mHeader->mToTwister, &mHeader->mToMove, to);

	// Add all Ccs
	if (cc)
	{
		SetTwistList(&mHeader->mCCField, &mHeader->mCCTwister, &mHeader->mCCMove, cc);
		
		// Make sure Cc is visible
		if (!mHeaderState.mCcVisible && cc->size())
			OnDraftAddCc();
	}	

	// Add all Bccs
	if (bcc)
	{
		SetTwistList(&mHeader->mBCCField, &mHeader->mBCCTwister, &mHeader->mBCCMove, bcc);
		
		// Make sure Bcc is visible
		if (!mHeaderState.mBccVisible && bcc->size())
			OnDraftAddBcc();
	}	

	// Set target to text if some items inserted into list
	if (mHeaderState.mExpanded)
	{
		if (to || cc || bcc)
			mHeader->mSubjectField.SetFocus();
		else
			mHeader->mToField.SetFocus();
	}
}


// Get address lists
void CLetterWindow::GetAddressLists(CAddressList*& to,
									CAddressList*& cc,
									CAddressList*& bcc)
{
	// Get to text
	cdstring addrs;
	mHeader->mToField.GetText(addrs);
	to = new CAddressList(addrs, addrs.length());

	// Get ccs
	mHeader->mCCField.GetText(addrs);
	cc = new CAddressList(addrs, addrs.length());

	// Get Bccs
	mHeader->mBCCField.GetText(addrs);
	bcc = new CAddressList(addrs, addrs.length());
}

void CLetterWindow::SetSubject(const cdstring& text)
{
	mHeader->mSubjectField.SetText(text);
	OnChangeSubject();
}

void CLetterWindow::GetSubject(cdstring& text) const
{
	cdustring utext;
	mHeader->mSubjectField.GetText(utext);

	// Remove any curly quotes
	unichar_t* p = utext.c_str_mod();
	while(*p)
	{
		if ((*p == 0x201C) || (*p == 0x201D))
			*p = '\"';
		else if ((*p == 0x2018) || (*p == 0x2019))
			*p = '\'';
		p++;
	}
	text = utext.ToUTF8();
}

// Do common bits after setting a message
void CLetterWindow::PostSetMessage(const CIdentity* id)
{
	// Enable append replied if not sub-message (not for rejects)
	CMbox* found = NULL;
	if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce) &&
		mHeader->mCopyTo.GetSelectedMbox(found, false) &&
		(mMsgs->front()->GetMbox() != found) &&
			!mMsgs->front()->IsSubMessage())
	{
		mCopyOriginalAllowed = true;
	}

	// Must make sure window is listening to protocol logoffs
	if (mMsgs)
	{
		for(CMessageList::const_iterator iter = mMsgs->begin(); iter != mMsgs->end(); iter++)
			(*iter)->GetMbox()->GetMsgProtocol()->Add_Listener(this);
	}

	// Init the identity
	InitIdentity(id);

	// If message comes from INBOX and pref set check it as true
	// Do this after init'ing the identity
	if (mMsgs && mMsgs->size() && !mReject &&
		!::strcmpnocase(mMsgs->front()->GetMbox()->GetName(), cINBOX) &&
		CPreferences::sPrefs->inbox_append.GetValue())
	{
		mCopyOriginal = true;
	}

	// Need to reset focus
	if (mReply || mReject || mSendAgain || !mHeaderState.mExpanded)
	{
		// Set target to text
		GetParentFrame()->SetActiveView(mText);
		mText->SetFocus();
	}
	else
	{
		// Set target to to address
		GetParentFrame()->SetActiveView(mHeader);
		mHeader->mToField.SetFocus();
	}

	// Disable certain controls for rejects
	if (mReject)
	{
		// Disable all edit fields
		mHeader->mToField.EnableWindow(false);
		mHeader->mCCField.EnableWindow(false);
		mHeader->mBCCField.EnableWindow(false);
		mHeader->mSubjectField.EnableWindow(false);

		// Disable copy to controls
		mHeader->mCopyTo.EnableWindow(false);
		mCopyOriginalAllowed = false;
	}

	// Finally force redraw
	//mText->UpdateMargins();
	UpdatePartsCaption();
	RedrawWindow();
	
	// Make sure window items are not dirty
	SetDirty(false);
	
	// Force main toolbar to update
	mView.RefreshToolbar();
	
	// Make sure its visible
	GetParentFrame()->ShowWindow(SW_SHOW);
	
	// Update certain items after its visible
	OnChangeSubject();
	mText->UpdateMargins();
	
	// Do address capture if needed
	if ((mReply || mForward) && mMsgs && CPreferences::sPrefs->mCaptureRespond.GetValue())
		CAddressBookManager::sAddressBookManager->CaptureAddress(*mMsgs);
}

#pragma mark ____________________________Sending

// Copy to mailbox now
void CLetterWindow::OnDraftAppendNow()
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);
	CopyNow(option_key);
}

void CLetterWindow::OnDraftAppendNowBtn()
{
	CopyNow(false);
}

void CLetterWindow::OnDraftAppendNowOptionBtn()
{
	CopyNow(true);
}

// Copy to mailbox now
void CLetterWindow::OnDraftAppend(UINT nID)
{
	bool option_key = (::GetKeyState(VK_MENU) < 0);

	// Get mailbox to append to
	CMbox* append_mbox = NULL;
	CCopyToMenu::GetMbox(false, nID - IDM_AppendToMailboxChoose, append_mbox);
	CopyNow(append_mbox, option_key);
}

#pragma mark ____________________________Quoting

// Include message selection indented
void CLetterWindow::IncludeMessage(CMessage* theMsg, bool forward, int start, int length, bool header)
{
	// Only do if text available
	if (!theMsg->HasText())
		return;

	// Reset undo action in edit control
	mText->GetRichEditCtrl().EmptyUndoBuffer();
	
	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return;

	// Read it in
	try
	{
		// Get first suitable quotable part
		CAttachment* attach = theMsg->FirstDisplayPart();

		if (attach)
		{
			// Get header if required
			const char* msg_hdr = header ? theMsg->GetHeader() : NULL;

			// Read text of specific message part
			const char* msg_txt = attach->ReadPart(theMsg);
			i18n::ECharsetCode charset = attach->GetContent().GetCharset();

			// Convert to utf8
			cdstring utf8;
			if (msg_txt != NULL)
			{
				if (charset == i18n::eUTF8)
				{
					utf8 = msg_txt;
				}
				else
				{
					std::ostrstream sout;
					i18n::CCharsetManager::sCharsetManager.ToUTF8(charset, msg_txt, ::strlen(msg_txt), sout);
					sout << std::ends;
					utf8.steal(sout.str());
				}
			}

			// Quote it if available
			if (header && msg_hdr || msg_txt)
				IncludeMessageTxt(theMsg, msg_hdr, utf8, forward, attach->GetContent().GetContentSubtype(), attach->GetContent().IsFlowed());
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CLetterWindow::IncludeMessageTxt(CMessage* theMsg, const char* msg_hdr, const char* msg_txt,
										bool forward, EContentSubType type, bool is_flowed)
{
	long selStart;
	long selEnd;
	mText->GetSelectionRange(selStart, selEnd);
	cdstring theTxt;
	
	// Force insert of CRLF if at very top of message of cursor will be placed at top
	if ((selStart == 0) &&
		((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
		 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue())))
	{
		cdstring convertTxt = os_endl2;
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
			theTxt += convertTxt;
			break;
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
			theTxt += converted.get();
			break;
		}
		}
	}

	// Get start caption
	{
		cdstring convertTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardStart.GetValue() :
														CPreferences::sPrefs->mReplyStart.GetValue(), theMsg);;
		if (convertTxt.length())
		{
			switch(mCurrentPart->GetContent().GetContentSubtype())
			{
			case eContentSubPlain:
				theTxt += convertTxt;
				break;
			case eContentSubEnriched:
			{
				std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
				theTxt += converted.get();
				break;
			}
			case eContentSubHTML:
			{
				std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
				theTxt += converted.get();
				break;
			}
			}
		}
	}

	// Start styled quotation
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		// Must add line end after caption - the other two sub-types will automatically add
		// a line end when the tag is encountered
		theTxt += os_endl;
		break;
	}
	case eContentSubEnriched:
	{
		theTxt += "<excerpt>";
		break;
	}
	case eContentSubHTML:
	{
		theTxt += "<blockquote>";
		break;
	}
	}

	// Include the message's header as addition if required
	if (msg_hdr)
	{
		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
		{
			std::auto_ptr<const char> quoted(QuoteText(msg_hdr, forward, true));
			theTxt += quoted.get();
			break;
		}
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(msg_hdr));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(msg_hdr));
			theTxt += converted.get();
			break;
		}
		}
	}

	// Always flow Enriched and HTML
	switch(type)
	{
	case eContentSubPlain:
	default:
		break;
	case eContentSubEnriched:
	case eContentSubHTML:
		is_flowed = true;
		break;
	}

	// Include the message's text as addition
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		// Parse the (possibly styled) original text out into plain text format
		cdustring text_utf16(msg_txt);
		mText->GetFormatter()->ParseBody(text_utf16, type, eViewPlain, -1);
		
		// Get parsed text as utf8
		cdstring text_utf8 = mText->GetFormatter()->GetParsedText().ToUTF8();
		
		// Quote it and add to the text being added to the draft
		std::auto_ptr<const char> quoted(QuoteText(text_utf8, forward, false, is_flowed));
		theTxt += quoted.get();
		break;
	}
	case eContentSubEnriched:
	{
		switch(type)
		{
		case eContentSubPlain:
		{
			// Must wrap if flowed
			std::auto_ptr<const char> wrapped;
			const char* txt = msg_txt;
			if (is_flowed)
			{
				wrapped.reset(CTextEngine::WrapLines(msg_txt, ::strlen(msg_txt), CRFC822::GetWrapLength(), false));
				txt = wrapped.get();
			}
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubEnriched:
		{
			theTxt += msg_txt;
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ConvertToEnriched(msg_txt));
			theTxt += converted.get();
			break;
		}
		}
		break;
	}
	case eContentSubHTML:
	{
		switch(type)
		{
		case eContentSubPlain:
		{
			// Must wrap if flowed
			std::auto_ptr<const char> wrapped;
			const char* txt = msg_txt;
			if (is_flowed)
			{
				wrapped.reset(CTextEngine::WrapLines(msg_txt, ::strlen(msg_txt), CRFC822::GetWrapLength(), false));
				txt = wrapped.get();
			}
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ConvertToHTML(msg_txt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			theTxt += msg_txt;
			break;
		}
		}
		break;
	}
	}

	// Stop styled quotation before line end
	switch(mCurrentPart->GetContent().GetContentSubtype())
	{
	case eContentSubPlain:
	{
		break;
	}
	case eContentSubEnriched:
	{
		theTxt += "</excerpt>";
		break;
	}
	case eContentSubHTML:
	{
		theTxt += "</blockquote>";
		break;
	}
	}

	// Finish with CRs
	{
		cdstring convertTxt;
		size_t len = theTxt.length();
		if ((theTxt[len - 2] != '\r') || (theTxt[len - 1] != '\n'))
			convertTxt += os_endl;
		convertTxt += os_endl;

		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubPlain:
			theTxt += convertTxt;
			break;
		case eContentSubEnriched:
		{
			std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
			theTxt += converted.get();
			break;
		}
		case eContentSubHTML:
		{
			std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
			theTxt += converted.get();
			break;
		}
		}
	}
	// Get end caption
	{
		cdstring convertTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardEnd.GetValue() :
														CPreferences::sPrefs->mReplyEnd.GetValue(), theMsg);;
		if (convertTxt.length())
		{
			convertTxt += os_endl;
			switch(mCurrentPart->GetContent().GetContentSubtype())
			{
			case eContentSubPlain:
				theTxt += convertTxt;
				break;
			case eContentSubEnriched:
			{
				std::auto_ptr<const char> converted(CEnrichedUtils::ToEnriched(convertTxt));
				theTxt += converted.get();
				break;
			}
			case eContentSubHTML:
			{
				std::auto_ptr<const char> converted(CHTMLUtils::ToHTML(convertTxt));
				theTxt += converted.get();
				break;
			}
			}
		}
	}

	mText->SetToolbar(CPreferences::sPrefs->compose_as.GetValue());
	{
		// Convert to utf16 and insert
		cdustring text_utf16(theTxt);
		mText->GetFormatter()->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);
	}
	mText->UpdateMargins();

	SyncPart();
	
	if ((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
		 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue()))
		mText->SetSelectionRange(selStart, selStart);
	
	// Force caltext, adjust image & refresh
	mText->Reset();

	// Force dirty off
	mText->GetRichEditCtrl().SetModify(false);
}


// Include the replied to mail
void CLetterWindow::IncludeFromReply(bool forward, bool header)
{
	// Beep if not replying
	if (!mMsgs)
	{
		::MessageBeep(1);
	}
	else
	{
		// See whether cursor at top or bottom
		if ((forward && CPreferences::sPrefs->mForwardCursorTop.GetValue()) ||
			 (!forward && CPreferences::sPrefs->mReplyCursorTop.GetValue()))
		{
			// Add all to message (cursor at top => in reverse)
			// Protect against mMsgs changing whilst doing ops
			size_t old_size = mMsgs->size();
			for(CMessageList::reverse_iterator riter = mMsgs->rbegin(); mMsgs && (mMsgs->size() == old_size) && (riter != mMsgs->rend()); riter++)
				IncludeMessage(*riter, forward, 0, 0, header);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			// Protect against mMsgs changing whilst doing ops
			size_t old_size = mMsgs->size();
			for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
				IncludeMessage(*iter, forward, 0, 0, header);
		}
		
		// Force dirty as IncludeMessage clears dirty flag
		mText->GetRichEditCtrl().SetModify(true);
	}
	
	// Reset focus to text
	GetParentFrame()->SetActiveView(mText);
	mText->SetFocus();
}

// Include some text indented
void CLetterWindow::IncludeText(const char* theText, bool forward, bool adding)
{
	cdstring theTxt;

	if (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain)
	{
		// Start with CR if not adding
		if (!adding)
			theTxt += os_endl;

		std::auto_ptr<const char> quoted(QuoteText(theText, forward));
		theTxt += quoted.get();

		// Finish with CR
		theTxt += os_endl;
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched)
	{
		// Start with <br> if not adding
		theTxt += "<excerpt>\r\n";
		theTxt += theText;
		theTxt += "</excerpt>";

		// Finish with CR
		theTxt += os_endl2;
	}
	else if(mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML)
	{
		// Start with <br> if not adding
		theTxt += "<blockquote>\r\n";
		theTxt += theText;
		theTxt += "\r\n</blockquote>\r\n";

		// Finish with CR
		theTxt += "<br>\r\n";
	}

	cdustring text_utf16(theTxt);
	mText->GetFormatter()->InsertFormattedText(text_utf16, mCurrentPart->GetContent().GetContentSubtype(), true, forward);
	mText->GetRichEditCtrl().SetModify(true);
}

#pragma mark ____________________________Parts

// Update attachments caption
void CLetterWindow::UpdatePartsCaption()
{
	unsigned long count = mPartsTable.CountParts();
	mHeader->mPartsField.SetValue(count);
	
	mHeader->mAttachments.ShowWindow(mPartsTable.HasAttachments());

	mHeader->mPartsTitle.ShowWindow(!mPartsTable.GetAttachmentsOnly());
	mHeader->mAttachmentsTitle.ShowWindow(mPartsTable.GetAttachmentsOnly());
}

// Set body content
void CLetterWindow::SetBody(CAttachment* attach, bool converting)
{
	// Sync current data if converting to ensure it is not lost.
	// When converting we know that the current part is valid.
	if (converting)
		SyncPart();

	// Just replace - do not delete
	mBody = attach;
	mCurrentPart = NULL;
	CAttachment* part = (mBody != NULL) ? mBody->FirstEditPart() : NULL;

	if (part == NULL)
	{
		cdstring txt;
		txt.FromResource("Alerts::Letter::NoTextWarning");
		mText->SetText(txt);
		mText->GetRichEditCtrl().SetReadOnly(true);
	}
	else
		SetCurrentPart(part);
}

// Set entire text with formatting changes
void CLetterWindow::SetText(const char* text, bool plain)
{
	{
		StStopRedraw redraw(mText);

		mText->SetToolbar(mBody->GetContent());
		bool html = ((mBody->GetContent().GetContentSubtype() == eContentSubEnriched) ||
					 (mBody->GetContent().GetContentSubtype() == eContentSubHTML)) &&
								CAdminLock::sAdminLock.mAllowStyledComposition;
		ResetFont(CFontCache::GetListFont(),
					html ? CFontCache::GetHTMLFont() : CFontCache::GetDisplayFont());

		if (text && *text)
		{
			mText->Reset();
			{
				cdustring text_utf16(text);
				mText->GetFormatter()->ParseBody(text_utf16, plain ? eContentSubPlain : mBody->GetContent().GetContentSubtype(), eViewFormatted, -1);
			}
			mText->GetFormatter()->InsertFormatted(html ? eViewFormatted : eViewPlain);
		}
		else
			mText->SetWindowText(_T(""));

		mText->SetSelectionRange(0, 0);
	}
	mText->RedrawWindow();
}

// Reset message text
void CLetterWindow::SetCurrentPart(CAttachment* part)
{
	// Sync current part
	SyncPart();

	// Get new part and copy text into window
	if (mBody)
	{
		StStopRedraw redraw(mText);

		// Must turn off drawing
		long old_mask = mText->GetRichEditCtrl().GetEventMask();

		if (part != NULL)
		{
			mCurrentPart = part;
			const char* data = mCurrentPart->GetData();
			mText->SetToolbar(mCurrentPart->GetContent());
			i18n::ECharsetCode charset = mCurrentPart->GetContent().GetCharset();
			
			// Must turn off notification which may have been turned on by SetToolbar
			old_mask = mText->GetRichEditCtrl().GetEventMask();
			mText->GetRichEditCtrl().SetEventMask(old_mask & ~ENM_SELCHANGE);

			// Do charset conversion to utf16
			cdustring data_utf16;
			std::ostrstream sout;
			if ((data != NULL))
			{
				if ((charset != i18n::eUTF16) && i18n::CCharsetManager::sCharsetManager.ToUTF16(charset, data, ::strlen(data), sout))
				{
					sout << std::ends << std::ends;
					data_utf16.steal(reinterpret_cast<unichar_t*>(sout.str()));
				}
				else
					data_utf16.assign(reinterpret_cast<const unichar_t*>(data));
			}

			const CMIMEContent& mime = mCurrentPart->GetContent();

			if (mime.GetContentType() == eContentText)
			{
				mText->Reset();
				bool html = ((mime.GetContentSubtype() == eContentSubEnriched) || (mime.GetContentSubtype() == eContentSubHTML)) &&
								CAdminLock::sAdminLock.mAllowStyledComposition;
				ResetFont(CFontCache::GetListFont(),
							html ? CFontCache::GetHTMLFont() : CFontCache::GetDisplayFont());
				mText->GetFormatter()->ParseBody(data_utf16, mime.GetContentSubtype(), eViewFormatted, -1);
				mText->GetFormatter()->InsertFormatted(html ? eViewFormatted : eViewPlain);
			}
			else
			{
				mText->SetText(data);
			}
	
			mText->GetRichEditCtrl().SetEventMask(old_mask);
			mText->SetSelectionRange(0, 0);

			// Don't allow edit if reject
			mText->GetRichEditCtrl().SetReadOnly(mReject);
		}
		else
		{
			mCurrentPart = NULL;
			cdstring txt;
			txt.FromResource("Alerts::Letter::NoTextWarning");
			mText->SetText(txt);
			mText->GetRichEditCtrl().SetReadOnly(true);
			mText->HideToolbar();
		}
	}

	mText->GetRichEditCtrl().RedrawWindow();
}

// Sync text in current part
void CLetterWindow::SyncPart()
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		// Restore previous selection after styled text parsing
		CCmdEditView::StPreserveSelection _save_selection(mText);

		char* data = NULL;

		switch(mCurrentPart->GetContent().GetContentSubtype())
		{
		case eContentSubEnriched:
			{
				// Must turn off selection notification
				long old_mask = mText->GetRichEditCtrl().GetEventMask();
				mText->GetRichEditCtrl().SetEventMask(old_mask & ~ENM_SELCHANGE);
				{
					StStopRedraw redraw(mText);

					CEnrichedTransformer trans(mText);
					data = trans.Transform();

					mText->GetRichEditCtrl().SetEventMask(old_mask);
				}
			}
			break;
		case eContentSubHTML:
			{
				// Must turn off selection notification
				long old_mask = mText->GetRichEditCtrl().GetEventMask();
				mText->GetRichEditCtrl().SetEventMask(old_mask & ~ENM_SELCHANGE);
				{
					StStopRedraw redraw(mText);

					CHTMLTransformer trans(mText);
					data = trans.Transform();

					mText->GetRichEditCtrl().SetEventMask(old_mask);
				}
			}
			break;
		default:
			{
				// Fix for curly quotes
				cdustring utemp;
				mText->GetText(utemp);
				
				// Remove any curly quotes
				unichar_t* p = utemp.c_str_mod();
				while(*p)
				{
					if ((*p == 0x201C) || (*p == 0x201D))
						*p = '\"';
					else if ((*p == 0x2018) || (*p == 0x2019))
						*p = '\'';
					p++;
				}
				cdstring temp = utemp.ToUTF8();
				data = temp.release();
			}
			break;
		}

		mCurrentPart->GetContent().SetCharset(i18n::eUTF8);
		mCurrentPart->SetData(data);
	}
}

// Sync text size in current part
void CLetterWindow::SyncPartSize()
{
	// Copy current text to current part
	if (mCurrentPart)
	{
		int length = mText->GetWindowTextLength();
		mCurrentPart->GetContent().SetContentSize(length);
	}
}

// Handle style text popup command
void CLetterWindow::OnChangeTextStyle(UINT nID)
{
	EContentSubType subtype;

	switch(nID)
	{
	case IDM_DRAFT_CURRENT_PLAIN:
	case IDM_DRAFT_CURRENT_PLAIN_WRAPPED:
		subtype = eContentSubPlain;
		break;
	case IDM_DRAFT_CURRENT_ENRICHED:
		subtype = eContentSubEnriched;
		break;
	case IDM_DRAFT_CURRENT_HTML:
		subtype = eContentSubHTML;
		break;
	}

	if ((mCurrentPart != NULL) && (mCurrentPart->GetContent().GetContentType() == eContentText))
	{
		// Do warning if changing from styled to unstyled
		const CMIMEContent& mime = mCurrentPart->GetContent();
		if ((mime.GetContentSubtype() != eContentSubPlain) &&
			(subtype == eContentSubPlain))
		{
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::WarnLoseFormatting") == CErrorHandler::Cancel)
			{
				// Change back to previous value
				switch(mime.GetContentSubtype())
				{
				case eContentSubPlain:
					nID = mText->GetHardWrap() ? IDM_DRAFT_CURRENT_PLAIN_WRAPPED : IDM_DRAFT_CURRENT_PLAIN;
					break;
				case eContentSubEnriched:
					nID = IDM_DRAFT_CURRENT_ENRICHED;
					break;
				case eContentSubHTML:
					nID = IDM_DRAFT_CURRENT_HTML;
					break;
				}
				return;
			}
		}

		// Make sure wrapping correctly set before doing content change
		switch(nID)
		{
		case IDM_DRAFT_CURRENT_PLAIN:
		case IDM_DRAFT_CURRENT_ENRICHED:
		case IDM_DRAFT_CURRENT_HTML:
			mText->SetHardWrap(false);
			break;
		case IDM_DRAFT_CURRENT_PLAIN_WRAPPED:
			mText->SetHardWrap(true);
			break;
		}

		mCurrentPart->GetContent().SetContent(eContentText, subtype);
		mPartsTable.ChangedCurrent();
	}
}

#pragma mark ____________________________Identities

// Handle identities popup command
void CLetterWindow::SetIdentityPopup(unsigned long index)
{
	mHeader->mIdentityPopup.SetValue(IDM_IDENTITYStart + index);
}

// Handle identities popup command
void CLetterWindow::OnChangeIdentity(UINT nID)
{
	switch(nID)
	{
	// New identity wanted
	case IDM_IDENTITY_NEW:
		mHeader->mIdentityPopup.DoNewIdentity(CPreferences::sPrefs);
		break;
	
	// Edit identity
	case IDM_IDENTITY_EDIT:
		if (mHeader->mIdentityPopup.DoEditIdentity(CPreferences::sPrefs))
			// Reset to get display items in sync with changes
			SetIdentity(GetIdentity());
		break;
	
	// Delete existing identity
	case IDM_IDENTITY_DELETE:
		mHeader->mIdentityPopup.DoDeleteIdentity(CPreferences::sPrefs);
		break;
	
	// Set custom identity
	case IDM_IDENTITY_CUSTOM:
		SetIdentity(mHeader->mIdentityPopup.GetCustomIdentity(), true);
		mHeader->mIdentityPopup.SetValue(nID);
		break;
	
	// Select an identity
	default:
		{
			const CIdentity& identity = CPreferences::sPrefs->mIdentities.GetValue()[nID - IDM_IDENTITYStart];
			SetIdentity(&identity);
			mHeader->mIdentityPopup.SetValue(nID);
		}
		break;
	}
	
	// Check value and show/hide custom edit item
	mHeader->mIdentityEditBtn.ShowWindow(mHeader->mIdentityPopup.GetValue() == IDM_IDENTITY_CUSTOM);
}


// Edit custom identity
void CLetterWindow::OnEditCustom()
{
	if (mHeader->mIdentityPopup.EditCustomIdentity())
		SetIdentity(mHeader->mIdentityPopup.GetCustomIdentity(), true);
}

// Change identity
void CLetterWindow::SetIdentity(const CIdentity* identity, bool custom, bool add_signature)
{
	// Preserve focus
	CView* view = GetParentFrame()->GetActiveView();
	CWnd* focus = CWnd::GetFocus();

	// Must have identity
	if (!identity)
		identity = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Cache identity for use in message
	const CIdentity* old_identity = mCustomIdentity ? NULL : CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mIdentity);
	mIdentity = identity->GetIdentity();
	mCustomIdentity = custom;

	// Change over addresses only if not reject
	if (!mReject)
	{
		// First remove existing extra addresses if prvious identity still exists
		if (old_identity)
		{
			cdstring temp;
			temp = old_identity->GetAddTo(true);
			CAddressList to_addrs(temp, temp.length());
			temp = old_identity->GetAddCC(true);
			CAddressList cc_addrs(temp, temp.length());
			temp = old_identity->GetAddBcc(true);
			CAddressList bcc_addrs(temp, temp.length());
			RemoveAddressLists(&to_addrs, &cc_addrs, &bcc_addrs);
		}

		// Add new extra addresses
		{
			cdstring temp;
			temp = identity->GetAddTo(true);
			CAddressList to_addrs(temp, temp.length());
			temp = identity->GetAddCC(true);
			CAddressList cc_addrs(temp, temp.length());
			temp = identity->GetAddBcc(true);
			CAddressList bcc_addrs(temp, temp.length());
			AddAddressLists(&to_addrs, &cc_addrs, &bcc_addrs);
		}
	}

	// Do Copy To
	mHeader->mCopyTo.SetSelectedMbox(identity->GetCopyTo(true),
										identity->GetCopyToNone(true),
										identity->GetCopyToChoose(true));

	// Do Also Copy
	mCopyOriginal = identity->GetCopyReplied(true);

	// Do signature insert
	if (CPreferences::sPrefs->mAutoInsertSignature.GetValue() && !mReject)
	{
		// Preserve selection
		CCmdEditView::StPreserveSelection selection(mText);

		// Get new and old sigs
		cdstring old_sig = mSignature;
		cdstring new_sig = identity->GetSignature(true);

		// May need sigdashes before signature
		CPreferences::sPrefs->SignatureDashes(new_sig);

		// Cache unmodifed version
		cdstring newsig = new_sig;

		// Filter out LFs as RichEdit2.0 does not use \n
		old_sig.ConvertEndl(eEndl_CR);
		new_sig.ConvertEndl(eEndl_CR);

		// Replace signature
		mText->SetSelectionRange(mText->GetTextLengthEx() - mSignatureLength, mText->GetTextLengthEx());
		if (add_signature)
		{
			ReplaceSignature(old_sig, new_sig);
			mText->GetRichEditCtrl().RedrawWindow();
			mSignature = newsig;
		}
		else
		{
			mText->InsertUTF8(cdstring::null_str);
			mSignature = cdstring::null_str;
			mSignatureLength = 0;
		}
	}
	
	// Set sign/encryption buttons
	if (CPluginManager::sPluginManager.HasSecurity() && !mReject)
	{
		// Determine mode and any key
		mDoSign = identity->GetSign(true);
		mDoEncrypt = identity->GetEncrypt(true);
	}
	
	// Set DSN
	mDSN = identity->GetDSN(true);
	if (!mDSN.GetRequest() || mReject)
	{
		mDSN.SetSuccess(false);
		mDSN.SetFailure(false);
		mDSN.SetDelay(false);
		mDSN.SetFull(false);
	}

	// Restore focus
	GetParentFrame()->SetActiveView(view);
	if (focus)
		focus->SetFocus();

	// Force main toolbar to update
	mView.RefreshToolbar();
}

// Insert signature
void CLetterWindow::OnDraftInsertSignature()
{
	InsertSignature(GetIdentity()->GetSignature(true));
}

// Insert signature
void CLetterWindow::InsertSignature(const cdstring& signature)
{
	// Cannot do this in read-only text
	if (mText->GetRichEditCtrl().GetStyle() & ES_READONLY)
		return;

	// Get pos of insert cursor
	long selStart;
	long selEnd;
	mText->GetSelectionRange(selStart, selEnd);

	// Look for preceeding CRs
	cdstring sel_text;
	{
		StStopRedraw redraw(mText);

		mText->SetSelectionRange(selStart - 2 >= 0 ? selStart - 2 : 0, selStart);
		mText->GetSelectedText(sel_text);
		mText->SetSelectionRange(selStart, selEnd);

		// Must strip out \n for sigtemp as RichEdit2.0 does not use them
		sel_text.ConvertEndl(eEndl_CR);
	}

	bool char1 = false;
	bool char2 = false;
	// Get current text and lock while dereferences
	if (selStart > 0)
	{
		if (selStart == 1)
			char1 = (sel_text[(cdstring::size_type)0] == '\r');
		{
			char1 = (sel_text[(cdstring::size_type)1] == '\r');
			char2 = (sel_text[(cdstring::size_type)0] == '\r');
		}
	}

	// Accumulate chages into one operation (for undo)
	cdstring replace_with;

	// Replace existing
	if (!char1 && CPreferences::sPrefs->mSignatureEmptyLine.GetValue())
		replace_with = "\r\r";
	else if (!char1 && !CPreferences::sPrefs->mSignatureEmptyLine.GetValue() ||
			 char1 && !char2 && CPreferences::sPrefs->mSignatureEmptyLine.GetValue())
		replace_with = "\r";

	// May need sigdashes before signature
	cdstring sigtemp(signature);
	CPreferences::sPrefs->SignatureDashes(sigtemp);

	// Must strip out \n for signature as RichEdit2.0 does not use them
	sigtemp.ConvertEndl(eEndl_CR);

	replace_with += sigtemp;

	// Replace text
	mText->InsertUTF8(replace_with);
	mText->GetRichEditCtrl().RedrawWindow();
}

#pragma mark ____________________________Window changes

// Reset toolbar commands
void CLetterWindow::RefreshToolbar()
{
	// Force main toolbar to update
	mView.RefreshToolbar();
}

void CLetterWindow::ResetFont(CFont* list_font, CFont* display_font)
{
	mPartsTitles.ResetFont(list_font);
	mPartsTable.ResetFont(list_font);
	mPartsTitles.SyncTable(&mPartsTable, true);
	mText->ResetFont(display_font);
}

// Temporarily add header summary for printing
void CLetterWindow::AddPrintSummary(const CMessage* msg)
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue())
	{
		// Get summary from envelope
		std::ostrstream hdr;
		msg->GetEnvelope()->GetSummary(hdr);
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
void CLetterWindow::RemovePrintSummary(const CMessage* msg)
{
	// Only if required
	if (CPreferences::sPrefs->mPrintSummary.GetValue())
	{
		// Get summary from envelope
		std::ostrstream hdr;
		msg->GetEnvelope()->GetSummary(hdr);
		hdr << std::ends;

		// Must filter out LFs for RichEdit 2.0
		cdstring header_insert;
		header_insert.steal(hdr.str());
		::FilterOutLFs(header_insert.c_str_mod());

		// Parse as header
		mText->SetSelectionRange(0, header_insert.length());
		mText->InsertUTF8(cdstring::null_str);
	}
}

// Check dirty state
BOOL CLetterWindow::IsDirty()
{
	bool dirty = false;
	dirty = dirty || mHeader->mToField.GetModify();
	dirty = dirty || mHeader->mCCField.GetModify();
	dirty = dirty || mHeader->mBCCField.GetModify();
	dirty = dirty || mHeader->mSubjectField.GetModify();
	dirty = dirty || mPartsTable.IsDirty();
	dirty = dirty || mText->GetRichEditCtrl().GetModify();
	
	return dirty;
}

//	Set dirty state
void CLetterWindow::SetDirty(bool dirty)
{
	// Make sure window items are not dirty
	mHeader->mToField.SetModify(dirty);
	mHeader->mCCField.SetModify(dirty);
	mHeader->mBCCField.SetModify(dirty);
	mHeader->mSubjectField.SetModify(dirty);
	mPartsTable.SetDirty(dirty);
	mText->GetRichEditCtrl().SetModify(dirty);
}

// Save data to file
void CLetterWindow::Serialize(CArchive& ar)
{
	// Check Open or Save
	if (ar.IsLoading())	// Opening
	{
		// Get tos
		CString temp;
		ar.ReadString(temp);
		cdstring field(temp);
		mHeader->mToField.SetText(field.c_str() + ::strlen(cHDR_TO));

		// Get ccs
		ar.ReadString(temp);
		field = temp;
		mHeader->mCCField.SetText(field.c_str() + ::strlen(cHDR_CC));
		if ((field.length() > ::strlen(cHDR_CC)) && !mHeaderState.mCcVisible)
			OnDraftAddCc();

		// Get bccs
		ar.ReadString(temp);
		field = temp;
		mHeader->mBCCField.SetText(field.c_str() + ::strlen(cHDR_BCC));
		if ((field.length() > ::strlen(cHDR_BCC)) && !mHeaderState.mBccVisible)
			OnDraftAddBcc();

		// Get subject
		ar.ReadString(temp);
		field = temp;
		mHeader->mSubjectField.SetText(field.c_str() + ::strlen(cHDR_SUBJECT));

		// Check for files or content
		ar.ReadString(temp);
		field = temp;
		
		if (field.compare_start(cHDR_COPYTO))
		{
			cdstring copyto = &field[sizeof(cHDR_COPYTO) - 1];
			mHeader->mCopyTo.SetSelectedMbox(copyto, false, copyto.length() == 0);

			// Read next line
			ar.ReadString(temp);
			field = temp;
		}

		if (field.compare_start(cHDR_MIME_TYPE))
		{
			mBody->GetContent().SetContent(&field[sizeof(cHDR_MIME_TYPE) - 1]);
			mText->SetToolbar(mBody->GetContent());

			// Get files
			CString files;
			ar.ReadString(files);
			ar.ReadString(temp);
			field = temp;
		}
		else
		{
			// Get files
			CString files;
			ar.ReadString(files);
		}
		
		// Get main text one line at a time
		CString txt_line;
		cdstring txt;
		{
			while(ar.ReadString(txt_line))
			{
				txt += txt_line;
				txt += os_endl;
			}
			SetText(txt);
		}

		ResetState();

	}
	else // Saving
	{
		CAddressList* to_list = NULL;
		CAddressList* cc_list = NULL;
		CAddressList* bcc_list = NULL;
		cdstring	subjectText;
		cdstring	mailText;

		try
		{
			// Get to text
			cdstring addrs;
			mHeader->mToField.GetText(addrs);
			int text_length = addrs.length();
			to_list = new CAddressList(addrs, text_length);

			// Write to archive
			ar.WriteString(cdstring(cHDR_TO).win_str());
			bool first = true;
			for(CAddressList::const_iterator iter = to_list->begin(); iter != to_list->end(); iter++)
			{
				cdstring str = (*iter)->GetFullAddress();
				if (!first)
					ar.WriteString(_T(", "));
				else
					first = false;
				ar.WriteString(str.win_str());
			}
			ar.WriteString(_T("\r\n"));

			// Get cc text
			mHeader->mCCField.GetText(addrs);
			text_length = addrs.length();
			cc_list = new CAddressList(addrs, text_length);

			// Write to archive
			ar.WriteString(cdstring(cHDR_CC).win_str());
			first = true;
			for(CAddressList::const_iterator iter = cc_list->begin(); iter != cc_list->end(); iter++)
			{
				cdstring str = (*iter)->GetFullAddress();
				if (!first)
					ar.WriteString(_T(", "));
				else
					first = false;
				ar.WriteString(str.win_str());
			}
			ar.WriteString(_T("\r\n"));

			// Get bcc text
			mHeader->mBCCField.GetText(addrs);
			text_length = addrs.length();
			bcc_list = new CAddressList(addrs, text_length);

			// Write to archive
			ar.WriteString(cdstring(cHDR_BCC).win_str());
			first = true;
			for(CAddressList::const_iterator iter = bcc_list->begin(); iter != bcc_list->end(); iter++)
			{
				cdstring str = (*iter)->GetFullAddress();
				if (!first)
					ar.WriteString(_T(", "));
				else
					first = false;
				ar.WriteString(str.win_str());
			}
			ar.WriteString(_T("\r\n"));

			// Get subject
			mHeader->mSubjectField.GetText(subjectText);
			ar.WriteString(cdstring(cHDR_SUBJECT).win_str());
			ar.WriteString(subjectText.win_str());
			ar.WriteString(_T("\r\n"));

			// Get copy to
			cdstring copy_to;
			mHeader->mCopyTo.GetSelectedMboxName(copy_to, false);
			ar.WriteString(cdstring(cHDR_COPYTO).win_str());
			ar.WriteString(copy_to.win_str());
			ar.WriteString(_T("\r\n"));

			// Get content type/subtype
			ar.WriteString(cdstring(cHDR_MIME_TYPE).win_str());
			if (mCurrentPart)
			{
				cdstring content = CMIMESupport::GenerateContentHeader(mCurrentPart, false, lendl, false);
				ar.WriteString(content.win_str());
			}
			ar.WriteString(_T("\r\n"));

			// Get files
			ar.WriteString(cdstring(cHDR_FILES).win_str());
			ar.WriteString(_T("\r\n"));
			ar.WriteString(_T("\r\n"));

			// Get mail text & length
			if (mCurrentPart)
			{
				SyncPart();
				mailText = mCurrentPart->GetData();
				ar.WriteString(cdstring(mailText).win_str());
			}
			ar.WriteString(_T("\r\n"));

			delete to_list;
			delete cc_list;
			delete bcc_list;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up
			delete to_list;
			delete cc_list;
			delete bcc_list;

			CLOG_LOGRETHROW;
			throw;
		}
	}
	
	// Always clean
	SetDirty(false);
}

#pragma mark ____________________________________Command Updaters

void CLetterWindow::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CLetterWindow::OnUpdateNotReject(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);	// Not for rejects
}

void CLetterWindow::OnUpdateFileImport(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable((mText == GetFocus()) && !mReject);
	CString txt;
	txt.LoadString(IDS_LETTER_IMPORTTEXT);
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CLetterWindow::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	mUndoer.FindUndoStatus(pCmdUI);
}

void CLetterWindow::OnUpdateEditSpeak(CCmdUI* pCmdUI)
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

void CLetterWindow::OnUpdateEditSpellCheck(CCmdUI* pCmdUI)
{
	// Must have editable part and speller plugin and not reject
	pCmdUI->Enable(mCurrentPart && CPluginManager::sPluginManager.HasSpelling() && !mReject);
}

void CLetterWindow::OnUpdateAllowStyled(CCmdUI* pCmdUI)
{
	// Check for allowed and not reject
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowStyledComposition && !mReject);
}

void CLetterWindow::OnUpdatePasteIndented(CCmdUI* pCmdUI)
{
	// Check for text in clipboard and not reject
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT) && (mText == GetFocus()) && !mReject);
}

void CLetterWindow::OnUpdateDraftInclude(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(mMsgs && mMsgs->size() && mReply);
}

void CLetterWindow::OnUpdateDraftAddCc(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(!mHeaderState.mCcVisible);
}

void CLetterWindow::OnUpdateDraftAddBcc(CCmdUI* pCmdUI)
{
	// Only if in reply
	pCmdUI->Enable(!mHeaderState.mBccVisible);
}

void CLetterWindow::OnUpdateDraftNotReject(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(!mReject);
}

void CLetterWindow::OnUpdateTextStyle(CCmdUI* pCmdUI)
{
	// Only if allowed by admin and not rejecting
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllowStyledComposition && !mReject &&
							(mCurrentPart != NULL) && (mCurrentPart->GetContent().GetContentType() == eContentText));

	if (mCurrentPart != NULL)
	{
		bool marked = false;
		switch(pCmdUI->m_nID)
		{
		case IDM_DRAFT_CURRENT_PLAIN:
			marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && !mText->GetHardWrap();
			break;
		case IDM_DRAFT_CURRENT_PLAIN_WRAPPED:
			marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain) && mText->GetHardWrap();
			break;
		case IDM_DRAFT_CURRENT_ENRICHED:
			marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubEnriched);
			break;
		case IDM_DRAFT_CURRENT_HTML:
			marked = (mCurrentPart->GetContent().GetContentSubtype() == eContentSubHTML);
			break;
		}
		pCmdUI->SetCheck(marked);
	}
}

void CLetterWindow::OnUpdateDraftAttachFile(CCmdUI* pCmdUI)
{
	// Only if not locked out and not reject
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoAttachments && !mReject);
}

void CLetterWindow::OnUpdateDraftSendMail(CCmdUI* pCmdUI)
{
	// Only if in SMTP sender is availble
	const CIdentity* id = GetIdentity();
	pCmdUI->Enable(CConnectionManager::sConnectionManager.IsConnected() ||
					CSMTPAccountManager::sSMTPAccountManager && CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected(*id));
}

void CLetterWindow::OnUpdateDraftAppend(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	CCopyToMenu::ResetMenuList();

	// Do update if not reject
	OnUpdateNotReject(pCmdUI);
}

void CLetterWindow::OnUpdateDraftCopyOriginal(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mCopyOriginalAllowed);
	pCmdUI->SetCheck(mCopyOriginal);
}

void CLetterWindow::OnUpdateDraftSign(CCmdUI* pCmdUI)
{
	// Only if plugin and not reject
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity() && !mReject);
	pCmdUI->SetCheck(mDoSign);
}

void CLetterWindow::OnUpdateDraftEncrypt(CCmdUI* pCmdUI)
{
	// Only if plugin and not reject
	pCmdUI->Enable(CPluginManager::sPluginManager.HasSecurity() && !mReject);
	pCmdUI->SetCheck(mDoEncrypt);
}

void CLetterWindow::OnUpdateDraftMDN(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetMDN());
}

void CLetterWindow::OnUpdateDraftDSNSuccess(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetSuccess());
}

void CLetterWindow::OnUpdateDraftDSNFailure(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetFailure());
}

void CLetterWindow::OnUpdateDraftDSNDelay(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetDelay());
}

void CLetterWindow::OnUpdateDraftDSNFull(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mReject);
	pCmdUI->SetCheck(mDSN.GetFull());
}

void CLetterWindow::OnUpdateMenuExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mHeaderState.mExpanded ? IDS_COLLAPSEHEADER : IDS_EXPANDHEADER);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CLetterWindow::OnUpdateExpandHeader(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
}

void CLetterWindow::OnUpdateWindowsShowParts(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(!mHeader->mPartsTwister.IsPushed() ? IDS_SHOWPARTS : IDS_HIDEPARTS);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

#pragma mark ____________________________________Command Handlers

void CLetterWindow::OnFileNewDraft()
{
	// Pass up to app
	CMulberryApp::sApp->OnAppNewDraft();
}

void CLetterWindow::OnFileSave()
{
	// Have to route this through the view so it gets to the document
	GetDocument()->DoFileSave();
}

// Command handlers
void CLetterWindow::OnFileImport()
{
	// Do standard open dialog
	// prompt the user
	CString filter = _T("Text File (*.txt) | *.txt; *.doc|Anyfile (*.*) | *.*||");
	CFileDialog dlgFile(true, NULL, NULL, OFN_FILEMUSTEXIST, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Reset undo action in edit control
		mText->GetRichEditCtrl().EmptyUndoBuffer();
		
		try
		{
			// Create file object
			CFile import(dlgFile.GetPathName(), CFile::modeRead | CFile::shareDenyNone);

			// Set up buffer
			cdstring buf;
			buf.reserve(8192);
			
			// Get file length
			int flen = import.GetLength();

			// Read in file one buffer at a time
			bool got_cr = false;
			while(flen > 0)
			{
				// Read in cstring
				int rlen = import.Read(buf.c_str_mod(), (flen > 8191 ? 8191 : flen));
				buf[(cdstring::size_type)rlen] = 0;
				
				// Do line end translation
				std::ostrstream out;
				const char* p = buf.c_str();
				while(*p)
				{
					switch(*p)
					{
					case '\r':
						got_cr = true;
						out << os_endl;
						p++;
						break;
					case '\n':
						if (got_cr)
							got_cr = false;
						else
							out << os_endl;
						p++;
						break;
					default:
						got_cr = false;
						out.put(*p++);
						break;
					}
				}
				out << std::ends;

				// Replace current selection with new text and decrement count
				mText->InsertUTF8(out.str());
				out.freeze(false);
				flen -= rlen;
			}
		}
		catch (CFileException* ex)
		{
			CLOG_LOGCATCH(CFileException*);

			GetDocument()->ReportSaveLoadException(dlgFile.GetPathName(), ex,
													FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
	}
}

void CLetterWindow::OnFilePrint()
{
	// Print the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT);
}

void CLetterWindow::OnFilePrintPreview()
{
	// Print preview the message on view
	mText->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
}

void CLetterWindow::OnEditUndo()
{
	mUndoer.ToggleAction();
}

void CLetterWindow::OnPasteIndented()
{
	if (!::OpenClipboard(*this))
		return;
	HANDLE hglb = ::GetClipboardData(CF_UNICODETEXT);
	if (hglb)
	{
		LPWSTR lptstr = (LPWSTR) ::GlobalLock(hglb);
		
		// Reset undo action in edit control
		mText->GetRichEditCtrl().EmptyUndoBuffer();
		
		// Now add text
		IncludeText(cdstring(lptstr), false);
		::GlobalUnlock(hglb);
	}
	::CloseClipboard();
}

void CLetterWindow::OnEditSpeak()
{
	if (!CSpeechSynthesis::OnEditSpeak(IDM_EDIT_SPEAK))
		SpeakMessage();
}

void CLetterWindow::OnEditSpellCheck()
{
	DoSpellCheck(false);
}

void CLetterWindow::OnDraftAttachFile()
{
	// Only if not locked out
	if (!CAdminLock::sAdminLock.mNoAttachments)
	{
		mPartsTable.OnDraftAttachFile();
		UpdatePartsCaption();
	}
}

// Include the replied to mail
void CLetterWindow::OnDraftInclude()
{
	IncludeFromReply(false, false);
}

void CLetterWindow::OnDraftNewPlainPart()
{
	mPartsTable.OnDraftNewPlainPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftNewEnrichedPart()
{
	mPartsTable.OnDraftNewEnrichedPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftNewHTMLPart()
{
	mPartsTable.OnDraftNewHTMLPart();
	//mText->SetToolbar();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartMixed()
{
	mPartsTable.OnDraftMultipartMixed();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartParallel()
{
	mPartsTable.OnDraftMultipartParallel();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartDigest()
{
	mPartsTable.OnDraftMultipartDigest();
	UpdatePartsCaption();
}

void CLetterWindow::OnDraftMultipartAlternative()
{
	mPartsTable.OnDraftMultipartAlternative();
	UpdatePartsCaption();
}

// Expand/collapse to full size header
void CLetterWindow::OnLetterHeaderTwister()
{
	// Expand and layout the header
	mHeaderState.mExpanded = !mHeaderState.mExpanded;

	// Update recipient text if that is being shown
	if (!mHeaderState.mExpanded)
		UpdateRecipients();

	LayoutHeader();
}

void CLetterWindow::OnLetterToTwister()
{
	// Toggle state
	mHeaderState.mToExpanded = mHeader->mToTwister.IsPushed();
	DoTwist(&mHeader->mToMove, &mHeader->mToField, &mHeader->mToTwister);
}

void CLetterWindow::OnLetterCCTwister()
{
	// Toggle state
	mHeaderState.mCcExpanded = mHeader->mCCTwister.IsPushed();
	DoTwist(&mHeader->mCCMove, &mHeader->mCCField, &mHeader->mCCTwister);
}

void CLetterWindow::OnLetterBCCTwister()
{
	// Toggle state
	mHeaderState.mBccExpanded = mHeader->mBCCTwister.IsPushed();
	DoTwist(&mHeader->mBCCMove, &mHeader->mBCCField, &mHeader->mBCCTwister);
}

// Handle twist
void CLetterWindow::DoTwist(CWnd* aMover, CCmdEdit* aField, CTwister* aTwister)
{
	if (aTwister->IsPushed())
	{
		int moveby = -cLtrWndTwistMove;
		aField->ModifyStyle(WS_VSCROLL, ES_AUTOHSCROLL);
		aField->SetSel(0, 0);
		aField->SetFocus();
		aTwister->SetPushed(false);
		::ResizeWindowBy(aMover, 0, moveby, false);
	}
	else
	{
		int moveby = cLtrWndTwistMove;
		aField->ModifyStyle(ES_AUTOHSCROLL, WS_VSCROLL);
		aField->SetSel(0, 0);
		aField->SetFocus();
		aTwister->SetPushed(true);
		::ResizeWindowBy(aMover, 0, moveby, false);
	}

	// Do header layout after change
	LayoutHeader();
}

// Add Cc button clicked
void CLetterWindow::OnDraftAddCc()
{
	// Make the Cc header visible
	mHeaderState.mCcVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();

	// Focus on the CC field
	mHeader->mCCField.SetFocus();

	// Force main toolbar to update
	RefreshToolbar();
}

// Add Bcc button clicked
void CLetterWindow::OnDraftAddBcc()
{
	// Make the Cc header visible
	mHeaderState.mBccVisible = true;
	
	// Layout the header if not collapsed
	if (mHeaderState.mExpanded)
		LayoutHeader();
	
	// Focus on the BCC field
	mHeader->mBCCField.SetFocus();

	// Force main toolbar to update
	RefreshToolbar();
}

// Layout header component based on state
void CLetterWindow::LayoutHeader()
{
	// Start with some initial offset
	int top = 0;
	
	// Get the from area position and size which is always visible
	// Get current position
	CRect size;
	mHeader->mHeaderMove.GetWindowRect(&size);
	mHeader->mHeaderMove.GetParent()->ScreenToClient(&size);

	// Move current top to bottom of from area
	top += size.Height();

	// Expanded => show address fields
	if (mHeaderState.mExpanded)
	{
		// Hide recipients
		mHeader->mRecipientMove.ShowWindow(SW_HIDE);
		
		// Show from (if more than one identity)
		// NB We need to determine numner of identities from the identity popup, because during a preference
		// change, the sPrefs may actually point to the old set, not the new one we need for this
		if (mHeader->mIdentityPopup.GetCount() > 1)
		{
			mHeader->mFromMove.ShowWindow(SW_SHOW);
		}
		else
		{
			mHeader->mFromMove.ShowWindow(SW_HIDE);
			top -= size.Height();
		}

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

		// Check for Bcc field
		if (mHeaderState.mBccVisible)
		{
			// Get current size
			CRect size;
			mHeader->mBCCMove.GetWindowRect(&size);
			mHeader->mBCCMove.GetParent()->ScreenToClient(&size);

			// Move to current top and increment top by height
			::MoveWindowBy(&mHeader->mBCCMove, 0, top - size.top, false);
			top += size.Height();

			// Always show it
			mHeader->mBCCMove.ShowWindow(SW_SHOW);
		}
		else
			// Always hide it
			mHeader->mBCCMove.ShowWindow(SW_HIDE);

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
		// Hide from show recipients
		mHeader->mFromMove.ShowWindow(SW_HIDE);
		mHeader->mRecipientMove.ShowWindow(SW_SHOW);

		// Hide all addresses and subject
		mHeader->mToMove.ShowWindow(SW_HIDE);
		mHeader->mCCMove.ShowWindow(SW_HIDE);
		mHeader->mBCCMove.ShowWindow(SW_HIDE);
		mHeader->mSubjectMove.ShowWindow(SW_HIDE);
	}
	
	// Get current size
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

// Update recipients caption
void CLetterWindow::UpdateRecipients()
{
	// Get the address lists
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	
	GetAddressLists(to_list, cc_list, bcc_list);
	
	// Get the first one
	const CAddress* first_addr = NULL;
	if (to_list->size() != 0)
	{
		first_addr = to_list->front();
	}
	else if (cc_list->size() != 0)
	{
		first_addr = cc_list->front();
	}
	else if (bcc_list->size() != 0)
	{
		first_addr = bcc_list->front();
	}
	
	// Get total size
	unsigned long addrs = to_list->size() + cc_list->size() + bcc_list->size();

	// Create appropriate string
	cdstring txt;
	if (addrs == 0)
		txt.FromResource("UI::Letter::NoRecipients");
	else
	{
		txt += first_addr->GetFullAddress();
		if (addrs > 1)
		{
			cdstring temp;
			temp.FromResource("UI::Letter::AdditionalRecipients");
			temp.Substitute(addrs - 1);
			txt += temp;
		}
	}
	delete to_list;
	delete cc_list;
	delete bcc_list;
	
	// Do recipient text
	CUnicodeUtils::SetWindowTextUTF8(&mHeader->mRecipientText, txt);

}

// Resort addresses
void CLetterWindow::OnLetterCopyToPopup(UINT nID)
{
	mHeader->mCopyTo.SetValue(nID);
	mHeader->mCopyTo.GetSelectedMboxName(sLastAppendTo, false);

	// Disable buttons
	CMbox* mbox = NULL;
	mHeader->mCopyTo.GetSelectedMbox(mbox, false);
	if (mbox == (CMbox*) -1)
	{
		mCopyOriginalAllowed = false;
	}
	else
	{
		// Stop appending replied to same mailbox
		if (mMsgs && mMsgs->size() && (mReply || mForward || mBounce))
		{
			if ((mMsgs->front()->GetMbox() == mbox) || mMsgs->front()->IsSubMessage())
			{
				mCopyOriginalAllowed = false;
			}
			else
			{
				mCopyOriginalAllowed = true;
			}
		}
		else
		{
			mCopyOriginalAllowed = false;
		}
	}
}

bool CLetterWindow::IsPartsTwist() const
{
	return mHeader->mPartsTwister.IsPushed();
}

void CLetterWindow::OnLetterPartsTwister()
{
	bool expanding = !mHeader->mPartsTwister.IsPushed();
	DoPartsTwist(expanding, true);
}

void CLetterWindow::DoPartsTwist(bool expand, bool change_focus)
{
	CRect parts_size;
	mPartsPane.GetWindowRect(parts_size);
	int moveby = parts_size.Height();

	// Determine motion size and hide/show
	if (!expand)
	{
		// Turn off resize of parts before collapsing
		mPartsTableAlign->SetAlignment(true, true, true, false);

		// Hide the parts table pane in the splitter
		mSplitterView.ShowView(false, true);

		// Change twister
		mPartsPane.ShowWindow(SW_HIDE);
		mHeader->mPartsTwister.SetPushed(false);

		// Focus on text if requested
		if (change_focus)
		{
			GetParentFrame()->SetActiveView(mText);
			mText->SetFocus();
		}
	}
	else
	{
		// Show the parts table pane in the splitter
		mSplitterView.ShowView(true, true);

		// Change twister
		mPartsPane.ShowWindow(SW_SHOW);
		mHeader->mPartsTwister.SetPushed(true);
		
		// Turn on resize of parts after expanding
		mPartsTableAlign->SetAlignment(true, true, true, true);
		
		// Focus on parts if requested
		if (change_focus)
		{
			GetParentFrame()->SetActiveView(mHeader);
			mPartsTable.SetFocus();
		}
	}

	RedrawWindow();
}

void CLetterWindow::OnChangeSubject()
{
	// Only if not specified
	if (GetDocument()->GetPathName().IsEmpty())
	{
		// Cache original doc title in case subject is deleted
		if (mOriginalTitle.empty())
			mOriginalTitle = GetDocument()->GetTitle();

		cdstring subjectText;
		mHeader->mSubjectField.GetText(subjectText);

		// Check for empty subject and use original title
		if (subjectText.empty())
			subjectText = mOriginalTitle;

		GetDocument()->SetTitle(subjectText.win_str());
	}
}

CMailboxPopup* CLetterWindow::GetAppendPopup()
{
	return &mHeader->mCopyTo;
}

CFileTable* CLetterWindow::GetPartsTable()
{
	return &mPartsTable;
}

void CLetterWindow::ShowSentIcon()
{
	mHeader->mSentIcon.ShowWindow(SW_SHOW);
}

bool CLetterWindow::DoAppendReplied() const
{
	return mCopyOriginalAllowed && mCopyOriginal;
}

bool CLetterWindow::IsSigned() const
{
	return mDoSign;
}

bool CLetterWindow::IsEncrypted() const
{
	return mDoEncrypt;
}

// Reset state from prefs
void CLetterWindow::ResetState()
{
	// Check for available state
	CLetterWindowState& state = CPreferences::sPrefs->mLetterWindowDefault.Value();

	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Do not set if empty
	CRect set_rect = state.GetBestRect(CPreferences::sPrefs->mLetterWindowDefault.GetValue());
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
		OnLetterPartsTwister();
	mHeaderState.mExpanded = !state.GetCollapsed();

	// Force layout
	LayoutHeader();
	mSplitterView.SetPixelSplitPos(state.GetSplitChange());

	// Do zoom
	if (state.GetState() == eWindowStateMax)
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	//if (!GetParentFrame()->IsWindowVisible())
	//	GetParentFrame()->ActivateFrame();
	//GetParentFrame()->RedrawWindow();
	
	// Redo wrap here to allow for first time update
	mText->SetHardWrap(!mText->GetHardWrap());
	mText->SetHardWrap(!mText->GetHardWrap());

}

// Save current state as default
void CLetterWindow::OnSaveDefaultState()
{
	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CLetterWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mSplitterView.GetPixelSplitPos(), mHeader->mPartsTwister.IsPushed(), !mHeaderState.mExpanded);
	if (CPreferences::sPrefs->mLetterWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mLetterWindowDefault.SetDirty();

}

// Reset to default state
void CLetterWindow::OnResetDefaultState()
{
	ResetState();
}

void CLetterWindow::OnUpdateStyle(CCmdUI* pCmdUI)
{
	mText->GetToolbar()->OnUpdateStyle(pCmdUI);
}

void CLetterWindow::OnUpdateAlignment(CCmdUI* pCmdUI)
{
	mText->GetToolbar()->OnUpdateAlignment(pCmdUI);
}

void CLetterWindow::OnUpdateFont(CCmdUI* pCmdUI)
{
	mText->GetToolbar()->OnUpdateFont(pCmdUI);
}

void CLetterWindow::OnUpdateFontSize(CCmdUI* pCmdUI)
{
	mText->GetToolbar()->OnUpdateSize(pCmdUI);
}

void CLetterWindow::OnUpdateColor(CCmdUI* pCmdUI)
{
	mText->GetToolbar()->OnUpdateColor(pCmdUI);
}

void CLetterWindow::OnStyle(UINT nID)
{
	mText->GetToolbar()->OnStyle(nID);
}

void CLetterWindow::OnAlignment(UINT nID)
{
	mText->GetToolbar()->OnAlignment(nID);
}

void CLetterWindow::OnFont(UINT nID)
{
	mText->GetToolbar()->OnFont(nID);
}

void CLetterWindow::OnFontSize(UINT nID)
{	
	mText->GetToolbar()->OnFontSize(nID);
}

void CLetterWindow::OnColor(UINT nID)
{	
	mText->GetToolbar()->OnColor(nID);
}

