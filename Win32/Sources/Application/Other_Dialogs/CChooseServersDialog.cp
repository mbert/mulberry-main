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


// CChooseServersDialog.cp : implementation file
//


#include "CChooseServersDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CChooseServersDialog::CChooseServersDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CChooseServersDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenameMailboxDialog)
	//}}AFX_DATA_INIT
}


void CChooseServersDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseServersDialog)
	DDX_UTF8Text(pDX, IDC_CHOOSESERVERSMAIL, mMail);
	DDX_UTF8Text(pDX, IDC_CHOOSESERVERSSMTP, mSMTP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseServersDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CChooseServersDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseServersDialog message handlers

bool CChooseServersDialog::PoseDialog(cdstring& mail, cdstring& smtp)
{
	// Create the dialog (use old name as starter)
	CChooseServersDialog dlog(CSDIFrame::GetAppTopWindow());
	
	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		mail = dlog.mMail;
		smtp = dlog.mSMTP;
		return true;
	}
	else
		return false;
}
