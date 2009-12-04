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

// CViewCertDialog.cp : implementation file
//

#include "CViewCertDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CViewCertDialog dialog


CViewCertDialog::CViewCertDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CViewCertDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewCertDialog)
	//}}AFX_DATA_INIT
}


void CViewCertDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewCertDialog)
	DDX_UTF8Text(pDX, IDC_VIEWCERT_CERT, mCert);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewCertDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CViewCertDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewCertDialog message handlers

void CViewCertDialog::PoseDialog(const cdstring& certificate)
{
	CViewCertDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mCert = certificate;

	dlog.DoModal();
}
