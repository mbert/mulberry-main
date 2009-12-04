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


// CCmdEdit.cpp : implementation file
//

#include "CCmdEdit.h"

#include "CMulberryCommon.h"
#include "CStringUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCmdEdit

IMPLEMENT_DYNCREATE(CCmdEdit, CRichEditCtrl)

CCmdEdit::CCmdEdit()
{
	mContextMenuID = IDR_POPUP_CONTEXT_EDIT;
	mContextView = nil;

	mSpacesPerTab = 0;
	
	mShowStyled = false;
	mReturnCmd = false;
	mAnyCmd = false;
}

CCmdEdit::~CCmdEdit()
{
}


BEGIN_MESSAGE_MAP(CCmdEdit, CRichEditCtrl)
	//{{AFX_MSG_MAP(CCmdEdit)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditReadWrite)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditReadWrite)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPasteReadWrite)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateAlways)

	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_CLEAR, OnClear)
	ON_COMMAND(ID_EDIT_CUT, OnCut)
	ON_COMMAND(ID_EDIT_COPY, Copy)
	ON_COMMAND(ID_EDIT_PASTE, OnPaste)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	
	ON_WM_MOUSEACTIVATE()

	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCmdEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!AfxInitRichEdit())
		return FALSE;

	CString strClass = RICHEDIT_CLASSW;

	CWnd* pWnd = this;
	return pWnd->Create(strClass, NULL, dwStyle, rect, pParentWnd, nID);
}

// Display and track context menu
void CCmdEdit::OnContextMenu(CWnd* wnd, CPoint point)
{
	HandleContextMenu(wnd, point);
}

// Clicked somewhere
void CCmdEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (mContextView)
	{
		mContextView->GetParentFrame()->BringWindowToTop();

		// Force its view to be active and focus
		mContextView->GetParentFrame()->SetActiveView(mContextView);
		SetFocus();

		// Do context menu
		ClientToScreen(&point);
		OnContextMenu(NULL, point);
	}
}

// Display and track context menu
void CCmdEdit::HandleContextMenu(CWnd*, CPoint point)
{
	CMenu menu;
	VERIFY(menu.LoadMenu(mContextMenuID));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
	pWndPopupOwner = pWndPopupOwner->GetParent();
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);
}

// Handle keyboard equiv for context menu
BOOL CCmdEdit::PreTranslateMessage(MSG* pMsg)
{
	// Shift+F10: show pop-up menu.
	if (((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
		(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||
		(pMsg->message == WM_CONTEXTMENU)) &&
		mContextView)	// Natural keyboard key	
	{
		// Calculate hit point
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		CPoint point = rect.TopLeft();
		point.Offset(5, 5);
			
		// Force its view to be active and focus
		mContextView->GetParentFrame()->SetActiveView(mContextView);
		SetFocus();

		// Do context menu
		OnContextMenu(NULL, point);
		return TRUE;
	}

	return CRichEditCtrl::PreTranslateMessage(pMsg);
}

// Change font in whole control
void CCmdEdit::ResetFont(CFont* pFont)
{
	// Temporarily stop screen drawing
	StNoRedraw _noredraw(this);

	// Set new font
	SetFont(pFont);
	
	// Store old selection and select everything
	StPreserveSelection selection(this);
	SetSel(0, -1);
	
	// Set selection format to dedault format
	CHARFORMAT cf;
	GetDefaultCharFormat(cf);
	cf.dwMask = CFM_FACE | CFM_SIZE;
	SetSelectionCharFormat(cf);

	// Set selection format to default format
	PARAFORMAT pf;
	pf.dwMask = PFM_ALIGNMENT;
	pf.wAlignment = PFA_LEFT;
	SetParaFormat(pf);
	
	LRESULT langopts = ::SendMessage(*this, EM_GETLANGOPTIONS, 0, 0);
	langopts |= IMF_DUALFONT;
	::SendMessage(*this, EM_SETLANGOPTIONS, 0, langopts);
}

long CCmdEdit::GetTextLengthEx() const
{
	GETTEXTLENGTHEX gex;
	gex.flags = GTL_DEFAULT;
	gex.codepage = 1200;
	return ::SendMessageW(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM) &gex, 0);
}

void CCmdEdit::GetSelectedText(cdstring& selected) const
{
	// get as UTF16 first
	cdustring utf16;
	GetSelectedText(utf16);
	selected = utf16.ToUTF8();
}

void CCmdEdit::GetSelectedText(cdustring& selected) const
{
	// Get current selection length (NB this is the raw length with CR - not CRLF - line end encoding)
	long sel_start;
	long sel_end;
	GetSel(sel_start, sel_end);
	UINT length = sel_end - sel_start;

	// Get all the text using the CR line end character
	if (length > 0)
	{
		selected.reserve(length + 1);

		GETTEXTEX getex;
		getex.cb = (length + 1) * 2;
		getex.flags = GT_SELECTION;
		getex.codepage = 1200;
		getex.lpDefaultChar = NULL;
		getex.lpUsedDefChar = NULL;
		::SendMessageW(m_hWnd, EM_GETTEXTEX, (WPARAM) &getex, (LPARAM)selected.c_str());
	}

	selected.ConvertEndl();
}

// UTF8 in
void CCmdEdit::SetText(const cdstring& all)
{
	cdustring utf16(all);
	SetText(utf16);
}

// UTF16 in
void CCmdEdit::SetText(const cdustring& all)
{
	SETTEXTEX setex;
	setex.flags = ST_DEFAULT;
	setex.codepage = 1200;
	::SendMessageW(m_hWnd, EM_SETTEXTEX, (WPARAM) &setex, (LPARAM)all.c_str());
}

// Insert unstyled utf8 at selection
void CCmdEdit::InsertUTF8(const cdstring& txt)
{
	cdustring utf16(txt);
	InsertText(utf16);
}

// Insert unstyled utf8 at selection
void CCmdEdit::InsertUTF8(const char* txt, size_t len)
{
	cdstring utf8(txt, len);
	InsertUTF8(utf8);
}

// Insert unstyled utf16 at selection
void CCmdEdit::InsertText(const cdustring& utxt)
{
	SETTEXTEX setex;
	setex.flags = ST_SELECTION;
	setex.codepage = 1200;
	::SendMessageW(m_hWnd, EM_SETTEXTEX, (WPARAM) &setex, (LPARAM)utxt.c_str());
}

// Insert unstyled utf16 at selection
void CCmdEdit::InsertText(const unichar_t* utxt, size_t len)
{
	cdustring utf16(utxt, len);
	InsertText(utf16);
}

// UTF8 out
void CCmdEdit::GetText(cdstring& all) const
{
	cdustring utf16;
	GetText(utf16);
	all = utf16.ToUTF8();
}

cdstring CCmdEdit::GetText() const
{
	cdstring result;
	GetText(result);
	return result;
}

// UTF16 out
void CCmdEdit::GetText(cdustring& all) const
{
	// Get all the text using the CRLF line end character
	GETTEXTLENGTHEX gex;
	gex.flags = GTL_USECRLF | GTL_NUMCHARS;
	gex.codepage = 1200;
	UINT length = ::SendMessageW(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM) &gex, 0);
	if (length > 0)
	{
		all.reserve(length + 1);

		GETTEXTEX getex;
		getex.cb = (length + 1) * 2;
		getex.flags = GT_USECRLF;
		getex.codepage = 1200;
		getex.lpDefaultChar = NULL;
		getex.lpUsedDefChar = NULL;
		::SendMessageW(m_hWnd, EM_GETTEXTEX, (WPARAM) &getex, (LPARAM)all.c_str());
	}
	else
		all.clear();
}

// UTF8 out
void CCmdEdit::GetRawText(cdstring& all) const
{
	cdustring utf16;
	GetRawText(utf16);
	all = utf16.ToUTF8();
}

// UTF16 out
void CCmdEdit::GetRawText(cdustring& all) const
{
	// Get all the text using the native/internal line end character
	// RichEdit2.0 uses CR internally not CRLF
	GETTEXTLENGTHEX gex;
	gex.flags = GTL_NUMCHARS;
	gex.codepage = 1200;
	UINT length = ::SendMessageW(m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM) &gex, 0);
	if (length > 0)
	{
		all.reserve(length + 1);

		GETTEXTEX getex;
		getex.cb = (length + 1) * 2;
		getex.flags = GT_DEFAULT;
		getex.codepage = 1200;
		getex.lpDefaultChar = NULL;
		getex.lpUsedDefChar = NULL;
		::SendMessageW(m_hWnd, EM_GETTEXTEX, (WPARAM) &getex, (LPARAM)all.c_str());
	}
	else
		all.clear();
}

/////////////////////////////////////////////////////////////////////////////
// CCmdEdit message handlers

void CCmdEdit::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CCmdEdit::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
	long nStartChar = 0;
	long nEndChar = 0;
	GetSel(nStartChar, nEndChar);
	pCmdUI->Enable(nStartChar != nEndChar);
}

void CCmdEdit::OnUpdateNeedClip(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT));
}

void CCmdEdit::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanUndo());
}

void CCmdEdit::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanRedo());
}

void CCmdEdit::OnUpdateEditReadWrite(CCmdUI* pCmdUI)
{
	// No Enable if read-only
	if (GetStyle() & ES_READONLY)
		pCmdUI->Enable(false);
	else
		OnUpdateNeedSel(pCmdUI);
}

void CCmdEdit::OnUpdateEditPasteReadWrite(CCmdUI* pCmdUI)
{
	// No Enable if read-only
	if (GetStyle() & ES_READONLY)
		pCmdUI->Enable(false);
	else
		OnUpdateNeedClip(pCmdUI);
}


void CCmdEdit::OnSetFocus(CWnd* pOldWnd)
{
	CRichEditCtrl::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);
}

void CCmdEdit::OnKillFocus(CWnd* pNewWnd)
{
	CRichEditCtrl::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());
}

BOOL CCmdEdit::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try to let this handle it
	if (CRichEditCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// Try super commander if any
	else if (GetSuperCommander() && GetSuperCommander()->HandleCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// Pass message up to owning window if not handled here
	return GetParent() ? GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : false;
}

void CCmdEdit::OnPaste()
{
	if (mShowStyled)
		Paste();
	
	else
	{
		if (!::OpenClipboard(*this))
			return;
		HANDLE hglb = ::GetClipboardData(CF_UNICODETEXT);
		if (hglb)
		{
			LPWSTR lptstr = (LPWSTR) ::GlobalLock(hglb);
			ReplaceSel(lptstr, true);
			::GlobalUnlock(hglb);
		}
		::CloseClipboard();
	}

	if (mAnyCmd)
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}

void CCmdEdit::OnCut()
{
	Cut();

	if (mAnyCmd)
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}

void CCmdEdit::OnClear()
{
	Clear();

	if (mAnyCmd)
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}

void CCmdEdit::OnEditUndo()
{
	Undo();

	if (mAnyCmd)
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}

void CCmdEdit::OnEditRedo()
{
	Redo();

	if (mAnyCmd)
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}

void CCmdEdit::OnEditSelectAll(void)
{
	SetSel(0, -1);
}

void CCmdEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	long selStart;
	long selEnd;
	GetSel(selStart, selEnd);
	if ((nChar == VK_DELETE) && (selStart == selEnd))
	{
		// Check for fwd-delete
		if (selStart < GetTextLengthEx())
		{
			SetSel(selStart + 1, selStart + 1);
			SendMessage(WM_KEYDOWN, 0x08, 1L);
		}
	}
	else
		CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

int CCmdEdit::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// Tell parent view to activate
	GetParentFrame()->SetActiveView(mContextView);
	return CRichEditCtrl::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CCmdEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	// Look for insert of spaces per tab
	if ((nChar == VK_TAB) && (mSpacesPerTab > 0) &&
		((GetStyle() & ES_READONLY) == 0) &&
		((::GetKeyState(VK_SHIFT) < 0) ||
		 (::GetKeyState(VK_CONTROL) < 0) ||
		 (::GetKeyState(VK_MENU) < 0)))
	{
		int tab_pos = 0;
		long selStart = 0;
		long selEnd = 0;
		int lineStart;
		int lineNum;
		GetSel(selStart, selEnd);
		lineNum = LineFromChar(selStart);
		lineStart = LineIndex(lineNum);
		tab_pos = selStart - lineStart;
		short add_space = mSpacesPerTab - (tab_pos % mSpacesPerTab);
		CString spaces;
		for(short i=0; i<add_space ; i++)
			spaces += ' ';
		ReplaceSel(spaces, true);
	}
	else if (nChar == VK_TAB)
		DoTab();
	else if (mReturnCmd && (nChar == VK_RETURN))
	{
		// Send command to parent
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
	{
		CRichEditCtrl::OnChar(nChar, nRepCnt, nFlags);

		// Send command to parent if required
		if (mAnyCmd)
			GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
}
