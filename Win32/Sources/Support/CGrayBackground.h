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


// CGrayBackground.h : header file
//

#ifndef __CGRAYBACKGROUND__MULBERRY__
#define __CGRAYBACKGROUND__MULBERRY__

#include "CWndAligner.h"

/////////////////////////////////////////////////////////////////////////////
// CGrayBackground window

class CGrayBackground : public CStatic, public CWndAligner
{
	DECLARE_DYNAMIC(CGrayBackground)

// Construction
public:
	static CBrush sBkBrush;

	CGrayBackground();
	virtual ~CGrayBackground();

	virtual void	ResizeImage(int imageWidth, int imageHeight, bool client_resize = false);
	virtual void	ScrollImage(int dx, int dy);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual void	CreateDialogItems(UINT nID, CWnd* pParent);

			void	SetBorderFocus(bool focus);
			void	SetFocusBorder();
			bool	CanFocus() const
			{
				return mUseFocus;
			}

	// Generated message map functions
protected:
	int mImageHeight;
	int mImageWidth;
	bool mUseFocus;
	bool mHasFocus;

	virtual void InitDialog() {}

	//{{AFX_MSG(CGrayBackground)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
