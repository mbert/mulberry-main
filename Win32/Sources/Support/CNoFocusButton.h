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


// CNoFocusButton.h : header file
//

#ifndef __CNOFOCUSBUTTON__MULBERRY__
#define __CNOFOCUSBUTTON__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CNoFocusButton window

class CNoFocusButton : public CButton
{
// Construction
public:
	CNoFocusButton()
		{ mLastFocus = NULL; }
	virtual ~CNoFocusButton() {}

protected:
	HWND mLastFocus;

	// message handlers
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);

	DECLARE_MESSAGE_MAP()
};

class CNoFocusBitmapButton : public CBitmapButton
{
// Construction
public:
	CNoFocusBitmapButton()
		{ mLastFocus = NULL; }
	virtual ~CNoFocusBitmapButton() {}

protected:
	HWND mLastFocus;

	// message handlers
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
