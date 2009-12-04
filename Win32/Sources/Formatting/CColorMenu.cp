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


// Source for CSizePopup class


#include "CColorMenu.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CDrawUtils.h"
#include "Colors.h"
#include "CSDIFrame.h"

IMPLEMENT_DYNCREATE(CColorPopup, CPopupButton)

BEGIN_MESSAGE_MAP(CColorPopup, CPopupButton)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CColorPopup::CColorPopup()
{
	mAlwaysEnable = true;
	mColor = 0;
	
	SetButtonText(false);
}

// Default destructor
CColorPopup::~CColorPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________


void CColorPopup::OnUpdateColor(CCmdUI* pCmdUI, bool enable)
{
	pCmdUI->Enable(enable);
	pCmdUI->SetCheck((pCmdUI->m_nID == mValue) && enable);
}
	

void CColorPopup::SetColor(RGBColor color)
{
	if (mColor != color)
	{
		mColor = color;

		if (color == kColorBlack)
			SetValue(IDM_COLOUR_BLACK);
		else if (color == kColorRed)
			SetValue(IDM_COLOUR_RED);
		else if (color == kColorGreen)
			SetValue(IDM_COLOUR_GREEN);
		else if (color == kColorBlue)
			SetValue(IDM_COLOUR_BLUE);
		else if (color == kColorYellow)
			SetValue(IDM_COLOUR_YELLOW);
		else if (color == kColorCyan)
			SetValue(IDM_COLOUR_CYAN);
		else if (color == kColorMagenta)
			SetValue(IDM_COLOUR_MAGENTA);
		else if (color == kColorMulberry)
			SetValue(IDM_COLOUR_MULBERRY);
		else if (color == kColorWhite)
			SetValue(IDM_COLOUR_WHITE);
		else
			SetValue(IDM_COLOUR_OTHER);
	
		// Force redraw
		RedrawWindow();
	}
}

RGBColor CColorPopup::RunPicker(void)
{
	CColorDialog dlg(0, 0, CSDIFrame::GetAppTopWindow());
	dlg.SetCurrentColor(mColor);
	dlg.m_cc.Flags |= CC_FULLOPEN;

	if (dlg.DoModal() == IDOK)
	{
		SetColor(dlg.GetColor());
	}
	return mColor;
}

void CColorPopup::DrawContent(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw default
	CPopupButton::DrawContent(lpDIS);

	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Fill color
	CBrush fill(mColor);
	CRect rect = CRect(lpDIS->rcItem);
	rect.DeflateRect(4, 4);
	rect.right -= 16;
	pDC->FillRect(&rect, &fill);
}