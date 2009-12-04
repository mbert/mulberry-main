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


// CSplashBox.cpp : implementation file
//


#include "CSplashBox.h"

#include "CUnicodeUtils.h"

BOOL GetBitmapAndPalette(UINT nIDResource, CBitmap &bitmap, CPalette &pal);
BOOL GetBitmapAndPalette(UINT nIDResource, CBitmap &bitmap, CPalette &pal)
{
	LPCTSTR lpszResourceName = (LPCTSTR)nIDResource;

	HBITMAP hBmp = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(), lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);

	if (hBmp == NULL) 
		return FALSE;

	bitmap.Attach(hBmp);

	// Create a logical palette for the bitmap
	DIBSECTION ds;
	BITMAPINFOHEADER &bmInfo = ds.dsBmih;
	bitmap.GetObject(sizeof(ds), &ds);

	int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

	// Create a halftone palette if colors > 256. 
	CClientDC dc(NULL);                     // Desktop DC
	if (nColors > 256)
		pal.CreateHalftonePalette(&dc);
	else
	{
		// Create the palette

		RGBQUAD *pRGB = new RGBQUAD[nColors];
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		memDC.SelectObject(&bitmap);
		::GetDIBColorTable(memDC, 0, nColors, pRGB);

		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE*) new BYTE[nSize];

		pLP->palVersion = 0x300;
		pLP->palNumEntries = nColors;

		for(int i=0; i < nColors; i++)
		{
			pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
			pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
			pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
			pLP->palPalEntry[i].peFlags = 0;
		}

		pal.CreatePalette(pLP);

		delete[] (BYTE*) pLP;
		delete[] pRGB;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSplashBox dialog


CSplashBox::CSplashBox()
{
	//{{AFX_DATA_INIT(CSplashBox)
	//}}AFX_DATA_INIT
}


void CSplashBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplashBox)
	//DDX_UTF8Text(pDX, IDC_SPLASHLICENSEE, mLicensee);
	//DDX_UTF8Text(pDX, IDC_SPLASHSERIAL, mSerial);
	DDX_UTF8Text(pDX, IDC_SPLASHVERSION, mVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSplashBox, CDialog)
	//{{AFX_MSG_MAP(CSplashBox)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSplashBox::OnPaint()
{
	//CDialog::OnPaint();
	
    CPaintDC dc(this);

    // Create a memory DC compatible with the paint DC
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);

    CBitmap bitmap;
    CPalette palette;

    GetBitmapAndPalette(IDB_SPLASH, bitmap, palette);
    memDC.SelectObject(&bitmap);

    // Select and realize the palette
    if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE && palette.m_hObject != NULL)
	{
		dc.SelectPalette( &palette, FALSE );
		dc.RealizePalette();
	}
	CRect wnd_size;
	GetClientRect(wnd_size);
    dc.BitBlt(0, 0, wnd_size.Width(), wnd_size.Height(), &memDC, 0, 0,SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////
// CSplashBox message handlers

HBRUSH CSplashBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Change background to transparent
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH) GetStockObject(HOLLOW_BRUSH);
}

