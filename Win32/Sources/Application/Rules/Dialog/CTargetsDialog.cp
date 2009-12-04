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

// CTargetsDialog.cpp : implementation file
//

#include "CTargetsDialog.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CRulesTarget.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog dialog

CTargetsDialog::CTargetsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CTargetsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTargetsDialog)
	//}}AFX_DATA_INIT
}

// Default destructor
CTargetsDialog::~CTargetsDialog()
{
}


void CTargetsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetsDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargetsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CTargetsDialog)
	ON_COMMAND_RANGE(IDM_SCHEDULE_NEVER, IDM_SCHEDULE_ALWAYS, OnSetSchedule)
	ON_COMMAND(IDC_TARGETSDIALOG_MORETARGETS, OnMoreTargets)
	ON_COMMAND(IDC_TARGETSDIALOG_FEWERTARGETS, OnFewerTargets)
	ON_COMMAND(IDC_TARGETSDIALOG_CLEARTARGETS, OnClearTargets)

	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog message handlers

const int cWindowWidth = 512;
const int cWindowHeight = 210;

const int cHeaderHeight = cWindowWidth;
const int cBtnTop = 4;
const int cBtnStart = 4;
const int cBtnOffset = 44;
const int cCaptionHeight = 16;
const int cEditVOffset = -5;
const int cEditHeight = 22;

const int cNameTitleHOffset = 8;
const int cNameTitleVOffset = 12;
const int cNameTitleWidth = 48;
const int cNameTitleHeight = cCaptionHeight;

const int cNameHOffset = cNameTitleHOffset + cNameTitleWidth + 4;
const int cNameVOffset = cNameTitleVOffset + cEditVOffset;
const int cNameWidth = 128;
const int cNameHeight = cEditHeight;

const int cEnabledWidth = 170;
const int cEnabledHOffset = cNameHOffset + cNameWidth + 32;
const int cEnabledVOffset = 12;
const int cEnabledHeight = cCaptionHeight;

const int cApplyTitleHOffset = 8;
const int cApplyTitleVOffset = cNameTitleVOffset + cEditHeight + 8;
const int cApplyTitleWidth = 74;
const int cApplyTitleHeight = cCaptionHeight;

const int cApplyHOffset = cApplyTitleHOffset + cApplyTitleWidth + 4;
const int cApplyVOffset = cApplyTitleVOffset + cEditVOffset;
const int cApplyWidth = 128;
const int cApplyHeight = cEditHeight;

const int cScrollerVOffset = cApplyVOffset + cApplyHeight + 8;
const int cScrollbarInset = 20;
const int cScrollerWidth = cWindowWidth - 4;
const int cScrollerHeight = cWindowHeight - 64 - cScrollerVOffset;

const int cTargetsHOffset = 4;
const int cTargetsVOffset = 2;
const int cTargetsWidth = cScrollerWidth - 2 * cTargetsHOffset - cScrollbarInset;
const int cTargetsHeight = 16;

const int cTargetsMoveHOffset = 0;
const int cTargetsMoveVOffset = cTargetsVOffset + cTargetsHeight + 2;
const int cTargetsMoveWidth = cScrollerWidth - 2 * cTargetsMoveHOffset - cScrollbarInset;
const int cTargetsMoveHeight = cScrollerHeight - 4 - cTargetsMoveVOffset;

const int cMoreBtnWidth = 80;
const int cMoreBtnHeight = 24;
const int cMoreBtnHOffset = cTargetsHOffset;
const int cMoreBtnVOffset = 12;

const int cFewerBtnWidth = 80;
const int cFewerBtnHeight = 24;
const int cFewerBtnHOffset = cMoreBtnHOffset + cMoreBtnWidth + 8;
const int cFewerBtnVOffset = cMoreBtnVOffset;

const int cClearBtnWidth = 80;
const int cClearBtnHeight = 24;
const int cClearBtnHOffset = cTargetsHOffset + cClearBtnWidth;
const int cClearBtnVOffset = cMoreBtnVOffset;

const int cTargetsDivideHOffset = cTargetsHOffset;
const int cTargetsDivideVOffset = cMoreBtnVOffset + cMoreBtnHeight + 4;
const int cTargetsDivideWidth = cTargetsWidth;
const int cTargetsDivideHeight = 2;

const int cCancelBtnWidth = 80;
const int cCancelBtnHeight = 24;
const int cCancelBtnHOffset = 32 + cCancelBtnWidth;
const int cCancelBtnVOffset = cCancelBtnHeight + 12;

const int cOKBtnWidth = 80;
const int cOKBtnHeight = 24;
const int cOKBtnHOffset = cCancelBtnHOffset + cTargetsHOffset + cOKBtnWidth;
const int cOKBtnVOffset = cCancelBtnVOffset;

BOOL CTargetsDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	//const int cPopupHeight1 = cPopupHeight + small_offset;
	//const int cPopupWidthExtra = 4*small_offset;

	// Create header pane
	mHeader.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cWindowWidth, cWindowHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);

	CString s;
	s.LoadString(IDS_TARGETSDIALOG_NAME);
	CRect r = CRect(cNameTitleHOffset, cNameTitleVOffset, cNameTitleHOffset + cNameTitleWidth, cNameTitleVOffset + cNameTitleHeight);
	mNameTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mNameTitle.SetFont(CMulberryApp::sAppFont);

	r = CRect(cNameHOffset, cNameVOffset, cNameHOffset + cNameWidth, cNameVOffset + cNameHeight + small_offset);
	mName.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_TARGETSDIALOG_NAME);
	mName.SetFont(CMulberryApp::sAppFont);
	mName.SetReturnCmd(true);

	s.LoadString(IDS_TARGETSDIALOG_ENABLED);
	r = CRect(cEnabledHOffset, cEnabledVOffset, cEnabledHOffset + cEnabledWidth, cEnabledVOffset + cEnabledHeight + small_offset);
	mEnabled.Create(s, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, r, &mHeader, IDC_TARGETSDIALOG_ENABLED);
	mEnabled.SetFont(CMulberryApp::sAppFont);

	s.LoadString(IDS_TARGETSDIALOG_TRIGGER);
	r = CRect(cApplyTitleHOffset, cApplyTitleVOffset, cApplyTitleHOffset + cApplyTitleWidth, cApplyTitleVOffset + cApplyTitleHeight);
	mApplyTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mApplyTitle.SetFont(CMulberryApp::sAppFont);

	r = CRect(cApplyHOffset, cApplyVOffset, cApplyHOffset + cApplyWidth, cApplyVOffset + cApplyHeight + small_offset);
	mApplyPopup.Create(_T(""), r, &mHeader, IDC_TARGETSDIALOG_ACTIONS, IDC_STATIC, IDI_POPUPBTN);
	mApplyPopup.SetMenu(IDR_POPUP_SCHEDULE);
	mApplyPopup.SetValue(IDM_SCHEDULE_NEVER);
	mApplyPopup.SetFont(CMulberryApp::sAppFont);

	r = CRect(0, cScrollerVOffset, cScrollerWidth, cScrollerVOffset + cScrollerHeight);
	mScroller.CreateEx(WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT, NULL, _T(""), WS_CHILD | WS_VISIBLE| WS_BORDER | WS_VSCROLL, r, &mHeader, IDC_STATIC);
	mScroller.GetClientRect(r);
	mScrollerImageHeight = r.Height();
	mScroller.ResizeImage(0, mScrollerImageHeight);

	// Items
	s.LoadString(IDS_TARGETSDIALOG_APPLY);
	r = CRect(cTargetsHOffset, cTargetsVOffset, cTargetsHOffset + cTargetsWidth - small_offset, cTargetsVOffset + cTargetsHeight + large_offset);
	mTargets.Create(s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, &mScroller, IDC_STATIC);
	mTargets.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	mTargets.SetFont(CMulberryApp::sAppFont);

	r = CRect(cTargetsMoveHOffset, cTargetsMoveVOffset + large_offset, cTargetsMoveHOffset + cTargetsMoveWidth, cTargetsMoveVOffset + cTargetsMoveHeight);
	mTargetsMove.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mScroller, IDC_STATIC);
	mTargetsMove.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	s.LoadString(IDS_TARGETS_MOREBTN);
	r = CRect(cMoreBtnHOffset, cMoreBtnVOffset, cMoreBtnHOffset + cMoreBtnWidth + 2*large_offset, cMoreBtnVOffset + cMoreBtnHeight);
	mMoreTargetsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mTargetsMove, IDC_TARGETSDIALOG_MORETARGETS);
	mMoreTargetsBtn.SetFont(CMulberryApp::sAppFont);

	s.LoadString(IDS_TARGETS_FEWERBTN);
	r = CRect(cFewerBtnHOffset + 2*large_offset, cFewerBtnVOffset, cFewerBtnHOffset + cFewerBtnWidth + 4*large_offset, cFewerBtnVOffset + cFewerBtnHeight);
	mFewerTargetsBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mTargetsMove, IDC_TARGETSDIALOG_FEWERTARGETS);
	mFewerTargetsBtn.SetFont(CMulberryApp::sAppFont);

	s.LoadString(IDS_TARGETS_CLEARBTN);
	r = CRect(cTargetsWidth - cClearBtnHOffset, cClearBtnVOffset, cTargetsWidth - cClearBtnHOffset + cClearBtnWidth, cClearBtnVOffset + cClearBtnHeight);
	mClearTargetsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mTargetsMove, IDC_TARGETSDIALOG_CLEARTARGETS);
	mClearTargetsBtn.SetFont(CMulberryApp::sAppFont);

	r = CRect(cTargetsDivideHOffset, cTargetsDivideVOffset, cTargetsDivideHOffset + cTargetsDivideWidth, cTargetsDivideVOffset + cTargetsDivideHeight);
	mTargetsDivider.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, &mTargetsMove, IDC_STATIC);

	s.LoadString(IDS_OK);
	r = CRect(cWindowWidth - cOKBtnHOffset - 2*large_offset, cWindowHeight - cOKBtnVOffset - cOKBtnHeight, cWindowWidth - cOKBtnHOffset + cOKBtnWidth - large_offset, cWindowHeight - cOKBtnVOffset);
	mOKBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mHeader, IDOK);
	mOKBtn.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mOKBtn, CWndAlignment::eAlign_BottomRight));

	s.LoadString(IDS_Cancel);
	r = CRect(cWindowWidth - cCancelBtnHOffset - 3*large_offset, cWindowHeight - cCancelBtnVOffset - cCancelBtnHeight, cWindowWidth - cCancelBtnHOffset + cCancelBtnWidth - 2*large_offset, cWindowHeight - cCancelBtnVOffset);
	mCancelBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mHeader, IDCANCEL);
	mCancelBtn.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mCancelBtn, CWndAlignment::eAlign_BottomRight));

	r = CRect(0, 0, cWindowWidth, cWindowHeight);
	MoveWindow(r, false);

	SetTarget(mTarget);

	// Recenter on screen after setting current details
	CenterWindow(CWnd::GetDesktopWindow());

	// Focus and select all in name field
	mName.SetSel(0, -1);
	mName.SetFocus();

	return false;
}

// Resize columns
void CTargetsDialog::OnSize(UINT nType, int cx, int cy)
{
	CHelpDialog::OnSize(nType, cx, cy);
}


#pragma mark ____________________________Commands

void CTargetsDialog::OnSetSchedule(UINT nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID - IDM_SCHEDULE_NEVER)
	{
	case CTargetItem::eNever:
	case CTargetItem::eAll:
	default:
		OnClearTargets();
		mMoreTargetsBtn.EnableWindow(false);
		mFewerTargetsBtn.EnableWindow(false);
		mClearTargetsBtn.EnableWindow(false);
		break;
	case CTargetItem::eIncomingMailbox:
	case CTargetItem::eOpenMailbox:
	case CTargetItem::eCloseMailbox:
		mTargets.EnableWindow(true);
		mMoreTargetsBtn.EnableWindow(true);
		mFewerTargetsBtn.EnableWindow(true);
		mClearTargetsBtn.EnableWindow(true);
		break;
	}
	
	mApplyPopup.SetValue(nID);
}

void CTargetsDialog::OnMoreTargets()
{
	AddTarget();
}

void CTargetsDialog::OnFewerTargets()
{
	RemoveTarget();
}

void CTargetsDialog::OnClearTargets()
{
	// Remove all but the first one
	while(mTargetItems.size() > 1)
		RemoveTarget();
}

BOOL CTargetsDialog::OnCmdMsg(UINT nID, int nCode, void* pExtra,
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

void CTargetsDialog::SetTarget(CTargetItem* target)
{
	if (target)
	{
		CUnicodeUtils::SetWindowTextUTF8(&mName, target->GetName());
		mEnabled.SetCheck(target->IsEnabled());

		mApplyPopup.SetValue(target->GetSchedule() + IDM_SCHEDULE_NEVER);
		OnSetSchedule(target->GetSchedule() + IDM_SCHEDULE_NEVER);
	}

	// Remove existing
	while(mTargetItems.size())
		RemoveTarget();

	InitTargets(target ? target->GetTargets() : NULL);
}

#pragma mark ____________________________Criteria

const long cMaxWindowHeight = 400L;

#pragma mark ____________________________Actions

void CTargetsDialog::InitTargets(const CFilterTargetList* targets)
{
	// Add each target
	if (targets)
	{
		for(CFilterTargetList::const_iterator iter = targets->begin(); iter != targets->end(); iter++)
			AddTarget(*iter);
	}
}

const int cTargetPanelHeight = 26;
const int cTargetPanelHOffset = 4;
const int cTargetPanelVInitOffset = 14;
const int cTargetPanelVOffset = 0;

void CTargetsDialog::AddTarget(const CFilterTarget* spec)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;
	const int cTargetPanelVInitOffset1 = cTargetPanelVInitOffset + small_offset;
	const int cTargetPanelHeight1 = cTargetPanelHeight + small_offset;

	// Create a new target panel
	CRect r;
	mTargets.GetWindowRect(r);
	r.OffsetRect(-r.left, -r.top);
	r.left += cTargetPanelHOffset;
	r.right -= cTargetPanelHOffset;
	r.bottom = cTargetPanelHeight1;
	CRulesTarget* target = new CRulesTarget;
	target->Create(r, this, &mTargets, NULL);
	if (spec)
		target->SetFilterTarget(spec);

	// Get last view in target bottom
	POINT new_pos = {cTargetPanelHOffset, cTargetPanelVInitOffset1};
	if (mTargetItems.size())
	{
		CRulesTarget* prev = static_cast<CRulesTarget*>(mTargetItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		target->SetBottom(true);

		// Now get position
		CRect prev_rect;
		prev->GetWindowRect(prev_rect);
		mTargets.ScreenToClient(prev_rect);
		new_pos.y = prev_rect.bottom + cTargetPanelVOffset;
	}
	else
	{
		target->SetTop(true);
		target->SetBottom(true);
	}

	// Add to list
	mTargetItems.push_back(target);

	// Now adjust sizes
	CRect size;
	target->GetWindowRect(size);
	ResizedTargets(size.Height() + cTargetPanelVOffset);
	
	// Position new sub-panel, show and focus it
	target->SetWindowPos(nil, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	target->ShowWindow(SW_SHOW);
	target->SetFocus();
	
	// Do button state
	mFewerTargetsBtn.ShowWindow(SW_SHOW);
}

void CTargetsDialog::RemoveTarget()
{
	// Get last view in criteria
	CWnd* target = mTargetItems.back();
	mTargetItems.pop_back();
	target->ShowWindow(SW_HIDE);

	// Now adjust sizes
	CRect size;
	target->GetWindowRect(size);
	ResizedTargets(-size.Height() - cTargetPanelVOffset);

	// Now delete the pane
	target->DestroyWindow();
	delete target;
	
	if (mTargetItems.size())
	{
		CRulesTarget* prev = static_cast<CRulesTarget*>(mTargetItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (mTargetItems.size() < 1)
		mFewerTargetsBtn.ShowWindow(SW_HIDE);
}

const int cScrollerStartHeight = 200;

void CTargetsDialog::ResizedTargets(int dy)
{
	// Now adjust sizes
	::MoveWindowBy(&mTargetsMove, 0, dy, true);
	::ResizeWindowBy(&mTargets, 0, dy, true);
	
	bool do_resize = (dy >= 0 ? (mScrollerImageHeight + dy < cScrollerStartHeight) :
								(mScrollerImageHeight < cScrollerStartHeight));
	mScrollerImageHeight += dy;

	// Check for resize of window

	if (do_resize)
	{
		::ResizeWindowBy(&mScroller, 0, dy, true);
		::ResizeWindowBy(&mHeader, 0, dy, true);
		::ResizeWindowBy(this, 0, dy);
		
		mScroller.ResizeImage(0, mScrollerImageHeight);
	}
	else
	{
		mScroller.ResizeImage(0, mScrollerImageHeight);
		mScroller.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
	}
}

#pragma mark ____________________________Build

void CTargetsDialog::OnOK()
{
	// Force update here as dialog is destroyed before exiting DoModal
	ConstructTarget(mTarget);
	
	// Do inherited
	CHelpDialog::OnOK();
}

void CTargetsDialog::ConstructTarget(CTargetItem* spec) const
{
	cdstring txt;
	mName.GetText(txt);
	spec->SetName(txt);
	spec->SetEnabled(mEnabled.GetCheck());

	spec->SetSchedule(static_cast<CTargetItem::ESchedule>(mApplyPopup.GetValue() - IDM_SCHEDULE_NEVER));

	spec->SetTargets(ConstructTargets());
}

CFilterTargetList* CTargetsDialog::ConstructTargets() const
{
	CFilterTargetList* targets = new CFilterTargetList;

	// Count items
	long num = mTargetItems.size();
	for(long i = 0; i < num; i++)
		targets->push_back(static_cast<const CRulesTarget*>(mTargetItems.at(i))->GetFilterTarget());

	return targets;
}

bool CTargetsDialog::PoseDialog(CTargetItem* spec)
{
	// Create the dialog
	CTargetsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mTarget = spec;
	
	// Let DialogHandler process events
	return (dlog.DoModal() == IDOK);
}