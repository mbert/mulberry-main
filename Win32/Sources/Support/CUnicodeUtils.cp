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


// CUnicodeUtils.cpp : implementation file
//

#include "CUnicodeUtils.h"

#include "cdustring.h"

// Get window text as utf8
cdstring CUnicodeUtils::GetWindowTextUTF8(HWND wnd)
{
	cdustring utf16;
	int nLen = ::GetWindowTextLengthW(wnd);
	utf16.reserve(nLen);
	::GetWindowTextW(wnd, utf16.c_str_mod(), nLen+1);
	
	return utf16.ToUTF8();
}

// Set window text as utf8
void CUnicodeUtils::SetWindowTextUTF8(HWND wnd, const cdstring& text)
{
	::SetWindowTextW(wnd, cdustring(text));
}

void CUnicodeUtils::AppendMenuUTF8(CMenu* menu, UINT nID)
{
	::AppendMenuW(*menu, nID, 0, (const WCHAR*) NULL);
}

void CUnicodeUtils::AppendMenuUTF8(CMenu* menu, UINT nID, UINT_PTR nIDNewItem, const cdstring& text)
{
	::AppendMenuW(*menu, nID, nIDNewItem, cdustring(text));
}

// Add UTF8 text to menu
void CUnicodeUtils::InsertMenuUTF8(CMenu* menu, UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const cdstring& text)
{
	::InsertMenuW(*menu, nPosition, nFlags, nIDNewItem, cdustring(text));
}

// Modify UTF8 text in menu
void CUnicodeUtils::ModifyMenuUTF8(CMenu* menu, UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const cdstring& text)
{
	::ModifyMenuW(*menu, nPosition, nFlags, nIDNewItem, cdustring(text));
}

// Get menu item text as utf8
cdstring CUnicodeUtils::GetMenuStringUTF8(const CMenu* menu, UINT nIDItem, UINT nFlags)
{
	cdustring utf16;
	int nStringLen = ::GetMenuString(*menu, nIDItem, NULL, 0, nFlags);
	if (nStringLen > 0)
	{
		utf16.reserve(nStringLen);
		::GetMenuStringW(*menu, nIDItem, utf16.c_str_mod(), nStringLen + 1, nFlags);
	}
	
	return utf16.ToUTF8();
}

void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, cdstring& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		value = CUnicodeUtils::GetWindowTextUTF8(hWndCtrl);
	}
	else
	{
		CUnicodeUtils::SetWindowTextUTF8(hWndCtrl, value);
	}
}

void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, UINT& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		cdstring txt = CUnicodeUtils::GetWindowTextUTF8(hWndCtrl);
		value = ::strtoul(txt.c_str(), NULL, 10);
	}
	else
	{
		cdstring txt((unsigned long)value);
		CUnicodeUtils::SetWindowTextUTF8(hWndCtrl, txt);
	}
}

void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		cdstring txt = CUnicodeUtils::GetWindowTextUTF8(hWndCtrl);
		value = ::strtoul(txt.c_str(), NULL, 10);
	}
	else
	{
		cdstring txt((long)value);
		CUnicodeUtils::SetWindowTextUTF8(hWndCtrl, txt);
	}
}

void AFXAPI DDV_UTF8MaxChars(CDataExchange* pDX, cdstring const& value, int nChars)
{
	ASSERT(nChars >= 1);        // allow them something
	if (pDX->m_bSaveAndValidate && value.length() > nChars)
	{
		TCHAR szT[32];
		wsprintf(szT, _T("%d"), nChars);
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_PARSE_STRING_SIZE, szT);
		AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
		prompt.Empty(); // exception prep
		pDX->Fail();
	}
	else if (pDX->m_idLastControl != 0 && pDX->m_bEditLastControl)
	{
	  HWND hWndLastControl;
	  pDX->m_pDlgWnd->GetDlgItem(pDX->m_idLastControl, &hWndLastControl);
		// limit the control max-chars automatically
		::SendMessage(hWndLastControl, EM_LIMITTEXT, nChars, 0);
	}
}
