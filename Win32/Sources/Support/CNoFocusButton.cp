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


// CNoFocusButton.cpp : implementation file
//


#include "CNoFocusButton.h"

BEGIN_MESSAGE_MAP(CNoFocusButton, CButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoFocusButton

void CNoFocusButton::OnLButtonDown(UINT nFlags, CPoint pt)
{
	// Cache current focus item
	mLastFocus = ::GetFocus();

	CButton::OnLButtonDown(nFlags, pt);
}

void CNoFocusButton::OnLButtonUp(UINT nFlags, CPoint pt)
{
	// Cache mLastFocus/this here since button up can cause window (and
	// therefore this button) to be closed/destroyed
	HWND last = mLastFocus;
	mLastFocus = NULL;
	
	CButton::OnLButtonUp(nFlags, pt);

	// Reset focus to previous focus if this still holds the focus
	// NB As a result of the click a new window may be created and we should not
	// steal focus from that, hence the check to see if this is still focussed
	if (last && (GetFocus() == this))
		::SetFocus(last);
}

BEGIN_MESSAGE_MAP(CNoFocusBitmapButton, CBitmapButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoFocusBitmapButton

void CNoFocusBitmapButton::OnLButtonDown(UINT nFlags, CPoint pt)
{
	// Cache current focus item
	mLastFocus = ::GetFocus();

	CBitmapButton::OnLButtonDown(nFlags, pt);
}

void CNoFocusBitmapButton::OnLButtonUp(UINT nFlags, CPoint pt)
{
	// Cache mLastFocus/this here since button up can cause window (and
	// therefore this button) to be closed/destroyed
	HWND last = mLastFocus;
	mLastFocus = NULL;
	
	CBitmapButton::OnLButtonUp(nFlags, pt);

	// Reset focus to previous focus if this still holds the focus
	// NB As a result of the click a new window may be created and we should not
	// steal focus from that, hence the check to see if this is still focussed
	if (last && ((GetFocus() == this) || (GetFocus() == NULL)))
		::SetFocus(last);
}
