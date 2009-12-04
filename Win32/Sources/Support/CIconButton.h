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


// CIconButton.h : header file
//

#ifndef __CICONBUTTON__MULBERRY__
#define __CICONBUTTON__MULBERRY__

#include "CNoFocusButton.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CIconButton window

class CIconButton : public CNoFocusBitmapButton
{
	DECLARE_DYNAMIC(CIconButton)

// Construction
public:
	CIconButton();
	virtual ~CIconButton();

	virtual BOOL Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd,
							UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel = 0,
							UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0, bool frame = true);
	virtual BOOL SubclassDlgItem(UINT nID, CWnd* pParent,
								 UINT nIDIcon, UINT nIDIconSel = 0,
								 UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0, bool frame = true, bool clip = true);

	virtual void SetTitle(const cdstring& s);				// Change to new title
	virtual void GetTitle(cdstring& s) const;			// Get title

	virtual void SetPushed(bool pushed);			// Set pushed state
	virtual bool IsPushed(void) const			// Get pushed state
				{ return mPushed; }

	virtual void SetSimpleFrame(bool simple)
		{ mSimpleFrame = simple; }

	virtual void SetIcons(UINT nIDIcon, UINT nIDIconSel = 0,
							UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0);

protected:
	CStatic mTitle;
	bool mUseTitle;
	UINT nIcon;
	UINT nIconSel;
	UINT nIconPushed;
	UINT nIconPushedSel;
	bool mDisposeSel;
	bool mDisposePushed;
	bool mDisposePushedSel;
	bool mPushed;
	int mSize;
	bool mFrame;
	bool mSimpleFrame;
	bool mLargeTitle;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawFrame(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawContent(LPDRAWITEMSTRUCT lpDIS);

	// message handlers
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
