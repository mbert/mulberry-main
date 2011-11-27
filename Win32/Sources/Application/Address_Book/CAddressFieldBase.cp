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


// Source for CAddressFieldBase class

#include "CAddressFieldBase.h"

#include "CAddressFieldSubContainer.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CToolbarButton.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

BEGIN_MESSAGE_MAP(CAddressFieldBase, CStatic)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	
	ON_COMMAND(IDC_ADDRESSADVANCED_ADD, OnAdd)
	ON_COMMAND(IDC_ADDRESSADVANCED_REMOVE, OnRemove)
	ON_COMMAND_RANGE(IDM_ADDRESSADVANCED_TYPE_1, IDM_ADDRESSADVANCED_TYPE_8, OnSetType)
END_MESSAGE_MAP()

const int cMoveBtnWidth = 20;
const int cMoveBtnHeight = 20;
const int cMoveBtnHOffset = 0;
const int cMoveBtnVOffset = 1;

const int cTitleWidth = 60;
const int cTitleHeight = 16;
const int cTitleHOffset = cMoveBtnHOffset + cMoveBtnWidth + 4;
const int cTitleVOffset = 6;

const int cPopupWidth = 80;
const int cPopupHeight = 22;
const int cPopupHOffset = cTitleHOffset + cTitleWidth + 4;
const int cPopupVOffset = 0;


// Default constructor
CAddressFieldBase::CAddressFieldBase()
{
    mUsesType = true;
}

// Default destructor
CAddressFieldBase::~CAddressFieldBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

BOOL CAddressFieldBase::Create(const CRect& rect, CWnd* pParentWnd)
{
	return CStatic::CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, rect, pParentWnd, IDC_STATIC);
}

int CAddressFieldBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	// Header
	CRect size;
	GetWindowRect(size);
	mHeader.CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, size.Width(), size.Height(), GetSafeHwnd(), (HMENU)IDC_STATIC);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_WidthHeight));

	// Move buttons
	CRect r;
	r = CRect(cMoveBtnHOffset, cMoveBtnVOffset, cMoveBtnHOffset + cMoveBtnWidth, cMoveBtnVOffset + cMoveBtnHeight);
	mAdd.Create(_T(""), r, &mHeader, IDC_ADDRESSADVANCED_ADD, 0, IDI_ACL_CREATE);
    mAdd.SetSmallIcon(true);
    mAdd.SetShowIcon(true);
    mAdd.SetShowCaption(false);
    mAdd.SetShowFrame(false);
	mHeader.AddAlignment(new CWndAlignment(&mAdd, CWndAlignment::eAlign_TopLeft));

	r = CRect(cMoveBtnHOffset, cMoveBtnVOffset, cMoveBtnHOffset + cMoveBtnWidth, cMoveBtnVOffset + cMoveBtnHeight);
	mRemove.Create(_T(""), r, &mHeader, IDC_ADDRESSADVANCED_REMOVE, 0, IDI_ACL_DELETE);
	mRemove.SetSmallIcon(true);
	mRemove.SetShowIcon(true);
	mRemove.SetShowCaption(false);
	mRemove.SetShowFrame(false);
	mHeader.AddAlignment(new CWndAlignment(&mRemove, CWndAlignment::eAlign_TopLeft));

	r = CRect(cTitleHOffset, cTitleVOffset, cTitleHOffset + cTitleWidth, cTitleVOffset + cTitleHeight + small_offset);
	mTitle.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mHeader, IDC_STATIC);
	mTitle.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopupHOffset, cPopupVOffset, cPopupHOffset + cPopupWidth, cPopupVOffset + cPopupHeight + small_offset);
	mType.Create(_T(""), r, &mHeader, IDC_STATIC, IDC_STATIC, IDI_POPUPBTN);
	mType.SetMenu(IDR_ADDRESSADVANCED_GENERICTYPE);
	mType.SetValue(IDM_ADDRESSADVANCED_TYPE_1);
	mType.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mType, CWndAlignment::eAlign_TopLeft));

	return 0;
}

// Resize sub-views
void CAddressFieldBase::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CStatic::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

void CAddressFieldBase::OnSetType(UINT nID)
{
	mType.SetValue(nID);
}

void CAddressFieldBase::SetSingleInstance()
{
    mAdd.ShowWindow(SW_HIDE);
    mRemove.ShowWindow(SW_HIDE);
    HideType();
}

void CAddressFieldBase::SetMultipleInstance()
{
    mAdd.ShowWindow(SW_HIDE);
    HideTitle();
}

void CAddressFieldBase::SetBottom()
{
    mAdd.ShowWindow(SW_SHOW);
    mRemove.ShowWindow(SW_HIDE);
    mTitle.ShowWindow(SW_HIDE);
    mType.ShowWindow(SW_HIDE);
    mDataMove->ShowWindow(SW_HIDE);
}

const int cButtonHOffset = 22;

void CAddressFieldBase::HideType()
{
    mType.ShowWindow(SW_HIDE);
    mUsesType = false;

    // Move and resize data field
    ::MoveWindowBy(&mTitle, -cButtonHOffset, 0, false);
    CRect size;
    mType.GetWindowRect(size);
    ::MoveWindowBy(mDataMove, -size.Width() - cButtonHOffset, 0, false);
    ::ResizeWindowBy(mDataMove, size.Width() + cButtonHOffset, 0, false);
}

void CAddressFieldBase::HideTitle()
{
    mTitle.ShowWindow(SW_HIDE);

    // Move and resize type/data fields
    CRect size;
    mTitle.GetWindowRect(size);
    ::MoveWindowBy(&mType, -size.Width(), 0, false);
    ::MoveWindowBy(mDataMove, -size.Width(), 0, false);
    ::ResizeWindowBy(mDataMove, size.Width(), 0, false);
}

void CAddressFieldBase::SetMenu(bool typePopup)
{
	mType.SetMenu(IDR_ADDRESSADVANCED_PHONETYPE);
	mType.SetValue(IDM_ADDRESSADVANCED_TYPE_1);
}


void CAddressFieldBase::OnAdd()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetParent());
    parent->AppendField();
}

void CAddressFieldBase::OnRemove()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetParent());
    parent->RemoveField(this);
}
