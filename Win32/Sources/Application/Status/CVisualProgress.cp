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


// CVisualProgress.cpp : implementation file
//

#include "CVisualProgress.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog dialog


CProgressDialog::CProgressDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CProgressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDialog)
	//}}AFX_DATA_INIT
}


void CProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDialog)
	DDX_UTF8Text(pDX, IDC_PROGRESSDESC, mTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CProgressDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDialog message handlers

BOOL CProgressDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Clone progress control and make it indeterminate
	mBarPane.SubclassDlgItem(IDC_PROGRESSPROGRESS, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Start modal loop
CProgressDialog* CProgressDialog::StartDialog(const cdstring& rsrc)
{
	CProgressDialog* dlg = new CProgressDialog(NULL);

	// Do SDI modal
	CSDIFrame::EnterModal(dlg);
	
	// Do MDI disable
	AfxGetMainWnd()->EnableWindow(false);
	
	// Create it from template
	cdstring status;
	status.FromResource(rsrc);
	dlg->SetDescriptor(status);
	dlg->CreateDlg(dlg->m_lpszTemplateName, dlg->m_pParentWnd);
	dlg->SetIndeterminate();
	
	// Show/enable this window
	dlg->ShowWindow(SW_SHOW);
	dlg->EnableWindow(true);
	
	return dlg;
}

// Stop modal loop
void CProgressDialog::StopDialog(CProgressDialog* dlog)
{
	// Do MDI disable
	AfxGetMainWnd()->EnableWindow(true);

	// Do SDI modal
	CSDIFrame::ExitModal();
	
	delete dlog;
}

