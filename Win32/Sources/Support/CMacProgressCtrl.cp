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

// MacProgressCtrl.cpp : implementation file
//
//	CMacProgressCtrl class, version 1.0
//
//	Copyright (c) 1999 Paul M. Meidinger (pmmeidinger@yahoo.com)
//
// Feel free to modifiy and/or distribute this file, but
// do not remove this header.
//
// I would appreciate a notification of any bugs discovered or 
// improvements that could be made.
//
// This file is provided "as is" with no expressed or implied warranty.
//
//	History:
//		PMM	12/21/1999		Initial implementation.		


//#include "stdafx.h"
#include "CMacProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	IDT_INDETERMINATE		100
#define	IND_BAND_WIDTH			20

// Funtion prototypes.
COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);

//-------------------------------------------------------------------
//
COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Lightens a color by increasing the RGB values by the given number.
//
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if ((byRed + byIncreaseVal) <= 255)
		byRed = BYTE(byRed + byIncreaseVal);
	if ((byGreen + byIncreaseVal)	<= 255)
		byGreen = BYTE(byGreen + byIncreaseVal);
	if ((byBlue + byIncreaseVal) <= 255)
		byBlue = BYTE(byBlue + byIncreaseVal);

	return RGB(byRed, byGreen, byBlue);
}	// LightenColorref

//-------------------------------------------------------------------
//
COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal)
//
// Return Value:	None.
//
// Parameters	:	crColor - References a COLORREF structure.
//						byReduceVal - The amount to reduce the RGB values by.
//
// Remarks		:	Darkens a color by reducing the RGB values by the given number.
//
{
	BYTE byRed = GetRValue(crColor);
	BYTE byGreen = GetGValue(crColor);
	BYTE byBlue = GetBValue(crColor);

	if (byRed >= byReduceVal)
		byRed = BYTE(byRed - byReduceVal);
	if (byGreen >= byReduceVal)
		byGreen = BYTE(byGreen - byReduceVal);
	if (byBlue >= byReduceVal)
		byBlue = BYTE(byBlue - byReduceVal);

	return RGB(byRed, byGreen, byBlue);
}	// DarkenColorref

/////////////////////////////////////////////////////////////////////////////
// CMacProgressCtrl

//-------------------------------------------------------------------
//
CMacProgressCtrl::CMacProgressCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Standard constructor.
//
{
	m_bIndeterminate = FALSE;
	m_nIndOffset = 0;
	mTimerID = 0;

	// Scale highlight colour to full brightness
	m_crColor = ::GetSysColor(COLOR_HIGHLIGHT);
	UINT scale = std::max(GetRValue(m_crColor), std::max(GetGValue(m_crColor), GetBValue(m_crColor)));
	if (scale == 0)
		scale = 1;
	UINT red_scaled = ((float) GetRValue(m_crColor) * 0xFF) / scale;
	red_scaled = (red_scaled + 2 * 0xA0) / 3;
	if (red_scaled > 0xFF)
		red_scaled = 0xFF;
	UINT green_scaled = ((float) GetGValue(m_crColor) * 0xFF) / scale;
	green_scaled = (green_scaled + 2 * 0xA0) / 3;
	if (green_scaled > 0xFF)
		green_scaled = 0xFF;
	UINT blue_scaled = ((float) GetBValue(m_crColor) * 0xFF) / scale;
	blue_scaled = (blue_scaled + 2 * 0xA0) / 3;
	if (blue_scaled > 0xFF)
		blue_scaled = 0xFF;

	m_crColor = RGB(red_scaled, green_scaled, blue_scaled);

	m_crColorGray = RGB(0xA0, 0xA0, 0xA0);

	GetColors();
	CreatePens();
}	// CMacProgressCtrl

//-------------------------------------------------------------------
//
CMacProgressCtrl::~CMacProgressCtrl()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	None.
//
{
	DeletePens();
}	// ~CMacProgressCtrl


BEGIN_MESSAGE_MAP(CMacProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CMacProgressCtrl)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacProgressCtrl message handlers

//-------------------------------------------------------------------
//
void CMacProgressCtrl::OnPaint() 
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	The framework calls this member function when Windows 
//						or an application makes a request to repaint a portion 
//						of an application’s window.
//
{
	CPaintDC dcPaint(this); // device context for painting
	CRect rect, rectClient;
	GetClientRect(rectClient);
	rect = rectClient;
	BOOL bVertical = GetStyle() & PBS_VERTICAL;

	// Create a memory DC for drawing.
	CDC dc;
	dc.CreateCompatibleDC(&dcPaint);
 	int nSavedDC = dc.SaveDC();
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dcPaint, rect.Width(), rect.Height());
	CBitmap *pOldBmp = dc.SelectObject(&bmp);
	
	CBrush br1(m_crColorLightest);
	CBrush br2(::GetSysColor(COLOR_3DFACE));
	dc.FillRect(rect, &br2);

	int nLower, nUpper;
	GetRange(nLower, nUpper);

	// Determine the size of the bar and draw it.
	if (bVertical)
	{
		if (!m_bIndeterminate && (nUpper - nLower != 0))
			rect.top = rect.bottom - int(((float)rect.Height() * float(GetPos() - nLower)) / float(nUpper - nLower));
		dc.FillRect(rect, &br1);
		DrawVerticalBar(&dc, rect);
	}
	else
  	{
		if (!m_bIndeterminate && (nUpper - nLower != 0))
			rect.right = int(((float)rect.Width() * float(GetPos() - nLower)) / float(nUpper - nLower));
		dc.FillRect(rect, &br1);
		DrawHorizontalBar(&dc, rect);
	}

	dcPaint.BitBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), 
						&dc, rectClient.left, rectClient.top, SRCCOPY);

	dc.SelectObject(pOldBmp);
	dc.RestoreDC(nSavedDC);
	dc.DeleteDC();
}	// OnPaint	

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DrawHorizontalBar(CDC *pDC, const CRect rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies the device context object.
//						rect - Specifies the rectangle of the progess bar.
//
// Remarks		:	Draws a horizontal progress bar.
//
{
	if (!rect.Width())
		return;

	int nLeft = rect.left;
	int nTop = rect.top;
	int nBottom = rect.bottom;

	// Assume we're not drawing the indeterminate state.
	CPen *pOldPen = pDC->SelectObject(&m_penColorLight);

	if (m_bIndeterminate)
	{
		pOldPen = pDC->SelectObject(&m_penColor);
		int nNumBands = (rect.Width() / IND_BAND_WIDTH) + 2;
		int nHeight = rect.Height() + 1;

		int nAdjust = nLeft - IND_BAND_WIDTH + m_nIndOffset;
		int nXpos = 0;

		for (int i = 0; i <= nBottom - nTop; i++)
		{
			// Choose colour for band
			int color_band = (i * 9) / (nBottom - nTop + 1);
			switch(color_band)
			{
			case 0:
			case 8:
				pDC->SelectObject(&m_penColorDarker);
				break;
			case 1:
			case 7:
				pDC->SelectObject(&m_penColorDark);
				break;
			case 2:
			case 6:
				pDC->SelectObject(&m_penColor);
				break;
			case 3:
			case 5:
				pDC->SelectObject(&m_penColorLight);
				break;
			case 4:
				pDC->SelectObject(&m_penColorLighter);
				break;
			}

			for (int j = -1; j < nNumBands; j++)
			{
				nXpos = nAdjust + (j * IND_BAND_WIDTH) + i;

				pDC->MoveTo(nXpos, nTop + i);
				pDC->LineTo(nXpos + IND_BAND_WIDTH/2, nTop + i);
			}	// for the number of bands

			// Choose colour for gap
			switch(color_band)
			{
			case 0:
			case 8:
				pDC->SelectObject(&m_penColorGrayDarker);
				break;
			case 1:
			case 7:
				pDC->SelectObject(&m_penColorGrayDark);
				break;
			case 2:
			case 6:
				pDC->SelectObject(&m_penColorGray);
				break;
			case 3:
			case 5:
				pDC->SelectObject(&m_penColorGrayLight);
				break;
			case 4:
				pDC->SelectObject(&m_penColorGrayLighter);
				break;
			}

			for (int j = -1; j < nNumBands; j++)
			{
				nXpos = nAdjust + (j * IND_BAND_WIDTH) + i;

				pDC->MoveTo(nXpos + IND_BAND_WIDTH/2, nTop + i);
				pDC->LineTo(nXpos + IND_BAND_WIDTH, nTop + i);
			}	// for the number of bands
		}
	}	// if indeterminate
	else
	{
		int nRight = rect.right;
	
		pDC->MoveTo(nLeft + 2, nBottom - 4);
		pDC->LineTo(nRight - 2, nBottom - 4);
		pDC->MoveTo(nLeft + 2, nTop + 2);
		pDC->LineTo(nRight - 2, nTop + 2);
		pDC->SetPixel(nLeft + 1, nBottom - 3, m_crColorLight);
		pDC->SetPixel(nLeft + 1, nTop + 1, m_crColorLight);

		pDC->SelectObject(&m_penColorLighter);
		pDC->MoveTo(nLeft + 2, nBottom - 5);
		pDC->LineTo(nRight - 3, nBottom - 5);
		pDC->LineTo(nRight - 3, nTop + 3);
		pDC->LineTo(nLeft + 1, nTop + 3);
		pDC->SetPixel(nLeft + 1, nBottom - 4, m_crColorLighter);
		pDC->SetPixel(nLeft + 1, nTop + 2, m_crColorLighter);

		pDC->SelectObject(&m_penColor);
		pDC->MoveTo(nLeft, nBottom - 1);
		pDC->LineTo(nLeft, nTop);
		pDC->LineTo(nLeft + 2, nTop);
		pDC->SetPixel(nLeft + 1, nBottom - 2, m_crColor);
		pDC->MoveTo(nLeft + 2, nBottom - 3);
		pDC->LineTo(nRight - 2, nBottom - 3);
		pDC->MoveTo(nLeft + 2, nTop + 1);
		pDC->LineTo(nRight - 1, nTop + 1);
		
		pDC->SelectObject(&m_penColorDark);
		pDC->MoveTo(nLeft + 2, nBottom - 2);
		pDC->LineTo(nRight - 2, nBottom - 2);
		pDC->LineTo(nRight - 2, nTop + 1);
		pDC->MoveTo(nLeft + 2, nTop);
		pDC->LineTo(nRight, nTop);
		pDC->SetPixel(nLeft + 1, nBottom - 1, m_crColorDark);

		pDC->SelectObject(&m_penColorDarker);
		pDC->MoveTo(nLeft + 2, nBottom - 1);
		pDC->LineTo(nRight - 1, nBottom - 1);
		pDC->LineTo(nRight - 1, nTop);

		pDC->SelectObject(&m_penShadow);
		pDC->MoveTo(nRight, nTop);
 		pDC->LineTo(nRight, nBottom);

		pDC->SelectObject(&m_penLiteShadow);
 		pDC->MoveTo(nRight + 1, nTop);
		pDC->LineTo(nRight + 1, nBottom);
	}	// if not indeterminate

	pDC->SelectObject(pOldPen);
}	// DrawHorizontalBar

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DrawVerticalBar(CDC *pDC, const CRect rect)
//
// Return Value:	None.
//
// Parameters	:	pDC - Specifies the device context object.
//						rect - Specifies the rectangle of the progess bar.
//
// Remarks		:	Draws a vertical progress bar.
//
{
	int nHeight = rect.Height();
	if (!nHeight)
		return;

	int nLeft = rect.left;
	int nTop = rect.top;
	int nRight = rect.right;
	int nBottom = rect.bottom;

	CPen *pOldPen = pDC->SelectObject(&m_penColor);

	if (m_bIndeterminate)
	{
		int nNumBands = (nHeight / IND_BAND_WIDTH) + 2;
		int nHeight = rect.Width() + 1;

		int nAdjust = nBottom - m_nIndOffset;
		int nXpos1 = nLeft;
		int nXpos2 = nRight + 1;
		int nYpos = nTop + 1;

		for (int i = 0; i < nNumBands; i++)
		{
			nYpos = nAdjust - (i * IND_BAND_WIDTH);

			pDC->SelectObject(&m_penColorDarker);
			pDC->MoveTo(nXpos1, nYpos);
			pDC->LineTo(nXpos2, nYpos + nHeight);

			pDC->SelectObject(&m_penColorDark);
			pDC->MoveTo(nXpos1, nYpos + 1);
			pDC->LineTo(nXpos2, nYpos + nHeight + 1);
			pDC->MoveTo(nXpos1, nYpos + 9);
			pDC->LineTo(nXpos2, nYpos + nHeight + 9);

			pDC->SelectObject(&m_penColor);
			pDC->MoveTo(nXpos1, nYpos + 2);
			pDC->LineTo(nXpos2, nYpos + nHeight + 2);
			pDC->MoveTo(nXpos1, nYpos + 8);
			pDC->LineTo(nXpos2, nYpos + nHeight + 8);

			pDC->SelectObject(&m_penColorLight);
			pDC->MoveTo(nXpos1, nYpos + 3);
			pDC->LineTo(nXpos2, nYpos + nHeight + 3);
			pDC->MoveTo(nXpos1, nYpos + 7);
			pDC->LineTo(nXpos2, nYpos + nHeight + 7);

			pDC->SelectObject(&m_penColorLighter);
			pDC->MoveTo(nXpos1, nYpos + 4);
			pDC->LineTo(nXpos2, nYpos + nHeight + 4);
			pDC->MoveTo(nXpos1, nYpos + 6);
			pDC->LineTo(nXpos2, nYpos + nHeight + 6);
		}	// for the number of bands
	}	// if indeterminate
	else
	{
		if (nHeight > 3)
		{
			pDC->MoveTo(nLeft, nTop + 1);
			pDC->LineTo(nLeft, nTop);
			pDC->LineTo(nRight, nTop);
			pDC->MoveTo(nLeft + 1, nBottom - 2);
			pDC->LineTo(nLeft + 1, nTop + 1);
			pDC->MoveTo(nRight - 3, nBottom - 3);
			pDC->LineTo(nRight - 3, nTop + 1);
			pDC->SetPixel(nRight - 2, nTop + 1, m_crColor);

			pDC->SelectObject(&m_penColorLight);
			pDC->MoveTo(nLeft + 2, nBottom - 3);
			pDC->LineTo(nLeft + 2, nTop + 1);
			pDC->MoveTo(nRight - 4, nBottom - 3);
			pDC->LineTo(nRight - 4, nTop + 1);
			pDC->SetPixel(nLeft + 1, nTop + 1, m_crColorLight);
			pDC->SetPixel(nRight - 3, nTop + 1, m_crColorLight);
			
			pDC->SelectObject(&m_penColorLighter);
			pDC->MoveTo(nLeft + 3, nBottom - 3);
			pDC->LineTo(nLeft + 3, nTop + 1);
			pDC->MoveTo(nRight - 5, nBottom - 3);
			pDC->LineTo(nRight - 5, nTop + 1);
			pDC->SetPixel(nLeft + 2, nTop + 1, m_crColorLighter);
			pDC->SetPixel(nRight - 4, nTop + 1, m_crColorLighter);

			pDC->SelectObject(&m_penColorDark);
			pDC->MoveTo(nLeft, nBottom - 1);
			pDC->LineTo(nLeft, nTop + 1);
			pDC->MoveTo(nLeft + 2, nBottom - 2);
			pDC->LineTo(nRight - 2, nBottom - 2);
			pDC->LineTo(nRight - 2, nTop + 1);
			pDC->SetPixel(nRight - 1, nTop + 1, m_crColorDark);

			pDC->SelectObject(&m_penColorDarker);
			pDC->MoveTo(nLeft + 1, nBottom - 1);
			pDC->LineTo(nRight - 1, nBottom - 1);
			pDC->LineTo(nRight - 1, nTop + 1);
		}
		else
		{
			CBrush br(m_crColor);
			CBrush *pOldBrush = pDC->SelectObject(&br);
			pDC->SelectObject(&m_penColorDark);
			pDC->Rectangle(rect);
			pDC->SelectObject(pOldBrush);
		}
	}	// if not indeterminate

	pDC->SelectObject(pOldPen);
}	// DrawVerticalBar

//-------------------------------------------------------------------
//
BOOL CMacProgressCtrl::OnEraseBkgnd(CDC* pDC) 
//
// Return Value:	Nonzero if it erases the background; otherwise 0.
//
// Parameters	:	pDC - Specifies the device-context object.
//
// Remarks		:	The framework calls this member function when the 
//						CWnd object background needs erasing (for example, 
//						when resized). It is called to prepare an invalidated 
//						region for painting.
//
{
		return TRUE;
}	// OnEraseBkgnd

//-------------------------------------------------------------------
//
void CMacProgressCtrl::GetColors()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Calculates the lighter and darker colors, as well as 
//						the shadow colors.
//
{
	m_crColorLight = LightenColor(m_crColor, 51);
	m_crColorLighter = LightenColor(m_crColorLight, 51);
	m_crColorLightest = LightenColor(m_crColorLighter, 51);
	m_crColorDark = DarkenColor(m_crColor, 51);
	m_crColorDarker = DarkenColor(m_crColorDark, 51);

	m_crColorGrayLight = LightenColor(m_crColorGray, 51);
	m_crColorGrayLighter = LightenColor(m_crColorGrayLight, 51);
	m_crColorGrayLightest = LightenColor(m_crColorGrayLighter, 51);
	m_crColorGrayDark = DarkenColor(m_crColorGray, 51);
	m_crColorGrayDarker = DarkenColor(m_crColorGrayDark, 51);

	m_crDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
	m_crLiteShadow = ::GetSysColor(COLOR_3DSHADOW);

	// Get a color halfway between COLOR_3DDKSHADOW and COLOR_3DSHADOW
	BYTE byRed3DDkShadow = GetRValue(m_crDkShadow);
	BYTE byRed3DLiteShadow = GetRValue(m_crLiteShadow);
	BYTE byGreen3DDkShadow = GetGValue(m_crDkShadow);
	BYTE byGreen3DLiteShadow = GetGValue(m_crLiteShadow);
	BYTE byBlue3DDkShadow = GetBValue(m_crDkShadow);
	BYTE byBlue3DLiteShadow = GetBValue(m_crLiteShadow);

	m_crShadow = RGB(byRed3DLiteShadow + ((byRed3DDkShadow - byRed3DLiteShadow) >> 1),
						  byGreen3DLiteShadow + ((byGreen3DDkShadow - byGreen3DLiteShadow) >> 1),
						  byBlue3DLiteShadow + ((byBlue3DDkShadow - byBlue3DLiteShadow) >> 1));
}	// GetColors

//-------------------------------------------------------------------
//
void CMacProgressCtrl::SetColor(COLORREF crColor)
//
// Return Value:	None.
//
// Parameters	:	crColor - New color.
//
// Remarks		:	Sets the progress	bar control's color. The lighter
//						darker colors are recalculated, and the pens recreated.
//
{
	m_crColor = crColor;
	GetColors();
	CreatePens();
	RedrawWindow();
}	// SetColor

//-------------------------------------------------------------------
//
COLORREF CMacProgressCtrl::GetColor()
//
// Return Value:	The current color.
//
// Parameters	:	None.
//
// Remarks		:	Returns the progress bar control's current color.
//
{
	return m_crColor;
}	// GetColor

//-------------------------------------------------------------------
//
void CMacProgressCtrl::CreatePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects, if necessary, and creates them.
//
{
	DeletePens();

	m_penColorLight.CreatePen(PS_SOLID, 1, m_crColorLight);
	m_penColorLighter.CreatePen(PS_SOLID, 1, m_crColorLighter);
	m_penColor.CreatePen(PS_SOLID, 1, m_crColor);
	m_penColorDark.CreatePen(PS_SOLID, 1, m_crColorDark);
	m_penColorDarker.CreatePen(PS_SOLID, 1, m_crColorDarker);

	m_penColorGrayLight.CreatePen(PS_SOLID, 1, m_crColorGrayLight);
	m_penColorGrayLighter.CreatePen(PS_SOLID, 1, m_crColorGrayLighter);
	m_penColorGray.CreatePen(PS_SOLID, 1, m_crColorGray);
	m_penColorGrayDark.CreatePen(PS_SOLID, 1, m_crColorGrayDark);
	m_penColorGrayDarker.CreatePen(PS_SOLID, 1, m_crColorGrayDarker);

	m_penDkShadow.CreatePen(PS_SOLID, 1, m_crDkShadow);
	m_penShadow.CreatePen(PS_SOLID, 1, m_crShadow);
	m_penLiteShadow.CreatePen(PS_SOLID, 1, m_crLiteShadow);
}	// CreatePens

//-------------------------------------------------------------------
//
void CMacProgressCtrl::DeletePens()
//
// Return Value:	None.
//
// Parameters	:	None.
//
// Remarks		:	Deletes the pen objects.
//
{
	if (m_penColorLight.m_hObject)
		m_penColorLight.DeleteObject();
	if (m_penColorLighter.m_hObject)
		m_penColorLighter.DeleteObject();
	if (m_penColor.m_hObject)
		m_penColor.DeleteObject();
	if (m_penColorDark.m_hObject)
		m_penColorDark.DeleteObject();
	if (m_penColorDarker.m_hObject)
		m_penColorDarker.DeleteObject();

	if (m_penColorGrayLight.m_hObject)
		m_penColorGrayLight.DeleteObject();
	if (m_penColorGrayLighter.m_hObject)
		m_penColorGrayLighter.DeleteObject();
	if (m_penColorGray.m_hObject)
		m_penColorGray.DeleteObject();
	if (m_penColorGrayDark.m_hObject)
		m_penColorGrayDark.DeleteObject();
	if (m_penColorGrayDarker.m_hObject)
		m_penColorGrayDarker.DeleteObject();

	if (m_penDkShadow.m_hObject)
		m_penDkShadow.DeleteObject();
	if (m_penShadow.m_hObject)
		m_penShadow.DeleteObject();
	if (m_penLiteShadow.m_hObject)
		m_penLiteShadow.DeleteObject();
}	// DeletePens

//-------------------------------------------------------------------
//
void CMacProgressCtrl::SetIndeterminate(BOOL bIndeterminate)
//
// Return Value:	None.
//
// Parameters	:	bIndeterminate - Specifies the indeterminate state.
//
// Remarks		:	Sets the indeterminate flag.
//
{
	m_bIndeterminate = bIndeterminate;

	if (m_bIndeterminate)
	{
		CRect rect;
		GetClientRect(rect);
		m_nIndOffset = 0;

		RedrawWindow();
		mTimerID = SetTimer(IDT_INDETERMINATE, 25, NULL);
	}
	else
	{
		if (mTimerID != 0)
			KillTimer(mTimerID);
		mTimerID = 0;
		RedrawWindow();
	}
}	// SetIndeterminate

//-------------------------------------------------------------------
//
BOOL CMacProgressCtrl::GetIndeterminate()
//
// Return Value:	m_bIndeterminate.
//
// Parameters	:	None.
//
// Remarks		:	Returns m_bIndeterminate.
//
{
	return m_bIndeterminate;
}	// GetIndeterminate

//-------------------------------------------------------------------
//
void CMacProgressCtrl::OnTimer(UINT nIDEvent) 
//
// Return Value:	None.
//
// Parameters	:	nIDEvent - Specifies the identifier of the timer.
//
// Remarks		:	The framework calls this member function after each 
//						interval specified in the SetTimer member function used 
//						to install a timer.
//
{
	// Increment the indeterminate bar offset and redraw the window.
	if (nIDEvent == mTimerID)
	{
		KillTimer(mTimerID);
		mTimerID = 0;

		if (++m_nIndOffset > IND_BAND_WIDTH - 1)
			m_nIndOffset = 0;
		RedrawWindow();

		mTimerID = SetTimer(IDT_INDETERMINATE, 25, NULL);
	}
}	// OnTimer
