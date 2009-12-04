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


// CSearchWindow.cp : implementation of the CSearchWindow class
//

#include "CSearchWindow.h"

#include "CGetStringDialog.h"
#include "CIconTextTable.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CSearchCriteria.h"
#include "CSearchCriteriaContainer.h"
#include "CSearchEngine.h"
#include "CSearchItem.h"
#include "CSearchOptionsDialog.h"
#include "CTextListChoice.h"
#include "CUnicodeUtils.h"
#include "CWindowStates.h"
#include "CXStringResources.h"

const int cWindowWidth = 512;
const int cWindowHeight = 300;

const int cHeaderHeight = cWindowWidth;
const int cBtnTop = 4;
const int cBtnStart = 4;
const int cBtnOffset = 44;
const int cCaptionHeight = 16;
const int cEditHeight = 22;

const int cSearchStyleWidth = 24;
const int cSearchStyleHeight = 16;
const int cSearchStyleHOffset = 40;
const int cSearchStyleVOffset = 10;
const int cSearchStyleTitleWidth = 64;
const int cSearchStyleTitleHOffset = cSearchStyleHOffset + cSearchStyleTitleWidth;
const int cSearchStyleTitleVOffset = cSearchStyleVOffset + 2;

const int cScrollerHOffset = 2;
const int cScrollerVOffset = cSearchStyleVOffset + 22;
const int cScrollerWidth = cWindowWidth - 2 * cScrollerHOffset;
const int cScrollerHeight = 28;
const int cScrollbarInset = 20;

const int cCriteriaHOffset = 4;
const int cCriteriaVOffset = 2;
const int cCriteriaWidth = cScrollerWidth - 2 * cCriteriaHOffset - cScrollbarInset;
const int cCriteriaHeight = 16;

const int cBottomAreaVOffset = cScrollerVOffset + cScrollerHeight;

const int cMoreBtnWidth = 80;
const int cMoreBtnHeight = 24;
const int cMoreBtnHOffset = 8;
const int cMoreBtnVOffset = 4;

const int cFewerBtnWidth = 80;
const int cFewerBtnHeight = 24;
const int cFewerBtnHOffset = cMoreBtnHOffset + cMoreBtnWidth + 8;
const int cFewerBtnVOffset = cMoreBtnVOffset;

const int cSearchBtnWidth = 50;
const int cSearchBtnHeight = 24;
const int cSearchBtnHOffset = cMoreBtnHOffset + cSearchBtnWidth;
const int cSearchBtnVOffset = cMoreBtnVOffset;

const int cClearBtnWidth = 50;
const int cClearBtnHeight = 24;
const int cClearBtnHOffset = cSearchBtnHOffset + cClearBtnWidth + 8;
const int cClearBtnVOffset = cMoreBtnVOffset;

const int cMailboxListPanelHOffset = 0;
const int cMailboxListPanelVOffset = cMoreBtnVOffset + 28;
const int cMailboxListPanelWidth = cWindowWidth;
const int cMailboxListPanelHeight = cWindowHeight - cBottomAreaVOffset - cMailboxListPanelVOffset;

/////////////////////////////////////////////////////////////////////////////
// CSearchWindow

IMPLEMENT_DYNCREATE(CSearchWindow, CView)

BEGIN_MESSAGE_MAP(CSearchWindow, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)

	ON_COMMAND_RANGE(IDM_SEARCH_STYLES_SAVEAS, IDM_SEARCH_STYLES_End, OnStyles)
	ON_COMMAND(IDC_SEARCH_MORE, OnMoreBtn)
	ON_COMMAND(IDC_SEARCH_FEWER, OnFewerBtn)
	ON_COMMAND(IDC_SEARCH_CLEAR, OnClearBtn)
	ON_COMMAND(IDC_SEARCH_SEARCH, OnSearch)
	ON_COMMAND(IDC_SEARCH_CANCEL, OnCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchWindow construction/destruction

CSearchWindow* CSearchWindow::sSearchWindow = NULL;
CMultiDocTemplate* CSearchWindow::sSearchDocTemplate = NULL;

enum
{
	eStyle_SaveAs = IDM_SEARCH_STYLES_SAVEAS,
	eStyle_Delete,
	eStyle_Separator1,
	eStyle_First
};

CSearchWindow::CSearchWindow() :
	CSearchBase(false)
{
	sSearchWindow = this;
	mScrollerImageHeight = 0;
}

CSearchWindow::~CSearchWindow()
{
	sSearchWindow = NULL;
}

// Manually create document
CSearchWindow* CSearchWindow::ManualCreate(void)
{
	CDocument* aDoc = sSearchDocTemplate->OpenDocumentFile(NULL, false);
	CString strDocName;
	sSearchDocTemplate->GetDocString(strDocName, CDocTemplate::docName);
	aDoc->SetTitle(strDocName);

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSearchWindow)))
		return (CSearchWindow*) pWnd;

	return NULL;
}

int CSearchWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	GetParentFrame()->ModifyStyle(WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 0);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	//const int cPopupHeight1 = cPopupHeight + small_offset;
	//const int cPopupWidthExtra = 4*small_offset;

	const int cWindowHeight1 = cWindowHeight + large_offset;

	// Set initial size for control repositioner
	lpCreateStruct->cx = cWindowWidth;
	lpCreateStruct->cy = cWindowHeight1;
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	// Create header pane
	mHeader.CreateEx(WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cWindowWidth, cWindowHeight1, GetSafeHwnd(), (HMENU)IDC_STATIC);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_WidthHeight));

	// Search styles popup
	CString s;
	s.LoadString(IDS_SEARCH_STYLESTITLE);
	CRect r = CRect(cWindowWidth - cSearchStyleTitleHOffset - 2*large_offset, cSearchStyleTitleVOffset - small_offset, cWindowWidth - cSearchStyleTitleHOffset + cSearchStyleTitleWidth +  2*large_offset, cSearchStyleTitleVOffset + cCaptionHeight);
	mSearchStylesTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mSearchStylesTitle.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mSearchStylesTitle, CWndAlignment::eAlign_TopRight));

	r = CRect(cWindowWidth - cSearchStyleHOffset, cSearchStyleVOffset, cWindowWidth - cSearchStyleHOffset + cSearchStyleWidth, cSearchStyleVOffset + cSearchStyleHeight);
	mSearchStyles.Create(_T(""), r, &mHeader, IDC_SEARCH_STYLESPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mSearchStyles.SetMenu(IDR_POPUP_SEARCH_STYLES);
	mSearchStyles.SetButtonText(false);
	mSearchStyles.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mSearchStyles, CWndAlignment::eAlign_TopRight));
	
	r = CRect(cScrollerHOffset, cScrollerVOffset, cScrollerHOffset + cScrollerWidth, cScrollerVOffset + cScrollerHeight + large_offset);
	mScroller.CreateEx(WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT, NULL, _T(""), WS_CHILD | WS_VISIBLE| WS_BORDER | WS_VSCROLL, r, &mHeader, IDC_STATIC);
	mScroller.GetClientRect(r);
	mScrollerImageHeight = r.Height();
	mScroller.ResizeImage(0, mScrollerImageHeight);
	mHeader.AddAlignment(new CWndAlignment(&mScroller, CWndAlignment::eAlign_WidthHeight));

	s.LoadString(IDS_SEARCH_CRITERIATITLE);
	r = CRect(cCriteriaHOffset, cCriteriaVOffset, cCriteriaHOffset + cCriteriaWidth - small_offset, cCriteriaVOffset + cCriteriaHeight + large_offset);
	mCriteria.CreateEx(WS_EX_CONTROLPARENT, _T("BUTTON"), s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, &mScroller, IDC_STATIC);
	mCriteria.SetFont(CMulberryApp::sAppFont);
	mScroller.AddAlignment(new CWndAlignment(&mCriteria, CWndAlignment::eAlign_TopWidth));

	r = CRect(0, cBottomAreaVOffset + large_offset, cWindowWidth, cWindowHeight1);
	mBottomArea.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mBottomArea, CWndAlignment::eAlign_BottomWidth));

	s.LoadString(IDS_SEARCH_MOREBTN);
	r = CRect(cMoreBtnHOffset, cMoreBtnVOffset, cMoreBtnHOffset + cMoreBtnWidth + 2*large_offset, cMoreBtnVOffset + cMoreBtnHeight);
	mMoreBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mBottomArea, IDC_SEARCH_MORE);
	mMoreBtn.SetFont(CMulberryApp::sAppFont);
	mBottomArea.AddAlignment(new CWndAlignment(&mMoreBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_FEWERBTN);
	r = CRect(cFewerBtnHOffset + 2*large_offset, cFewerBtnVOffset, cFewerBtnHOffset + cFewerBtnWidth + 4*large_offset, cFewerBtnVOffset + cFewerBtnHeight);
	mFewerBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mBottomArea, IDC_SEARCH_FEWER);
	mFewerBtn.SetFont(CMulberryApp::sAppFont);
	mBottomArea.AddAlignment(new CWndAlignment(&mFewerBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_CLEARBTN);
	r = CRect(cWindowWidth - cClearBtnHOffset - 2*large_offset, cClearBtnVOffset, cWindowWidth - cClearBtnHOffset + cClearBtnWidth - large_offset, cClearBtnVOffset + cClearBtnHeight);
	mClearBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mBottomArea, IDC_SEARCH_CLEAR);
	mClearBtn.SetFont(CMulberryApp::sAppFont);
	mBottomArea.AddAlignment(new CWndAlignment(&mClearBtn, CWndAlignment::eAlign_TopRight));

	s.LoadString(IDS_SEARCH_SEARCHBTN);
	r = CRect(cWindowWidth - cSearchBtnHOffset - large_offset, cSearchBtnVOffset, cWindowWidth - cSearchBtnHOffset + cSearchBtnWidth, cSearchBtnVOffset + cSearchBtnHeight);
	mSearchBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, r, &mBottomArea, IDC_SEARCH_SEARCH);
	mSearchBtn.SetFont(CMulberryApp::sAppFont);
	mBottomArea.AddAlignment(new CWndAlignment(&mSearchBtn, CWndAlignment::eAlign_TopRight));

	s.LoadString(IDS_SEARCH_CANCELBTN);
	r = CRect(cWindowWidth - cSearchBtnHOffset - large_offset, cSearchBtnVOffset, cWindowWidth - cSearchBtnHOffset + cSearchBtnWidth, cSearchBtnVOffset + cSearchBtnHeight);
	mCancelBtn.Create(s, WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, r, &mBottomArea, IDC_SEARCH_CANCEL);
	mCancelBtn.SetFont(CMulberryApp::sAppFont);
	mBottomArea.AddAlignment(new CWndAlignment(&mCancelBtn, CWndAlignment::eAlign_TopRight));

	// Mailbox list panel
	r = CRect(cMailboxListPanelHOffset, cMailboxListPanelVOffset, cMailboxListPanelHOffset + cMailboxListPanelWidth, cMailboxListPanelVOffset + cMailboxListPanelHeight);
	mMailboxListPanel.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mBottomArea, IDC_STATIC);
	mMailboxListPanel.CreateSelf(GetParentFrame(), &mScroller, &mBottomArea, cMailboxListPanelWidth, cMailboxListPanelHeight);
	mBottomArea.AddAlignment(new CWndAlignment(&mMailboxListPanel, CWndAlignment::eAlign_TopWidth));

	// Make sure changes to cabinets are notified
	CMailAccountManager::sMailAccountManager->Add_Listener(this);

	return 0;
}

void CSearchWindow::OnDestroy(void)
{
	// Save state
	OnSaveDefaultState();

	// Do default action now
	CView::OnDestroy();
}

// Resize sub-views
void CSearchWindow::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CView::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
	if (mScrollerImageHeight != 0)
		mScroller.ResizeImage(0, mScrollerImageHeight, true);
}

// Get child windows
void CSearchWindow::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	// Focus on search criteria
	if (bActivate && GetParentFrame()->IsWindowVisible())
		SelectNextCriteria(NULL);
}

// Handle key down
void CSearchWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		OnSearch();
		break;
	default:
		CView::OnChar(nChar, nRepCnt, nFlags);
	}
}


#pragma mark ____________________________Statics
// Create it or bring it to the front
void CSearchWindow::CreateSearchWindow(CSearchItem* spec)
{
	CSearchEngine::sSearchEngine.ClearTargets();

	// Create find & replace window or bring to front
	if (sSearchWindow)
	{
		if (spec)
			sSearchWindow->SetStyle(spec);
		FRAMEWORK_WINDOW_TO_TOP(sSearchWindow)
	}
	else
	{
		CSearchWindow* search = CSearchWindow::ManualCreate();
		search->ResetState();
		if (spec)
			search->SetStyle(spec);
		search->GetParentFrame()->ShowWindow(SW_SHOW);
	}
}

void CSearchWindow::DestroySearchWindow()
{
	FRAMEWORK_DELETE_WINDOW(sSearchWindow)
	sSearchWindow = NULL;
}

// Add a mailbox
void CSearchWindow::AddMbox(const CMbox* mbox, bool reset)
{
	// Only add unique
	if (CSearchEngine::sSearchEngine.TargetsAddMbox(mbox))
	{
		// Reset visual display
		if (reset && sSearchWindow)
			sSearchWindow->mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
																CSearchEngine::sSearchEngine.GetTargetHits());
	}
}

// Add a list of mailboxes
void CSearchWindow::AddMboxList(const CMboxList* list)
{
	for(CMboxList::const_iterator iter = list->begin(); iter != list->end(); iter++)
		AddMbox(static_cast<const CMbox*>(*iter), false);
	
	// Reset visual display
	if (sSearchWindow)
		sSearchWindow->mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
																CSearchEngine::sSearchEngine.GetTargetHits());
}

void CSearchWindow::SearchAgain(CMbox* mbox)
{
	CSearchEngine::sSearchEngine.ClearTargets();
	CSearchWindow::AddMbox(mbox);
	
	// Fire off the search engine thread!
	CSearchEngine::sSearchEngine.StartSearch();
}

void CSearchWindow::SearchAgain(CMboxList* list)
{
	CSearchEngine::sSearchEngine.ClearTargets();
	CSearchWindow::AddMboxList(list);
}

void CSearchWindow::StartSearch()
{
	// Clear icons in list and set to top
	STableCell aCell(1, 1);
	mMailboxListPanel.mMailboxList.ScrollCellIntoFrame(aCell);
	mMailboxListPanel.mMailboxList.SetAllIcons(0);
	cdstring num_txt = 0L;
	CUnicodeUtils::SetWindowTextUTF8(&mMailboxListPanel.mFound, num_txt);
	CUnicodeUtils::SetWindowTextUTF8(&mMailboxListPanel.mMessages, num_txt);
	mMailboxListPanel.mProgress.SetCount(0);
	mMailboxListPanel.mProgress.SetTotal(CSearchEngine::sSearchEngine.TargetSize());
	mMailboxListPanel.mProgress.ShowWindow(SW_SHOW);

	// Lock out UI
	SearchInProgress(true);
}

void CSearchWindow::NextSearch(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel.mMailboxList.SetIcon(item, IDI_SEARCH_POINTER);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel.mMailboxList.ScrollCellIntoFrame(aCell);
}

void CSearchWindow::EndSearch()
{
	// Set to top
	STableCell aCell(CSearchEngine::sSearchEngine.GetCurrentTarget() >= 0 ?
						CSearchEngine::sSearchEngine.GetCurrentTarget() + 1 : 1, 1);
	mMailboxListPanel.mMailboxList.ScrollCellIntoFrame(aCell);
		
	mMailboxListPanel.mProgress.ShowWindow(SW_HIDE);
	mMailboxListPanel.mMailboxArea.RedrawWindow();	// CProgressCtrl does not refresh when hidden! Have to do this instead

	// Enable UI
	SearchInProgress(false);
	
	// Do open first if required
	if (CPreferences::sPrefs->mOpenFirstSearchResult.GetValue())
		mMailboxListPanel.mMailboxList.SelectCell(aCell);
}

void CSearchWindow::SetProgress(unsigned long progress)
{
	mMailboxListPanel.SetProgress(progress);
}

void CSearchWindow::SetFound(unsigned long found)
{
	mMailboxListPanel.SetFound(found);
}

void CSearchWindow::SetMessages(unsigned long msgs)
{
	mMailboxListPanel.SetMessages(msgs);
}

void CSearchWindow::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxListPanel.SetHitState(item, hit, clear);
}

bool CSearchWindow::CloseAction(void)
{
	// Cancel and wait for search termination
	if (mMailboxListPanel.mInProgress)
	{
		// Begin a busy operation
		StMailBusy busy_lock(NULL, NULL);

		OnCancel();
		while(!CSearchEngine::sSearchEngine.AbortCompleted())
			CMailControl::ProcessBusy(NULL);
	}
	
	return true;
}

#pragma mark ____________________________Visual Commands

// Respond to list changes
void CSearchWindow::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMboxProtocol::eBroadcast_NewList:
		// Force reset
		mMailboxListPanel.InitCabinets();
		break;

	case CMboxProtocol::eBroadcast_RemoveList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
		{
			unsigned long index = CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(param));
			mMailboxListPanel.mCabinets.GetPopupMenu()->RemoveMenu(index, MF_BYPOSITION);
		}
		break;
	default:
		CSearchBase::ListenTo_Message(msg, param);
		break;
	}
}

#pragma mark ____________________________Commands

void CSearchWindow::SearchInProgress(bool searching)
{
	mSearchStyles.EnableWindow(!searching);
	mCriteria.EnableWindow(!searching);
	mMoreBtn.EnableWindow(!searching);
	mFewerBtn.EnableWindow(!searching);
	mClearBtn.EnableWindow(!searching);
	mSearchBtn.EnableWindow(!searching);
	mSearchBtn.ShowWindow(!searching ? SW_SHOW : SW_HIDE);
	mCancelBtn.ShowWindow(searching ? SW_SHOW : SW_HIDE);
	mCancelBtn.EnableWindow(searching);

	mMailboxListPanel.SetInProgress(searching);
}

void CSearchWindow::OnStyles(UINT nID)
{
	switch(nID)
	{
	case eStyle_SaveAs:
		SaveStyleAs();
		break;
	case eStyle_Delete:
		DeleteStyle();
		break;
	default:
		SetStyle(CPreferences::sPrefs->mSearchStyles.GetValue().at(nID - eStyle_First)->GetSearchItem());
	}
}

void CSearchWindow::OnMoreBtn()
{
	OnMore();
}

void CSearchWindow::OnFewerBtn()
{
	OnFewer();
}

void CSearchWindow::OnClearBtn()
{
	OnClear();
}

void CSearchWindow::OnSearch()
{
	CSearchItem* spec = ConstructSearch();
	CSearchEngine::sSearchEngine.SetSearchItem(spec);
	delete spec;
	
	// Fire off the search engine thread!
	CSearchEngine::sSearchEngine.StartSearch();
}

void CSearchWindow::OnCancel()
{
	// Abort search in progress
	CSearchEngine::sSearchEngine.Abort();
}

BOOL CSearchWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try focused child window
	CWnd* focus = GetFocus();
	if (focus && (typeid(*focus) != typeid(CGrayBackground)))
		if (IsChild(focus) && focus->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;

	// Ask parent to handle
	//if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	//	return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

#pragma mark ____________________________Styles

void CSearchWindow::InitStyles()
{
	// Remove any existing items from main menu
	short num_menu = mSearchStyles.GetPopupMenu()->GetMenuItemCount();
	for(short i = eStyle_First - eStyle_SaveAs; i < num_menu; i++)
		mSearchStyles.GetPopupMenu()->RemoveMenu(eStyle_First - eStyle_SaveAs, MF_BYPOSITION);

	short menu_id = eStyle_First;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(mSearchStyles.GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());
}

void CSearchWindow::SaveStyleAs()
{
	// Get a new name for the mailbox (use old name as starter)
	cdstring style_name;
	if (CGetStringDialog::PoseDialog("Alerts::Search::SaveStyleAsTitle", "Alerts::Search::SaveStyleAs", style_name))
	{
		// Create new style
		CSearchStyle* style = new CSearchStyle(style_name, ConstructSearch());
		CPreferences::sPrefs->mSearchStyles.Value().push_back(style);
		CPreferences::sPrefs->mSearchStyles.SetDirty();
		
		// Reset menu
		InitStyles();
	}
}

void CSearchWindow::DeleteStyle()
{
	// Create the dialog
	CTextListChoice dlog(CSDIFrame::GetAppTopWindow());
	dlog.mSingleSelection = false;
	dlog.mButtonTitle = rsrc::GetString("Alerts::Search::DeleteStyleButton");
	dlog.mTextListDescription = rsrc::GetString("Alerts::Search::DeleteStyleDesc");

	// Add all styles to list
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		dlog.mItems.push_back((*iter)->GetName());

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		// Delete styles in reverse
		CSearchStyleList& list = CPreferences::sPrefs->mSearchStyles.Value();
		for(ulvector::reverse_iterator riter = dlog.mSelection.rbegin(); riter != dlog.mSelection.rend(); riter++)
			list.erase(list.begin() + *riter);
		CPreferences::sPrefs->mSearchStyles.SetDirty();
		
		// Reset menu
		InitStyles();
	}
}

void CSearchWindow::SetStyle(const CSearchItem* spec)
{
	// Remove all
	RemoveAllCriteria();

	// Reset current item
	CSearchEngine::sSearchEngine.SetSearchItem(spec);
	
	InitCriteria(CSearchEngine::sSearchEngine.GetSearchItem());
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaMaxHeight = 200;

void CSearchWindow::Resized(int dy)
{
	::ResizeWindowBy(&mCriteria, 0, dy, true);
	mScrollerImageHeight += dy;
	mScroller.ResizeImage(0, mScrollerImageHeight);
}

#pragma mark ____________________________Window State

// Reset state from prefs
void CSearchWindow::ResetState(bool force)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	const int cWindowHeight1 = cWindowHeight + large_offset;

	// Get default state
	CSearchWindowState* state = &CPreferences::sPrefs->mSearchWindowDefault.Value();

	// Set button states before window resize
	if (state->GetExpanded() != mMailboxListPanel.mTwisted)
		mMailboxListPanel.OnTwist();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mSearchWindowDefault.GetValue());
	if (!set_rect.IsRectNull() && set_rect.right && set_rect.bottom)
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
										set_rect.Width(), set_rect.Height(), SWP_NOZORDER | SWP_NOREDRAW);
	}

	mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
																CSearchEngine::sSearchEngine.GetTargetHits());

	// Initialise items
	InitStyles();
	InitCriteria(CSearchEngine::sSearchEngine.GetSearchItem());
	mMailboxListPanel.InitCabinets();

	if (force) 
		OnSaveDefaultState();

	if (!force)
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		GetParentFrame()->RedrawWindow();
	}
	else
		GetParentFrame()->RedrawWindow();

	mGroupItems->SetInitialFocus();
}

// Save current state in prefs
void CSearchWindow::OnSaveDefaultState(void)
{
	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CSearchWindowState state(NULL, &bounds, eWindowStateNormal, mMailboxListPanel.mTwisted);
	if (CPreferences::sPrefs->mSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSearchWindowDefault.SetDirty();
}
