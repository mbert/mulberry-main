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


// CGrayBorder.cpp : implementation file
//


#include "CGrayBorder.h"

#include "CDrawUtils.h"
#include "CGrayBackground.h"

IMPLEMENT_DYNAMIC(CGrayBorder, CButton)

/////////////////////////////////////////////////////////////////////////////
// CGrayBorder

CGrayBorder::CGrayBorder()
{
}

CGrayBorder::~CGrayBorder()
{
}


BEGIN_MESSAGE_MAP(CGrayBorder, CButton)
	//{{AFX_MSG_MAP(CGrayBorder)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Gray border support

int CGrayBorder::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	return 0;
}

// CContainerWnd columns
void CGrayBorder::OnSize(UINT nType, int cx, int cy)
{
	CButton::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

HBRUSH CGrayBorder::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Change background for CStatics only
	if ((nCtlColor == CTLCOLOR_STATIC) ||
		(nCtlColor == CTLCOLOR_BTN))
	{
		pDC->SetBkColor(::GetSysColor(COLOR_BTNFACE));
		pDC->SetTextColor(nCtlColor == CTLCOLOR_STATIC ? CDrawUtils::sWindowTextColor : CDrawUtils::sBtnTextColor);
		return CGrayBackground::sBkBrush;
	}
	else
		return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

HBRUSH CGrayBorder::CtlColor(CDC* pDC, UINT nCtlColor)
{
	// use helper in CWnd
	//return GrayCtlColor(pDC, this, nCtlColor);
	return CGrayBackground::sBkBrush;
}

BOOL CGrayBorder::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
   // Extend the framework's command route from the view to
   // the application-specific CMyShape that is currently selected
   // in the view. m_pActiveShape is NULL if no shape object
   // is currently selected in the view.
   if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
      return true;

   // If the object(s) in the extended command route don't handle
   // the command, then let the base class OnCmdMsg handle it.
   return CButton::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
