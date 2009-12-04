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


// CUnicodeUtils.h : header file
//

#ifndef __CUnicodeUtils__MULBERRY__
#define __CUnicodeUtils__MULBERRY__

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CColorPickerButton window

class CUnicodeUtils
{
public:
	static cdstring GetWindowTextUTF8(const CWnd* wnd)				// Get window text as utf8
	{
		return GetWindowTextUTF8(*wnd);
	}
	static cdstring GetWindowTextUTF8(HWND wnd);					// Get window text as utf8

	static void SetWindowTextUTF8(CWnd* wnd, const cdstring& text)	// Set window text as utf8
	{
		SetWindowTextUTF8(*wnd, text);
	}
	static void SetWindowTextUTF8(HWND wnd, const cdstring& text);	// Set window text as utf8

	static void AppendMenuUTF8(CMenu* menu, UINT nID);												// Add separator to menu
	static void AppendMenuUTF8(CMenu* menu, UINT nID, UINT_PTR nIDNewItem, const cdstring& text);	// Add UTF8 text to menu
	static void InsertMenuUTF8(CMenu* menu, UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const cdstring& text);	// Add UTF8 text to menu
	static void ModifyMenuUTF8(CMenu* menu, UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const cdstring& text);	// Modify UTF8 text in menu
	static cdstring GetMenuStringUTF8(const CMenu* menu, UINT nIDItem, UINT nFlags);						// Get menu item text as utf8

};

void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, cdstring& value);		// Dialog edit field data exchange
void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, UINT& value);			// Dialog edit field data exchange
void AFXAPI DDX_UTF8Text(CDataExchange* pDX, int nIDC, int& value);				// Dialog edit field data exchange
void AFXAPI DDV_UTF8MaxChars(CDataExchange* pDX, cdstring const& value, int nChars);

/////////////////////////////////////////////////////////////////////////////

#endif
