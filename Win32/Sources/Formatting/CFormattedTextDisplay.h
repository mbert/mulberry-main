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


#ifndef __CFORMATTEDTEXTDISPLAY__MULBERRY__
#define __CFORMATTEDTEXTDISPLAY__MULBERRY__

#include "CSpacebarEditView.h"
#include "CDisplayFormatter.h"

class CFormattedTextDisplay : public CSpacebarEditView
{
	DECLARE_DYNCREATE(CFormattedTextDisplay)
	
public:
					CFormattedTextDisplay();
	virtual			~CFormattedTextDisplay();
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormattedTextDisplay)
	//}}AFX_VIRTUAL
	
	
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	virtual int		OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL	OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnCaptureAddress();
	
	CDisplayFormatter* GetFormatter() const
		{ return mFormatter; }

	void	WipeText(CFont* pFont, long scale);		// Change font in whole control
	void	Reset(bool click = false, long scale = 0);
	
	DECLARE_MESSAGE_MAP()

protected:
	virtual void HandleContextMenu(CWnd*, CPoint point);

private:
	CDisplayFormatter*	mFormatter;
	CClickElement*		mClickable;
	CClickElement*		mTooltipElement;
#ifdef _UNICODE
	cdustring			mTooltipText;
#else
	cdstring			mTooltipText;
#endif
	bool				mCanClick;
	static HCURSOR		sHandCursor;
};

#endif
