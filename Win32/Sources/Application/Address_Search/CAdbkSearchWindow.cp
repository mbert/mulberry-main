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


// CAdbkSearchWindow.cp : implementation of the CAdbkSearchWindow class
//



#include "CAdbkSearchWindow.h"

#include "CFontCache.h"
#include "CGeneralException.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CWindowStates.h"

const int cWindowWidth = 300;
const int cWindowHeight = 200;

const int cHeaderHeight = 112;
const int cBtnTop = cHeaderHeight - 52;
const int cBtnStart = 4;
const int cBtnOffset = 38;

const int cFirstLineTop = 4;
const int cSecondLineTop = 30;
const int cTitleTopOffset = 5;
const int cSourceTitleLeft = 4;
const int cSourceTitleWidth = 56;
const int cSourcePopupLeft = cSourceTitleLeft + cSourceTitleWidth;
const int cSourcePopupWidth = 128;
const int cFieldTitle1Left = cSourcePopupLeft + cSourcePopupWidth + 8;
const int cFieldTitle1Width = 38;
const int cServersTitleLeft = cFieldTitle1Left;
const int cServersTitleWidth = 48;
const int cServersPopupLeft = cServersTitleLeft + cServersTitleWidth;
const int cServersPopupWidth = 22;
const int cFieldTitle2Left = cServersPopupLeft + cServersPopupWidth + 8;
const int cFieldTitle2Width = 48;
const int cFieldPopupLeft = 4;
const int cFieldPopupWidth = 96;
const int cMethodPopupLeft = cFieldPopupLeft + cFieldPopupWidth + 16;
const int cMethodPopupWidth = 96;
const int cSearchTextLeft = cMethodPopupLeft + cMethodPopupWidth + 16;
const int cSearchTextROffset = 8;
const int cPopupHeight = 22;
const int cTitleHeight = 16;

const int cTitleTableHeight = 16;

/////////////////////////////////////////////////////////////////////////////
// CAdbkSearchWindow

IMPLEMENT_DYNCREATE(CAdbkSearchWindow, CView)

BEGIN_MESSAGE_MAP(CAdbkSearchWindow, CTableWindow)
	ON_COMMAND(IDC_SEARCHBTN, OnSearchBtn)
	ON_COMMAND(IDC_SEARCHCLEARBTN, OnClearBtn)
	ON_COMMAND(IDC_SEARCHNEWMSGBTN, OnNewMessageBtn)
	ON_COMMAND(IDC_SEARCHTEXT, OnSearchBtn)

	ON_COMMAND_RANGE(IDM_ADBK_SOURCE_MULBERRY, IDM_ADBK_SOURCE_FINGER, OnChangeSource)
	ON_COMMAND_RANGE(IDM_ADBK_SERVERS_Start, IDM_ADBK_SERVERS_End, OnChangeServers)
	ON_COMMAND_RANGE(IDM_ADBK_FIELD_NAME, IDM_ADBK_FIELD_NOTES, OnChangeField)
	ON_COMMAND_RANGE(IDM_ADBK_METHOD_IS, IDM_ADBK_METHOD_CONTAINS, OnChangeMethod)
	
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdbkSearchWindow construction/destruction

CAdbkSearchWindow* CAdbkSearchWindow::sAdbkSearch = NULL;
CMultiDocTemplate* CAdbkSearchWindow::sAdbkSearchDocTemplate = NULL;

CAdbkSearchWindow::CAdbkSearchWindow()
{
	sAdbkSearch = this;
}

CAdbkSearchWindow::~CAdbkSearchWindow()
{
	sAdbkSearch = NULL;
}

// Manually create document
CAdbkSearchWindow* CAdbkSearchWindow::ManualCreate(void)
{
	CDocument* aDoc = sAdbkSearchDocTemplate->OpenDocumentFile(NULL, false);
	CString strDocName;
	sAdbkSearchDocTemplate->GetDocString(strDocName, CDocTemplate::docName);
	aDoc->SetTitle(strDocName);

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CAdbkSearchWindow)))
		return (CAdbkSearchWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return NULL;
}

int CAdbkSearchWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	const int cPopupHeight1 = cPopupHeight + small_offset;
	const int cPopupWidthExtra = 4*small_offset;

	// Create header pane
	mHeader.CreateEx(WS_EX_DLGMODALFRAME, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cWindowWidth, cHeaderHeight + large_offset, GetSafeHwnd(), (HMENU)IDC_STATIC);

	// Buttons
	int left = cBtnStart + small_offset;
	int top = cBtnTop + 2*small_offset;
	CString s;
	s.LoadString(IDS_SEARCH);
	mSearchBtn.Create(s, CRect(left, top, left + 32, top + 32),  &mHeader, IDC_SEARCHBTN, IDC_STATIC, IDI_ADDRESS_SEARCH);
	left += cBtnOffset + small_offset;
	s.LoadString(IDS_CLEAR);
	mClearBtn.Create(s, CRect(left, top, left + 32, top + 32),  &mHeader, IDC_SEARCHCLEARBTN, IDC_STATIC, IDI_ADDRESS_DELETE);
	left = cWindowWidth - cBtnOffset + small_offset;
	s.LoadString(IDS_ADDRESS_NEWMSG);
	mNewMessageBtn.Create(s, CRect(left, top, left + 32, top + 32),  &mHeader, IDC_SEARCHNEWMSGBTN, IDC_STATIC, IDI_ADDRESS_NEWMSG);
	mHeader.AddAlignment(new CWndAlignment(&mNewMessageBtn, CWndAlignment::eAlign_TopRight));

	// Popups & statics
	s.LoadString(IDS_ADBKSEARCHTITLE1);
	CRect r = CRect(cSourceTitleLeft, cFirstLineTop + cTitleTopOffset, cSourceTitleLeft + cSourceTitleWidth + large_offset, cFirstLineTop + cTitleTopOffset + cTitleHeight);
	mSourceTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mSourceTitle.SetFont(CMulberryApp::sAppFont);

	r = CRect(cSourcePopupLeft + large_offset, cFirstLineTop, cSourcePopupLeft + cSourcePopupWidth + cPopupWidthExtra, cFirstLineTop + cPopupHeight1);
	mSourcePopup.Create(_T(""), r, &mHeader, IDC_SEARCH_SOURCEPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mSourcePopup.SetFont(CMulberryApp::sAppFont);
	mSourcePopup.SetMenu(IDR_POPUP_ADBK_SOURCE);
	mSourcePopup.SetValue(IDM_ADBK_SOURCE_MULBERRY);

	s.LoadString(IDS_ADBKSEARCHTITLE2);
	r = CRect(cFieldTitle1Left + large_offset + cPopupWidthExtra, cFirstLineTop + cTitleTopOffset, cFieldTitle1Left + large_offset + cPopupWidthExtra + cFieldTitle1Width, cFirstLineTop + cTitleTopOffset + cTitleHeight);
	mFieldTitle1.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mFieldTitle1.SetFont(CMulberryApp::sAppFont);

	s.LoadString(IDS_ADBKSEARCHTITLE3);
	r = CRect(cServersTitleLeft + large_offset + cPopupWidthExtra, cFirstLineTop + cTitleTopOffset, cServersTitleLeft + large_offset + cPopupWidthExtra + cServersTitleWidth, cFirstLineTop + cTitleTopOffset + cTitleHeight);
	mServersTitle.Create(s, WS_CHILD, r, &mHeader, IDC_STATIC);
	mServersTitle.SetFont(CMulberryApp::sAppFont);

	r = CRect(cServersPopupLeft + large_offset + cPopupWidthExtra, cFirstLineTop, cServersPopupLeft + cServersPopupWidth + 2*cPopupWidthExtra, cFirstLineTop + cPopupHeight1);
	mServerPopup.Create(_T(""), r, &mHeader, IDC_SEARCH_SERVERPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mServerPopup.SetFont(CMulberryApp::sAppFont);
	mServerPopup.SetButtonText(false);
	mServerPopup.SetMenu(IDR_POPUP_ADBK_SERVERS);
	mServerPopup.SyncMenu();
	mServerPopup.ShowWindow(SW_HIDE);

	s.LoadString(IDS_ADBKSEARCHTITLE2);
	r = CRect(cFieldTitle2Left + large_offset + 2*cPopupWidthExtra, cFirstLineTop + cTitleTopOffset, cFieldTitle2Left + large_offset + 2*cPopupWidthExtra + cFieldTitle2Width, cFirstLineTop + cTitleTopOffset + cTitleHeight);
	mFieldTitle2.Create(s, WS_CHILD, r, &mHeader, IDC_STATIC);
	mFieldTitle2.SetFont(CMulberryApp::sAppFont);

	r = CRect(cFieldPopupLeft, cSecondLineTop + small_offset, cFieldPopupLeft + cFieldPopupWidth + cPopupWidthExtra, cSecondLineTop + cPopupHeight1 + small_offset);
	mFieldPopup.Create(_T(""), r, &mHeader, IDC_SEARCH_FIELDPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mFieldPopup.SetFont(CMulberryApp::sAppFont);
	mFieldPopup.SetMenu(IDR_POPUP_ADBK_FIELD);
	mFieldPopup.SetValue(IDM_ADBK_FIELD_NAME);

	r = CRect(cMethodPopupLeft + cPopupWidthExtra, cSecondLineTop + small_offset, cMethodPopupLeft + cFieldPopupWidth + 2*cPopupWidthExtra, cSecondLineTop + cPopupHeight1 + small_offset);
	mMethodPopup.Create(_T(""), r, &mHeader, IDC_SEARCH_METHODPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mMethodPopup.SetFont(CMulberryApp::sAppFont);
	mMethodPopup.SetMenu(IDR_POPUP_ADBK_METHOD);
	mMethodPopup.SetValue(IDM_ADBK_METHOD_CONTAINS);

	r = CRect(cSearchTextLeft + 2*cPopupWidthExtra, cSecondLineTop + small_offset, cWindowWidth - cSearchTextROffset, cSecondLineTop + small_offset + cPopupHeight1);
	mSearchText.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOHSCROLL, r, &mHeader, IDC_SEARCHTEXT);
	mSearchText.SetFont(CMulberryApp::sAppFont);
	mSearchText.SetReturnCmd(true);
	mSearchText.SetContextView(this);
	mHeader.AddAlignment(new CWndAlignment(&mSearchText, CWndAlignment::eAlign_TopWidth));
	
	// Table
	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
					CRect(0, cHeaderHeight + large_offset + cTitleTableHeight, cWindowWidth, cWindowHeight), this, IDC_SERVERTABLE);
	mTable.ResetFont(CFontCache::GetListFont());
	mTable.SetColumnInfo(mColumnInfo);
	mTable.SetContextMenuID(IDR_POPUP_CONTEXT_ADDRMANAGER);
	mTable.SetContextView(this);

	// Get titles
	mTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
					CRect(0, cHeaderHeight + large_offset, cWindowWidth, cHeaderHeight + large_offset + cTitleTableHeight), this, IDC_SERVERTABLE);
	mTitles.SetFont(CFontCache::GetListFont());

	PostCreate(&mTable, &mTitles);

	// Set status
	SetOpen();

	// Add import/export menu items
	CMenu* menu = GetParentFrame()->GetMenu();
	CMenu* addr_menu = menu->GetSubMenu(2);
	for(int i = 0; i < addr_menu->GetMenuItemCount(); i++)
	{
		if (addr_menu->GetMenuItemID(i) == IDM_ADDR_RENAME)
		{
			CMenu* import_menu = addr_menu->GetSubMenu(i + 2);
			CMenu* export_menu = addr_menu->GetSubMenu(i + 3);
			CPluginManager::sPluginManager.SyncAdbkIOPluginMenu(import_menu, export_menu);
			break;
		}
	}

	return 0;
}

// Get child windows
void CAdbkSearchWindow::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CTableWindow::OnActivateView(bActivate, pActivateView, pDeactiveView);

	// Focus on search criteria
	if (bActivate)
	{
		mSearchText.SetFocus();
		mSearchText.SetSel(0, -1);
	}
	
}

BOOL CAdbkSearchWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Extend the framework's command route from the view to
	// the application-specific CMyShape that is currently selected
	// in the view. m_pActiveShape is NULL if no shape object
	// is currently selected in the view.
	
	// Must route to edit field except for return key command
	if ((GetFocus()->GetSafeHwnd() == mSearchText.GetSafeHwnd()) &&
		(nID != IDC_SEARCHTEXT) &&
		mSearchText.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CTableWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Resize sub-views
void CAdbkSearchWindow::OnSize(UINT nType, int cx, int cy)
{
	CTableWindow::OnSize(nType, cx, cy);

	// Resize its header
	mHeader.MoveWindow(CRect(0, 0, cx, cHeaderHeight));
	
	// Resize its titles (keep current height)
	CRect wrect;
	mTitles.GetWindowRect(wrect);
	mTitles.MoveWindow(CRect(0, cHeaderHeight, cx, cHeaderHeight + wrect.Height()));
	
	// Resize its table (keep current top)
	mTable.MoveWindow(CRect(0, cHeaderHeight + wrect.Height(), cx, cy));
}

void CAdbkSearchWindow::OnSearchBtn(void)
{
	mTable.OnSearch();
}

void CAdbkSearchWindow::OnClearBtn(void)
{
	mTable.OnClear();
}

void CAdbkSearchWindow::OnNewMessageBtn(void)
{
	mTable.OnNewLetter();
}

void CAdbkSearchWindow::OnChangeSource(UINT nID)
{
	mSourcePopup.SetValue(nID);
	
	switch(nID)
	{
	case IDM_ADBK_SOURCE_MULBERRY:
		// Enable all field items
		for(UINT id = IDM_ADBK_FIELD_NAME; id <= IDM_ADBK_FIELD_NOTES; id++)
			mFieldPopup.GetPopupMenu()->EnableMenuItem(id, MF_BYCOMMAND);
		break;
	case IDM_ADBK_SOURCE_LDAP:
		mServerPopup.InitServersMenu(CINETAccount::eLDAP);
		// Enable all field items
		for(UINT id = IDM_ADBK_FIELD_NAME; id <= IDM_ADBK_FIELD_NOTES; id++)
			mFieldPopup.GetPopupMenu()->EnableMenuItem(id, MF_BYCOMMAND);
		break;
	}
	switch(nID)
	{
	case IDM_ADBK_SOURCE_MULBERRY:
		mFieldTitle1.ShowWindow(SW_SHOW);
		mServersTitle.ShowWindow(SW_HIDE);
		mServerPopup.ShowWindow(SW_HIDE);
		mFieldTitle2.ShowWindow(SW_HIDE);
		break;
	case IDM_ADBK_SOURCE_LDAP:
		mFieldTitle1.ShowWindow(SW_HIDE);
		mServersTitle.ShowWindow(SW_SHOW);
		mServerPopup.ShowWindow(SW_SHOW);
		mFieldTitle2.ShowWindow(SW_SHOW);
		break;
	}
}

void CAdbkSearchWindow::OnChangeServers(UINT nID)
{
	mServerPopup.SetValue(nID);
}

void CAdbkSearchWindow::OnChangeField(UINT nID)
{
	mFieldPopup.SetValue(nID);
}

void CAdbkSearchWindow::OnChangeMethod(UINT nID)
{
	mMethodPopup.SetValue(nID);
}

void CAdbkSearchWindow::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();
	
	// Add default columns
	
	// Name, width = 240
	InsertColumn(1);
	SetColumnType(1, eAddrColumnName);
	SetColumnWidth(1, 240);
}

// Reset state from prefs
void CAdbkSearchWindow::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	CAdbkSearchWindowState* state = &CPreferences::sPrefs->mAdbkSearchWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
										set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue()));

	// When state reset must always show

	//if (state->mHide)
	//	GetParentFrame()->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	//else
	//	GetParentFrame()->SendMessage(WM_SYSCOMMAND, SC_RESTORE);

	// Set state of popups
	OnChangeSource(state->GetMethod() + IDM_ADBK_SOURCE_MULBERRY - 1);
	OnChangeField(state->GetField() + IDM_ADBK_FIELD_NAME - 1);
	OnChangeMethod(state->GetCriteria() + IDM_ADBK_METHOD_IS - 1);

	if (force) 
		OnSaveDefaultState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (state->GetState() == eWindowStateMax)
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	if (!force)
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		GetParentFrame()->RedrawWindow();
	}
	else
		GetParentFrame()->RedrawWindow();
}

// Save current state in prefs
void CAdbkSearchWindow::SaveDefaultState(void)
{
	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Add info to prefs
	CAdbkSearchWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo,
									(EAdbkSearchMethod) (mSourcePopup.GetValue() - IDM_ADBK_SOURCE_MULBERRY + 1),
									(EAddrColumn) (mFieldPopup.GetValue() - IDM_ADBK_FIELD_NAME + 1),
									(EAdbkSearchCriteria) (mMethodPopup.GetValue() - IDM_ADBK_METHOD_IS + 1),
									GetParentFrame()->IsIconic());
	if (CPreferences::sPrefs->mAdbkSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkSearchWindowDefault.SetDirty();
}

// Reset the table
void CAdbkSearchWindow::ResetTable(void)
{
	mTable.ResetTable();
}
