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


// CEditMacro.cpp : implementation file
//


#include "CEditMacro.h"

#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditMacro dialog


CEditMacro::CEditMacro(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CEditMacro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMacro)
	//}}AFX_DATA_INIT
}


void CEditMacro::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMacro)
	DDX_UTF8Text(pDX, IDC_EDITMACRO_NAME, mName);
	DDX_UTF8Text(pDX, IDC_EDITMACRO_TEXT, mEditMacro);
	DDX_Control(pDX, IDC_EDITMACRO_TEXT, mEditMacroCtrl);
	DDX_Control(pDX, IDC_EDITMACRO_RULER, mEditRuler);
	DDX_Control(pDX, IDOK, mOKBtn);
	DDX_Control(pDX, IDCANCEL, mCancelBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditMacro, CHelpDialog)
	//{{AFX_MSG_MAP(CEditMacro)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMacro message handlers

BOOL CEditMacro::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Set fonts to prefs default
	mEditMacroCtrl.SetFont(CFontCache::GetDisplayFont());
	mEditRuler.SetFont(CFontCache::GetDisplayFont());

	// Resize window to wrap length
	SetRuler();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Resize Window to wrap length
void CEditMacro::SetRuler()
{
	unsigned long spaces = CPreferences::sPrefs->spaces_per_tab.GetValue();
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();

	// Make wrap safe
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	// Get average width of font chars
	TEXTMETRIC tm;
	CDC* pDC = mEditMacroCtrl.GetDC();
	CFont* old_font = pDC->SelectObject(CFontCache::GetDisplayFont());
	pDC->GetTextMetrics(&tm);
	pDC->SelectObject(old_font);
	ReleaseDC(pDC);
	long width = (tm.tmMaxCharWidth + tm.tmAveCharWidth)/2;
	long height = tm.tmHeight;

	// Do fixed pitch alert
	if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
		CErrorHandler::PutCautionAlert(false, IDS_FIXEDWIDTHWARN);
	else
		width = tm.tmAveCharWidth;

	// Resize to new wrap length
	CRect wndSize;
	mEditMacroCtrl.GetClientRect(wndSize);
	int change_width = wrap * width - wndSize.Width() + 24;
	mEditRuler.GetClientRect(wndSize);
	int change_height = 2 * height - wndSize.Height() + 4;
	if (change_width || change_height)
	{
		// Resize and move all items
		CRect r;
		mEditMacroCtrl.GetWindowRect(r);
		ScreenToClient(r);
		mEditMacroCtrl.SetWindowPos(nil, r.left, r.top + change_height, r.Width() + change_width, r.Height(), SWP_NOZORDER);

		mEditRuler.GetWindowRect(r);
		mEditRuler.SetWindowPos(nil, 0, 0, r.Width() + change_width, r.Height() + change_height, SWP_NOZORDER | SWP_NOMOVE);

		mOKBtn.GetWindowRect(r);
		ScreenToClient(r);
		mOKBtn.SetWindowPos(nil, r.left + change_width, r.top + change_height, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		mCancelBtn.GetWindowRect(r);
		ScreenToClient(r);
		mCancelBtn.SetWindowPos(nil, r.left + change_width, r.top + change_height, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		GetWindowRect(r);
		SetWindowPos(nil, 0, 0, r.Width() + change_width, r.Height() + change_height, SWP_NOZORDER | SWP_NOMOVE);
	}
	// Create ruler text
	cdstring ruler_txt;
	ruler_txt.reserve(256);

	// Clip wrap to fit in 256
	if (wrap > 120) wrap = 120;

	// Top line of ruler
	for(short i = 0; i < wrap; i++)
		ruler_txt[(cdstring::size_type)i] = '-';
	ruler_txt[(cdstring::size_type)wrap] = '\r';
	ruler_txt[(cdstring::size_type)(wrap + 1)] = '\n';

	// Do top line text
	ruler_txt[(cdstring::size_type)0] = '<';
	ruler_txt[(cdstring::size_type)(wrap - 1)] = '>';

	cdstring wrapper = (long) wrap;
	wrapper += " characters";
	
	if (wrapper.length() + 4 < wrap)
	{
		unsigned long start = (wrap - wrapper.length())/2;
		::memcpy(&ruler_txt[(cdstring::size_type)start], wrapper.c_str(), wrapper.length());
	}
	
	// Bottom line of ruler
	for(short i = 0; i < wrap; i++)
	{
		if (i % spaces)
			ruler_txt[(cdstring::size_type)(wrap + i + 2)] = '\'';
		else
			ruler_txt[(cdstring::size_type)(wrap + i + 2)] = '|';
	}
	ruler_txt[(cdstring::size_type)(2 * wrap + 2)] = '\0';

	// Set ruler text
	mEditRuler.SetWindowText(ruler_txt.win_str());
}

// Set text in editor
void CEditMacro::SetData(const cdstring& name, const cdstring& text)
{
	// Put name into edit field
	mName = name;

	// Put text into editor
	mEditMacro = text;
}

// Get text from editor
void CEditMacro::GetData(cdstring& name, cdstring& text)
{
	// Put name into edit field
	name = mName;

	// Copy info from panel into prefs
	text = mEditMacro;
}

bool CEditMacro::PoseDialog(cdstring& name, cdstring& macro)
{
	bool result = false;

	// Create dlog
	CEditMacro dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetData(name, macro);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetData(name, macro);
		result = true;
	}
	
	return result;
}
