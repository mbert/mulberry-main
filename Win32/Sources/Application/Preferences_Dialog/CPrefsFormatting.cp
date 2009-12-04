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


// CPrefsFormatting.cpp : implementation file
//


#include "CPrefsFormatting.h"

#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsFormatting dialog

IMPLEMENT_DYNCREATE(CPrefsFormatting, CPrefsPanel)

CPrefsFormatting::CPrefsFormatting() : CPrefsPanel(CPrefsFormatting::IDD)
{
	//{{AFX_DATA_INIT(CPrefsFormatting)
	//}}AFX_DATA_INIT
}

CPrefsFormatting::~CPrefsFormatting()
{
}

void CPrefsFormatting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsFormatting)
	DDX_UTF8Text(pDX, IDC_LISTFONT, mListFont);
	DDX_Control(pDX, IDC_LISTFONT, mListFontCtrl);
	DDX_UTF8Text(pDX, IDC_LISTFONTSIZE, mListFontSize);
	DDX_Control(pDX, IDC_LISTFONTSIZE, mListFontSizeCtrl);
	DDX_UTF8Text(pDX, IDC_DISPLAYFONT, mDisplayFont);
	DDX_Control(pDX, IDC_DISPLAYFONT, mDisplayFontCtrl);
	DDX_UTF8Text(pDX, IDC_DISPLAYFONTSIZE, mDisplayFontSize);
	DDX_Control(pDX, IDC_DISPLAYFONTSIZE, mDisplayFontSizeCtrl);
	DDX_UTF8Text(pDX, IDC_PRINTERFONT, mPrinterFont);
	DDX_Control(pDX, IDC_PRINTERFONT, mPrinterFontCtrl);
	DDX_UTF8Text(pDX, IDC_PRINTERFONTSIZE, mPrinterFontSize);
	DDX_Control(pDX, IDC_PRINTERFONTSIZE, mPrinterFontSizeCtrl);
	DDX_UTF8Text(pDX, IDC_CAPTIONFONT, mCaptionFont);
	DDX_Control(pDX, IDC_CAPTIONFONT, mCaptionFontCtrl);
	DDX_UTF8Text(pDX, IDC_CAPTIONFONTSIZE, mCaptionFontSize);
	DDX_Control(pDX, IDC_CAPTIONFONTSIZE, mCaptionFontSizeCtrl);
	DDX_UTF8Text(pDX, IDC_FIXEDFONT, mFixedFont);
	DDX_Control(pDX, IDC_FIXEDFONT, mFixedFontCtrl);
	DDX_UTF8Text(pDX, IDC_FIXEDFONTSIZE, mFixedFontSize);
	DDX_Control(pDX, IDC_FIXEDFONTSIZE, mFixedFontSizeCtrl);
	DDX_UTF8Text(pDX, IDC_HTMLFONT, mHTMLFont);
	DDX_Control(pDX, IDC_HTMLFONT, mHTMLFontCtrl);
	DDX_UTF8Text(pDX, IDC_HTMLFONTSIZE, mHTMLFontSize);
	DDX_Check(pDX, IDC_USESTYLEDFONTS, mUseStyles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsFormatting, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsFormatting)
	ON_BN_CLICKED(IDC_LISTFONTBTN, OnListFontBtn)
	ON_BN_CLICKED(IDC_DISPLAYFONTBTN, OnDisplayFontBtn)
	ON_BN_CLICKED(IDC_PRINTERFONTBTN, OnPrinterFontBtn)
	ON_BN_CLICKED(IDC_CAPTIONFONTBTN, OnCaptionFontBtn)
	ON_BN_CLICKED(IDC_FIXEDFONTBTN, OnFixedFontBtn)
	ON_BN_CLICKED(IDC_HTMLFONTBTN, OnHTMLFontBtn)
	ON_COMMAND_RANGE(IDM_SIZE_8, IDM_SIZE_24, OnChangeMinimumSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrefsFormatting::InitControls(void)
{
	// Subclass buttons
	mMinimumFontCtrl.SubclassDlgItem(IDC_MINIMUMFONTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mMinimumFontCtrl.SetMenu(IDR_POPUP_FONTSIZE);
}

void CPrefsFormatting::SetControls(void)
{
	// Subclass buttons
	mMinimumFontCtrl.SetSize(mCopyPrefs->mMinimumFont.GetValue());
}

// Set up params for DDX
void CPrefsFormatting::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	HDC hDC = ::GetDC(nil);
	int logpix = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (logpix == 0)
		logpix = 1;
	::ReleaseDC(nil, hDC);

	// Set values
	mListTextFont = prefs->mListTextFontInfo.GetValue();
	mListFont = mListTextFont.logfont.lfFaceName;
	int ht = mListTextFont.logfont.lfHeight;
	int pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mListFontSize = (unsigned long)pt_size;

	mDisplayTextFont = prefs->mDisplayTextFontInfo.GetValue();
	mDisplayFont = mDisplayTextFont.logfont.lfFaceName;
	ht = mDisplayTextFont.logfont.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mDisplayFontSize = (unsigned long)pt_size;

	mPrintTextFont = prefs->mPrintTextFontInfo.GetValue();
	mPrinterFont = mPrintTextFont.logfont.lfFaceName;
	ht = mPrintTextFont.logfont.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mPrinterFontSize = (unsigned long)pt_size;

	mCaptionTextFont = prefs->mCaptionTextFontInfo.GetValue();
	mCaptionFont = mCaptionTextFont.logfont.lfFaceName;
	ht = mCaptionTextFont.logfont.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mCaptionFontSize = (unsigned long)pt_size;

	mFixedTextFont = prefs->mFixedTextFontInfo.GetValue();
	mFixedFont = mFixedTextFont.logfont.lfFaceName;
	ht = mFixedTextFont.logfont.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mFixedFontSize = (unsigned long)pt_size;

	mHTMLTextFont = prefs->mHTMLTextFontInfo.GetValue();
	mHTMLFont = mHTMLTextFont.logfont.lfFaceName;
	ht = mHTMLTextFont.logfont.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mHTMLFontSize = (unsigned long)pt_size;

	mUseStyles = prefs->mUseStyles.GetValue();
}

// Get params from DDX
void CPrefsFormatting::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	prefs->mListTextFontInfo.SetValue(mListTextFont);
	prefs->mDisplayTextFontInfo.SetValue(mDisplayTextFont);
	prefs->mPrintTextFontInfo.SetValue(mPrintTextFont);
	prefs->mCaptionTextFontInfo.SetValue(mCaptionTextFont);
	prefs->mFixedTextFontInfo.SetValue(mFixedTextFont);
	prefs->mHTMLTextFontInfo.SetValue(mHTMLTextFont);

	prefs->mUseStyles.SetValue(mUseStyles);
	prefs->mMinimumFont.SetValue(mMinimumFontCtrl.GetSize());
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsFormatting message handlers

void CPrefsFormatting::OnListFontBtn()
{
	DoFontChange(mListTextFont, mListFontCtrl, mListFontSizeCtrl);
}

void CPrefsFormatting::OnDisplayFontBtn()
{
	DoFontChange(mDisplayTextFont, mDisplayFontCtrl, mDisplayFontSizeCtrl);
}

void CPrefsFormatting::OnPrinterFontBtn()
{
	DoFontChange(mPrintTextFont, mPrinterFontCtrl, mPrinterFontSizeCtrl);
}

void CPrefsFormatting::OnCaptionFontBtn()
{
	DoFontChange(mCaptionTextFont, mCaptionFontCtrl, mCaptionFontSizeCtrl);
}

void CPrefsFormatting::OnFixedFontBtn()
{
	DoFontChange(mFixedTextFont, mFixedFontCtrl, mFixedFontSizeCtrl);
}

void CPrefsFormatting::OnHTMLFontBtn()
{
	DoFontChange(mHTMLTextFont, mHTMLFontCtrl, mHTMLFontSizeCtrl);
}

void CPrefsFormatting::DoFontChange(SLogFont& font, CStatic& font_ctrl, CStatic& size_ctrl)
{
	CFontDialog dlg(&font.logfont, CF_SCREENFONTS, NULL, CSDIFrame::GetAppTopWindow());
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&font.logfont);

		// Update captions
		font_ctrl.SetWindowText(font.logfont.lfFaceName);
		CString size_txt;
		size_txt.Format(_T("%d"), dlg.GetSize()/10);
		size_ctrl.SetWindowText(size_txt);
	}
}

void CPrefsFormatting::OnChangeMinimumSize(UINT nID)
{
	mMinimumFontCtrl.SetValue(nID);
}
