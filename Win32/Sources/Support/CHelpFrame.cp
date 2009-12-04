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


// CHelpFrame.cpp : implementation file
//


#include "CHelpFrame.h"

void PASCAL _MyFilterToolTipMessage(MSG* pMsg, CWnd* pWnd1);
void PASCAL _MyFilterToolTipMessage(MSG* pMsg, CWnd* pWnd1)
{
	UINT message = pMsg->message;
	if ((pWnd1->m_nFlags & WF_TOOLTIPS) &&
		(message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
		 message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
		 message == WM_MBUTTONUP) &&
		(GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
		 GetKeyState(VK_MBUTTON) >= 0))
	{
		// make sure that tooltips are not already being handled
		CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);
		while (pWnd != NULL && pWnd != pWnd1 && !(pWnd->m_nFlags & WF_TOOLTIPS))
			pWnd = pWnd->GetParent();
		if (pWnd != pWnd1)
			return;

		AFX_MODULE_THREAD_STATE* pModuleThreadState = AfxGetModuleThreadState();
		CToolTipCtrl* pToolTip = pModuleThreadState->m_pToolTip;
		CWnd* pOwner = pWnd1->GetParentOwner();
		if (pToolTip != NULL && pToolTip->GetOwner() != pOwner)
		{
			pToolTip->DestroyWindow();
			delete pToolTip;
			pModuleThreadState->m_pToolTip = NULL;
			pToolTip = NULL;
		}
		if (pToolTip == NULL)
		{
			pToolTip = new CBetterToolTipCtrl;
			if (!pToolTip->Create(pOwner, TTS_ALWAYSTIP))
			{
				delete pToolTip;
				return;
			}
			pToolTip->SendMessage(TTM_ACTIVATE, FALSE);
			pToolTip->SendMessage(TTM_SETMAXTIPWIDTH, 0, 200);
			pModuleThreadState->m_pToolTip = pToolTip;
		}
		
		pWnd1->FilterToolTipMessage(pMsg);
	}
}

#pragma mark ____________________________CToolTipHelp

int CToolTipHelp::ToolHitTest(const CWnd* aWnd, CPoint point, TOOLINFO* pTI) const
{
	// Only if not minimised
	if (aWnd->IsIconic())
		return -1;

	// Convert point to screen co-ordinates
	POINT pt = point;
	aWnd->ClientToScreen(&pt);
	
	// Find deep child hit by point
	HWND child = DeepChildWindowFromPoint(aWnd->GetSafeHwnd(), pt);

	if (child)
	{
		// First see if window object has its own handler
		CWnd* pWnd = CWnd::FromHandlePermanent(child);
		if (pWnd != NULL)
		{
			// Adjust point to child's client co-ords
			CPoint cpoint(pt);
			pWnd->ScreenToClient(&cpoint);
			
			// See if child wants to handle it
			int result = pWnd->OnToolHitTest(cpoint, pTI);
			if (result != -1)
				return result;
		}
			
		// Find appropriate tooltip resource ID
		int nHit = GetToolTipID(child);

		// hits against child windows always center the tip
		if ((nHit > 0) && (pTI != NULL) && (pTI->cbSize >= sizeof(AFX_OLDTOOLINFO)))
		{
			// setup the TOOLINFO structure
			pTI->hwnd = aWnd->GetSafeHwnd();
			pTI->uId = (UINT)child;
			pTI->uFlags |= TTF_IDISHWND;
			pTI->lpszText = (TCHAR*) ::malloc(200 * sizeof(TCHAR));
			if (::LoadString(::AfxGetInstanceHandle(), nHit, pTI->lpszText, 200) == 0)
			{
				ASSERT(FALSE);
			}

			// set TTF_NOTBUTTON and TTF_CENTERTIP if it isn't a button
			if (!(::SendMessage(child, WM_GETDLGCODE, 0, 0) & DLGC_BUTTON))
				pTI->uFlags |= TTF_NOTBUTTON|TTF_CENTERTIP;

			//_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
			//CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
			//pToolTip->SendMessage(TTM_SETMAXTIPWIDTH, 0, 200);

		}
		return nHit;
	}
	return -1;  // not found
}

HWND CToolTipHelp::DeepChildWindowFromPoint(HWND parent, POINT pt)
{
	// check child windows
	HWND child = ::GetWindow(parent, GW_CHILD);
	while(child)
	{
		// Must be visible
		if (::IsWindowVisible(child))
		{
			// see if point hits the child window
			CRect rect;
			::GetWindowRect(child, rect);
			if (rect.PtInRect(pt))
			{
				// Look inside child for nested children
				HWND deep = DeepChildWindowFromPoint(child, pt);

				// If child found return it
				if (deep)
					return deep;
				
				// If this window has a valid id return it
				else
				{
					int nID = ::GetDlgCtrlID(child);
					if (nID != -1)
						return child;
				}
			}
		}
		
		// Get next child
		child = ::GetWindow(child, GW_HWNDNEXT);
	}

	return NULL;    // not found
}

int CToolTipHelp::GetToolTipID(HWND aWnd) const
{
	// return positive hit if control ID isn't -1
	int nHit = _AfxGetDlgCtrlID(aWnd);

	// Look for mapping
	ulmap::const_iterator found = mToolTipMap.find(nHit);
	if (found != mToolTipMap.end())
		nHit = (*found).second;
	else
		nHit = -1;
	
	return nHit;
}

#pragma mark ____________________________CBetterToolTipCtrl

BEGIN_MESSAGE_MAP(CBetterToolTipCtrl, CToolTipCtrl)
	//{{AFX_MSG_MAP(CToolTipCtrl)
	ON_MESSAGE(TTM_WINDOWFROMPOINT, OnWindowFromPoint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CBetterToolTipCtrl::OnWindowFromPoint(WPARAM, LPARAM lParam)
{
	ASSERT(lParam != NULL);

	// the default implementation of tooltips just calls WindowFromPoint
	// which does not work for certain kinds of combo boxes
	CPoint pt = *(POINT*)lParam;
	
	HWND hWnd = ::WindowFromPoint(pt);
	if (hWnd == NULL)
		return 0;

	// Do special
	HWND temp = CToolTipHelp::DeepChildWindowFromPoint(hWnd, pt);
	if (temp)
		hWnd = temp;

	// try to hit combobox instead of edit control for CBS_DROPDOWN styles
	HWND hWndTemp = ::GetParent(hWnd);
	if (hWndTemp != NULL && _AfxIsComboBoxControl(hWndTemp, CBS_DROPDOWN))
		return (LRESULT)hWndTemp;

	// handle special case of disabled child windows
	::ScreenToClient(hWnd, &pt);
	hWndTemp = _AfxChildWindowFromPoint(hWnd, pt);
	if (hWndTemp != NULL && !::IsWindowEnabled(hWndTemp))
		return (LRESULT)hWndTemp;

	return (LRESULT)hWnd;
}

#pragma mark ____________________________CHelpFrame
/////////////////////////////////////////////////////////////////////////////
// CHelpFrame

BEGIN_MESSAGE_MAP(CHelpFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CHelpFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CHelpFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Enable tooltips
	InitToolTipMap();
	EnableToolTips(true);

	//AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
	//pModuleState->m_pfnFilterToolTipMessage = &_MyFilterToolTipMessage;

	return 0;
}

int CHelpFrame::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	return ToolHitTest(this, point, pTI);
}

#pragma mark ____________________________CHelpMDIChildWnd
/////////////////////////////////////////////////////////////////////////////
// CHelpFrame

BEGIN_MESSAGE_MAP(CHelpMDIChildWnd, CMDIChildWnd)
	//{{AFX_MSG_MAP(CHelpFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CHelpMDIChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Enable tooltips
	InitToolTipMap();
	EnableToolTips(true);

	//AFX_MODULE_STATE* pModuleState = _AFX_CMDTARGET_GETSTATE();
	//pModuleState->m_pfnFilterToolTipMessage = &_MyFilterToolTipMessage;

	return 0;
}

int CHelpMDIChildWnd::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	return ToolHitTest(this, point, pTI);
}
