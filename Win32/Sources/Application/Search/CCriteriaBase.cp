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


// Source for CCriteriaBase class

#include "CCriteriaBase.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSearchBase.h"
#include "CSearchStyle.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

BEGIN_MESSAGE_MAP(CCriteriaBase, CStatic)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	
	ON_COMMAND_RANGE(IDC_SEARCHCRITERIA_MOVEUP, IDC_SEARCHCRITERIA_MOVEDOWN, OnSetMove)
END_MESSAGE_MAP()

const int cMoveBtnWidth = 16;
const int cMoveBtnHeight = 12;
const int cMoveBtnHOffset = 0;
const int cMoveUpBtnVOffset = 1;
const int cMoveDownBtnVOffset = cMoveUpBtnVOffset + cMoveBtnHeight;

// Default constructor
CCriteriaBase::CCriteriaBase()
{
}

// Default destructor
CCriteriaBase::~CCriteriaBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CCriteriaBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
	CString s;
	mMoveUp.Create(s, CRect(cMoveBtnHOffset, cMoveUpBtnVOffset, cMoveBtnHOffset + cMoveBtnWidth, cMoveUpBtnVOffset + cMoveBtnHeight), &mHeader, IDC_SEARCHCRITERIA_MOVEUP, IDC_STATIC, IDI_MOVEUP);
	mHeader.AddAlignment(new CWndAlignment(&mMoveUp, CWndAlignment::eAlign_TopLeft));
	mMoveDown.Create(s, CRect(cMoveBtnHOffset, cMoveDownBtnVOffset, cMoveBtnHOffset + cMoveBtnWidth, cMoveDownBtnVOffset + cMoveBtnHeight), &mHeader, IDC_SEARCHCRITERIA_MOVEDOWN, IDC_STATIC, IDI_MOVEDOWN);
	mHeader.AddAlignment(new CWndAlignment(&mMoveDown, CWndAlignment::eAlign_TopLeft));

	return 0;
}

// Resize sub-views
void CCriteriaBase::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CStatic::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

void CCriteriaBase::SetTop(bool top)
{
	mMoveUp.EnableWindow(!top);
}

void CCriteriaBase::SetBottom(bool bottom)
{
	mMoveDown.EnableWindow(!bottom);
}

void CCriteriaBase::OnSetMove(UINT nID)
{
	bool up = (nID == IDC_SEARCHCRITERIA_MOVEUP);

	CCriteriaBaseList& list = GetList();
	CCriteriaBaseList::iterator found = std::find(list.begin(), list.end(), this);
	unsigned long index = 0;
	if (found != list.end())
		index = found - list.begin();

	if (up && (index != 0))
		index--;
	else if (!up && (index != list.size() - 1))
		index++;
	CCriteriaBase* switch_with = list.at(index);

	if (switch_with != this)
		SwitchWith(switch_with);
}

