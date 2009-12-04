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


// CHelpPropertySheet.cp : implementation file
//

#include "CHelpPropertySheet.h"

#include "CLog.h"

#ifdef __MULBERRY
#include "CSDIFrame.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CHelpPropertySheet dialog

IMPLEMENT_DYNAMIC(CHelpPropertySheet, CPropertySheet)

CHelpPropertySheet::CHelpPropertySheet(LPCTSTR pszCaption, CWnd* pParent /*=NULL*/)
	: CPropertySheet(pszCaption, pParent, 0)
{
}

CHelpPropertySheet::CHelpPropertySheet(UINT nIDCaption, CWnd* pParent /*=NULL*/)
	: CPropertySheet(nIDCaption, pParent, 0)
{
}


BEGIN_MESSAGE_MAP(CHelpPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CHelpPropertySheet)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CHelpPropertySheet::DoModal()
{
	// Need to do special hack to get this to work when IE 4 is NOT present
	// MFC headers & lib compiled with _WIN32_IE >= 0x0400
	// This causes size for PROPSHEETHEADER to be bigger (normally 40, now 52)
	// Without IE4 propsheets do not display :-(
	// So change size of structure here back to old value - seems to work :-)
	
	//m_psh.dwSize = 40;
	
#ifdef __MULBERRY
	CSDIFrame::EnterModal(this);
	int result = 0;

	try
	{
		result = CPropertySheet::DoModal();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		CSDIFrame::ExitModal();
		CLOG_LOGRETHROW;
		throw;
	}
	
	CSDIFrame::ExitModal();
	
	return result;
#else
	return CPropertySheet::DoModal();
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CHelpPropertySheet message handlers

BOOL CHelpPropertySheet::OnInitDialog()
{
	BOOL result = CPropertySheet::OnInitDialog();

	CRect rc;
	GetWindowRect(rc);
	SetWindowPos(nil,
					(::GetSystemMetrics(SM_CXSCREEN) - rc.Width())/2,
					(::GetSystemMetrics(SM_CYSCREEN) - rc.Height())/2,
					0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

	return result;
}

BOOL CHelpPropertySheet::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) && (pHelpInfo->iCtrlId > 0) &&
		(pHelpInfo->iCtrlId > GetTabControl()->GetDlgCtrlID()))
	{
		// finally, run the Windows Help engine
		if (!::WinHelp((HWND) pHelpInfo->hItemHandle, ::AfxGetApp()->m_pszHelpFilePath, HELP_CONTEXTPOPUP, (DWORD) HID_BASE_RESOURCE + pHelpInfo->iCtrlId))
			::AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
		
		return TRUE;
	}
	else
		return CPropertySheet::OnHelpInfo(pHelpInfo);
}
