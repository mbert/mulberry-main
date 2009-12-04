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



#include "CStyleToolbar.h"
#include "CLetterWindow.h"
#include "CDisplayFormatter.h"

#include "CMulberryApp.h"

#include "ETag.h"
#include "Colors.h"

#include "CPreferences.h"

int cToolbarHeight = 30;
int cButtonHeight = 22;
int cButtonWidth = 24;
int cButtonSpacing = 2;
int cFontWidth = 150;
int cSizeWidth = 48;
int cColorWidth = 40;

int cOffsetX = 3;
int cOffsetY = 4;

int cBoldULX = cOffsetX;
int cBoldULY = cOffsetY;
int cBoldLLX = cOffsetX + cButtonWidth;
int cBoldLLY = cOffsetY + cButtonHeight;

int cItalicULX = cBoldLLX + cButtonSpacing;
int cItalicULY = cOffsetY;
int cItalicLLX = cItalicULX + cButtonWidth;
int cItalicLLY = cItalicULY + cButtonHeight;

int cUnderlineULX = cItalicLLX + cButtonSpacing;
int cUnderlineULY = cOffsetY;
int cUnderlineLLX = cUnderlineULX + cButtonWidth;
int cUnderlineLLY = cUnderlineULY + cButtonHeight;

int cFontMenuULX = cUnderlineLLX + cButtonSpacing;
int cFontMenuULY = cOffsetY;
int cFontMenuLLX = cUnderlineULX + cFontWidth;
int cFontMenuLLY = cUnderlineULY + cButtonHeight;

int cSizeMenuULX = cFontMenuLLX + cButtonSpacing;
int cSizeMenuULY = cOffsetY;
int cSizeMenuLLX = cSizeMenuULX + cSizeWidth;
int cSizeMenuLLY = cSizeMenuULY + cButtonHeight;

int cColorMenuULX = cSizeMenuLLX + cButtonSpacing;
int cColorMenuULY = cOffsetY;
int cColorMenuLLX = cColorMenuULX + cColorWidth;
int cColorMenuLLY = cColorMenuULY + cButtonHeight;

int cLeftAlignULX = cColorMenuLLX + cButtonSpacing;
int cLeftAlignULY = cOffsetY;
int cLeftAlignLLX = cLeftAlignULX + cButtonWidth;
int cLeftAlignLLY = cLeftAlignULY + cButtonHeight;

int cCenterAlignULX = cLeftAlignLLX + cButtonSpacing;
int cCenterAlignULY = cOffsetY;
int cCenterAlignLLX = cCenterAlignULX + cButtonWidth;
int cCenterAlignLLY = cCenterAlignULY + cButtonHeight;

int cRightAlignULX = cCenterAlignLLX + cButtonSpacing;
int cRightAlignULY = cOffsetY;
int cRightAlignLLX = cRightAlignULX + cButtonWidth;
int cRightAlignLLY = cRightAlignULY + cButtonHeight;





IMPLEMENT_DYNCREATE(CStyleToolbar, CGrayBackground)

CStyleToolbar::CStyleToolbar()
{
}

CStyleToolbar::~CStyleToolbar()
{
}

void CStyleToolbar::UpdateUIPos(void)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;
	
	cToolbarHeight += large_offset;
}


BEGIN_MESSAGE_MAP(CStyleToolbar, CGrayBackground)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDC_LETTER_ENR_BOLD, OnBold)
	ON_COMMAND(IDC_LETTER_ENR_ITALIC, OnItalic)
	ON_COMMAND(IDC_LETTER_ENR_UNDERLINE, OnUnderline)
	ON_COMMAND(IDC_LETTER_ENR_LEFT, OnLeftAlign)
	ON_COMMAND(IDC_LETTER_ENR_CENTER, OnCenterAlign)
	ON_COMMAND(IDC_LETTER_ENR_RIGHT, OnRightAlign)
END_MESSAGE_MAP()


void CStyleToolbar::OnStyle(UINT nID)
{
	switch(nID)
	{
	case IDM_STYLE_PLAIN:
		OnPlain();
		break;
	case IDM_STYLE_BOLD:
		OnBold();
		break;
	case IDM_STYLE_ITALIC:
		OnItalic();
		break;
	case IDM_STYLE_UNDERLINE:
		OnUnderline();
		break;
	}
}


void CStyleToolbar::OnAlignment(UINT nID)
{
	switch(nID)
	{
	case IDM_ALIGN_LEFT:
		OnLeftAlign();
		break;
	case IDM_ALIGN_CENTER:
		OnCenterAlign();
		break;
	case IDM_ALIGN_RIGHT:
		OnRightAlign();
		break;
	}
}
void CStyleToolbar::OnFont(UINT nID)
{
	mFormatter->FontFormat(mFontMenu.GetFont(nID));
	mFontMenu.SetValue(nID);
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnFontSize(UINT nID)
{
	int mySize;
	switch(nID)
	{
	case IDM_SIZE_8:
		mySize = 8;
		break;
	case IDM_SIZE_9:
		mySize = 9;
		break;
	case IDM_SIZE_10:
		mySize = 10;
		break;
	case IDM_SIZE_11:
		mySize = 11;
		break;
	case IDM_SIZE_12:
		mySize = 12;
		break;
	case IDM_SIZE_13:
		mySize = 13;
		break;
	case IDM_SIZE_14:
		mySize = 14;
		break;
	case IDM_SIZE_16:
		mySize = 16;
		break;
	case IDM_SIZE_18:
		mySize = 18;
		break;
	case IDM_SIZE_20:
		mySize = 20;
		break;
	case IDM_SIZE_24:
		mySize = 24;
		break;
	default:
		mySize = 9;
		break;
	}
			
	mFormatter->FontSizeFormat(mySize, false);
	mSize.SetValue(nID);
	
	mFormatter->GetRichEditCtrl().SetFocus();
}


void CStyleToolbar::OnColor(UINT nID)
{
	RGBColor myColor;
	
	if(nID == IDM_COLOUR_OTHER)
	{
		myColor = mColor.RunPicker();
		mFormatter->ColorFormat(myColor);
	}	
	
	else
	{
		switch(nID)
		{
		case IDM_COLOUR_BLACK:
			myColor = kColorBlack;
			break;
		case IDM_COLOUR_RED:
			myColor = kColorRed;
			break;
		case IDM_COLOUR_GREEN:
			myColor = kColorGreen;
			break;
		case IDM_COLOUR_BLUE:
			myColor = kColorBlue;
			break;
		case IDM_COLOUR_YELLOW:
			myColor = kColorYellow;
			break;
		case IDM_COLOUR_CYAN:
			myColor = kColorCyan;
			break;
		case IDM_COLOUR_MAGENTA:
			myColor = kColorMagenta;
			break;
		case IDM_COLOUR_MULBERRY:
			myColor = kColorMulberry;
			break;
		case IDM_COLOUR_WHITE:
			myColor = kColorWhite;
			break;
		}
		mFormatter->ColorFormat(myColor);
		mColor.SetColor(myColor);
	}
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnUpdateStyle(CCmdUI* pCmdUI)
{
	bool enable;
	
	switch(mToolbarType)
	{
	case eContentSubPlain:
		enable = false;
		break;
	case eContentSubEnriched:
		enable = true;
		break;
	case eContentSubHTML:	
		enable = true;
		break;
	}
	
	if (enable)
	{
		pCmdUI->Enable(true);
		if (Is(pCmdUI->m_nID))
			pCmdUI->SetCheck(1);
		else
			pCmdUI->SetCheck(0);
	}
	else
		pCmdUI->Enable(false);
}



void CStyleToolbar::OnUpdateAlignment(CCmdUI* pCmdUI)
{
	bool enable;
	
	switch(mToolbarType)
	{
	case eContentSubPlain:
		enable = false;
		break;
	case eContentSubEnriched:
		enable = true;
		break;
	case eContentSubHTML:	
		enable = true;
		break;
	}
	
	if(enable)
	{
		pCmdUI->Enable(true);
		if(Is(pCmdUI->m_nID))
			pCmdUI->SetCheck(1);
		else
			pCmdUI->SetCheck(0);
	}
	else
		pCmdUI->Enable(false);
}

void CStyleToolbar::OnUpdateFont(CCmdUI* pCmdUI)
{
	mFontMenu.OnUpdateFont(pCmdUI, IsWindowVisible());
}

void CStyleToolbar::OnUpdateSize(CCmdUI* pCmdUI)
{
	mSize.OnUpdateSize(pCmdUI, IsWindowVisible());
}

void CStyleToolbar::OnUpdateColor(CCmdUI* pCmdUI)
{
	mColor.OnUpdateColor(pCmdUI, IsWindowVisible());
}


void CStyleToolbar::OnSelChange()
{
	CHARFORMAT2 currentFormat;
	DWORD continuous = mFormatter->GetContinuousCharSelection(currentFormat);
	
	if((continuous & CFM_BOLD) && (currentFormat.dwEffects & CFE_BOLD))
		mBold.SetPushed(1);
	else
		mBold.SetPushed(0);
	
	if(continuous & CFM_ITALIC && currentFormat.dwEffects & CFE_ITALIC)
		mItalic.SetPushed(1);
	else
		mItalic.SetPushed(0);
	
	if(continuous & CFM_UNDERLINE && currentFormat.dwEffects & CFE_UNDERLINE)
		mUnderline.SetPushed(1);
	else
		mUnderline.SetPushed(0);
	
	if(continuous & CFM_FACE)
		mFontMenu.SetValue(cdstring(currentFormat.szFaceName));
	else
		mFontMenu.SetValue(0);
	
	if(continuous & CFM_SIZE)
		mSize.SetSize((currentFormat.yHeight + 10) / 20);
	else
		mSize.SetSize(0);
	
	if(continuous & CFM_COLOR)
		mColor.SetColor(currentFormat.crTextColor);
	else
		mColor.SetValue(0);
	
	PARAFORMAT paraFormat;
	continuous = mFormatter->GetContinuousParaSelection(paraFormat);
	
	if((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_LEFT))
		mLeftAlignment.SetPushed(1);
	else
		mLeftAlignment.SetPushed(0);
	
	if((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_CENTER))
		mCenterAlignment.SetPushed(1);
	else
		mCenterAlignment.SetPushed(0);
	
	if((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_RIGHT))
		mRightAlignment.SetPushed(1);
	else
		mRightAlignment.SetPushed(0);
}


void CStyleToolbar::OnPlain()
{
	mFormatter->FaceFormat(E_PLAIN);
	mBold.SetPushed(false);
	mItalic.SetPushed(false);
	mUnderline.SetPushed(false);
}

void CStyleToolbar::OnBold()
{
	if(mBold.IsPushed())
	{
		mFormatter->FaceFormat(E_UNBOLD);
		mBold.SetPushed(false);
	}
	else
	{
		mFormatter->FaceFormat(E_BOLD);
		mBold.SetPushed(true);
	}
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnItalic()
{
	if(mItalic.IsPushed())
	{
		mFormatter->FaceFormat(E_UNITALIC);
		mItalic.SetPushed(false);
	}
	else
	{
		mFormatter->FaceFormat(E_ITALIC);
		mItalic.SetPushed(true);
	}
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnUnderline()
{
	if(mUnderline.IsPushed())
	{
		mFormatter->FaceFormat(E_UNUNDERLINE);
		mUnderline.SetPushed(false);
	}
	else
	{
		mFormatter->FaceFormat(E_UNDERLINE);
		mUnderline.SetPushed(true);
	}
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnLeftAlign()
{
	mFormatter->AlignmentFormat(E_FLEFT);
	mLeftAlignment.SetPushed(true);
	mCenterAlignment.SetPushed(false);
	mRightAlignment.SetPushed(false);
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnCenterAlign()
{
	mFormatter->AlignmentFormat(E_CENTER);
	mLeftAlignment.SetPushed(false);
	mCenterAlignment.SetPushed(true);
	mRightAlignment.SetPushed(false);
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

void CStyleToolbar::OnRightAlign()
{
	mFormatter->AlignmentFormat(E_FRIGHT);
	mLeftAlignment.SetPushed(false);
	mCenterAlignment.SetPushed(false);
	mRightAlignment.SetPushed(true);
	
	mFormatter->GetRichEditCtrl().SetFocus();
}

	
bool CStyleToolbar::GetBoldButton()
{
	return mBold.IsPushed();
}

bool CStyleToolbar::GetItalicButton()
{
	return mItalic.IsPushed();
}

bool CStyleToolbar::GetUnderlineButton()
{
	return mUnderline.IsPushed();
}

bool CStyleToolbar::GetLeftButton()
{
	return mLeftAlignment.IsPushed();
}

bool CStyleToolbar::GetCenterButton()
{
	return mCenterAlignment.IsPushed();
}

bool CStyleToolbar::GetRightButton()
{
	return mRightAlignment.IsPushed();
}

int CStyleToolbar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;

	// Create toolbar pane
	mToolbar.CreateEx(WS_EX_DLGMODALFRAME, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 400, cToolbarHeight, GetSafeHwnd(), (HMENU)IDC_LETTERHEADER);
	
	CString s;
	mBold.Create(s, CRect(cBoldULX, cBoldULY, cBoldLLX, cBoldLLY), this, IDC_LETTER_ENR_BOLD, IDC_STATIC, IDI_BOLDBTN);
	mItalic.Create(s, CRect(cItalicULX, cItalicULY, cItalicLLX, cItalicLLY), this, IDC_LETTER_ENR_ITALIC, IDC_STATIC, IDI_ITALICBTN);
	mUnderline.Create(s, CRect(cUnderlineULX, cUnderlineULY, cUnderlineLLX, cUnderlineLLY), this, IDC_LETTER_ENR_UNDERLINE, IDC_STATIC, IDI_UNDERLINEBTN);	
	mFontMenu.Create(s, CRect(cFontMenuULX, cFontMenuULY, cFontMenuLLX, cFontMenuLLY), this, IDC_STATIC, IDC_STATIC, IDI_POPUPBTN);
	mFontMenu.SetFont(CMulberryApp::sAppFont);
	LOGFONT myFont = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().logfont;
	mFontMenu.SetValue(cdstring(myFont.lfFaceName));
	mSize.Create(s, CRect(cSizeMenuULX, cSizeMenuULY, cSizeMenuLLX, cSizeMenuLLY), this, IDC_LETTER_ENR_SIZE, IDC_STATIC, IDI_POPUPBTN);
	mSize.SetMenu(IDR_POPUP_FONTSIZE);
	mSize.SetFont(CMulberryApp::sAppFont);
	mSize.SetValue((myFont.lfHeight + 10)/ 20);
	mColor.Create(s, CRect(cColorMenuULX, cColorMenuULY, cColorMenuLLX, cColorMenuLLY), this, IDC_LETTER_ENR_COLOR, IDC_STATIC, IDI_POPUPBTN);
	mColor.SetMenu(IDR_POPUP_FONTCOLOR);
	mColor.SetValue(IDR_POPUP_FONTCOLOR);
	mColor.SetFont(CMulberryApp::sAppFont);
	mLeftAlignment.Create(s, CRect(cLeftAlignULX, cLeftAlignULY, cLeftAlignLLX, cLeftAlignLLY), this, IDC_LETTER_ENR_LEFT, IDC_STATIC, IDI_ALIGNLEFTBTN);
	mCenterAlignment.Create(s, CRect(cCenterAlignULX, cCenterAlignULY, cCenterAlignLLX, cCenterAlignLLY), this, IDC_LETTER_ENR_CENTER, IDC_STATIC, IDI_ALIGNCENTERBTN);
	mRightAlignment.Create(s, CRect(cRightAlignULX, cRightAlignULY, cRightAlignLLX, cRightAlignLLY), this, IDC_LETTER_ENR_RIGHT, IDC_STATIC, IDI_ALIGNRIGHTBTN);

	return 0;
}

void CStyleToolbar::OnSize(UINT nType, int cx, int cy)
{
	mToolbar.SetWindowPos(&wndTop, 0, cToolbarHeight, cx, cy, SWP_NOMOVE);
	CGrayBackground::OnSize(nType, cx, cy);
}


void CStyleToolbar::SetFormatter(CDisplayFormatter* formatter)
{
	mFormatter = formatter;
}

void CStyleToolbar::OnEnrichedToolbar()
{
	mToolbarType = eContentSubEnriched;
	mRightAlignment.ShowWindow(SW_SHOW);
}

void CStyleToolbar::OnPlainToolbar()
{
	mToolbarType = eContentSubPlain;
}

void CStyleToolbar::OnHTMLToolbar()
{
	mToolbarType = eContentSubHTML;
	mRightAlignment.ShowWindow(SW_HIDE);
}



bool CStyleToolbar::Is(int nID)
{
	CHARFORMAT2 currentFormat;
	PARAFORMAT paraFormat;
	DWORD continuous;
	
	switch(nID)
	{
	case IDM_STYLE_PLAIN:
		continuous = mFormatter->GetContinuousCharSelection(currentFormat);
		return (((continuous & CFM_BOLD) && !(currentFormat.dwEffects & CFE_BOLD)) &&
				((continuous & CFM_ITALIC) && !(currentFormat.dwEffects & CFE_ITALIC)) &&
				((continuous & CFM_UNDERLINE) && !(currentFormat.dwEffects & CFE_UNDERLINE)));
	case IDM_STYLE_BOLD:
		continuous = mFormatter->GetContinuousCharSelection(currentFormat);
		return ((continuous & CFM_BOLD) && (currentFormat.dwEffects & CFE_BOLD));
	case IDM_STYLE_ITALIC:
		continuous = mFormatter->GetContinuousCharSelection(currentFormat);
		return ((continuous & CFM_ITALIC) && (currentFormat.dwEffects & CFE_ITALIC));
	case IDM_STYLE_UNDERLINE:
		continuous = mFormatter->GetContinuousCharSelection(currentFormat);
		return ((continuous & CFM_UNDERLINE) && (currentFormat.dwEffects & CFE_UNDERLINE));
	case IDM_ALIGN_LEFT:
		continuous = mFormatter->GetContinuousParaSelection(paraFormat);
		return ((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_LEFT));
	case IDM_ALIGN_CENTER:
		continuous = mFormatter->GetContinuousParaSelection(paraFormat);
		return ((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_CENTER));
	case IDM_ALIGN_RIGHT:
		continuous = mFormatter->GetContinuousParaSelection(paraFormat);
		return ((continuous & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_RIGHT));
	}
	
	return false;
}