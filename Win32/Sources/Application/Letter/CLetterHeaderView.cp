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


// CLetterHeaderView.cpp : implementation file
//


#include "CLetterHeaderView.h"

#include "CAdminLock.h"
#include "CFontCache.h"
#include "CLetterWindow.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CPreferences.h"

int cLtrHeaderHeight = 122;
int cLtrHeaderWidth = 500;

int cLtrCaptionHeight = 16;
int cLtrEditHeight = 22;
int cLtrRightMargin = 8;

const int cHideHOffset = 2;
const int cHideVOffset = 4;
const int cShowHOffset = 0;
const int cShowVOffset = 2;
const int cShowHideSmall = 7;
const int cShowHideLarge = 32;

int cLtrHeaderTwisterLeft = 0;

int cLtrTwisterLeft = 0;
int cLtrTitleLeft = 15;
int cLtrTitleWidth = 58;
int cLtrTwisterTopOffset = 0;
int cLtrTwistOffset = 32;
int cLtrEditLeft = cLtrTitleLeft + cLtrTitleWidth;
int cLtrEditTopOffset = -5;
int cLtrEditWidth = cLtrHeaderWidth - 32 - cLtrEditLeft;

int cLtrSentIconWidth = 16;
int cLtrSentIconHeight = 16;
int cLtrSentIconLeft = cLtrHeaderWidth - 16 - cLtrRightMargin;

int cLtrPartsTwisterLeft = 0;
int cLtrPartsWidth = 48;
int cLtrAttachmentsLeft = cLtrEditLeft + cLtrPartsWidth + 4;

int cLtrCopyToTitleLeft = cLtrAttachmentsLeft + 20;
int cLtrCopyToTitleWidth = cLtrTitleWidth + 16;
int cLtrCopyToLeft = cLtrCopyToTitleLeft + cLtrCopyToTitleWidth;
int cLtrCopyToWidth = cLtrEditLeft + cLtrEditWidth - cLtrCopyToLeft;

int cLtrIdentityPopupLeft = cLtrEditLeft;
int cLtrIdentityEditLeft = cLtrIdentityPopupLeft + cLtrEditWidth + 8;
int cLtrIdentityEditWidth = 24;

int cLtrMoverTop = 0;
int cLtrMoverHeight = 24;
int cLtrTitleTop = 5;

int cLtrSmallBtnWidth = 24;
int cLtrSmallBtnHeight = 22;

int cLtrPartsHeaderHeight = 24;
int cLtrPartsTableTop = cLtrPartsHeaderHeight;
int cLtrMinimumPartsHeight = 84;

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView

IMPLEMENT_DYNCREATE(CLetterHeaderView, CContainerView)

CLetterHeaderView::CLetterHeaderView()
{
}

CLetterHeaderView::~CLetterHeaderView()
{
}

// Update positions of UI elements
void CLetterHeaderView::UpdateUIPos()
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;

	cLtrHeaderHeight += large_offset;

	cLtrCaptionHeight += small_offset;
	cLtrEditHeight += small_offset;

	cLtrTitleWidth += 2*small_offset;
	cLtrTwisterTopOffset += small_offset/2;
	cLtrTwistOffset += small_offset;
	cLtrEditLeft += 2*small_offset;
	cLtrEditWidth -= 4*small_offset;
	cLtrIdentityPopupLeft += 2*small_offset;

	cLtrMoverHeight += small_offset;

	cLtrPartsHeaderHeight += small_offset;
}

BEGIN_MESSAGE_MAP(CLetterHeaderView, CContainerView)
	//{{AFX_MSG_MAP(CLetterHeaderView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView message handlers

int CLetterHeaderView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CContainerView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	// Find window in super view chain
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CLetterWindow*>(parent))
		parent = parent->GetParent();
	CLetterWindow* ltrWnd = dynamic_cast<CLetterWindow*>(parent);

	// Gray background needed when all movers have gone
	mBackground.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cLtrHeaderWidth, cLtrMoverTop + 5 * cLtrMoverHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.MinimumResize(24, 20);
	AddAlignment(new CWndAlignment(&mBackground, CWndAlignment::eAlign_WidthHeight));

	// Header mover
	mHeaderMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop, cLtrHeaderWidth, cLtrMoverHeight, mBackground, (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mHeaderMove, CWndAlignment::eAlign_TopWidth));

	// From mover
	mFromMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop, cLtrHeaderWidth, cLtrMoverHeight, mHeaderMove, (HMENU)IDC_STATIC);
	mHeaderMove.AddAlignment(new CWndAlignment(&mFromMove, CWndAlignment::eAlign_WidthHeight));

	// Identity bits
	CString s;
	s.LoadString(IDS_LETTERFROMTITLE);
	CRect r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mFromTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mFromMove, IDC_STATIC);
	mFromTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrIdentityPopupLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrIdentityPopupLeft + cLtrEditWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mIdentityPopup.Create(_T(""), r, &mFromMove, IDC_LETTER_IDENTITYPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue(), true);
	mIdentityPopup.SetFont(CMulberryApp::sAppFont);
	mIdentityPopup.SetValue(IDM_IDENTITYStart);
	mFromMove.AddAlignment(new CWndAlignment(&mIdentityPopup, CWndAlignment::eAlign_TopWidth));

	r = CRect(cLtrIdentityEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrIdentityEditLeft + cLtrIdentityEditWidth, cLtrTitleTop + cLtrEditTopOffset + 22);
	mIdentityEditBtn.Create(_T(""), r, &mFromMove, IDC_LETTER_IDENTITYEDIT, IDC_STATIC, IDI_IDENTITYEDIT);
	mIdentityEditBtn.ShowWindow(false);
	mFromMove.AddAlignment(new CWndAlignment(&mIdentityEditBtn, CWndAlignment::eAlign_TopRight));

	// Recipient mover
	mRecipientMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop, cLtrHeaderWidth, cLtrMoverHeight, mHeaderMove, (HMENU)IDC_STATIC);
	mHeaderMove.AddAlignment(new CWndAlignment(&mRecipientMove, CWndAlignment::eAlign_WidthHeight));

	s.LoadString(IDS_LETTERRECIPIENTTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth + 16, cLtrTitleTop + cLtrCaptionHeight);
	mRecipientTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mRecipientMove, IDC_STATIC);
	mRecipientTitle.SetFont(CMulberryApp::sAppFontBold);

	r = CRect(cLtrEditLeft + 16, cLtrTitleTop, cLtrEditLeft + cLtrEditWidth - 16, cLtrTitleTop + cLtrCaptionHeight);
	mRecipientText.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mRecipientMove, IDC_STATIC);
	mRecipientText.SetFont(CMulberryApp::sAppFont);
	mRecipientMove.AddAlignment(new CWndAlignment(&mRecipientText, CWndAlignment::eAlign_TopWidth));

	// To mover
	mToMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop + cLtrMoverHeight, cLtrHeaderWidth, cLtrMoverHeight, mBackground, (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mToMove, CWndAlignment::eAlign_TopWidth));

	// To caption
	s.LoadString(IDS_LETTERTOTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mToTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mToMove, IDC_STATIC);
	mToTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrTwisterLeft, cLtrTitleTop + cLtrTwisterTopOffset, cLtrTwisterLeft + 16, cLtrTitleTop + cLtrTwisterTopOffset + 16);
	mToTwister.Create(r, &mToMove, IDC_LETTERTOTWISTER);
	r = CRect(cLtrEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrEditLeft + cLtrEditWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mToField.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, r, &mToMove, IDC_LETTERTOFIELD);
	mToField.SetFont(CMulberryApp::sAppFont);
	mToField.SetTabOrder(&mCCField, ltrWnd->mText, this, ltrWnd->mText);
	mToField.SetContextMenuID(IDR_POPUP_CONTEXT_LETTER_ADDRESS);
	mToField.SetContextView(this);
	mToField.SetSuperCommander(ltrWnd);
	mToMove.AddAlignment(new CWndAlignment(&mToField, CWndAlignment::eAlign_WidthHeight));

	// CC mover
	mCCMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop + 2 * cLtrMoverHeight, cLtrHeaderWidth, cLtrMoverHeight, mBackground, (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mCCMove, CWndAlignment::eAlign_TopWidth));

	// CC caption
	s.LoadString(IDS_LETTERCCTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mCCTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mCCMove, IDC_STATIC);
	mCCTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrTwisterLeft, cLtrTitleTop + cLtrTwisterTopOffset, cLtrTwisterLeft + 16, cLtrTitleTop + cLtrTwisterTopOffset + 16);
	mCCTwister.Create(r, &mCCMove, IDC_LETTERCCTWISTER);
	r = CRect(cLtrEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrEditLeft + cLtrEditWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mCCField.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, r, &mCCMove, IDC_LETTERCCFIELD);
	mCCField.SetFont(CMulberryApp::sAppFont);
	mCCField.SetTabOrder(&mBCCField, &mToField, this, this);
	mCCField.SetContextMenuID(IDR_POPUP_CONTEXT_LETTER_ADDRESS);
	mCCField.SetContextView(this);
	mCCField.SetSuperCommander(ltrWnd);
	mCCMove.AddAlignment(new CWndAlignment(&mCCField, CWndAlignment::eAlign_WidthHeight));

	// BCC mover
	mBCCMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop + 3 * cLtrMoverHeight, cLtrHeaderWidth, cLtrMoverHeight, mBackground, (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mBCCMove, CWndAlignment::eAlign_TopWidth));

	// BCC caption
	s.LoadString(IDS_LETTERBCCTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mBCCTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mBCCMove, IDC_STATIC);
	mBCCTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrTwisterLeft, cLtrTitleTop + cLtrTwisterTopOffset, cLtrTwisterLeft + 16, cLtrTitleTop + cLtrTwisterTopOffset + 16);
	mBCCTwister.Create(r, &mBCCMove, IDC_LETTERBCCTWISTER);
	r = CRect(cLtrEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrEditLeft + cLtrEditWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mBCCField.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, r, &mBCCMove, IDC_LETTERBCCFIELD);
	mBCCField.SetFont(CMulberryApp::sAppFont);
	mBCCField.SetTabOrder(&mSubjectField, &mCCField, this, this);
	mBCCField.SetContextMenuID(IDR_POPUP_CONTEXT_LETTER_ADDRESS);
	mBCCField.SetContextView(this);
	mBCCField.SetSuperCommander(ltrWnd);
	mBCCMove.AddAlignment(new CWndAlignment(&mBCCField, CWndAlignment::eAlign_WidthHeight));

	// Subject mover
	mSubjectMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop + 4 * cLtrMoverHeight, cLtrHeaderWidth, cLtrMoverHeight, mBackground, (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mSubjectMove, CWndAlignment::eAlign_TopWidth));

	// Subject caption
	s.LoadString(IDS_LETTERSUBJECTTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth + 4, cLtrTitleTop + cLtrCaptionHeight);
	mSubjectTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mSubjectMove, IDC_STATIC);
	mSubjectTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrEditLeft + cLtrEditWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mSubjectField.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_AUTOHSCROLL, r, &mSubjectMove, IDC_LETTERSUBJECTFIELD);
	mSubjectField.SetFont(CMulberryApp::sAppFont);
	mSubjectField.SetContextView(this);
	mSubjectField.SetAnyCmd(true);
	mSubjectField.SetSuperCommander(ltrWnd);
	mSubjectMove.AddAlignment(new CWndAlignment(&mSubjectField, CWndAlignment::eAlign_TopWidth));

	// Indicator
	mSentIcon.Create(NULL, WS_CHILD, CRect(cLtrSentIconLeft, cLtrTitleTop, cLtrSentIconLeft + cLtrSentIconWidth, cLtrTitleTop + cLtrSentIconHeight), &mSubjectMove, IDC_LETTERSENTICON);
	mSentIcon.SetIconID(IDI_FLAG_UNSEEN);
	mSubjectMove.AddAlignment(new CWndAlignment(&mSentIcon, CWndAlignment::eAlign_TopRight));

	// Parts list
	mPartsMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cLtrMoverTop + 5 * cLtrMoverHeight, cLtrHeaderWidth, cLtrMoverHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);
	AddAlignment(new CWndAlignment(&mPartsMove, CWndAlignment::eAlign_TopWidth));

	r = CRect(cLtrPartsTwisterLeft, cLtrTitleTop + cLtrTwisterTopOffset, cLtrPartsTwisterLeft + 16, cLtrTitleTop + cLtrTwisterTopOffset + 16);
	mPartsTwister.Create(r, &mPartsMove, IDC_LETTERPARTSTWISTER);

	s.LoadString(IDS_LETTERPARTSTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mPartsTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_STATIC);
	mPartsTitle.SetFont(CMulberryApp::sAppFontBold);

	s.LoadString(IDS_LETTERATTACHEDTITLE);
	r = CRect(cLtrTitleLeft, cLtrTitleTop, cLtrTitleLeft + cLtrTitleWidth + 8, cLtrTitleTop + cLtrCaptionHeight);
	mAttachmentsTitle.Create(s, WS_CHILD, r, &mPartsMove, IDC_STATIC);
	mAttachmentsTitle.SetFont(CMulberryApp::sAppFontBold);

	r = CRect(cLtrEditLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrEditLeft + cLtrPartsWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mPartsField.Create(r, &mPartsMove, IDC_LETTERPARTSFIELD, true);
	mPartsField.SetFont(CMulberryApp::sAppFont);
	mAttachments.Create(NULL, WS_CHILD, CRect(cLtrAttachmentsLeft, cLtrTitleTop, cLtrAttachmentsLeft + 16, cLtrTitleTop + 16), &mPartsMove, IDC_LETTERPARTSICON);
	mAttachments.SetIconID(IDI_MSG_ATTACHMENT);

	// Copy To
	s.LoadString(IDS_LETTERCOPYTOTITLE);
	r = CRect(cLtrCopyToTitleLeft, cLtrTitleTop, cLtrCopyToTitleLeft + cLtrCopyToTitleWidth, cLtrTitleTop + cLtrCaptionHeight);
	mCopyToTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_STATIC);
	mCopyToTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cLtrCopyToLeft, cLtrTitleTop + cLtrEditTopOffset, cLtrCopyToLeft + cLtrCopyToWidth, cLtrTitleTop + cLtrEditTopOffset + cLtrEditHeight);
	mCopyTo.Create(_T(""), r, &mPartsMove, IDC_LETTERCOPYTOPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mCopyTo.SetFont(CMulberryApp::sAppFont);
	mCopyTo.SetCopyTo(false);
	mCopyTo.SetDefault();
	mPartsMove.AddAlignment(new CWndAlignment(&mCopyTo, CWndAlignment::eAlign_TopWidth));

	return 0;
}

void CLetterHeaderView::OnSize(UINT nType, int cx, int cy)
{
	// By-pass CContainerView behvaiour
	CView::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}


BOOL CLetterHeaderView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CContainerView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	else
	{
		// Find window in super view chain
		CWnd* parent = GetParent();
		while(parent && !dynamic_cast<CLetterWindow*>(parent))
			parent = parent->GetParent();
		CLetterWindow* ltrWnd = dynamic_cast<CLetterWindow*>(parent);
		if (!ltrWnd)
			return false;
		
		CWnd* parts = ltrWnd->GetPartsTable();

		// Try focused child window
		CWnd* focus = GetFocus();
		if (focus && (focus == parts) && focus->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;
	}
	
	return false;
}

