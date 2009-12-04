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


// CTextButton.h : header file
//

#ifndef __CTEXTBUTTON__MULBERRY__
#define __CTEXTBUTTON__MULBERRY__

#include "CNoFocusButton.h"

/////////////////////////////////////////////////////////////////////////////
// CTextButton window

class CTextButton : public CNoFocusButton
{
	DECLARE_DYNAMIC(CTextButton)

// Construction
public:
	CTextButton();
	virtual ~CTextButton();

	virtual BOOL Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nIconID = 0);

	virtual void SetPushed(bool pushed);			// Set pushed state
	virtual bool IsPushed(void) const			// Get pushed state
				{ return mPushed; }

protected:
	bool mPushed;
	UINT mIconID;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawFrame(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawContent(LPDRAWITEMSTRUCT lpDIS);

	// message handlers
	afx_msg LRESULT OnSetCheck(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClick();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
