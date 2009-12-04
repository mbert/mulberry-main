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

#include "CYearPopup.h"

#include "CUnicodeUtils.h"

// ---------------------------------------------------------------------------
//	CYearPopup														  [public]
/**
	Default constructor */

CYearPopup::CYearPopup() :
	CPopupButton(true)
{
}


// ---------------------------------------------------------------------------
//	~CYearPopup														  [public]
/**
	Destructor */

CYearPopup::~CYearPopup()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CYearPopup, CPopupButton)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CYearPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPopupButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set menu
	SetMenu(IDR_POPUP_YEARS);

	// Init the popup menu
	Reset();
	
	return 0;
}

BOOL CYearPopup::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	CPopupButton::SubclassDlgItem(nID, pParent, IDI_POPUPBTN, 0, 0, 0, true, false);

	// Set menu
	SetMenu(IDR_POPUP_YEARS);

	return true;
}

void CYearPopup::SetValue(UINT inValue)
{
	// Do inherited first
	CPopupButton::SetValue(inValue);

	// Ignore if setting back to center item
	if (inValue != IDM_YEAR4)
	{
		SetYear(GetYear());
	}
}

void CYearPopup::SetYear(int32_t year)
{
	mYearStart = year - 3;
	Reset();
	
	// The value set is always the middle one
	mValue = 0;
	SetValue(IDM_YEAR4);
}

int32_t CYearPopup::GetYear() const
{
	return mYearStart + GetValue() - IDM_YEAR1;
}

void CYearPopup::Reset()
{
	// Change menu titles
	CMenu* pPopup = GetPopupMenu();
	for(int32_t i = 0; i < 7; i++)
	{
		cdstring buf = mYearStart + i;
		CUnicodeUtils::ModifyMenuUTF8(pPopup, IDM_YEAR1 + i, MF_BYCOMMAND | MF_STRING, IDM_YEAR1 + i, buf);
	}
}
