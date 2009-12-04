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


// CToolbarButton.h : header file
//

#ifndef __CTOOLBARBUTTON__MULBERRY__
#define __CTOOLBARBUTTON__MULBERRY__

#include "CIconButton.h"

/////////////////////////////////////////////////////////////////////////////
// CToolbarButton window

class CToolbarButton : public CIconButton
{
	DECLARE_DYNAMIC(CToolbarButton)

// Construction
public:
	CToolbarButton();
	virtual ~CToolbarButton();

	virtual BOOL Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd,
							UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel = 0,
							UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0, bool frame = true);
	virtual BOOL SubclassDlgItem(UINT nID, CWnd* pParent,
								 UINT nIDIcon, UINT nIDIconSel = 0,
								 UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0, bool frame = true, bool clip = true);

	virtual bool HasPopup() const;
	virtual bool GetClickAndPopup() const
		{ return mClickAndPopup; }
	virtual void SetClickAndPopup(bool clickpopup)
		{ mClickAndPopup = clickpopup; }

			
			void SetSmallIcon(bool small_icon);
			void SetShowIcon(bool icon);
			void SetShowCaption(bool caption);

	void SetDragMode(bool mode)
	{
		mDragMode = mode;
		OnMouseLeave(0, 0);
	}

protected:
	static HICON sPopupIcon;
	UINT nIconIDBase;
	bool mCapture;
	bool mTracking;
	bool mSmallIcon;
	bool mShowIcon;
	bool mShowCaption;
	bool mClickAndPopup;
	bool mDragMode;

	virtual void DrawFrame(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawContent(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawIcon(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawTitle(LPDRAWITEMSTRUCT lpDIS);
	virtual void DrawPopupGlyph(LPDRAWITEMSTRUCT lpDIS);

	// message handlers
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnSetCheck(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
