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


// CCancelDialog.cpp : implementation file
//

#include "CCancelDialog.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCancelDialog dialog


CCancelDialog::CCancelDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCancelDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCancelDialog)
	//}}AFX_DATA_INIT
	mLastSecs = 0;
	mCancelled = false;
}


void CCancelDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCancelDialog)
	DDX_Control(pDX, IDC_CANCELTIME, mTime);
	DDX_UTF8Text(pDX, IDC_CANCELDESC, mDescriptor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCancelDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCancelDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCancelDialog message handlers

BOOL CCancelDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Clone progress control and make it indeterminate
	mProgress.SubclassDlgItem(IDC_CANCELPROGRESS, this);
	mProgress.SetIndeterminate(true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCancelDialog::OnCancel() 
{
	// Just set the flag - the creator of the dialog will remove it
	mCancelled = true;
}

// Set descriptor text
void CCancelDialog::SetBusyDescriptor(const cdstring& desc)
{
	mDescriptor = desc;
}

// Called during idle
void CCancelDialog::SetTime(unsigned long secs)
{
	// Only if different
	if (mLastSecs != secs)
	{
		cdstring txt(secs);
		CUnicodeUtils::SetWindowTextUTF8(&mTime, txt);
		mLastSecs = secs;
	}
}

// Start modal loop
void CCancelDialog::StartModal()
{
	// Do SDI modal
	CSDIFrame::EnterModal(this);
	
	// Do MDI disable
	AfxGetMainWnd()->EnableWindow(false);
	
	// Create it from template
	CreateDlg(m_lpszTemplateName, m_pParentWnd);
	
	// Show/enable this window
	ShowWindow(SW_SHOW);
	EnableWindow(true);
}

// Start modal loop
// return: true if cancelled
bool CCancelDialog::ModalLoop()
{
    MSG msg;
    if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
    {
        if ((msg.message == WM_CHAR) && (msg.wParam == VK_ESCAPE))
            OnCancel();

        if (!AfxGetApp()->PumpMessage()) 
        {
            ::PostQuitMessage(0);
            return false;
        } 
    }

	return mCancelled;
}

// Stop modal loop
void CCancelDialog::StopModal()
{
	// Do MDI disable
	AfxGetMainWnd()->EnableWindow(true);

	// Do SDI modal
	CSDIFrame::ExitModal();
}

