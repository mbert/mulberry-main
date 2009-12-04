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

// CRulesDialog.cpp : implementation file
//

#include "CRulesDialog.h"

#include "CFilterItem.h"
#include "CFilterManager.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRulesAction.h"
#include "CRulesActionLocal.h"
#include "CRulesActionSIEVE.h"
#include "CSDIFrame.h"
#include "CSearchCriteria.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog dialog

enum
{
	eTriggersMenu_Apply = IDM_RULESTRIGGERS_Apply,
	//eTriggersMenuSeparator,
	eTriggersFirst
};

CRulesDialog::CRulesDialog(CWnd* pParent /*=NULL*/) :
	CHelpDialog(CRulesDialog::IDD, pParent),
	CSearchBase(true)
{
	//{{AFX_DATA_INIT(CRulesDialog)
	//}}AFX_DATA_INIT
	
	mTriggerChange = false;
	mScrollerImageHeight = 0;
}

// Default destructor
CRulesDialog::~CRulesDialog()
{
}


void CRulesDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRulesDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRulesDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CRulesDialog)
	ON_COMMAND_RANGE(IDM_RULESTRIGGERS_Apply, IDM_RULESTRIGGERS_End, OnSetTrigger)
	ON_COMMAND(IDC_RULESDIALOG_MORE, OnMoreBtn)
	ON_COMMAND(IDC_RULESDIALOG_FEWER, OnFewerBtn)
	ON_COMMAND(IDC_RULESDIALOG_CLEAR, OnClearBtn)
	ON_COMMAND(IDC_RULESDIALOG_MOREACTIONS, OnMoreActions)
	ON_COMMAND(IDC_RULESDIALOG_FEWERACTIONS, OnFewerActions)
	ON_COMMAND(IDC_RULESDIALOG_CLEARACTIONS, OnClearActions)
	ON_COMMAND(IDC_RULESDIALOG_EDITSCRIPT, OnEditScript)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog message handlers

const int cWindowWidth = 512;
const int cWindowHeight = 240;

const int cHeaderHeight = cWindowWidth;
const int cBtnTop = 4;
const int cBtnStart = 4;
const int cBtnOffset = 44;
const int cCaptionHeight = 16;
const int cEditVOffset = -5;
const int cEditHeight = 22;

const int cNameTitleHOffset = 8;
const int cNameTitleVOffset = 12;
const int cNameTitleWidth = 40;
const int cNameTitleHeight = cCaptionHeight;

const int cNameHOffset = cNameTitleHOffset + cNameTitleWidth + 4;
const int cNameVOffset = cNameTitleVOffset + cEditVOffset;
const int cNameWidth = 128;
const int cNameHeight = cEditHeight;

const int cTriggeredByTitleWidth = 64;
const int cTriggeredByTitleHOffset = cNameHOffset + cNameWidth + 32;
const int cTriggeredByTitleVOffset = cNameTitleVOffset;
const int cTriggeredByTitleHeight = cCaptionHeight;

const int cTriggeredByWidth = 24;
const int cTriggeredByHOffset = cTriggeredByTitleHOffset + cTriggeredByTitleWidth;
const int cTriggeredByVOffset = 12;
const int cTriggeredByHeight = 16;

const int cEditScriptWidth = 170;
const int cEditScriptHOffset = cWindowWidth - cEditScriptWidth;
const int cEditScriptVOffset = 12;
const int cEditScriptHeight = cCaptionHeight;

const int cScrollerVOffset = cNameVOffset + cNameHeight + 8;
const int cScrollbarInset = 20;
const int cScrollerWidth = cWindowWidth - 4;
const int cScrollerHeight = cWindowHeight - 64 - cScrollerVOffset;

const int cCriteriaHOffset = 4;
const int cCriteriaVOffset = 0;
const int cCriteriaWidth = cScrollerWidth - 2 * cCriteriaHOffset - cScrollbarInset;
const int cCriteriaHeight = 16;

const int cCriteriaMoveHOffset = 0;
const int cCriteriaMoveVOffset = cCriteriaVOffset + cCriteriaHeight + 2;
const int cCriteriaMoveWidth = cScrollerWidth - 2 * cCriteriaMoveHOffset - cScrollbarInset;
const int cCriteriaMoveHeight = cScrollerHeight - 4 - cCriteriaMoveVOffset;

const int cMoreBtnWidth = 80;
const int cMoreBtnHeight = 24;
const int cMoreBtnHOffset = cCriteriaHOffset;
const int cMoreBtnVOffset = 8;

const int cFewerBtnWidth = 80;
const int cFewerBtnHeight = 24;
const int cFewerBtnHOffset = cMoreBtnHOffset + cMoreBtnWidth + 8;
const int cFewerBtnVOffset = cMoreBtnVOffset;

const int cClearBtnWidth = 80;
const int cClearBtnHeight = 24;
const int cClearBtnHOffset = cCriteriaHOffset + cClearBtnWidth;
const int cClearBtnVOffset = cMoreBtnVOffset;

const int cCriteriaDivideHOffset = cCriteriaHOffset;
const int cCriteriaDivideVOffset = cMoreBtnVOffset + cMoreBtnHeight + 4;
const int cCriteriaDivideWidth = cCriteriaWidth;
const int cCriteriaDivideHeight = 2;

const int cActionsHOffset = cCriteriaHOffset;
const int cActionsVOffset = cCriteriaDivideVOffset + 6;
const int cActionsWidth = cCriteriaWidth;
const int cActionsHeight = cCriteriaHeight;

const int cActionsMoveHOffset = cCriteriaMoveHOffset;
const int cActionsMoveVOffset = cActionsVOffset + cActionsHeight + 2;
const int cActionsMoveWidth = cCriteriaMoveWidth;
const int cActionsMoveHeight = cCriteriaMoveHeight - cActionsMoveVOffset;

const int cMoreActionsBtnWidth = 80;
const int cMoreActionsBtnHeight = 24;
const int cMoreActionsBtnHOffset = cActionsHOffset;
const int cMoreActionsBtnVOffset = 8;

const int cFewerActionsBtnWidth = 80;
const int cFewerActionsBtnHeight = 24;
const int cFewerActionsBtnHOffset = cMoreActionsBtnHOffset + cMoreActionsBtnWidth + 8;
const int cFewerActionsBtnVOffset = cMoreActionsBtnVOffset;

const int cStopBtnWidth = 120;
const int cStopBtnHeight = cCaptionHeight;
const int cStopBtnHOffset = cFewerActionsBtnHOffset + cFewerActionsBtnWidth + 16;
const int cStopBtnVOffset = cFewerActionsBtnVOffset + 4;

const int cClearActionsBtnWidth = 80;
const int cClearActionsBtnHeight = 24;
const int cClearActionsBtnHOffset = cClearBtnHOffset;
const int cClearActionsBtnVOffset = cMoreActionsBtnVOffset;

const int cActionsDivideHOffset = cActionsHOffset;
const int cActionsDivideVOffset = cMoreActionsBtnVOffset + cMoreActionsBtnHeight + 4;
const int cActionsDivideWidth = cActionsWidth;
const int cActionsDivideHeight = 2;

const int cCancelBtnWidth = 80;
const int cCancelBtnHeight = 24;
const int cCancelBtnHOffset = 32 + cCancelBtnWidth;
const int cCancelBtnVOffset = cCancelBtnHeight + 12;

const int cOKBtnWidth = 80;
const int cOKBtnHeight = 24;
const int cOKBtnHOffset = cCancelBtnHOffset + cCriteriaHOffset + cOKBtnWidth;
const int cOKBtnVOffset = cCancelBtnVOffset;

BOOL CRulesDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	//const int cPopupHeight1 = cPopupHeight + small_offset;
	//const int cPopupWidthExtra = 4*small_offset;

	// resize window to the size we use to layout the controls
	CRect r = CRect(0, 0, cWindowWidth, cWindowHeight);
	MoveWindow(r, false);

	// Create header pane
	mHeader.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cWindowWidth, cWindowHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_WidthHeight));

	CString s;
	s.LoadString(IDS_RULESDIALOG_NAME);
	r = CRect(cNameTitleHOffset, cNameTitleVOffset, cNameTitleHOffset + cNameTitleWidth, cNameTitleVOffset + cNameTitleHeight);
	mNameTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mNameTitle.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mNameTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cNameHOffset, cNameVOffset, cNameHOffset + cNameWidth, cNameVOffset + cNameHeight + small_offset);
	mName.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_RULESDIALOG_NAME);
	mName.SetFont(CMulberryApp::sAppFont);
	mName.SetReturnCmd(true);
	mHeader.AddAlignment(new CWndAlignment(&mName, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_RULESDIALOG_TRIGGERS);
	r = CRect(cTriggeredByTitleHOffset, cTriggeredByTitleVOffset, cTriggeredByTitleHOffset + cTriggeredByTitleWidth, cTriggeredByTitleVOffset + cTriggeredByTitleHeight);
	mTriggeredByTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mTriggeredByTitle.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mTriggeredByTitle, CWndAlignment::eAlign_TopRight));

	r = CRect(cTriggeredByHOffset, cTriggeredByVOffset, cTriggeredByHOffset + cTriggeredByWidth, cTriggeredByVOffset + cTriggeredByHeight);
	mTriggeredBy.Create(_T(""), r, &mHeader, IDC_RULESDIALOG_TRIGGERS, IDC_STATIC, IDI_POPUPBTN);
	mTriggeredBy.SetMenu(IDR_POPUP_RULES_TRIGGERS);
	mTriggeredBy.SetButtonText(false);
	mTriggeredBy.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mTriggeredBy, CWndAlignment::eAlign_TopRight));

	s.LoadString(IDS_RULESDIALOG_EDITSCRIPT);
	r = CRect(cEditScriptHOffset, cEditScriptVOffset, cEditScriptHOffset + cEditScriptWidth, cEditScriptVOffset + cEditScriptHeight + small_offset);
	mEditScript.Create(s, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, r, &mHeader, IDC_RULESDIALOG_EDITSCRIPT);
	mEditScript.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mEditScript, CWndAlignment::eAlign_TopRight));

	r = CRect(cCriteriaHOffset, cScrollerVOffset, cScrollerWidth - 2 * cCriteriaHOffset, cScrollerVOffset + cScrollerHeight);
	mScriptEdit.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_BORDER | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL, r, &mHeader, IDC_STATIC);
	mScriptEdit.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mScriptEdit, CWndAlignment::eAlign_WidthHeight));

	r = CRect(0, cScrollerVOffset, cScrollerWidth, cScrollerVOffset + cScrollerHeight);
	mScroller.CreateEx(WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT, NULL, _T(""), WS_CHILD | WS_VISIBLE| WS_BORDER | WS_VSCROLL, r, &mHeader, IDC_STATIC);
	mScroller.GetClientRect(r);
	mScrollerImageHeight = r.Height();
	mScroller.ResizeImage(0, mScrollerImageHeight);
	mHeader.AddAlignment(new CWndAlignment(&mScroller, CWndAlignment::eAlign_WidthHeight));

	// Items
	s.LoadString(IDS_SEARCH_CRITERIATITLE);
	r = CRect(cCriteriaHOffset, cCriteriaVOffset, cCriteriaHOffset + cCriteriaWidth, cCriteriaVOffset + cCriteriaHeight + large_offset);
	mCriteria.Create(s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, &mScroller, IDC_STATIC);
	mCriteria.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	mCriteria.SetFont(CMulberryApp::sAppFont);
	mScroller.AddAlignment(new CWndAlignment(&mCriteria, CWndAlignment::eAlign_TopWidth));

	r = CRect(cCriteriaMoveHOffset, cCriteriaMoveVOffset + large_offset, cCriteriaMoveHOffset + cCriteriaMoveWidth, cCriteriaMoveVOffset + cCriteriaMoveHeight);
	mCriteriaMove.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mScroller, IDC_STATIC);
	mCriteriaMove.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	mScroller.AddAlignment(new CWndAlignment(&mCriteriaMove, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_SEARCH_MOREBTN);
	r = CRect(cMoreBtnHOffset, cMoreBtnVOffset, cMoreBtnHOffset + cMoreBtnWidth + 2*large_offset, cMoreBtnVOffset + cMoreBtnHeight);
	mMoreBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mCriteriaMove, IDC_RULESDIALOG_MORE);
	mMoreBtn.SetFont(CMulberryApp::sAppFont);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mMoreBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_FEWERBTN);
	r = CRect(cFewerBtnHOffset + 2*large_offset, cFewerBtnVOffset, cFewerBtnHOffset + cFewerBtnWidth + 4*large_offset, cFewerBtnVOffset + cFewerBtnHeight);
	mFewerBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mCriteriaMove, IDC_RULESDIALOG_FEWER);
	mFewerBtn.SetFont(CMulberryApp::sAppFont);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mFewerBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_CLEARBTN);
	r = CRect(cCriteriaWidth - cClearBtnHOffset, cClearBtnVOffset, cCriteriaWidth - cClearBtnHOffset + cClearBtnWidth, cClearBtnVOffset + cClearBtnHeight);
	mClearBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mCriteriaMove, IDC_RULESDIALOG_CLEAR);
	mClearBtn.SetFont(CMulberryApp::sAppFont);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mClearBtn, CWndAlignment::eAlign_TopRight));

	r = CRect(cCriteriaDivideHOffset, cCriteriaDivideVOffset, cCriteriaDivideHOffset + cCriteriaDivideWidth, cCriteriaDivideVOffset + cCriteriaDivideHeight);
	mCriteriaDivider.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, &mCriteriaMove, IDC_STATIC);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mCriteriaDivider, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_RULESDIALOG_ACTIONS);
	r = CRect(cActionsHOffset, cActionsVOffset, cActionsHOffset + cActionsWidth, cActionsVOffset + cActionsHeight + large_offset);
	mActions.Create(s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, &mCriteriaMove, IDC_STATIC);
	mActions.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	mActions.SetFont(CMulberryApp::sAppFont);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mActions, CWndAlignment::eAlign_TopWidth));

	r = CRect(cActionsMoveHOffset, cActionsMoveVOffset + large_offset, cActionsMoveHOffset + cActionsMoveWidth, cActionsMoveVOffset + cActionsMoveHeight);
	mActionsMove.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mCriteriaMove, IDC_STATIC);
	mActionsMove.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	mCriteriaMove.AddAlignment(new CWndAlignment(&mActionsMove, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_ACTIONS_MOREBTN);
	r = CRect(cMoreBtnHOffset, cMoreActionsBtnVOffset, cMoreActionsBtnHOffset + cMoreActionsBtnWidth + 2*large_offset, cMoreActionsBtnVOffset + cMoreActionsBtnHeight);
	mMoreActionsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mActionsMove, IDC_RULESDIALOG_MOREACTIONS);
	mMoreActionsBtn.SetFont(CMulberryApp::sAppFont);
	mActionsMove.AddAlignment(new CWndAlignment(&mMoreActionsBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_ACTIONS_FEWERBTN);
	r = CRect(cFewerActionsBtnHOffset + 2*large_offset, cFewerActionsBtnVOffset, cFewerActionsBtnHOffset + cFewerActionsBtnWidth + 4*large_offset, cFewerActionsBtnVOffset + cFewerActionsBtnHeight);
	mFewerActionsBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mActionsMove, IDC_RULESDIALOG_FEWERACTIONS);
	mFewerActionsBtn.SetFont(CMulberryApp::sAppFont);
	mActionsMove.AddAlignment(new CWndAlignment(&mFewerActionsBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_RULESDIALOG_STOP);
	r = CRect(cStopBtnHOffset + 4*large_offset, cStopBtnVOffset, cStopBtnHOffset + cStopBtnWidth + 4*large_offset, cStopBtnVOffset + cStopBtnHeight + small_offset);
	mStop.Create(s, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, r, &mActionsMove, IDC_RULESDIALOG_STOP);
	mStop.SetFont(CMulberryApp::sAppFont);
	mActionsMove.AddAlignment(new CWndAlignment(&mStop, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_ACTIONS_CLEARBTN);
	r = CRect(cActionsWidth - cClearBtnHOffset, cClearBtnVOffset, cActionsWidth - cClearBtnHOffset + cClearBtnWidth, cClearBtnVOffset + cClearBtnHeight);
	mClearActionsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mActionsMove, IDC_RULESDIALOG_CLEARACTIONS);
	mClearActionsBtn.SetFont(CMulberryApp::sAppFont);
	mActionsMove.AddAlignment(new CWndAlignment(&mClearActionsBtn, CWndAlignment::eAlign_TopRight));

	r = CRect(cActionsDivideHOffset, cActionsDivideVOffset, cActionsDivideHOffset + cActionsDivideWidth, cActionsDivideVOffset + cActionsDivideHeight);
	mActionsDivider.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, &mActionsMove, IDC_STATIC);
	mActionsMove.AddAlignment(new CWndAlignment(&mActionsDivider, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_OK);
	r = CRect(cWindowWidth - cOKBtnHOffset - 2*large_offset, cWindowHeight - cOKBtnVOffset - cOKBtnHeight, cWindowWidth - cOKBtnHOffset + cOKBtnWidth - large_offset, cWindowHeight - cOKBtnVOffset);
	mOKBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mHeader, IDOK);
	mOKBtn.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mOKBtn, CWndAlignment::eAlign_BottomRight));

	s.LoadString(IDS_Cancel);
	r = CRect(cWindowWidth - cCancelBtnHOffset - 2*large_offset, cWindowHeight - cCancelBtnVOffset - cCancelBtnHeight, cWindowWidth - cCancelBtnHOffset + cCancelBtnWidth - large_offset, cWindowHeight - cCancelBtnVOffset);
	mCancelBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mHeader, IDCANCEL);
	mCancelBtn.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mCancelBtn, CWndAlignment::eAlign_BottomRight));

	SetFilter(mFilter);

	// Reset state
	ResetState();

	// Focus and select all in name field
	mName.SetSel(0, -1);
	mName.SetFocus();

	return false;
}

int CRulesDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHelpDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	return 0;
}

void CRulesDialog::OnDestroy(void)
{
	// Save state
	OnSaveDefaultState();

	// Do default action now
	CHelpDialog::OnDestroy();
}

// Resize columns
void CRulesDialog::OnSize(UINT nType, int cx, int cy)
{
	CHelpDialog::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
	if (mScrollerImageHeight != 0)
		mScroller.ResizeImage(0, mScrollerImageHeight, true);
}


#pragma mark ____________________________Commands

void CRulesDialog::OnSetTrigger(UINT nID)
{
	// Just toggle check state
	mTriggeredBy.GetPopupMenu()->CheckMenuItem(nID, MF_BYCOMMAND |
					(mTriggeredBy.GetPopupMenu()->GetMenuState(nID, MF_BYCOMMAND) & MF_CHECKED ? MF_UNCHECKED : MF_CHECKED));

	mTriggerChange = true;
}

void CRulesDialog::OnMoreBtn()
{
	OnMore();
}

void CRulesDialog::OnFewerBtn()
{
	OnFewer();
}

void CRulesDialog::OnClearBtn()
{
	OnClear();
}

void CRulesDialog::OnMoreActions()
{
	AddAction();
}

void CRulesDialog::OnFewerActions()
{
	RemoveAction();
}

void CRulesDialog::OnClearActions()
{
	// Remove all but the first one
	while(mActionItems.size() > 1)
		RemoveAction();

	// Reset the first one
	CRulesAction* action = static_cast<CRulesAction*>(mActionItems.at(0));
	action->SetActionItem(NULL);
}

void CRulesDialog::OnEditScript()
{
	if (mEditScript.GetCheck())
	{
		// Convert current items to script
		CFilterItem spec(CFilterItem::eSIEVE);
		ConstructFilter(&spec, false, false);
		spec.SetUseScript(true);

		// Set script in display
		CUnicodeUtils::SetWindowTextUTF8(&mScriptEdit, spec.GetScript());

		// Switch to script display
		mScroller.ShowWindow(SW_HIDE);
		mScriptEdit.ShowWindow(SW_SHOW);
	}
	else
	{
		mScriptEdit.ShowWindow(SW_HIDE);
		mScroller.ShowWindow(SW_SHOW);
	}

	// Always force reset
	OnClear();
	OnClearActions();
}

BOOL CRulesDialog::OnCmdMsg(UINT nID, int nCode, void* pExtra,
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
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CRulesDialog::SetFilter(CFilterItem* filter)
{
	if (filter)
	{
		CUnicodeUtils::SetWindowTextUTF8(&mName, filter->GetName());
		mFilterType = filter->GetType();
		if (filter->GetType() == CFilterItem::eSIEVE)
			mEditScript.ShowWindow(SW_SHOW);
		else
			mEditScript.ShowWindow(SW_HIDE);
		mEditScript.SetCheck(filter->GetUseScript());
		mStop.SetCheck(filter->Stop());
	}

	// Remove existing
	RemoveAllCriteria();
	while(mActionItems.size())
		RemoveAction();

	InitTriggers(filter);
	InitCriteria(filter ? filter->GetCriteria() : NULL);
	InitActions(filter ? filter->GetActions() : NULL);

	if (filter && filter->GetUseScript())
	{
		// Set script in display
		CUnicodeUtils::SetWindowTextUTF8(&mScriptEdit, filter->GetScript());

		// Switch to script display
		mScroller.ShowWindow(SW_HIDE);
		mScriptEdit.ShowWindow(SW_SHOW);
	}
}

void CRulesDialog::InitTriggers(CFilterItem* filter)
{
	bool has_manual = filter && (filter->GetType() == CFilterItem::eLocal);

	// Remove any existing items from main menu
	short num_menu = mTriggeredBy.GetPopupMenu()->GetMenuItemCount();
	for(short i = has_manual ? 3 : 1; i <= num_menu; i++)
		mTriggeredBy.GetPopupMenu()->RemoveMenu(has_manual ? 2 : 0, MF_BYPOSITION);

	// Set Apply item check mark
	if (has_manual)
		mTriggeredBy.GetPopupMenu()->CheckMenuItem(eTriggersMenu_Apply, MF_BYCOMMAND | (filter->GetManual() ? MF_CHECKED : MF_UNCHECKED));

	// Add all triggers/scripts
	if (filter)
	{
		short index = (has_manual ? eTriggersFirst : eTriggersMenu_Apply);
		if (filter->GetType() == CFilterItem::eLocal)
		{
			for(CTargetItemList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				CUnicodeUtils::AppendMenuUTF8(mTriggeredBy.GetPopupMenu(), MF_STRING, index, (*iter)->GetName());
				
				// Determine if this rule is in the trigger
				mTriggeredBy.GetPopupMenu()->CheckMenuItem(index, MF_BYCOMMAND | ((*iter)->ContainsFilter(filter) ? MF_CHECKED : MF_UNCHECKED));
			}
		}
		else
		{
			for(CFilterScriptList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				CUnicodeUtils::AppendMenuUTF8(mTriggeredBy.GetPopupMenu(), MF_STRING, index, (*iter)->GetName());
				
				// Determine if this rule is in the script
				mTriggeredBy.GetPopupMenu()->CheckMenuItem(index, MF_BYCOMMAND | ((*iter)->ContainsFilter(filter) ? MF_CHECKED : MF_UNCHECKED));
			}
		}
	}
}

#pragma mark ____________________________Criteria

const int cScrollerStartHeight = 260;

void CRulesDialog::Resized(int dy)
{
	// Now adjust sizes
	::MoveWindowBy(&mCriteriaMove, 0, dy, true);
	::ResizeWindowBy(&mCriteria, 0, dy, true);
	mScrollerImageHeight += dy;
	mScroller.ResizeImage(0, mScrollerImageHeight);
}

#pragma mark ____________________________Actions

void CRulesDialog::InitActions(const CActionItemList* actions)
{
	// Add each action
	if (actions && actions->size())
	{
		for(CActionItemList::const_iterator iter = actions->begin(); iter != actions->end(); iter++)
			AddAction(*iter);
	}
	else
		AddAction();
}

const int cActionPanelHeight = 26;
const int cActionPanelHOffset = 4;
const int cActionPanelVInitOffset = 14;
const int cActionPanelVOffset = 0;

void CRulesDialog::AddAction(const CActionItem* spec)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;
	const int cActionPanelVInitOffset1 = cActionPanelVInitOffset + large_offset;
	const int cActionPanelHeight1 = cActionPanelHeight + small_offset;

	// Create a new search criteria panel
	CRect r;
	mActions.GetWindowRect(r);
	r.OffsetRect(-r.left, -r.top);
	r.left += cActionPanelHOffset;
	r.right -= cActionPanelHOffset;
	r.bottom = cActionPanelHeight1;
	CRulesAction* action = NULL;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		action = new CRulesActionLocal;
		break;
	case CFilterItem::eSIEVE:
		action = new CRulesActionSIEVE;
		break;
	}
	action->Create(r, this, &mActions, NULL);
	mActions.AddAlignment(new CWndAlignment(action, CWndAlignment::eAlign_TopWidth));
	if (spec)
		action->SetActionItem(spec);

	// Get last view in action bottom
	POINT new_pos = {cActionPanelHOffset, cActionPanelVInitOffset1};
	if (mActionItems.size())
	{
		CRulesAction* prev = static_cast<CRulesAction*>(mActionItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		action->SetBottom(true);

		// Now get position
		CRect prev_rect;
		prev->GetWindowRect(prev_rect);
		mActions.ScreenToClient(prev_rect);
		new_pos.y = prev_rect.bottom + cActionPanelVOffset;
	}
	else
	{
		action->SetTop(true);
		action->SetBottom(true);
	}

	// Add to list
	mActionItems.push_back(action);

	// Now adjust sizes
	CRect size;
	action->GetWindowRect(size);
	ResizedActions(size.Height() + cActionPanelVOffset);
	
	// Position new sub-panel, show and focus it
	action->SetWindowPos(nil, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	action->ShowWindow(SW_SHOW);
	action->SetFocus();
	
	// Do button state
	mFewerActionsBtn.ShowWindow(SW_SHOW);
}

void CRulesDialog::RemoveAction()
{
	// Get last view in criteria
	CWnd* action = mActionItems.back();
	mActionItems.pop_back();
	action->ShowWindow(SW_HIDE);

	// Now adjust sizes
	CRect size;
	action->GetWindowRect(size);
	ResizedActions(-size.Height() - cActionPanelVOffset);

	// Now delete the pane
	action->DestroyWindow();
	delete action;
	
	if (mActionItems.size())
	{
		CRulesAction* prev = static_cast<CRulesAction*>(mActionItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (mActionItems.size() == 0)
		mFewerActionsBtn.ShowWindow(SW_HIDE);
}

void CRulesDialog::ResizedActions(int dy)
{
	// Now adjust sizes
	::ResizeWindowBy(&mCriteriaMove, 0, dy, true);
	::MoveWindowBy(&mActionsMove, 0, dy, true);
	::ResizeWindowBy(&mActions, 0, dy, true);
	mScrollerImageHeight += dy;
	mScroller.ResizeImage(0, mScrollerImageHeight);
}

#pragma mark ____________________________Build

void CRulesDialog::OnOK()
{
	// Lock to prevent filter manager changes whilst running
	cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

	// Force update here as dialog is destroyed before exiting DoModal
	ConstructFilter(mFilter, mEditScript.GetCheck());
	
	// Do inherited
	CHelpDialog::OnOK();
}

void CRulesDialog::ConstructFilter(CFilterItem* spec, bool script, bool change_triggers) const
{
	// Get name
	cdstring txt;
	mName.GetText(txt);
	spec->SetName(txt);

	// Get triggers
	if (change_triggers)
	{
		// Get triggers
		bool has_manual = (spec->GetType() == CFilterItem::eLocal);
		if (has_manual)
		{
			UINT mark = mTriggeredBy.GetPopupMenu()->GetMenuState(eTriggersMenu_Apply, MF_BYCOMMAND);
			spec->SetManual(mark & MF_CHECKED);
		}
		short num_menu = mTriggeredBy.GetPopupMenu()->GetMenuItemCount();
		short index = 0;
		for(short i = has_manual ? 2 : 0; i < num_menu; i++, index++)
		{
			// Check mark state
			UINT mark = mTriggeredBy.GetPopupMenu()->GetMenuState(i, MF_BYPOSITION);
		
			// Set the appropriate item
			if (mark & MF_CHECKED)
			{
				if (spec->GetType() == CFilterItem::eLocal)
					CPreferences::sPrefs->GetFilterManager()->GetTargets(spec->GetType()).at(index)->AddFilter(spec);
				else
					CPreferences::sPrefs->GetFilterManager()->GetScripts(spec->GetType()).at(index)->AddFilter(spec);
			}
			else
			{
				if (spec->GetType() == CFilterItem::eLocal)
					CPreferences::sPrefs->GetFilterManager()->GetTargets(spec->GetType()).at(index)->RemoveFilter(spec);
				else
					CPreferences::sPrefs->GetFilterManager()->GetScripts(spec->GetType()).at(index)->RemoveFilter(spec);
			}
		}
	}

	// Get details
	if (script)
	{
		spec->SetUseScript(script);

		// Copy info from panel into prefs
		cdstring txt;
		mScriptEdit.GetText(txt);
		spec->SetScript(txt);
	}
	else
	{
		spec->SetUseScript(false);
		spec->SetCriteria(ConstructSearch());
		spec->SetActions(ConstructActions());
		spec->SetStop(mStop.GetCheck());
	}
}

CActionItemList* CRulesDialog::ConstructActions() const
{
	CActionItemList* actions = new CActionItemList;

	// Count items
	long num = mActionItems.size();
	for(long i = 0; i < num; i++)
		actions->push_back(static_cast<CRulesAction*>(mActionItems.at(i))->GetActionItem());

	return actions;
}


#pragma mark ____________________________Window State

// Reset state from prefs
void CRulesDialog::ResetState()
{
	// Get default state
	CWindowState* state = &CPreferences::sPrefs->mRulesDialogDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mSearchWindowDefault.GetValue());
	if (!set_rect.IsRectNull() && set_rect.right && set_rect.bottom)
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		SetWindowPos(NULL, set_rect.left, set_rect.top,
										set_rect.Width(), set_rect.Height(), SWP_NOZORDER | SWP_NOREDRAW);
	}
}

// Save current state in prefs
void CRulesDialog::OnSaveDefaultState(void)
{
	// Get name as cstr
	cdstring name;

	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CWindowState state(name, &bounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mRulesDialogDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesDialogDefault.SetDirty();
}

#pragma mark ____________________________Do dialog

bool CRulesDialog::PoseDialog(CFilterItem* spec, bool& trigger_change)
{
	// Create the dialog
	CRulesDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mFilter = spec;
	
	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		trigger_change = dlog.mTriggerChange;
		return true;
	}
	else
		return false;
}