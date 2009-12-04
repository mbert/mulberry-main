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


// CColorButton.h : header file
//

#ifndef __CCOLORBUTTON__MULBERRY__
#define __CCOLORBUTTON__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CColorPickerButton window

class CColorPickerButton : public CButton
{
	DECLARE_DYNAMIC(CColorPickerButton)

// Construction
public:
	CColorPickerButton();
	virtual ~CColorPickerButton();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, COLORREF color, UINT nID);

	virtual void SetColor(COLORREF colour);			// Set color
	virtual COLORREF GetColor(void) const			// Get color
				{ return mColor; }

	afx_msg	void OnClicked();						// Clicked item

protected:
	COLORREF mColor;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

	// message handlers

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
