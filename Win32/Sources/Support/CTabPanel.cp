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


// CTabPanel

// Class that implements a tab panel in a CTabCtrl

#pragma once

#include "CTabPanel.h"

IMPLEMENT_DYNAMIC(CTabPanel, CHelpDialog)

BEGIN_MESSAGE_MAP(CTabPanel, CHelpDialog)
	//{{AFX_MSG_MAP(CFormView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTabPanel::CTabPanel(LPCTSTR lpszTemplateName)
{
	m_lpszTemplateName = lpszTemplateName;
	m_pCreateContext = NULL;
}

CTabPanel::CTabPanel(UINT nIDTemplate)
{
	ASSERT_VALID_IDR(nIDTemplate);
	m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
	m_pCreateContext = NULL;
}

CTabPanel::~CTabPanel()
{
}

// virtual override of CHelpDialog::Create
BOOL CTabPanel::Create(const RECT& rect, CWnd* pParentWnd, UINT nID,
	CCreateContext* pContext)
{
	ASSERT(pParentWnd != NULL);
	ASSERT(m_lpszTemplateName != NULL);

	m_pCreateContext = pContext;    // save state for later OnCreate

	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));

	// allow chance to modify styles
	CREATESTRUCT cs;
	memset(&cs, 0, sizeof(cs));
	cs.lpszClass = _afxWndControlBar;
	cs.style = WS_CHILD;
	cs.hMenu = (HMENU)nID;
	cs.hInstance = AfxGetInstanceHandle();
	cs.hwndParent = pParentWnd->GetSafeHwnd();
	if (!PreCreateWindow(cs))
		return FALSE;

	// create a modeless dialog
	if (!CreateDlg(m_lpszTemplateName, pParentWnd))
		return FALSE;

	m_pCreateContext = NULL;

	// we use the style from the template - but make sure that
	//  the WS_BORDER bit is correct
	// the WS_BORDER bit will be whatever is in dwRequestedStyle
	ModifyStyle(WS_BORDER|WS_CAPTION, cs.style & (WS_BORDER|WS_CAPTION));
	ModifyStyleEx(WS_EX_CLIENTEDGE, cs.dwExStyle & WS_EX_CLIENTEDGE);

	SetDlgCtrlID(nID);

	// force WS_CLIPSIBLINGS
	ModifyStyle(0, WS_CLIPSIBLINGS);

	// initialize controls etc
	if (!ExecuteDlgInit(m_lpszTemplateName))
		return FALSE;

	// force the size requested
	SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER|SWP_NOACTIVATE);

	return TRUE;
}

int CTabPanel::OnCreate(LPCREATESTRUCT lpcs)
{
	// since we can't get the create context parameter passed in
	//  through CreateDialog, we use a temporary member variable
	lpcs->lpCreateParams = (LPVOID)m_pCreateContext;
	return CHelpDialog::OnCreate(lpcs);
}

void CTabPanel::OnOK()
{
	// Always ignore in panel
}

void CTabPanel::OnCancel()
{
	// Always ignore in panel
}
