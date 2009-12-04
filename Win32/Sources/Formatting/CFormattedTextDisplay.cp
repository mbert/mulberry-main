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



#include "CFormattedTextDisplay.h"

#include "CAddressBookManager.h"
#include "CClickElement.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CURL.h"

#include <math.h>

IMPLEMENT_DYNCREATE(CFormattedTextDisplay, CSpacebarEditView)

HCURSOR CFormattedTextDisplay::sHandCursor = NULL;


CFormattedTextDisplay::CFormattedTextDisplay()
{
	mFormatter = new CDisplayFormatter(this);
	mClickable = NULL;
	mTooltipElement = NULL;
	mCanClick = false;
	
	// Tooltips are active
	EnableToolTips();

	if (!sHandCursor)
		sHandCursor = AfxGetApp()->LoadCursor(IDC_HANDCURSOR);
}

CFormattedTextDisplay::~CFormattedTextDisplay()
{
	delete mFormatter;
}

BEGIN_MESSAGE_MAP(CFormattedTextDisplay, CSpacebarEditView)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_CAPTURE, OnUpdateNeedSel)
	ON_COMMAND(IDM_ADDR_CAPTURE, OnCaptureAddress)
END_MESSAGE_MAP()

// Change font in whole control
void CFormattedTextDisplay::WipeText(CFont* pFont, long scale)
{
	// Cleanup cached URL & click element
	mClickable = NULL;
	mTooltipElement = NULL;
	CancelToolTips(TRUE);

	// First clear out existing text
	GetRichEditCtrl().SetSel(0, -1);
	GetRichEditCtrl().ReplaceSel(_T(""));

	// Set background to white
	GetRichEditCtrl().SetBackgroundColor(FALSE, ::GetSysColor(COLOR_WINDOW));
	
	// Now set the font
	ResetFont(pFont);
	
	if (scale)
	{
		// Get current size
		CHARFORMAT format;
		format.dwMask = CFM_SIZE;
		GetRichEditCtrl().GetDefaultCharFormat(format);

		// Scale the current size by the amount we want
		format.yHeight = max(1.0, pow(1.2, scale) * format.yHeight);

		// Now do size change for default
		GetRichEditCtrl().SetSel(0, -1);
		format.dwMask = CFM_SIZE;
		GetRichEditCtrl().SetSelectionCharFormat(format);
	}
}

void CFormattedTextDisplay::Reset(bool click, long scale)
{
	// Cleanup cached URL & click element
	mClickable = NULL;
	mTooltipElement = NULL;
	CancelToolTips(TRUE);

	// Now reset the display formatter
	mFormatter->Reset(click, scale);
}

void CFormattedTextDisplay::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Check that mouse up was in same URL
	int po = mFormatter->CharFromPos(point);
	CPoint this_pt = GetRichEditCtrl().GetCharPos(po);
	CPoint next_pt = GetRichEditCtrl().GetCharPos(po + 1);
	CClickElement *element = mFormatter->FindCursor(po);
	bool not_line_end = (next_pt.x > this_pt.x);

	if (mClickable && (mClickable == element) && (not_line_end || (point.x < this_pt.x + 8)))
	{
		// Select URL first
		SetSelectionRange(mClickable->getStart(), mClickable->getStop());
		
		if (element->IsAnchor() || element->Act(mFormatter))	// WARNING mClickable sometimes gets set to NULL
											// during this call so revert to using 'element'
		{
			// Flash selection
			SetSelectionRange(element->getStart(), element->getStart());
			Sleep(50);
			SetSelectionRange(element->getStart(), element->getStop());
			Sleep(50);
			SetSelectionRange(element->getStart(), element->getStart());
			Sleep(50);
			SetSelectionRange(element->getStart(), element->getStop());
			
			// Change style to URL seen if formatted display
			if (mFormatter->mView == eViewFormatted)
			{
				mFormatter->ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
					mFormatter->FaceFormat(E_BOLD, element->getStart(), element->getStop());
				else
					mFormatter->FaceFormat(E_UNBOLD, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
					mFormatter->FaceFormat(E_UNDERLINE, element->getStart(), element->getStop());
				else
					mFormatter->FaceFormat(E_UNUNDERLINE, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
					mFormatter->FaceFormat(E_ITALIC, element->getStart(), element->getStop());
				else
					mFormatter->FaceFormat(E_UNITALIC, element->getStart(), element->getStop());
			}
			
			// Now do anchor action
			if (element->IsAnchor())
				element->Act(mFormatter);
		}
	}
	else
		// Do inherited
		CSpacebarEditView::OnLButtonUp(nFlags, point);

	mClickable = NULL;
}

void CFormattedTextDisplay::OnLButtonDown(UINT nFlags, CPoint point)
{
	int po = mFormatter->CharFromPos(point);
	CClickElement* element = mFormatter->FindCursor(po);
	if (mCanClick && (::GetKeyState(VK_MENU) >= 0) && (::GetKeyState(VK_SHIFT) >= 0) && (::GetKeyState(VK_CONTROL) >= 0))
		mClickable = element;
	else
	{
		mClickable = NULL;
		CSpacebarEditView::OnLButtonDown(nFlags, point);
	}
}

// Display and track context menu
void CFormattedTextDisplay::HandleContextMenu(CWnd* wnd, CPoint point)
{
	if (mCanClick)
	{
		// Get character at point
		CPoint client_point(point);
		ScreenToClient(&client_point);

		// Check that mouse up was in same URL
		int po = mFormatter->CharFromPos(client_point);
		CPoint this_pt = GetRichEditCtrl().GetCharPos(po);
		CPoint next_pt = GetRichEditCtrl().GetCharPos(po + 1);
		CClickElement *element = mFormatter->FindCursor(po);
		bool not_line_end = (next_pt.x > this_pt.x);

		if (element && !element->IsAnchor() && (not_line_end || (client_point.x < this_pt.x + 8)))
		{
			// Set selection
			SetSelectionRange(element->getStart(), element->getStart());

			// Get popup menu
			CMenu menu;
			menu.LoadMenu(IDR_POPUP_CONTEXT_URL_TEXT);
			CMenu* pPopup = menu.GetSubMenu(0);

			// Do popup menu of suggestions
			UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);

			if (popup_result == IDM_EDIT_OPENURL)
			{
				// Handle target action
				if (element->Act(mFormatter))
				{
					// Flash selection
					SetSelectionRange(element->getStart(), element->getStart());
					Sleep(50);
					SetSelectionRange(element->getStart(), element->getStop());
					Sleep(50);
					SetSelectionRange(element->getStart(), element->getStart());
					Sleep(50);
					SetSelectionRange(element->getStart(), element->getStop());
					
					// Change style to URL seen if formatted display
					if (mFormatter->mView == eViewFormatted)
					{
						mFormatter->ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, element->getStart(), element->getStop());
						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
							mFormatter->FaceFormat(E_BOLD, element->getStart(), element->getStop());
						else
							mFormatter->FaceFormat(E_UNBOLD, element->getStart(), element->getStop());
						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
							mFormatter->FaceFormat(E_UNDERLINE, element->getStart(), element->getStop());
						else
							mFormatter->FaceFormat(E_UNUNDERLINE, element->getStart(), element->getStop());
						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
							mFormatter->FaceFormat(E_ITALIC, element->getStart(), element->getStop());
						else
							mFormatter->FaceFormat(E_UNITALIC, element->getStart(), element->getStop());
					}
				}
			}
			else if (popup_result == IDM_EDIT_COPYURL)
			{
				// get url text
				cdstring url = element->GetDescriptor().win_str();

				// Convert to utf16
				cdustring utf16(url);
			
				// Now copy to scrap
				if (!::OpenClipboard(*this))
					return;
				::EmptyClipboard();

				// Allocate global memory for the text
				HANDLE hglb = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
				if (hglb)
				{
					// Copy to global after lock
					unichar_t* lptstr = (unichar_t*) ::GlobalLock(hglb);
					::unistrcpy(lptstr, utf16);
					::GlobalUnlock(hglb);
					
					::SetClipboardData(CF_UNICODETEXT, hglb);
				}
				::CloseClipboard();
			}
			else
				// Do normal command processing
				SendMessage(WM_COMMAND, popup_result);
			
			return;
		}
	}

	CSpacebarEditView::HandleContextMenu(wnd, point);
}

BOOL CFormattedTextDisplay::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ((nHitTest == HTCLIENT) && (pWnd == this) && mCanClick &&
		(::GetKeyState(VK_MENU) >= 0) && (::GetKeyState(VK_SHIFT) >= 0) && (::GetKeyState(VK_CONTROL) >= 0))
	{
		::SetCursor(sHandCursor);
		return TRUE;
	}

	return CSpacebarEditView::OnSetCursor(pWnd, nHitTest, message);
}

// Clip split move
void CFormattedTextDisplay::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect words;
	GetRichEditCtrl().GetRect(words);
	if (words.PtInRect(point))
	{
		int po = mFormatter->CharFromPos(point);
		CPoint this_pt = GetRichEditCtrl().GetCharPos(po);
		CPoint next_pt = GetRichEditCtrl().GetCharPos(po + 1);
		CClickElement* element = mFormatter->FindCursor(po);

		bool not_line_end = (next_pt.x > this_pt.x);
		mCanClick = element && (po < GetTextLengthEx() - 1) &&
						(not_line_end || (point.x < this_pt.x + 8));
	}
	else
		mCanClick = false;

	CSpacebarEditView::OnMouseMove(nFlags, point);
}

// Capture selected text as an address
void CFormattedTextDisplay::OnCaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// Do capture if address capability available
	if (CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}

#pragma mark ____________________________________Tooltips

int CFormattedTextDisplay::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect words;
	GetRichEditCtrl().GetRect(words);
	if (words.PtInRect(point))
	{
		int po = mFormatter->CharFromPos(point);
		CPoint this_pt = GetRichEditCtrl().GetCharPos(po);
		CPoint next_pt = GetRichEditCtrl().GetCharPos(po + 1);
		const_cast<CFormattedTextDisplay*>(this)->mTooltipElement = mFormatter->FindCursor(po);

		bool not_line_end = (next_pt.x > this_pt.x);
		if (mTooltipElement && (po < GetTextLengthEx() - 1) &&
						(not_line_end || (point.x < this_pt.x + 8)))
		{
			// hits against child windows always center the tip
			if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
			{
				// setup the TOOLINFO structure
				pTI->hwnd = m_hWnd;
				pTI->uId = 1;
				pTI->lpszText = LPSTR_TEXTCALLBACK;
				pTI->rect = words;
				return 1;
			}
		}
	}

	// Not found
	return -1;
}

BOOL CFormattedTextDisplay::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	UINT nID = pNMHDR->idFrom;
	if (nID != 1)
		return FALSE;
	
	// Tooltip element may have been set to NULL between the initial hit test and this attempt to display it
	if (!mTooltipElement)
		return FALSE;

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

	// Get text from element
	cdstring text = mTooltipElement->GetDescriptor();
	text.DecodeURL();
	
	// Check for mailto and decode =?'s
	if (text.compare_start(cMailtoURLScheme, true))
		CRFC822::TextFrom1522(text);
	
#ifdef _UNICODE
	mTooltipText = cdustring(text);
#else
	mTooltipText = text;
#endif

#ifdef _UNICODE
	pTTTW->lpszText = mTooltipText.c_str_mod();
#else
	pTTTA->lpszText = mTooltipText.c_str_mod();
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}
