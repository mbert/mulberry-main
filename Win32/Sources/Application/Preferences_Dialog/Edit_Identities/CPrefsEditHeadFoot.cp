/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CPrefsEditHeadFoot.cpp : implementation file
//


#include "CPrefsEditHeadFoot.h"

#include "CErrorHandler.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSDIFrame.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CTextEngine.h"
#include "CUnicodeUtils.h"
#include "CURL.h"

#include "cdfstream.h"

#include <strstream>

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditHeadFoot dialog


CPrefsEditHeadFoot::CPrefsEditHeadFoot(bool allow_file, CWnd* pParent /*=NULL*/)
	: CHelpDialog(allow_file ? IDD2 : IDD1, pParent)
{
	mAllowFile = allow_file;

	//{{AFX_DATA_INIT(CPrefsEditHeadFoot)
	//}}AFX_DATA_INIT
}


void CPrefsEditHeadFoot::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsEditHeadFoot)
	DDX_UTF8Text(pDX, IDC_EDITHEADFOOT_TEXT, mEditHeadFoot);
	DDX_Control(pDX, IDC_EDITHEADFOOT_TEXT, mEditHeadFootCtrl);
	DDX_Control(pDX, IDC_EDITHEADFOOT_RULER, mEditRuler);
	if (mAllowFile)
	{
		DDX_Radio(pDX, IDC_EDITHEADFOOT_USEFILE, mUseFile);
		DDX_UTF8Text(pDX, IDC_EDITHEADFOOT_FILE, mFileName);
		DDX_Control(pDX, IDC_EDITHEADFOOT_FILE, mFileNameCtrl);
	}
	DDX_Control(pDX, IDOK, mOKBtn);
	DDX_Control(pDX, IDCANCEL, mCancelBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsEditHeadFoot, CHelpDialog)
	//{{AFX_MSG_MAP(CPrefsEditHeadFoot)
	ON_BN_CLICKED(IDC_EDITHEADFOOT_USEFILE, OnUseFile)
	ON_BN_CLICKED(IDC_EDITHEADFOOT_USETEXT, OnUseText)
	ON_BN_CLICKED(IDC_EDITHEADFOOT_CHOOSE, OnChooseFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditHeadFoot message handlers

BOOL CPrefsEditHeadFoot::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Set fonts to prefs default
	mEditHeadFootCtrl.SetFont(mFont);
	mEditRuler.SetFont(mFont);

	// Resize window to wrap length
	ResizeToWrap();
	
	if (mAllowFile)
		SetUseFile(mUseFile == 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Resize Window to wrap length
void CPrefsEditHeadFoot::ResizeToWrap(void)
{

	// Get average width of font chars
	TEXTMETRIC tm;
	CDC* pDC = mEditHeadFootCtrl.GetDC();
	CFont* old_font = pDC->SelectObject(mFont);
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
	mEditHeadFootCtrl.GetClientRect(wndSize);
	int change_width = mWrap * width - wndSize.Width() + 24;
	mEditRuler.GetClientRect(wndSize);
	int change_height = 2 * height - wndSize.Height() + 4;
	if (change_width || change_height)
	{
		// Resize and move all items
		CRect r;
		mEditHeadFootCtrl.GetWindowRect(r);
		ScreenToClient(r);
		mEditHeadFootCtrl.SetWindowPos(nil, r.left, r.top + change_height, r.Width() + change_width, r.Height(), SWP_NOZORDER);

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
	char ruler_txt[256];

	// Clip wrap to fit in 256
	if (mWrap > 120) mWrap = 120;

	// Top line of ruler
	for(short i = 0; i < mWrap; i++)
		ruler_txt[i] = '-';
	ruler_txt[mWrap] = '\r';
	ruler_txt[mWrap + 1] = '\n';

	// Do top line text
	ruler_txt[0] = '<';
	ruler_txt[mWrap - 1] = '>';

	cdstring wrapper = (long) mWrap;
	wrapper += " characters";
	
	if (wrapper.length() + 4 < mWrap)
	{
		unsigned long start = (mWrap - wrapper.length())/2;
		::memcpy(&ruler_txt[start], wrapper.c_str(), wrapper.length());
	}
	
	// Bottom line of ruler
	for(short i = 0; i < mWrap; i++)
	{
		if (i % mSpacesPerTab)
			ruler_txt[mWrap + i + 2] = '\'';
		else
			ruler_txt[mWrap + i + 2] = '|';
	}
	ruler_txt[2 * mWrap + 2] = '\0';

	// Set ruler text
	mEditRuler.SetWindowText(cdstring(ruler_txt).win_str());
}

// Set text in editor
void CPrefsEditHeadFoot::SetEditorText(const cdstring& text)
{
	if (mAllowFile && (::strncmpnocase(text.c_str(), cFileURLScheme, ::strlen(cFileURLScheme)) == 0))
	{
		// Set radio button if present
		if (mUseFile)
			mUseFile = 0;

		// Decode the URL to local path
		cdstring fpath(text.c_str() + ::strlen(cFileURLScheme));
		cdstring temp(cURLHierarchy);
		::strreplace(fpath.c_str_mod(), temp, os_dir_delim);
		fpath.DecodeURL();
		mFileName = fpath;

		UpdateFile();
	}
	else
	{
		// Set radio button if present
		if (mAllowFile)
			mUseFile = 1;

		// Cache ptr to prefs item
		mOldText = mEditHeadFoot = text;
	}
}

// Get text from editor
void CPrefsEditHeadFoot::GetEditorText(cdstring& text)
{
	// Look for choice of file rather than text
	if (mAllowFile && (mUseFile == 0))
	{
		// Convert path to URL & convert directories
		cdstring fpath(mFileName);
		fpath.EncodeURL(os_dir_delim);
		cdstring temp(os_dir_delim);
		::strreplace(fpath.c_str_mod(), temp, cURLHierarchy);
			
		// File spec uses file URL scheme at start
		text = cFileURLScheme;
		text += fpath;
	}
	else
	{
		text = mEditHeadFoot;

		// Only add if non-zero
		if (text.length())
		{
			// Save old, and set current wrap length
			short old_wrap = CRFC822::GetWrapLength();
			CRFC822::SetWrapLength(mWrap);

			// Create space for footer
			text.steal(const_cast<char*>(CTextEngine::WrapLines(text, text.length(), CRFC822::GetWrapLength(), false)));

			// Restore wrap length
			CRFC822::SetWrapLength(old_wrap);
		}
	}
}

// Set text in editor
void CPrefsEditHeadFoot::OnChooseFile()
{
	// Do standard open dialog
	// prompt the user
	CString filter = _T("All Files (*.*)|*.*||");
	CFileDialog dlgFile(true, NULL, NULL, OFN_FILEMUSTEXIST, filter, this);

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		mFileNameCtrl.SetWindowText(dlgFile.GetPathName());
		
		// Now read the file and put data into text
		UpdateFile();
	}
}

// Now read the file and put data into text
void CPrefsEditHeadFoot::UpdateFile()
{
	CString txt;
	mFileNameCtrl.GetWindowText(txt);
	cdstring fpath(txt);
	
	CString newtxt;
	if (!fpath.empty())
	{
		// Open file and read content
		cdifstream fin(fpath, std::ios::in | std::ios::binary);
		std::ostrstream out;
		::StreamCopy(fin, out, 0, ::StreamLength(fin));
		
		cdstring result = out.str();
		result.ConvertEndl();
		newtxt = result;
	}
	CUnicodeUtils::SetWindowTextUTF8(&mEditHeadFootCtrl, cdstring(newtxt));
}

// Set to use file
void CPrefsEditHeadFoot::OnUseFile()
{
	SetUseFile(true);
}

// Set to use text
void CPrefsEditHeadFoot::OnUseText()
{
	SetUseFile(false);
}

// Set text in editor
void CPrefsEditHeadFoot::SetUseFile(bool use_file)
{
	mFileNameCtrl.EnableWindow(use_file);
	GetDlgItem(IDC_EDITHEADFOOT_CHOOSE)->EnableWindow(use_file);
	mEditHeadFootCtrl.SetReadOnly(use_file);

	if (use_file)
	{
		mOldText = CUnicodeUtils::GetWindowTextUTF8(&mEditHeadFootCtrl);
		
		UpdateFile();
	}
	else
		CUnicodeUtils::SetWindowTextUTF8(&mEditHeadFootCtrl, mOldText);
}

bool CPrefsEditHeadFoot::PoseDialog(CPreferences* prefs, cdstring& txt, bool allow_file)
{
	bool result = false;

	// Create dlog
	CPrefsEditHeadFoot dlog(allow_file, CSDIFrame::GetAppTopWindow());

	// Make wrap safe
	unsigned long wrap = prefs->wrap_length.GetValue();
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	dlog.SetCurrentWrap(wrap);
	dlog.SetCurrentSpacesPerTab(prefs->spaces_per_tab.GetValue());
	
	CFont temp;
	temp.CreateFontIndirect(&prefs->mDisplayTextFontInfo.GetValue().logfont);
	dlog.SetFont(&temp);
	dlog.SetEditorText(txt);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetEditorText(txt);
		
		result = true;
	}
	
	return result;
}
