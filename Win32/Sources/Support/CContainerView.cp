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


// CContainerView.cpp : implementation file
//


#include "CContainerView.h"

/////////////////////////////////////////////////////////////////////////////
// CContainerView

IMPLEMENT_DYNCREATE(CContainerView, CView)

CContainerView::CContainerView()
{
	m_hWndFocus = NULL;
}

CContainerView::~CContainerView()
{
}


BEGIN_MESSAGE_MAP(CContainerView, CView)
	//{{AFX_MSG_MAP(CContainerView)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContainerView drawing

void CContainerView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CContainerView message handlers

void CContainerView::ExpandChildToFit()
{
	// Get client area of container
	CRect client;
	GetClientRect(client);

	// Resize top window
	CWnd* top = GetTopWindow();
	if (top)
		top->SetWindowPos(NULL, 0, 0, client.Width(), client.Height(), SWP_NOZORDER);
}

// CContainerView columns
void CContainerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// Resize top window
	CWnd* top = GetTopWindow();
	if (top)
		top->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);
	
}

BOOL CContainerView::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try focused child window
	CWnd* focus = GetFocus();
	if (focus && IsChild(focus) && focus->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// Ask parent to handle
	if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CContainerView::OnActivateView(
	BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (SaveFocusControl())
		return;     // don't call base class when focus is already set

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CContainerView::OnActivateFrame(UINT nState, CFrameWnd* /*pFrameWnd*/)
{
	if (nState == WA_INACTIVE)
		SaveFocusControl();     // save focus when frame loses activation
}

BOOL CContainerView::SaveFocusControl()
{
	// save focus window if focus is on this window's controls
	HWND hWndFocus = ::GetFocus();
	if (hWndFocus != NULL && ::IsChild(m_hWnd, hWndFocus))
	{
		m_hWndFocus = hWndFocus;
		return TRUE;
	}
	return FALSE;
}

void CContainerView::OnSetFocus(CWnd*)
{
	if (!::IsWindow(m_hWndFocus) || !::IsChild(m_hWnd, m_hWndFocus))
	{
		// invalid or unknown focus window... let windows handle it
		m_hWndFocus = NULL;
		Default();
		return;
	}
	// otherwise, set focus to the last known focus window
	::SetFocus(m_hWndFocus);
}
