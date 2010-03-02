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


// CIconMenu.cp : implements an owner-draw menu with icons

#include "CIconMenu.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

CIconMenu::CIconMenu()
{
}

CIconMenu::~CIconMenu()
{
}

void CIconMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CDC* pDC = AfxGetApp()->m_pMainWnd->GetDC();
	StDCState save(pDC);

	CFont* pFont = pDC->SelectObject(CMulberryApp::sAppFont);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	pDC->SelectObject(pFont);
	AfxGetApp()->m_pMainWnd->ReleaseDC(pDC);

	lpMeasureItemStruct->itemWidth = 16 + tm.tmAveCharWidth * reinterpret_cast<SIconMenuData*>(lpMeasureItemStruct->itemData)->mTxt.length() + 10;
	lpMeasureItemStruct->itemHeight = std::max(16L, tm.tmHeight + 6);
}

void CIconMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	SIconMenuData* data = reinterpret_cast<SIconMenuData*>(lpDIS->itemData);
	if (data == NULL)
		return;
	cdstring theTxt(data->mTxt);

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	CRect rect;
	rect.CopyRect(&lpDIS->rcItem);

	CFont* pFont = pDC->SelectObject(CMulberryApp::sAppFont);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	pDC->SelectObject(pFont);

	// Draw background
	if ((lpDIS->itemState & ODS_SELECTED) != 0)
	{
		pDC->SetBkColor(::GetSysColor(COLOR_MENUHILIGHT));
		pDC->SetTextColor((lpDIS->itemState & ODS_GRAYED) ? COLOR_GRAYTEXT : CDrawUtils::sWhiteColor);
	}
	else
	{
		pDC->SetBkColor(::GetSysColor(COLOR_MENU));
		pDC->SetTextColor(::GetSysColor((lpDIS->itemState & ODS_GRAYED) ? COLOR_GRAYTEXT : COLOR_MENUTEXT));
	}

	pDC->ExtTextOut(rect.left, rect.top, ETO_OPAQUE, rect, _T(""), 0, NULL);
	CIconLoader::DrawState(pDC, rect.left + 2, rect.top + (rect.Height() - 16) / 2, data->mIcon, 16, (lpDIS->itemState & ODS_GRAYED) ? DSS_DISABLED : DSS_NORMAL);
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(rect.left + 20, rect.top), rect, eDrawString_Left);
}
