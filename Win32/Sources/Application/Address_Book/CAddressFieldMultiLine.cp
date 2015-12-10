/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Source for CAddressFieldMultiLine class

#include "CAddressFieldMultiLine.h"

#include "CMulberryApp.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


BEGIN_MESSAGE_MAP(CAddressFieldMultiLine, CAddressFieldBase)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Default constructor
CAddressFieldMultiLine::CAddressFieldMultiLine()
{
}

// Default destructor
CAddressFieldMultiLine::~CAddressFieldMultiLine()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cMoveBtnWidth = 20;
const int cMoveBtnHOffset = 0;

const int cTitleWidth = 60;
const int cTitleHeight = 16;
const int cTitleHOffset = cMoveBtnHOffset + cMoveBtnWidth + 4;
const int cTitleVOffset = 6;

const int cPopupWidth = 80;
const int cPopupHeight = 22;
const int cPopupHOffset = cTitleHOffset + cTitleWidth + 4;
const int cPopupVOffset = 0;

const int cTextWidth = 120;
const int cTextHeight = 62;
const int cTextHOffset = cPopupHOffset + cPopupWidth + 4;
const int cTextVOffset = 0;

int CAddressFieldMultiLine::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CAddressFieldBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;
	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	CRect r;
	r = CRect(cTextHOffset, cTextVOffset, width - cTextHOffset, cTextVOffset + cTextHeight + small_offset);
	mData.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL | ES_MULTILINE,
						r, &mHeader, IDC_SEARCHCRITERIA_TEXT1);
	mData.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mData, CWndAlignment::eAlign_WidthHeight));

	mDataMove = &mData;

	return 0;
}

bool CAddressFieldMultiLine::SetInitialFocus()
{
	return false;
}

void CAddressFieldMultiLine::SetDetails(const cdstring& title, int type, const cdstring& data)
{
    // Cache this to check for changes later
    mOriginalType = type;
    mOriginalData = data;

    CUnicodeUtils::SetWindowTextUTF8(&mTitle, title);
    if (mUsesType && (type != 0))
	{
        mType.SetValue(IDM_ADDRESSADVANCED_TYPE_1 + type - 1);
	}
    mData.SetText(data);
}

bool CAddressFieldMultiLine::GetDetails(int& newtype, cdstring& newdata)
{
    bool changed = false;

    if (mUsesType)
    {
        newtype = mType.GetValue() - IDM_ADDRESSADVANCED_TYPE_1 + 1;
        if ((mOriginalType != 0) && (newtype != mOriginalType))
		{
            changed = true;
		}
    }

    newdata = mData.GetText();
    if (newdata != mOriginalData)
        changed = true;

    return changed;
}
