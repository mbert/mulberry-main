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


// CMessageHeaderView.cpp : implementation file
//


#include "CMessageHeaderView.h"

#include "CFontCache.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"

int cMsgHeaderWidth = 500;

int cCaptionHeight = 16;
int cEditHeight = 22;
int cRightMargin = 8;

int cTwisterLeft = 0;
int cTitleLeft = 15;
int cTitleWidth = 58;
int cTwisterTopOffset = 0;
int cTwistOffset = 32;
int cEditLeft = cTitleLeft + cTitleWidth;
int cEditTopOffset = -5;
int cEditWidth = cMsgHeaderWidth - cRightMargin - cEditLeft;

int cPartsTwisterLeft = 0;
int cPartsTitleLeft = 15;
int cPartsLeft = cEditLeft;
int cPartsWidth = 48;
int cAttachmentsLeft = cPartsLeft + cPartsWidth + 8;
int cHeaderBtnLeft = cAttachmentsLeft + 24;
int cFormatPopupLeft = cHeaderBtnLeft + 32;
int cFormatPopupWidth = 96;

int cFontIncreaseLeft = cFormatPopupLeft + cFormatPopupWidth + 24;
int cFontIncreaseWidth = 24;
int cFontDecreaseLeft = cFontIncreaseLeft + cFontIncreaseWidth;
int cFontDecreaseWidth = cFontIncreaseWidth;
int cFontScaleLeft = cFontDecreaseLeft + cFontDecreaseWidth + 8;
int cFontScaleWidth = 24;

int cQuoteDepthPopupLeft = cFontScaleLeft + cFontScaleWidth + 16;
int cQuoteDepthPopupWidth = 96;

int cMoverTop = 0;
int cMoverHeight = 24;
int cTitleTop = 5;

int cNumberWidth = 120;
int cDateTitleTop = cTitleTop + cMoverHeight;
int cDateWidth = cMsgHeaderWidth - cRightMargin - cNumberWidth - 16 - cEditLeft;
int cNumberLeft = cMsgHeaderWidth - cRightMargin - cNumberWidth;
int cNumberTop = cMoverTop + 3 * cMoverHeight + cDateTitleTop;

int cPartsHeaderHeight = 24;
int cPartsTableTop = cPartsHeaderHeight;
int cMinimumPartsHeight = 84;

/////////////////////////////////////////////////////////////////////////////
// CMessageHeaderView

IMPLEMENT_DYNCREATE(CMessageHeaderView, CContainerView)

CMessageHeaderView::CMessageHeaderView()
{
}

CMessageHeaderView::~CMessageHeaderView()
{
}

// Update positions of UI elements
void CMessageHeaderView::UpdateUIPos()
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int smallX_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;
	const int largeX_offset = CMulberryApp::sLargeFont ? 8 : 0;

	//cNumberOffset
	//cNumberWidth
	//cNumberTop
	cCaptionHeight += small_offset;
	cEditHeight += small_offset;

	//cTitleLeft
	cTitleWidth += 2*small_offset;
	cTwisterTopOffset += small_offset/2;
	cTwistOffset += small_offset;
	cEditLeft += 2*small_offset;
	//cEditTopOffset
	cEditWidth -= 2*small_offset;
	//cPartsWidth

	cFormatPopupWidth += 2*small_offset;
	cQuoteDepthPopupWidth += 2*small_offset;

	cMoverHeight += small_offset;

	cNumberTop += 4 * small_offset;

	cPartsHeaderHeight += small_offset;
}

BEGIN_MESSAGE_MAP(CMessageHeaderView, CContainerView)
	//{{AFX_MSG_MAP(CMessageHeaderView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageHeaderView message handlers

int CMessageHeaderView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	// Find window in super view chain
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CMessageWindow*>(parent))
		parent = parent->GetParent();
	CMessageWindow* msgWnd = dynamic_cast<CMessageWindow*>(parent);

	// Gray background needed when all movers have gone
	mBackground.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, cMsgHeaderWidth, cMoverTop + 5 * cMoverHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.MinimumResize(24, 20);
	AddAlignment(new CWndAlignment(&mBackground, CWndAlignment::eAlign_WidthHeight));

	// From mover
	mFromMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cMoverTop, cMsgHeaderWidth, cMoverHeight, mBackground.GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mFromMove, CWndAlignment::eAlign_TopWidth));

	// From caption
	CString s;
	s.LoadString(IDS_MESSAGEFROMTITLE);
	CRect r = CRect(cTitleLeft, cTitleTop, cTitleLeft + cTitleWidth, cTitleTop + cCaptionHeight);
	mFromTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mFromMove, IDC_STATIC);
	mFromTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cTwisterLeft, cTitleTop + cTwisterTopOffset, cTwisterLeft + 16, cTitleTop + cTwisterTopOffset + 16);
	mFromTwister.Create(r, &mFromMove, IDC_MESSAGEFROMTWISTER);
	r = CRect(cEditLeft, cTitleTop + cEditTopOffset, cEditLeft + cEditWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mFromField.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_VSCROLL | ES_MULTILINE, r, &mFromMove, IDC_MESSAGEFROMFIELD);
	mFromField.SetFont(CMulberryApp::sAppFont);
	mFromField.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_ADDR);
	mFromField.SetContextView(this);
	mFromField.SetTabOrder(&mToField, msgWnd->GetText(), this, msgWnd->GetText());
	mFromField.SetReadOnly(true);
	mFromField.SetMessageWindow(msgWnd);
	mFromField.SetSuperCommander(msgWnd);
	mFromMove.AddAlignment(new CWndAlignment(&mFromField, CWndAlignment::eAlign_WidthHeight));

	// To mover
	mToMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cMoverTop + cMoverHeight, cMsgHeaderWidth, cMoverHeight, mBackground.GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mToMove, CWndAlignment::eAlign_TopWidth));

	// To caption
	s.LoadString(IDS_MESSAGETOTITLE);
	r = CRect(cTitleLeft, cTitleTop, cTitleLeft + cTitleWidth, cTitleTop + cCaptionHeight);
	mToTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mToMove, IDC_STATIC);
	mToTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cTwisterLeft, cTitleTop + cTwisterTopOffset, cTwisterLeft + 16, cTitleTop + cTwisterTopOffset + 16);
	mToTwister.Create(r, &mToMove, IDC_MESSAGETOTWISTER);
	r = CRect(cEditLeft, cTitleTop + cEditTopOffset, cEditLeft + cEditWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mToField.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_VSCROLL | ES_MULTILINE, r, &mToMove, IDC_MESSAGETOFIELD);
	mToField.SetFont(CMulberryApp::sAppFont);
	mToField.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_ADDR);
	mToField.SetContextView(this);
	mToField.SetTabOrder(&mCCField, &mFromField, this, this);
	mToField.SetReadOnly(true);
	mToField.SetMessageWindow(msgWnd);
	mToField.SetSuperCommander(msgWnd);
	mToMove.AddAlignment(new CWndAlignment(&mToField, CWndAlignment::eAlign_WidthHeight));

	// CC mover
	mCCMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cMoverTop + 2 * cMoverHeight, cMsgHeaderWidth, cMoverHeight, mBackground.GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mCCMove, CWndAlignment::eAlign_TopWidth));

	// CC caption
	s.LoadString(IDS_MESSAGECCTITLE);
	r = CRect(cTitleLeft, cTitleTop, cTitleLeft + cTitleWidth, cTitleTop + cCaptionHeight);
	mCCTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mCCMove, IDC_STATIC);
	mCCTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cTwisterLeft, cTitleTop + cTwisterTopOffset, cTwisterLeft + 16, cTitleTop + cTwisterTopOffset + 16);
	mCCTwister.Create(r, &mCCMove, IDC_MESSAGECCTWISTER);
	r = CRect(cEditLeft, cTitleTop + cEditTopOffset, cEditLeft + cEditWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mCCField.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_VSCROLL | ES_MULTILINE, r, &mCCMove, IDC_MESSAGECCFIELD);
	mCCField.SetFont(CMulberryApp::sAppFont);
	mCCField.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_ADDR);
	mCCField.SetContextView(this);
	mCCField.SetTabOrder(&mSubjectField, &mToField, this, this);
	mCCField.SetReadOnly(true);
	mCCField.SetMessageWindow(msgWnd);
	mCCField.SetSuperCommander(msgWnd);
	mCCMove.AddAlignment(new CWndAlignment(&mCCField, CWndAlignment::eAlign_WidthHeight));

	// Subject mover
	mSubjectMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cMoverTop + 3 * cMoverHeight, cMsgHeaderWidth, 2 * cMoverHeight, mBackground.GetSafeHwnd(), (HMENU)IDC_STATIC);
	mBackground.AddAlignment(new CWndAlignment(&mSubjectMove, CWndAlignment::eAlign_TopWidth));

	// Subject caption
	s.LoadString(IDS_MESSAGESUBJECTTITLE);
	r = CRect(cTitleLeft, cTitleTop, cTitleLeft + cTitleWidth, cTitleTop + cCaptionHeight);
	mSubjectTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mSubjectMove, IDC_STATIC);
	mSubjectTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cEditLeft, cTitleTop + cEditTopOffset, cEditLeft + cEditWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mSubjectField.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL, r, &mSubjectMove, IDC_MESSAGESUBJECT);
	mSubjectField.SetFont(CMulberryApp::sAppFont);
	mSubjectField.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_SUBJ);
	mSubjectField.SetContextView(this);
	mSubjectField.SetTabOrder(&mDateField, &mCCField, this, this);
	mSubjectField.SetReadOnly(true);
	mSubjectField.SetMessageWindow(msgWnd);
	mSubjectField.SetSuperCommander(msgWnd);
	mSubjectMove.AddAlignment(new CWndAlignment(&mSubjectField, CWndAlignment::eAlign_WidthHeight));

	// Date caption
	s.LoadString(IDS_MESSAGEDATETITLE);
	r = CRect(cTitleLeft, cDateTitleTop, cTitleLeft + cTitleWidth, cDateTitleTop + cCaptionHeight);
	mDateTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mSubjectMove, IDC_STATIC);
	mDateTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cEditLeft, cDateTitleTop + cEditTopOffset, cEditLeft + cDateWidth, cDateTitleTop + cEditTopOffset + cEditHeight);
	mDateField.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE, r, &mSubjectMove, IDC_MESSAGEDATE);
	mDateField.SetFont(CMulberryApp::sAppFont);
	mDateField.SetContextMenuID(IDR_POPUP_CONTEXT_MESSAGE_SUBJ);
	mDateField.SetContextView(this);
	mDateField.SetReadOnly(true);
	mDateField.SetMessageWindow((CMessageWindow*) GetParent());
	mDateField.SetSuperCommander(msgWnd);
	mSubjectMove.AddAlignment(new CWndAlignment(&mDateField, CWndAlignment::eAlign_WidthHeight));

	// Number caption
	mNumberField.Create(_T("Message 10000 of 10000"), WS_CHILD | WS_VISIBLE | SS_RIGHT, CRect(cNumberLeft, cNumberTop, cNumberLeft + cNumberWidth, cNumberTop + cCaptionHeight), &mBackground, IDC_MESSAGENUMBER);
	mNumberField.SetFont(CMulberryApp::sAppFont);
	mBackground.AddAlignment(new CWndAlignment(&mNumberField, CWndAlignment::eAlign_BottomRight));

	// Parts list
	mPartsMove.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cMoverTop + 5 * cMoverHeight, cMsgHeaderWidth, cMoverHeight, GetSafeHwnd(), (HMENU)IDC_STATIC);
	AddAlignment(new CWndAlignment(&mPartsMove, CWndAlignment::eAlign_TopWidth));

	r = CRect(cPartsTwisterLeft, cTitleTop + cTwisterTopOffset, cPartsTwisterLeft + 16, cTitleTop + cTwisterTopOffset + 16);
	mPartsTwister.Create(r, &mPartsMove, IDC_MESSAGEPARTSTWISTER);
	s.LoadString(IDS_MESSAGEPARTSTITLE);
	r = CRect(cPartsTitleLeft, cTitleTop, cPartsTitleLeft + cTitleWidth, cTitleTop + cCaptionHeight);
	mPartsTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_STATIC);
	mPartsTitle.SetFont(CMulberryApp::sAppFontBold);
	r = CRect(cPartsLeft, cTitleTop, cPartsLeft + cPartsWidth, cTitleTop + cCaptionHeight);
	s = "";
	mPartsField.Create(s, WS_CHILD | WS_VISIBLE, r, &mPartsMove, IDC_MESSAGEPARTSFIELD);
	mPartsField.SetFont(CMulberryApp::sAppFont);

	mFlatHierarchyBtn.Create(_T(""), CRect(cPartsLeft, cTitleTop, cPartsLeft + 16, cTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSFLATBTN, IDC_STATIC, IDI_SERVERFLAT, IDI_SERVERHIERARCHIC);
	mFlatHierarchyBtn.ShowWindow(SW_HIDE);

	mAttachments.Create(NULL, WS_CHILD, CRect(cAttachmentsLeft, cTitleTop, cAttachmentsLeft + 16, cTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSATTACHMENTS);
	mAttachments.SetIconID(IDI_MSG_ATTACHMENT);
	mAlternative.Create(NULL, WS_CHILD, CRect(cAttachmentsLeft, cTitleTop, cAttachmentsLeft + 16, cTitleTop + 16), &mPartsMove, IDC_MESSAGEPARTSALTERNATIVE);
	mAlternative.SetIconID(IDI_MSG_ALTERNATIVE);

	// Extra parts list bits
	mHeaderBtn.Create(_T(""), CRect(cHeaderBtnLeft, cTitleTop + cEditTopOffset, cHeaderBtnLeft + 24, cTitleTop + cEditTopOffset + 22), &mPartsMove, IDM_MESSAGES_SHOW_HEADER, IDC_STATIC, IDI_HEADERMSG);

	r = CRect(cFormatPopupLeft, cTitleTop + cEditTopOffset, cFormatPopupLeft + cFormatPopupWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mTextFormat.Create(_T(""), r, &mPartsMove, IDM_MESSAGES_TEXT_FORMAT, IDC_STATIC, IDI_POPUPBTN);
	mTextFormat.SetMenu(IDR_POPUP_TEXTFORMAT);
	mTextFormat.SetValue(IDM_TEXTFORMAT_PLAIN);
	mTextFormat.SetFont(CMulberryApp::sAppFont);

	mFontIncreaseBtn.Create(_T(""), CRect(cFontIncreaseLeft, cTitleTop + cEditTopOffset, cFontIncreaseLeft + cFontIncreaseWidth, cTitleTop + cEditTopOffset + 22), &mPartsMove, IDC_MESSAGES_FONTINCREASE, IDC_STATIC, IDI_FONTINCREASE);
	mFontDecreaseBtn.Create(_T(""), CRect(cFontDecreaseLeft, cTitleTop + cEditTopOffset, cFontDecreaseLeft + cFontDecreaseWidth, cTitleTop + cEditTopOffset + 22), &mPartsMove, IDC_MESSAGES_FONTDECREASE, IDC_STATIC, IDI_FONTDECREASE);
	mFontScaleField.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(cFontScaleLeft, cTitleTop, cFontScaleLeft + cFontScaleWidth, cTitleTop + cCaptionHeight), &mPartsMove, IDC_MESSAGES_FONTSCALE);
	mFontScaleField.SetFont(CMulberryApp::sAppFont);

	r = CRect(cQuoteDepthPopupLeft, cTitleTop + cEditTopOffset, cQuoteDepthPopupLeft + cQuoteDepthPopupWidth, cTitleTop + cEditTopOffset + cEditHeight);
	mQuoteDepthPopup.Create(_T(""), r, &mPartsMove, IDM_QUOTEDEPTH_ALL, IDC_STATIC, IDI_POPUPBTN);
	mQuoteDepthPopup.SetMenu(IDR_POPUP_QUOTEDEPTH);
	mQuoteDepthPopup.SetValue(IDM_QUOTEDEPTH_ALL);
	mQuoteDepthPopup.SetFont(CMulberryApp::sAppFont);

	return 0;
}

// Resize sub-views (resize width - keep height the same)
void CMessageHeaderView::OnSize(UINT nType, int cx, int cy)
{
	// By-pass CContainerView behvaiour
	CView::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

BOOL CMessageHeaderView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CContainerView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	else
	{
		// Find window in super view chain
		CWnd* parent = GetParent();
		while(parent && !dynamic_cast<CMessageWindow*>(parent))
			parent = parent->GetParent();
		CMessageWindow* msgWnd = dynamic_cast<CMessageWindow*>(parent);
		if (!msgWnd)
			return false;
		
		CWnd* parts = &msgWnd->mPartsTable;

		// Try focused child window
		CWnd* focus = GetFocus();
		if (focus && (focus == parts) && focus->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return true;
	}
	
	return false;
}

