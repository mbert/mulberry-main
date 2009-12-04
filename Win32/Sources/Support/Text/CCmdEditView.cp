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


// CCmdEditView.cpp : implementation file
//


#include "CCmdEditView.h"

#include "CCharsetManager.h"
#include "CGrayBackground.h"
#include "CFindReplaceWindow.h"
#include "CFontCache.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CTextEngine.h"
#include "CTextMacros.h"
#include "CUnicodeUtils.h"
#include "CUStringUtils.h"

#include "StValueChanger.h"

/////////////////////////////////////////////////////////////////////////////
// CCmdEditView

IMPLEMENT_DYNCREATE(CCmdEditView, CRichEditView)

CCmdEditView::CCmdEditView()
{
	m_strClass = RICHEDIT_CLASSW;

	mContextMenuID = IDR_POPUP_CONTEXT_EDIT;

	mSpacesPerTab = 0;
	mWrap = 0;
	mHardWrap = false;
	mWrapWidth = 0;
	mFindAllowed = false;

	mShowStyled = false;
	
	mCtrlRedraw = true;
	mHasVScroll = false;
	
	// Init spell auto check - off
	//mRTSpell.tomDoc = NULL;
	SpellAutoCheck(false);
}

CCmdEditView::~CCmdEditView()
{
}


BEGIN_MESSAGE_MAP(CCmdEditView, CRichEditView)
	//{{AFX_MSG_MAP(CCmdEditView)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPasteReadWrite)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_FIND, OnUpdateFindText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_FINDNEXT, OnUpdateFindNextText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_FINDNEXTBACK, OnUpdateFindNextText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_FINDSELECTION, OnUpdateFindSelectionText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_FINDSELECTIONBACK, OnUpdateFindSelectionText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_REPLACE, OnUpdateReplaceText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_REPLACEFIND, OnUpdateReplaceFindText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_REPLACEFINDBACK, OnUpdateReplaceFindText)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_REPLACEALL, OnUpdateReplaceAllText)

	ON_UPDATE_COMMAND_UI(IDM_TEXT_WRAP_LINES, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_UNWRAP_LINES, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_QUOTE_LINES, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_UNQUOTE_LINES, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_REQUOTE_LINES, OnUpdateSelectionReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_SHIFT_LEFT, OnUpdateReadWrite)
	ON_UPDATE_COMMAND_UI(IDM_TEXT_SHIFT_RIGHT, OnUpdateReadWrite)

	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	//ON_WM_LBUTTONUP()
	//ON_WM_MOUSEACTIVATE()
	
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(IDM_EDIT_FIND, OnFindText)
	ON_COMMAND(IDM_EDIT_FINDNEXT, OnFindNextText)
	ON_COMMAND(IDM_EDIT_FINDNEXTBACK, OnFindNextBackText)
	ON_COMMAND(IDM_EDIT_FINDSELECTION, OnFindSelectionText)
	ON_COMMAND(IDM_EDIT_FINDSELECTIONBACK, OnFindSelectionBackText)
	ON_COMMAND(IDM_EDIT_REPLACE, OnReplaceText)
	ON_COMMAND(IDM_EDIT_REPLACEFIND, OnReplaceFindText)
	ON_COMMAND(IDM_EDIT_REPLACEFINDBACK, OnReplaceFindBackText)
	ON_COMMAND(IDM_EDIT_REPLACEALL, OnReplaceAllText)
	
	ON_COMMAND(IDM_TEXT_WRAP_LINES, OnWrapLines)
	ON_COMMAND(IDM_TEXT_UNWRAP_LINES, OnUnwrapLines)
	ON_COMMAND(IDM_TEXT_QUOTE_LINES, OnQuoteLines)
	ON_COMMAND(IDM_TEXT_UNQUOTE_LINES, OnUnquoteLines)
	ON_COMMAND(IDM_TEXT_REQUOTE_LINES, OnRequoteLines)
	ON_COMMAND(IDM_TEXT_SHIFT_LEFT, OnShiftLeft)
	ON_COMMAND(IDM_TEXT_SHIFT_RIGHT, OnShiftRight)

	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()

	ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)
	ON_CONTROL_REFLECT_EX(EN_CHANGE, OnEditChange)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const int cTextInset = 2;

int CCmdEditView::OnCreate(LPCREATESTRUCT lpcs)
{
	//if (CRichEditView::OnCreate(lpcs) != 0)
	//	return -1;

	if (CCtrlView::OnCreate(lpcs) != 0)
		return -1;
	GetRichEditCtrl().LimitText(lMaxSize);
	GetRichEditCtrl().SetEventMask(ENM_SELCHANGE | ENM_CHANGE | ENM_SCROLL);
	VERIFY(GetRichEditCtrl().SetOLECallback(&m_xRichEditOleCallback));
	m_lpRichEditOle = GetRichEditCtrl().GetIRichEditOle();
	//DragAcceptFiles();
	//GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION);	// Turned off to prevent auto-word selection problem
	WrapChanged();

	// Set selection format to default format
	CHARFORMAT2 cf;
	GetRichEditCtrl().GetDefaultCharFormat(cf);
	cf.dwEffects = 0;
	cf.bUnderlineType = 0;
	cf.crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE;
	GetRichEditCtrl().SetDefaultCharFormat(cf);

	ASSERT(m_lpRichEditOle != NULL);
	return 0;
}

// Resize columns
void CCmdEditView::OnSize(UINT nType, int cx, int cy)
{
	CRichEditView::OnSize(nType, cx, cy);

	// Inset drawing rect
	UpdateMargins();
}

void CCmdEditView::UpdateMargins()
{
	// Inset drawing rect
	CRect crect;
	GetRichEditCtrl().GetClientRect(crect);
	crect.DeflateRect(cTextInset, cTextInset);
	if (mHardWrap)
		crect.right = crect.left + mWrapWidth;
	GetRichEditCtrl().SetRect(crect);
	
	// Check change in scrollbar visibility
	if (((GetRichEditCtrl().GetStyle() & WS_VSCROLL) != 0) ^ mHasVScroll)
	{
		mHasVScroll = !mHasVScroll;
		RestoreMargins();
	}
}

void CCmdEditView::RestoreMargins()
{
	CRect wrect;
	GetRichEditCtrl().GetWindowRect(wrect);
	GetRichEditCtrl().PostMessage(WM_SIZE, SIZE_RESTORED, (wrect.Height() << 16) + wrect.Width());
	GetWindowRect(wrect);
	PostMessage(WM_SIZE, SIZE_RESTORED, (wrect.Height() << 16) + wrect.Width());
}

HRESULT CCmdEditView::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaFile)
{
	// Reject anything other than rich text and ordinary text
	//if (!bReally)
	{
		// if direct pasting a particular native format allow it
		if (IsRichEditFormat(*lpcfFormat))
			return S_OK;

		COleDataObject dataobj;
		dataobj.Attach(lpdataobj, FALSE);
		// if format is 0, then force particular formats if available
		if (*lpcfFormat == 0 && (m_nPasteType == 0))
		{
			if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextAndObjects)) // native avail, let richedit do as it wants
				return S_OK;
			else if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextFormat))
			{
				*lpcfFormat = CF_UNICODETEXT;	// Coerce to text to force text only drop if possible
				return S_OK;
			}
			else if (dataobj.IsDataAvailable(CF_UNICODETEXT))
			{
				*lpcfFormat = CF_UNICODETEXT;
				return S_OK;
			}
		}

		// fail the rest if not text
		*lpcfFormat = CF_UNICODETEXT;	// Coerce to text to force text only drop if possible
		return DV_E_CLIPFORMAT;
	}
	
	return CRichEditView::QueryAcceptData(lpdataobj, lpcfFormat, dwReco, bReally, hMetaFile);
}

//int CCmdEditView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
//{
//	// If dragging is support must prevent initial window activation to
//	// allow drags from background windows
//	return ((nHitTest == HTCLIENT) && (message == WM_LBUTTONDOWN)) ?
//				MA_NOACTIVATE : CRichEditView::OnMouseActivate(pDesktopWnd, nHitTest, message);
//}
//
//void CCmdEditView::OnLButtonUp(UINT nFlags, CPoint point)
//{
//	// Must activate if the mouse goes up in this window
//	SendMessage(WM_MOUSEACTIVATE, (UINT) GetTopLevelParent(), (WM_LBUTTONDOWN << 16) + HTCLIENT);
//	//GetParentFrame()->SetActiveView(this);
//	//SetFocus();
//}


BOOL CCmdEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dwDefaultStyle = AFX_WS_DEFAULT_VIEW |
						WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	return CCtrlView::PreCreateWindow(cs);
}

// Display and track context menu
void CCmdEditView::OnContextMenu(CWnd* wnd, CPoint point)
{
	HandleContextMenu(wnd, point);
}

// Clicked somewhere
void CCmdEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRichEditView::OnLButtonUp(nFlags, point);
	
	SpellTextChange();
}

// Clicked somewhere
void CCmdEditView::OnRButtonDown(UINT nFlags, CPoint point)
{
	GetParentFrame()->BringWindowToTop();

	// Force its view to be active and focus
	GetParentFrame()->SetActiveView(this);
	SetFocus();

	// Do context menu
	ClientToScreen(&point);
	OnContextMenu(NULL, point);	
}

// Display and track context menu
void CCmdEditView::HandleContextMenu(CWnd*, CPoint point)
{
	if (mRTSpell.enabled)
	{
		bool changed = false;
		bool misspelled = false;
		{
			//StStopRedraw redraw(this);
			//StPreserveSelection selection(this);

			// Get character at point
			CPoint client_point(point);
			ScreenToClient(&client_point);
			int over_char = GetRichEditCtrl().CharFromPos(client_point);
			
			// Look for red-wave underline
			if (IsMisspelled(over_char, over_char + 1))
			{
				// Indicate that no further right-click processing will be required
				misspelled = true;

				// Get the error word
				long sel_start = 0;
				cdustring text;
				GetMisspelled(over_char, text, sel_start);
				size_t tlen = text.length();
				
				// Set selection to error text
				SetSelectionRange(sel_start, sel_start + tlen);
				
				// Get the spell checker plugin
				CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
				CSpellPlugin::StLoadSpellPlugin _load(speller);

				// Get suggestions for error word
				cdstrvect suggestions;
				cdstring utf8 = text.ToUTF8();
				speller->GetQuickSuggestions(utf8, suggestions);

				// Delete previous menu items
				CMenu menu;
				menu.LoadMenu(IDR_POPUP_SPELLERROR);
				CMenu* pPopup = menu.GetSubMenu(0);

				// Add up to 15 suggestions
				int menu_id = 0;
				for(cdstrvect::const_iterator iter = suggestions.begin(); (menu_id < 15) && (iter != suggestions.end()); iter++, menu_id++)
					CUnicodeUtils::InsertMenuUTF8(pPopup, menu_id, MF_STRING | MF_BYPOSITION, IDM_SPELL_ERROR_1 + menu_id, *iter);

				// Do popup menu of suggestions
				UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);

				if (popup_result == IDM_SPELL_ERROR_ADD)
				{
					// Add current word to dictionary
					speller->AddWord(text.ToUTF8());
					
					// Recheck this word everywhere else so that corrections are made
					RecheckAllWords(text);
					changed = true;
				}
				else if ((popup_result >= IDM_SPELL_ERROR_1) && (popup_result <= IDM_SPELL_ERROR_15) && !(GetRichEditCtrl().GetStyle() & ES_READONLY))
				{
					cdstring replace(suggestions.at(popup_result - IDM_SPELL_ERROR_1));
					size_t rlen = replace.length();
					InsertUTF8(replace);
					//selection.Offset(sel_start + tlen, rlen - tlen);

					// Recheck it
					long length_checked = 0;
					while(true)
					{
						// Get the new word
						long wlen = GetWordAt(sel_start, text, &sel_start, true);
						CheckWord(text, sel_start);
						
						// Make sure we checked all of it
						if (wlen + length_checked < rlen)
						{
							// Update word start to next word
							sel_start += wlen + 1;
							length_checked += wlen + 1;
							
							// Make sure gap between words is cleared
							MarkMisspelled(sel_start - 1, sel_start, false);
						}
						else
							break;
					}
					changed = true;
				}
			}
		}
		
		// May need to redraw
		if (changed)
			GetRichEditCtrl().RedrawWindow();

		// Stop further context menu processing if misspelled work was processed
		if (misspelled)
			return;
	}
	
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
BOOL CCmdEditView::PreTranslateMessage(MSG* pMsg)
{
	// Shift+F10: show pop-up menu.
	if ((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
		(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||
		(pMsg->message == WM_CONTEXTMENU))	// Natural keyboard key	
	{
		// Calculate hit point
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		CPoint point = rect.TopLeft();
		point.Offset(5, 5);
			
		// Force this view to be active
		GetParentFrame()->SetActiveView(this);

		// Do context menu
		OnContextMenu(NULL, point);
		return TRUE;
	}

	return CRichEditView::PreTranslateMessage(pMsg);
}

// Change font in whole control
// pFont: font object to set
// preserve_styles: false - turn off bold, italic, underline (default)
//                  true  - leave existing styles intact
void CCmdEditView::ResetFont(CFont* pFont, bool preserve_styles)
{
	{
		StStopRedraw redraw(this);
		StPreserveSelection selection(this);

		// Special hack: SetFont removes the formatting of the first line
		// so we get the format of the first character here and then reset the
		// first word on the first line to that format after changing the font.
		// This handles the case of header fields in message bodies.

		CHARFORMAT2 cf;
		GetRichEditCtrl().SetSel(0, 1);
		GetRichEditCtrl().GetSelectionCharFormat(cf);
		DWORD chf = cf.dwEffects & (CFE_BOLD | CFE_ITALIC | CFM_UNDERLINE);

		// Set new font
		GetRichEditCtrl().SetFont(pFont);
		
		// Select everything
		GetRichEditCtrl().SetSel(0, -1);
		
		// Set selection format to default format
		GetRichEditCtrl().GetDefaultCharFormat(cf);
		cf.dwEffects = 0;
		cf.bUnderlineType = 0;
		cf.crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
		cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | (preserve_styles ? 0 : (CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE));
		GetRichEditCtrl().SetSelectionCharFormat(cf);

		// Set selection format to default format
		PARAFORMAT pf;
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_LEFT;
		GetRichEditCtrl().SetParaFormat(pf);
		
		// This is the hack - if the first char had a style and we are
		// preserving styles then reset it
		if (preserve_styles && chf)
		{
			GetRichEditCtrl().SetSel(0, GetRichEditCtrl().LineLength(0));
			CString temp = GetRichEditCtrl().GetSelText();
			const TCHAR* p = temp;
			while(*p && (*p != ' ')) p++;
			if (*p)
			{
				GetRichEditCtrl().SetSel(0, p - temp);
				cf.dwEffects = chf;
				cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
				GetRichEditCtrl().SetSelectionCharFormat(cf);
			}
		}
	}

	ResetHardWrap();
	GetRichEditCtrl().RedrawWindow();
}

// Change redraw state of control
void CCmdEditView::SetCtrlRedraw(bool redraw)
{
	mCtrlRedraw = redraw;
	GetRichEditCtrl().SetRedraw(redraw);
}

// Get horiz scrollbar pos
void CCmdEditView::GetScrollPos(long& h, long& v) const
{
	h = GetRichEditCtrl().GetScrollPos(SB_HORZ);
	v = GetRichEditCtrl().GetScrollPos(SB_VERT);
}

// Get horiz scrollbar pos
void CCmdEditView::SetScrollPos(long h, long v)
{
	GetRichEditCtrl().SendMessage(WM_HSCROLL, MAKELONG(SB_THUMBPOSITION, h));
	GetRichEditCtrl().SendMessage(WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, v));
}

/////////////////////////////////////////////////////////////////////////////
// CCmdEditView message handlers

#pragma mark ____________________________Command Updaters

void CCmdEditView::OnUpdateReadWrite(CCmdUI* pCmdUI)
{
	// Must be read-only
	pCmdUI->Enable(!(GetRichEditCtrl().GetStyle() & ES_READONLY));
}

void CCmdEditView::OnUpdateSelectionReadWrite(CCmdUI* pCmdUI)
{
	// No Enable if read-only
	if (GetRichEditCtrl().GetStyle() & ES_READONLY)
		pCmdUI->Enable(false);
	else
		CRichEditView::OnUpdateNeedSel(pCmdUI);
}

void CCmdEditView::OnUpdateEditPasteReadWrite(CCmdUI* pCmdUI)
{
	// No Enable if read-only
	if (GetRichEditCtrl().GetStyle() & ES_READONLY)
		pCmdUI->Enable(false);
	else
		CRichEditView::OnUpdateNeedClip(pCmdUI);
}

void CCmdEditView::OnUpdateFindText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mFindAllowed);
}

void CCmdEditView::OnUpdateFindNextText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mFindAllowed && !CFindReplaceWindow::sFindText.empty());
}

void CCmdEditView::OnUpdateFindSelectionText(CCmdUI* pCmdUI)
{
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	pCmdUI->Enable(mFindAllowed && (sel_start != sel_end));
}

void CCmdEditView::OnUpdateReplaceText(CCmdUI* pCmdUI)
{
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	pCmdUI->Enable(mFindAllowed && !(GetRichEditCtrl().GetStyle() & ES_READONLY) && (sel_start != sel_end));
}

void CCmdEditView::OnUpdateReplaceFindText(CCmdUI* pCmdUI)
{
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	pCmdUI->Enable(mFindAllowed && !(GetRichEditCtrl().GetStyle() & ES_READONLY) &&
					(sel_start != sel_end) && !CFindReplaceWindow::sFindText.empty());
}

void CCmdEditView::OnUpdateReplaceAllText(CCmdUI* pCmdUI)
{
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	pCmdUI->Enable(mFindAllowed && !(GetRichEditCtrl().GetStyle() & ES_READONLY) && !CFindReplaceWindow::sFindText.empty());
}

#pragma mark ____________________________Command Handlers

void CCmdEditView::OnSetFocus(CWnd* pOldWnd)
{
	CRichEditView::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);

	// Check whether parent is focus border
	CGrayBackground* focus = dynamic_cast<CGrayBackground*>(GetParent());
	if (focus)
		focus->SetBorderFocus(true);

	Broadcast_Message(eBroadcast_Activate, this);
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

void CCmdEditView::OnKillFocus(CWnd* pNewWnd)
{
	CRichEditView::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());

	// Check whether parent is focus border
	CGrayBackground* focus = dynamic_cast<CGrayBackground*>(GetParent());
	if (focus)
		focus->SetBorderFocus(false);

	Broadcast_Message(eBroadcast_Deactivate, this);
}

void CCmdEditView::OnEditPaste()
{
	if (mShowStyled)
		CRichEditView::OnEditPaste();

	else
	{
		if (!::OpenClipboard(*this))
			return;
		HANDLE hglb = ::GetClipboardData(CF_UNICODETEXT);
		if (hglb)
		{
			LPWSTR lptstr = (LPWSTR) ::GlobalLock(hglb);
			GetRichEditCtrl().ReplaceSel(lptstr, true);
			::GlobalUnlock(hglb);
		}
		::CloseClipboard();
	}
	
	// Trigger spell check
	SpellTextChange();
}	

BOOL CCmdEditView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try to let this handle it
	if (CRichEditView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// Try super commander if any
	else if (GetSuperCommander() && GetSuperCommander()->HandleCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// Pass message up to owning window if not handled here
	return GetParent() ? GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : false;
}

void CCmdEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Indicate pending spell check
	SpellTextRequired();

	// Look for macro trigger
	CKeyAction keyact(nChar, CKeyModifiers(0));
	if (keyact == CPreferences::sPrefs->mTextMacros.GetValue().GetKeyAction())
	{
		// Do macro expansion
		if (ExpandMacro())
		{
			// Do spell check
			SpellTextChange();

			return;
		}
	}
	
	// Do inherited
	CRichEditView::OnKeyDown(nChar, nRepCnt, nFlags);
	
	// Do pending spell check
	SpellTextRequiredCheck();
}

void CCmdEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!HandleChar(nChar, nRepCnt, nFlags))
		// Do inherited action
		CRichEditView::OnChar(nChar, nRepCnt, nFlags);
}

// Handle character
bool CCmdEditView::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	// Look for insert of spaces per tab
	if ((nChar == VK_TAB) && (mSpacesPerTab > 0) &&
		((GetStyle() & ES_READONLY) == 0) &&
		((::GetKeyState(VK_SHIFT) >= 0) &&
		 (::GetKeyState(VK_CONTROL) >= 0) &&
		 (::GetKeyState(VK_MENU) >= 0)))
	{
		// Check whether space substituion allowed
		if (CPreferences::sPrefs->mTabSpace.GetValue())
		{
			int tab_pos = 0;
			long selStart = 0;
			long selEnd = 0;
			int lineStart;
			int lineNum;
			GetRichEditCtrl().GetSel(selStart, selEnd);
			lineNum = GetRichEditCtrl().LineFromChar(selStart);
			lineStart = GetRichEditCtrl().LineIndex(lineNum);
			tab_pos = selStart - lineStart;
			short add_space = mSpacesPerTab - (tab_pos % mSpacesPerTab);
			CString spaces;
			for(short i=0; i<add_space ; i++)
				spaces += ' ';
			GetRichEditCtrl().ReplaceSel(spaces, true);
			return true;
		}
		else
			// Handle tab insert normally
			return false;
	}
	else if (nChar == VK_TAB)
	{
		DoTab();
		return true;
	}
	else if ((GetStyle() & ES_READONLY) != 0)
	{
		// Pass up to super commander
		return CCommander::HandleChar(nChar, nRepCnt, nFlags);
	}
	else
		return false;
}

void CCmdEditView::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Trigger spell check
	//SpellTextChange();
	*pResult = 0;
}

BOOL CCmdEditView::OnEditChange()
{
	// Trigger spell check
	SpellTextChange();
	return true;
}

#pragma mark ____________________________Wrapping

// Set wrap width
void CCmdEditView::SetWrap(short wrap)
{
	mWrap = (wrap == 0) ? 1000 : wrap;
	ResetHardWrap();
}

// Set wrapping mode
void CCmdEditView::SetHardWrap(bool hard_wrap)
{
	if (hard_wrap != mHardWrap)
	{
		mHardWrap = hard_wrap;
		if (mHardWrap)
			ResetHardWrap();
		else
		{
			// Turn on word wrap style
			GetRichEditCtrl().ModifyStyle(WS_HSCROLL | ES_AUTOHSCROLL, 0);
			//GetRichEditCtrl().SetOptions(ECOOP_XOR, ECO_AUTOHSCROLL);

			// Force resize of frame which will adjust wrap
			UpdateMargins();
		}
		
		// Touch window to get scrollbars redrawn properly
		CRect bounds;
		GetRichEditCtrl().GetWindowRect(&bounds);
		GetRichEditCtrl().SetWindowPos(nil, 0, 0, bounds.Width(), bounds.Height() - 5, SWP_NOZORDER | SWP_NOMOVE);
		GetRichEditCtrl().SetWindowPos(nil, 0, 0, bounds.Width(), bounds.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}
}

// Set wrapping mode
void CCmdEditView::ResetHardWrap()
{
	if (mHardWrap)
	{
		// Turn off word wrap style
		GetRichEditCtrl().ModifyStyle(0, WS_HSCROLL | ES_AUTOHSCROLL);
		GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOHSCROLL);

		// Force image size to width of font * wrap length
		CDC* pDC = GetRichEditCtrl().GetDC();
		CFont* old_font = pDC->SelectObject(CFontCache::GetDisplayFont());
		CRect tdraw(0, 0, 0, 0);
		pDC->DrawText(_T("1234567890im"), 12, tdraw, DT_CALCRECT | DT_LEFT);
		pDC->SelectObject(old_font);
		ReleaseDC(pDC);

		mWrapWidth = (mWrap * tdraw.Width()) / 12 + 2;
		
		UpdateMargins();
	}
}

// Set spaces per tab
void CCmdEditView::SetSpacesPerTab(short numSpacesPerTab)
{
	// Set spacing in format
	GetRichEditCtrl().SetSel(0, -1);
	PARAFORMAT pf;
	GetRichEditCtrl().GetParaFormat(pf);
	pf.dwMask = PFM_TABSTOPS;
	pf.cTabCount = 1;
	pf.rgxTabs[0] = 720/6*numSpacesPerTab;
	GetRichEditCtrl().SetParaFormat(pf);

	mSpacesPerTab = numSpacesPerTab;
}

#pragma mark ____________________________Text Processing

void CCmdEditView::OnWrapLines()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap
	const char* wrapped = CTextEngine::WrapLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), CPreferences::sPrefs->mFormatFlowed.GetValue());
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CCmdEditView::OnUnwrapLines()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap
	const char* unwrapped = CTextEngine::UnwrapLines(selected.c_str(), selected.length());
	
	// Replace selection
	EndTextProcessing(unwrapped, sel_start, sel_end);
}

void CCmdEditView::OnQuoteLines()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap with quote
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(),
													mQuotation, &CPreferences::sPrefs->mRecognizeQuotes.GetValue(), true);
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CCmdEditView::OnUnquoteLines()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do unquote
	const char* unquoted = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), mQuotation.c_str());
	
	// Replace selection
	EndTextProcessing(unquoted, sel_start, sel_end);
}

void CCmdEditView::OnRequoteLines()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do unquote
	const char* txt = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), mQuotation.c_str());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::UnwrapLines(selected.c_str(), selected.length());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(),
									mQuotation, &CPreferences::sPrefs->mRecognizeQuotes.GetValue(), true);
	
	// Replace selection
	EndTextProcessing(txt, sel_start, sel_end);
}

void CCmdEditView::OnShiftLeft()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Pretend that shift chars are quotes!
	cdstring shift;
	if (CPreferences::sPrefs->mTabSpace.GetValue())
	{
		// Fill with required number of spaces
		for(short i = 0; i < mSpacesPerTab; i++)
			shift += ' ';
	}
	else
		shift = '\t';

	// Now do unquote
	const char* unquoted = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), shift.c_str());
	
	// Replace selection
	EndTextProcessing(unquoted, sel_start, sel_end);
}

void CCmdEditView::OnShiftRight()
{
	// Prepare for line based processing
	long sel_start;
	long sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Pretend that shift chars are quotes!
	cdstring shift;
	if (CPreferences::sPrefs->mTabSpace.GetValue())
	{
		// Fill with required number of spaces
		for(short i = 0; i < mSpacesPerTab; i++)
			shift += ' ';
	}
	else
		shift = '\t';

	// Now do wrap with quote
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), shift, NULL, true);
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

bool CCmdEditView::ExpandMacro()
{
	{
		StStopRedraw _nodraw(this);

		// Look for selection
		long selStart;
		long selEnd;
		GetSelectionRange(selStart, selEnd);
		long origStart = selStart;
		cdstring selected;
		if (selStart == selEnd)
		{
			// Scan backwards looking for word break
			while(selStart--)
			{
				// Get the currently selected text
				SetSelectionRange(selStart, selEnd);
				GetSelectedText(selected);

				// Test for word break
				if (isspace(selected[(cdstring::size_type)0]) || ispunct(selected[(cdstring::size_type)0]))
				{
					// Bump up to start of word itself
					selStart++;
					SetSelectionRange(selStart, selEnd);
					GetSelectedText(selected);
					break;
				}
			}
			if (selStart < 0)
				selStart = 0;
		}
		else
			// Get the currently selected text
			GetSelectedText(selected);

		// Get the selected text
		selected.trimspace();
		if (selected.empty())
			return false;
		
		// Lookup the macro
		const cdstring& value = CPreferences::sPrefs->mTextMacros.GetValue().Lookup(selected);
		if (value.empty())
		{
			// Reset to old selection
			SetSelectionRange(origStart, selEnd);
			return false;
		}
		else
		{
			// Check whether spelling is active
			if (mRTSpell.enabled && !mRTSpell.busy)
			{
				// Always mark as not mispelled
				MarkMisspelled(selStart, selEnd, false);
			}

			// Make sure smart insert does not apply
			InsertUTF8(value);
		}
	}

	GetRichEditCtrl().Invalidate();
	return true;
}

void CCmdEditView::PrepareTextProcessing(long& sel_start, long& sel_end, cdstring& selected)
{
	// Make sure complete lines are selected
	SelectFullLines();
	GetRichEditCtrl().GetSel(sel_start, sel_end);

	// Get the selected text
	GetSelectedText(selected);
}

void CCmdEditView::EndTextProcessing(const char* insert_text, long& sel_start, long& sel_end)
{
	// Replace selection
	if (insert_text)
	{
		// Must remove any LFs from text being inserted since RichEdit 2.0
		// only uses CRs
		::FilterOutLFs(const_cast<char*>(insert_text));
		
		size_t insert_length = ::strlen(insert_text);
		InsertUTF8(insert_text);
		sel_end = sel_start + insert_length + ((insert_text[insert_length - 1] == '\r') ? 0 : 1);
	}
	GetRichEditCtrl().SetSel(sel_start, sel_end);
	delete insert_text;
}

void CCmdEditView::SelectFullLines()
{
	// Get current selection and text
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	
	// Get offsets to lines
	long line_start = GetRichEditCtrl().LineFromChar(sel_start);
	long line_end = GetRichEditCtrl().LineFromChar(sel_end);

	// Get range for new selection;
	long new_sel_start = GetRichEditCtrl().LineIndex(line_start);
	long new_sel_end = GetRichEditCtrl().LineIndex(line_end);
	if (new_sel_end != sel_end)
		new_sel_end = GetRichEditCtrl().LineIndex(line_end + 1);
	
	// Do new selection
	GetRichEditCtrl().SetSel(new_sel_start, new_sel_end);
}

long CCmdEditView::GetTextLengthEx() const
{
	GETTEXTLENGTHEX gex;
	gex.flags = GTL_DEFAULT;
	gex.codepage = 1200;
	return ::SendMessageW(GetRichEditCtrl().m_hWnd, EM_GETTEXTLENGTHEX, (WPARAM) &gex, 0);
}

void CCmdEditView::GetSelectedText(cdstring& selected) const
{
	// get as UTF16 first
	cdustring utf16;
	GetSelectedText(utf16);
	selected = utf16.ToUTF8();
}

void CCmdEditView::GetSelectedText(cdustring& selected) const
{
	// Get current selection length (NB this is the raw length with CR - not CRLF - line end encoding)
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
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
void CCmdEditView::SetText(const cdstring& all)
{
	cdustring utf16(all);
	SetText(utf16);
}

// UTF16 in
void CCmdEditView::SetText(const cdustring& all)
{
	SETTEXTEX setex;
	setex.flags = ST_DEFAULT;
	setex.codepage = 1200;
	::SendMessageW(m_hWnd, EM_SETTEXTEX, (WPARAM) &setex, (LPARAM)all.c_str());
}

// Insert unstyled utf8 at selection
void CCmdEditView::InsertUTF8(const cdstring& txt)
{
	cdustring utf16(txt);
	InsertText(utf16);
}

// Insert unstyled utf8 at selection
void CCmdEditView::InsertUTF8(const char* txt, size_t len)
{
	cdstring utf8(txt, len);
	InsertUTF8(utf8);
}

// Insert unstyled utf16 at selection
void CCmdEditView::InsertText(const cdustring& utxt)
{
	SETTEXTEX setex;
	setex.flags = ST_SELECTION;
	setex.codepage = 1200;
	::SendMessageW(m_hWnd, EM_SETTEXTEX, (WPARAM) &setex, (LPARAM)utxt.c_str());
}

// Insert unstyled utf16 at selection
void CCmdEditView::InsertText(const unichar_t* utxt, size_t len)
{
	cdustring utf16(utxt, len);
	InsertText(utf16);
}

void CCmdEditView::GetText(cdstring& all) const
{
	cdustring utf16;
	GetText(utf16);
	all = utf16.ToUTF8();
}

void CCmdEditView::GetText(cdustring& all) const
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

void CCmdEditView::GetRawText(cdstring& all) const
{
	cdustring utf16;
	GetRawText(utf16);
	all = utf16.ToUTF8();
}

void CCmdEditView::GetRawText(cdustring& all) const
{
	// Get all the text using the native/internal line end character,
	// which is needed when doing character formatting via offsets
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

#pragma mark ____________________________Find & Replace Commands

void CCmdEditView::OnFindText()
{
	// Bring find window to front or create it
	// Find window will 'drive' find operation
	CFindReplaceWindow::CreateFindReplaceWindow(this);
}

void CCmdEditView::OnFindNextText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindNextText(false);
}

void CCmdEditView::OnFindNextBackText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindNextText(true);
}

void CCmdEditView::OnFindSelectionText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindSelectionText(false);
}

void CCmdEditView::OnFindSelectionBackText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindSelectionText(true);
}

void CCmdEditView::OnReplaceText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceText();
}

void CCmdEditView::OnReplaceFindText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceFindText(false);
}

void CCmdEditView::OnReplaceFindBackText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceFindText(true);
}

void CCmdEditView::OnReplaceAllText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceAllText();
}

bool CCmdEditView::DoFindNextText(bool backwards, bool silent)
{
	bool result = false;

	// Get current selection and text
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	
	// Find starts after current selection (forward) or before it (backwards)
	if (sel_start != sel_end)
	{
		if (backwards)
			sel_end = sel_start;
		else
			sel_start = sel_end;
	}

	// Do find
	EFindMode mode = (EFindMode) (CFindReplaceWindow::sFindMode & ~eBackwards);
	if (backwards)
		mode = (EFindMode) (mode | eBackwards);

	long found = -1;

#if 0
	// RichEdit2.0 can do backwards find so special case not needed

	// Backards search cannot be done with built in CRichEditCtrl methods
	// Use strfind method, but this requires making a copy of the entire text
	// since there is no way to access the text in the control itself
	if (backwards)
	{
		CString text;
		GetRichEditCtrl().GetWindowText(text);
		const char* found_txt = ::strfind(text, GetTextLengthEx(), sel_start,
											CFindReplaceWindow::sFindText.c_str(), mode);
		if (found_txt)
			found = found_txt - (const char*) text;
	}
	else
#endif
	{
		DWORD flags = backwards ? 0 : FR_DOWN;
		if (mode & eCaseSensitive)
			flags |= FR_MATCHCASE;
		if (mode & eEntireWord)
			flags |= FR_WHOLEWORD;
		FINDTEXTEX find;
		CString temp = CFindReplaceWindow::sFindText.win_str();
		find.lpstrText = const_cast<TCHAR*>((const TCHAR*) temp);
		find.chrg.cpMin = sel_end;
		find.chrg.cpMax = GetTextLengthEx();
		found = GetRichEditCtrl().FindText(flags, &find);

		// Try wrap if not found and wrap turned on
		if ((found < 0) && (mode & eWrap))
		{
			find.chrg.cpMin = 0;
			find.chrg.cpMax = sel_start;
			found = GetRichEditCtrl().FindText(flags, &find);
		}
	}

	// Was it found
	if (found >= 0)
	{
		// Determine new selection
		sel_start = found;
		sel_end = sel_start + CFindReplaceWindow::sFindText.length();
		GetRichEditCtrl().SetSel(sel_start, sel_end);
		result = true;
	}
	else if (!silent)
		::MessageBeep(-1);
	
	return result;
}

void CCmdEditView::DoFindSelectionText(bool backwards)
{
	// Get selected text and give it to find & replace window
	cdstring selected;
	GetSelectedText(selected);
	CFindReplaceWindow::SetFindText(selected);

	// Do normal find operation
	DoFindNextText(backwards);
}

void CCmdEditView::DoReplaceText()
{
	// Make sure smart insert does not apply
	InsertUTF8(CFindReplaceWindow::sReplaceText);
}

void CCmdEditView::DoReplaceFindText(bool backwards)
{
	DoReplaceText();
	DoFindNextText(backwards);
}

void CCmdEditView::DoReplaceAllText()
{
	// Reset selection to first character to allow replace within the selection itself
	long sel_start;
	long sel_end;
	GetRichEditCtrl().GetSel(sel_start, sel_end);
	GetRichEditCtrl().SetSel(sel_start, sel_start);

	// Loop until no more found (silently) and do replace
	bool found_once = false;
	long first_start;
	long first_end;

	while(DoFindNextText(CFindReplaceWindow::sFindMode & eBackwards, true))
	{
		if (!found_once)
		{
			// Get the first item found and cache selection range
			GetRichEditCtrl().GetSel(first_start, first_end);
			found_once = true;
		}
		else
		{
			// Make sure we don't repeat ourselves
			GetRichEditCtrl().GetSel(sel_start, sel_end);
			if (sel_start == first_start)
				break;
		}
		DoReplaceText();
	}
	
	// Reset selection if no change
	if (!found_once)
	{
		GetRichEditCtrl().SetSel(sel_start, sel_end);
		::MessageBeep(-1);
	}

}

#pragma mark ____________________________Spelling commands

void CCmdEditView::SpellAutoCheck(bool auto_check)
{
	if (auto_check && CPluginManager::sPluginManager.HasSpelling())
	{
		// Make sure spell check plugin is loaded
		CSpellPlugin* plugin = CPluginManager::sPluginManager.GetSpelling();
		if (plugin->IsLoaded())
			plugin->LoadPlugin();
		else
		{
			plugin->LoadPlugin();
			plugin->SpInitialise();
		}
		
		mRTSpell.state = RTSpell::eNotInWord;
		mRTSpell.cursorPos = 0;
		mRTSpell.charCount = 0;
		mRTSpell.wordStartPos = 0;
		mRTSpell.contextWordPos = 0;
		mRTSpell.busy = false;
		mRTSpell.enabled = true;
		mRTSpell.pending = false;
		
		// Make sure ITextDocument exists
		if (mRTSpell.tomDoc == NULL)
		{
			CComPtr<IRichEditOle> iREOle(GetRichEditCtrl().GetIRichEditOle());
			if (iREOle != NULL)
			{
				ITextDocument* itDoc = NULL;
				HRESULT hr = iREOle->QueryInterface(__uuidof(ITextDocument), (void**) &itDoc);
				if (SUCCEEDED(hr) && (itDoc != NULL))
					mRTSpell.tomDoc.Attach(itDoc);
			}
		}
		
		// If we could not get the ITextDocument, then disable spell checking
		if (mRTSpell.tomDoc == NULL)
		{
			mRTSpell.busy = false;
			mRTSpell.enabled = false;
			mRTSpell.pending = false;
		}
	}
	else
	{
		mRTSpell.busy = false;
		mRTSpell.enabled = false;
		mRTSpell.pending = false;
	}
}

bool CCmdEditView::IsMisspelled(long sel_start, long sel_end) const
{
	bool wavy_underline = false;

	CComPtr<ITextRange> itRange;
	HRESULT hr = mRTSpell.tomDoc->Range(sel_start, sel_end, &itRange);
	if (SUCCEEDED(hr) && (itRange != NULL))
	{
		CComPtr<ITextFont> itFont;
		hr = itRange->GetFont(&itFont);
		if (SUCCEEDED(hr) && (itFont != NULL))
		{
			// Get underline type
			long underline = 0;
			itFont->GetUnderline(&underline);
			
			// If undefined need to look at each character in turn - ughh!
			if (underline == tomUndefined)
			{
				// Look at each character - if any one has wavy underline
				// treat entire select as wavy underline
				for(long pos = sel_start; pos < sel_end; pos++)
				{
					wavy_underline = IsMisspelled(pos, pos + 1);
					if (wavy_underline)
						break;
				}
			}
			else
				wavy_underline = (underline == (tomWave | 0x50));
		}
	}
	
	return wavy_underline;
}

// Definitions from ssce.h etc

#define FANCY_APOSTROPHE 0x92

/* Character-class masks: */
#define A_ 1
#define D_ 1
#define P_ 0
#define S_ 0
#define L_ 0
#define U_ 0
#define V_ 0

/** WS_Latin1_CharClass:
 * Used to classify Latin1 characters.
 */
const unsigned char WS_Latin1_CharClass[256] = {
	/* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, S_, S_, 0, 0, S_, 0, 0,
	/* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 20 */ S_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* 30 */ D_, D_, D_, D_, D_, D_, D_, D_, D_, D_, P_, P_, P_, P_, P_, P_,
	/* 40 */ P_, A_|U_|V_, A_|U_, A_|U_, A_|U_, A_|U_|V_, A_|U_, A_|U_,
	/* 48 */ A_|U_, A_|U_|V_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_|V_,
	/* 50 */ A_|U_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_|V_, A_|U_, A_|U_,
	/* 58 */ A_|U_, A_|U_, A_|U_, P_, P_, P_, P_, P_,
	/* 60 */ P_, A_|L_|V_, A_|L_, A_|L_, A_|L_, A_|L_|V_, A_|L_, A_|L_,
	/* 68 */ A_|L_, A_|L_|V_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_|V_,
	/* 70 */ A_|L_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_|V_, A_|L_, A_|L_,
	/* 78 */ A_|L_, A_|L_, A_|L_, P_, P_, P_, P_, P_,
#if (defined(_WIN16) || defined(_WIN32)) && !defined(SSCE_NO_WINDOWS)
	/* Additional alphabetic characters in the Latin1 Windows charset */
	/* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, A_|U_, 0, A_|U_|V_, 0, A_|U_, 0,
	/* 90 */ 0, 0, P_, P_, P_, P_, P_, P_, P_, P_, A_|L_, P_, A_|L_|V_, 0, A_|L_, A_|U_,
#else
	/* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 90 */ 0, 0, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, 0, 0, 0,
#endif
	/* A0 */ 0, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* B0 */ P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* C0 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_,
	/* C8 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_,
	/* D0 */ A_|U_, A_|U_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, P_,
	/* D8 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_, A_|U_, A_|L_,
	/* E0 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_,
	/* E8 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, 
	/* F0 */ A_|L_|V_, A_|L_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, P_,
	/* F8 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_, A_|L_, A_|L_
};


// Return TRUE if a character could be part of a word:
#define IS_WORD_CHAR(c) (WS_Latin1_CharClass[(unsigned char) c] == 1)


long CCmdEditView::GetWordAt(long pos, cdustring& word, long* startPos, bool word_start) const
{
	const long maxWordSize = 256;
	
	// Determine sensible buffer range around start pos
	long rstart = max(0L, pos - maxWordSize);
	long rend = pos + maxWordSize;
	
	// Get the text in that range
	cdustring buf;
	GetSpellTextRange(rstart, rend, buf);
	const unichar_t* _buf = buf.c_str();
	
	// Locate beginning of word by stepping back
	pos = min(pos, rstart + (long) buf.length());
	long i = min(pos, maxWordSize);
	while((i > 0) &&
			(IS_WORD_CHAR(_buf[(int) i - 1]) ||
			 (_buf[(int) i - 1] == '.') ||
			 (_buf[(int) i - 1] == '\'') ||
			 (_buf[(int) i - 1] == FANCY_APOSTROPHE)))
		i--;
		
	// May not be a word
	if (i < 0)
	{
		*startPos = -1;
		return 0;
	}
	
	// Advance past any periods and apostrophes at the beginning of the word
	while((_buf[(int) i] != 0) && !IS_WORD_CHAR(_buf[(int) i]))
		i++;
	*startPos = rstart + i;

	// Collect the word
	bool embeddedPeriods = false;
	while(IS_WORD_CHAR(_buf[(int) i]) ||
			(_buf[(int) i] == '.') || 
			(_buf[(int) i] == '\'') ||
			(_buf[(int) i] == FANCY_APOSTROPHE))
	{
		if ((_buf[(int) i] == '.') && (i > 0) &&
			IS_WORD_CHAR(_buf[(int) i - 1]) && IS_WORD_CHAR(_buf[(int) i + 1]))
			embeddedPeriods = true;
		i++;
	}
	long n = rstart + i - *startPos;

	// Copy the word
	if (n > 0)
		word.assign(buf.c_str() + *startPos - rstart, n);
	
	// Accept a terminating period only if the word is an initialism
	// (contains embedded periods, no more than 2 consecutive alpha-numerics)
	if ((n > 0) && (word[(cdustring::size_type) (n - 1)] == '.'))
	{
		bool initialism = false;
		if (embeddedPeriods)
		{
			long nAlphaNumerics = 0;

			initialism = true;
			for (const unichar_t* p = word.c_str(); *p != 0; p++)
			{
				if (IS_WORD_CHAR(*p))
				{
					if (++nAlphaNumerics > 2)
					{
						initialism = false;
						break;
					}
				}
				else
					nAlphaNumerics = 0;
			}
		}

		if (!initialism)
		{
			/* Exclude all trailing periods. */
			while ((n > 0) && (word[(cdustring::size_type) (n - 1)] == '.'))
				word[(cdustring::size_type) --n] = 0;
		}
	}

	return n;
}

unichar_t CCmdEditView::GetTextChar(long pos) const
{
	cdustring txt;
	long len = GetSpellTextRange(pos, pos + 1, txt);
	return (len > 0) ? txt[(cdstring::size_type)0] : 0;
}

long CCmdEditView::GetSpellTextRange(long start, long end, cdustring& text) const
{
	CComPtr<ITextRange>itRange;
	HRESULT hr = mRTSpell.tomDoc->Range(start, end, &itRange);
	if (SUCCEEDED(hr) && (itRange != NULL))
	{
		// Get text range
		BSTR bstr;
		itRange->GetText(&bstr);
		
		// Get as c-string
		CString temp;
		::AfxBSTR2CString(&temp, bstr);
		::SysFreeString(bstr);
		text = temp;
		return text.length();
	}

	return 0;
}

bool CCmdEditView::GetMisspelled(long pos, cdustring& word, long& word_start) const
{
	if (!IsMisspelled(pos, pos + 1))
		return false;

	// Locate the extent of the marked text
	for (word_start = pos; (word_start > 0) && IsMisspelled(word_start - 1, word_start) && !isspace(GetTextChar(word_start - 1)); word_start--)
	{
	}

	long word_end;
	for (word_end = word_start; IsMisspelled(word_end, word_end + 1) && !isspace(GetTextChar(word_end)); ++word_end)
	{
		// Do nothing.
	}

	if (word_start < word_end)
	{
		GetSpellTextRange(word_start, word_end, word);
		return true;
	}
	else
		return false;
}

void CCmdEditView::MarkMisspelled(long sel_start, long sel_end, bool misspelled)
{
	// Make sure ITextDocument exists - this can be called when spell-as-you-type is not enabled:
	// e.g. on an incoming message window when check spelling is selected
	if (mRTSpell.tomDoc == NULL)
	{
		CComPtr<IRichEditOle> iREOle(GetRichEditCtrl().GetIRichEditOle());
		if (iREOle != NULL)
		{
			ITextDocument* itDoc = NULL;
			HRESULT hr = iREOle->QueryInterface(__uuidof(ITextDocument), (void**) &itDoc);
			if (SUCCEEDED(hr) && (itDoc != NULL))
				mRTSpell.tomDoc.Attach(itDoc);
		}
	}
	
	// Cannot do mark text if no ITextDocument
	if (mRTSpell.tomDoc == NULL)
		return;

	// Ignore if currently not misspelled and not marking as misspelled
	if (!IsMisspelled(sel_start, sel_end) && !misspelled)
		return;

	mRTSpell.tomDoc->Undo(tomSuspend, NULL);
	CComPtr<ITextRange>itRange;
	HRESULT hr = mRTSpell.tomDoc->Range(sel_start, sel_end, &itRange);
	if (SUCCEEDED(hr) && (itRange != NULL))
	{
		CComPtr<ITextFont> itFont;
		hr = itRange->GetFont(&itFont);
		if (SUCCEEDED(hr) && (itFont != NULL))
		{
			// Preserve existing underline state if not misspelled
			long old_underline = 0;
			itFont->GetUnderline(&old_underline);
			if (old_underline != tomSingle)
				old_underline = tomNone;

			// Set underline type
			itFont->SetUnderline(misspelled ? (tomWave | 0x50) : old_underline);

			// Do background hilite if requested
			if (CPluginManager::sPluginManager.GetSpelling()->SpellColourBackground())
				itFont->SetBackColor(misspelled ? CPluginManager::sPluginManager.GetSpelling()->SpellBackgroundColour() : tomAutoColor);
		}
	}
	mRTSpell.tomDoc->Undo(tomResume, NULL);
}

bool CCmdEditView::CheckWord(const cdustring& word, long start)
{
	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return true;
	
	bool result = false;

	// Preserve scroll pos
	//StStopRedrawPreserveUpdate _preserve(this);
	//GetRichEditCtrl().LockWindowUpdate();
	//long horz_scroll = 0;
	//long vert_scroll = 0;
	//GetRichEditCtrl().GetScrollPos(horz_scroll, vert_scroll);

	// Canonicalise word
	cdustring temp(word);
	size_t wlen = temp.length();
	if ((wlen > 0) && ((temp[wlen - 1] == '\'') || (temp[wlen - 1] == 'Õ')))
	{
		if ((wlen > 1) && (temp[wlen - 2] != 's') && (temp[wlen - 2] != 'S'))
		{
			temp[wlen - 1] = '\0';
			wlen--;
		}
	}
	
	// Check word (convert to local charset first)
	cdstring utf8 = temp.ToUTF8();
	const CSpellPlugin::SpError* sperr = CPluginManager::sPluginManager.GetSpelling()->CheckWord(utf8);
	
	// Clear mark if no error
	if (sperr == NULL)
	{
		MarkMisspelled(start, start + wlen, false);
	}
	
	// Check for error with auto-correct
	else if (sperr->do_replace && sperr->replacement && *sperr->replacement)
	{
		// Correct the word
		StPreserveSelection _preserve(this);
		SetSelectionRange(start, start + wlen);

		cdustring utf16_replace(sperr->replacement);
		size_t new_wlen = utf16_replace.length();
		InsertText(utf16_replace);

		// Correct selection for change in length
		_preserve.Offset(start + wlen, new_wlen - wlen);
		result = true;
	}
	
	// Mark the error
	else
		MarkMisspelled(start, start + wlen, true);
	
	//GetRichEditCtrl().SetScrollPos(horz_scroll, vert_scroll);
	//GetRichEditCtrl().UnlockWindowUpdate();

	return result;
}

void CCmdEditView::RecheckAllWords(const cdustring& word)
{
	// Locate and re-check each occurrence of the word
	CString temp = word.c_str();
	FINDTEXTEX ft;
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = temp;
	long pos;
	while ((pos = GetRichEditCtrl().FindText(FR_DOWN | FR_WHOLEWORD, &ft)) >= 0)
	{
		cdustring old_word;
		long startPos = 0;
		GetWordAt(pos, old_word, &startPos, true);
		CheckWord(old_word, startPos);

		// Resume searching after this word
		ft.chrg.cpMin = pos + 1;
	}
}

void CCmdEditView::SpellInsertText(long start, long numchars)
{
	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return;

	// Save current selection pos
	long sel_start;
	long sel_end;
	GetSelectionRange(sel_start, sel_end);

	// Determine reasonable buffer around cursor
	long cursor_pos = start + numchars;
	long text_start = cursor_pos - (numchars + 256);
	text_start = max(text_start, 0L);
	long text_len = cursor_pos - text_start;
	text_len += max(text_len / 5L, 256L);

	// Copy text into buffer
	cdustring text;
	GetSpellTextRange(text_start, text_start + text_len, text);

	// Locate the beginning of the word around the first inserted character.
	unsigned long i = (cursor_pos - text_start) - numchars;
	while ((i > 0) && (IS_WORD_CHAR(text[(cdustring::size_type)(i - 1)]) || (text[(cdustring::size_type)(i - 1)] == '.') || (text[(cdustring::size_type)(i - 1)] == '\'') || (text[(cdustring::size_type)(i - 1)] == FANCY_APOSTROPHE)))
	{
		i--;
	}

	// Locate the end of the word around the last inserted character and terminate buffer there.
	unsigned long j = cursor_pos - text_start;
	while ((j < text_len) && IS_WORD_CHAR(text[(cdustring::size_type)j]) || (text[(cdustring::size_type)j] == '.') || (text[(cdustring::size_type)j] == '\'') || (text[(cdustring::size_type)j] == FANCY_APOSTROPHE))
	{
		j++;
	}
	text[(cdustring::size_type)j] = 0;

	long text_cursor = i;
	cdustring utf16_check(text, text_cursor);
	cdstring utf8_check = utf16_check.ToUTF8();
	CPluginManager::sPluginManager.GetSpelling()->CheckText(utf8_check);

	if (CPluginManager::sPluginManager.GetSpelling()->HasErrors())
	{
		for(const CSpellPlugin::SpError* sperr = CPluginManager::sPluginManager.GetSpelling()->NextError(utf8_check); 
					sperr != NULL; sperr = CPluginManager::sPluginManager.GetSpelling()->NextError(utf8_check))
		{
			// Do auto replace
			if (sperr->do_replace && sperr->replacement && *sperr->replacement)
			{
				// Convert utf8 sperr offsets to utf16
				long usel_start = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_start);
				long usel_end = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_end);

				// Replace misspelled word
				StPreserveSelection _preserve(this);
				SetSelectionRange(text_start + text_cursor + usel_start, text_start + text_cursor + usel_end);
				cdustring utf16_replace(sperr->replacement);
				InsertText(utf16_replace);

				// Replaced current error
				CPluginManager::sPluginManager.GetSpelling()->Replace();
				
				// Need to adjust text being tested for the replacement
				cdstring temp(utf8_check, 0, sperr->sel_start);
				temp += sperr->replacement;
				temp.append(utf8_check, sperr->sel_end, cdstring::npos);
				utf8_check.steal(temp.grab_c_str());
			}
			else if (CPluginManager::sPluginManager.GetSpelling()->ErrorIsPunct())
			{
				// Skip current error
				CPluginManager::sPluginManager.GetSpelling()->Skip();
			}
			else
			{
				// Convert utf8 sperr offsets to utf16
				long usel_start = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_start);
				long usel_end = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_end);

				// Mark as misspelled
				MarkMisspelled(text_start + text_cursor + usel_start, text_start + text_cursor + usel_end, true);

				// Skip current error
				CPluginManager::sPluginManager.GetSpelling()->Skip();
			}
		}
	}
}

void CCmdEditView::SpellTextChange()
{
	// Check whether processing can be done
	if (!mRTSpell.enabled || mRTSpell.busy)
		return;

	//GetRichEditCtrl().LockWindowUpdate();

	StValueChanger<bool> _change(mRTSpell.busy, true);
	mRTSpell.pending = false;

	long newCharCount = GetTextLength();
	long newWordStartPos = mRTSpell.wordStartPos;

	// Determine the new cursor position
	long selStart;
	long selEnd;
	GetSelectionRange(selStart, selEnd);
	long newCursorPos = selEnd;

	// Determine the dimensions of the current word
	cdustring word;
	long wordLen = 0;
	if (mRTSpell.wordStartPos >= 0)
	{
		// Adjust cached start pos if change occurs in front of cacched word pos
		if (newCursorPos < mRTSpell.wordStartPos)
		{
			if (newCharCount > mRTSpell.charCount)
				mRTSpell.wordStartPos += newCharCount - mRTSpell.charCount;
			else if (newCharCount < mRTSpell.charCount)
				mRTSpell.wordStartPos -= mRTSpell.charCount - newCharCount;
		}
	
		// Now get cached word at new position
		wordLen = GetWordAt(mRTSpell.wordStartPos, word, &newWordStartPos, true);
		mRTSpell.wordStartPos = newWordStartPos;
	}

	// Determine what events occurred, and process them according to the current state
	if (newCharCount - mRTSpell.charCount == 1)
	{
		// Single character inserted.
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Do nothing.
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			// Always unmark word
			if (wordLen)
				MarkMisspelled(mRTSpell.wordStartPos, mRTSpell.wordStartPos + wordLen, false);
			break;
		}
	}
	else if (newCharCount < mRTSpell.charCount)
	{
		// One or more characters deleted
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Do nothing
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			// Always unmark word
			MarkMisspelled(mRTSpell.wordStartPos, mRTSpell.wordStartPos + wordLen, false);
			break;
		}
	}
	else if (newCharCount - mRTSpell.charCount > 1)
	{
		// Multiple characters inserted
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			SpellInsertText(newCursorPos - (newCharCount - mRTSpell.charCount), newCharCount - mRTSpell.charCount);
			break;
		case RTSpell::eInUnmarkedWord:
			// Check any words inserted after the current word. We don't
			// want to check the current word here because it may not be complete
			if (newCursorPos - (mRTSpell.wordStartPos + wordLen) > 0)
				SpellInsertText(mRTSpell.wordStartPos + wordLen,
									newCursorPos - (mRTSpell.wordStartPos + wordLen));
			break;
		case RTSpell::eInMarkedWord:
			// Check all inserted text including the current word. The
			// insertion may have changed the current word from misspelled to OK
			SpellInsertText(mRTSpell.wordStartPos, newCursorPos - mRTSpell.wordStartPos);
		}
	}

	if ((mRTSpell.wordStartPos >= 0) && (wordLen > 0) &&
	  	((newCursorPos < mRTSpell.wordStartPos) || (newCursorPos > mRTSpell.wordStartPos + wordLen)))
	{
		// Cursor moved outside current word
		bool wordChanged = false;
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Can't happen
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			wordChanged = CheckWord(word, mRTSpell.wordStartPos);
			if (wordChanged)
			{
				// Adjust cached cursor positions for change in word
				long new_wordLen = GetWordAt(mRTSpell.wordStartPos, word, &mRTSpell.wordStartPos, true);
				newCursorPos += new_wordLen - wordLen;
				mRTSpell.cursorPos += new_wordLen - wordLen;
			}
			break;
		}

		// Must reset style in some cases
		if (newCursorPos == mRTSpell.cursorPos + 1)
		{
			MarkMisspelled(newCursorPos - 1, newCursorPos, false);
		}
	}

	// Update the current state.
	wordLen = GetWordAt(newCursorPos, word, &mRTSpell.wordStartPos, false);
	if (wordLen >= 0)
	{
		mRTSpell.state = RTSpell::eInUnmarkedWord;

		// If the selection is being extended by shift/arrow or dragging
		// the mouse, calling isMarkedMisspelled will interfere with the
		// extension (because isMarkedMisspelled must change the selection to
		// sample the character format). If the selection is being extended,
		// we won't	determine if the current word is marked, to avoid
		// interfering with	the selection; we'll assume the word is unmarked.
		// This means that a correction to the marked word (by deleting the
		// selected characters, for example) might be missed so the word
		// would remain marked, but that's not too serious since the user
		// can force a re-check by passing the cursor through the word.
		if (selStart == selEnd)
		{
			// Determine if the current word is marked. Check to left and right of cursor.
			if ((newCursorPos > 0) && IsMisspelled(newCursorPos - 1, newCursorPos) || IsMisspelled(newCursorPos, newCursorPos + 1))
				mRTSpell.state = RTSpell::eInMarkedWord;
		}
	}
	else
		mRTSpell.state = RTSpell::eNotInWord;
	mRTSpell.cursorPos = newCursorPos;
	mRTSpell.charCount = newCharCount;

	//GetRichEditCtrl().UnlockWindowUpdate();
}

bool CCmdEditView::PauseSpelling(bool pause)
{
	if (pause)
	{
		bool was_enabled = mRTSpell.enabled;
		mRTSpell.enabled = false;
		return was_enabled;
	}
	else if (!mRTSpell.enabled)
	{
		mRTSpell.enabled = true;
		
		// Determine the new cursor position
		long selStart;
		long selEnd;
		GetSelectionRange(selStart, selEnd);

		// Update state
		mRTSpell.state = RTSpell::eNotInWord;
		mRTSpell.wordStartPos = selEnd;
		mRTSpell.cursorPos = selEnd;
		mRTSpell.charCount = GetTextLength();
		
		return true;
	}
	
	return true;
}

StStopRedrawPreserveUpdate::StStopRedrawPreserveUpdate(CCmdEditView* view)
{
	mView = view;
	mRgn = NULL;
	mRedrawWasOn = mView->GetCtrlRedraw();
	if (mRedrawWasOn)
	{
		mRgn = new CRgn;
		mRgn->CreateRectRgn(0, 0, 0, 0);
		int result = mView->GetRichEditCtrl().GetUpdateRgn(mRgn);
		switch(result)
		{
		case ERROR:
		case NULLREGION:
			delete mRgn;
			mRgn = NULL;
			break;
		default:;
		}
		mView->SetCtrlRedraw(false);
	}
}

StStopRedrawPreserveUpdate::~StStopRedrawPreserveUpdate()
{
	if (mRedrawWasOn)
	{
		mView->SetCtrlRedraw(true);
		if (mRgn)
			mView->GetRichEditCtrl().InvalidateRgn(mRgn);
	}
	delete mRgn;
}
