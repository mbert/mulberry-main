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

// CAcceptCertDialog.cp : implementation file
//

#include "CAcceptCertDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CAcceptCertDialog dialog


CAcceptCertDialog::CAcceptCertDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CAcceptCertDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAcceptCertDialog)
	mCert = _T("");
	//}}AFX_DATA_INIT
}


void CAcceptCertDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAcceptCertDialog)
	DDX_Control(pDX, IDC_ACCEPTCERT_ERRORS, mErrors);
	DDX_UTF8Text(pDX, IDC_ACCEPTCERT_CERT, mCert);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAcceptCertDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CAcceptCertDialog)
	ON_BN_CLICKED(IDC_ACCEPTCERT_ACCEPTSAVE, OnAcceptSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAcceptCertDialog message handlers

// Called during startup
BOOL CAcceptCertDialog::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Add items to list
	for(cdstrvect::const_iterator iter = mItems.begin(); iter != mItems.end(); iter++)
		mErrors.AddString((*iter).win_str());

	return true;
}

void CAcceptCertDialog::OnAcceptSave() 
{
	EndDialog(IDC_ACCEPTCERT_ACCEPTSAVE);
}


int CAcceptCertDialog::PoseDialog(const char* certificate, const cdstrvect& errors)
{
	int result = 0;

	CAcceptCertDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mItems = errors;
	dlog.mCert = certificate;

	switch(dlog.DoModal())
	{
	case IDOK:
		result = eAcceptOnce;
		break;
	case IDCANCEL:
	default:
		result = eNoAccept;
		break;
	case IDC_ACCEPTCERT_ACCEPTSAVE:
		result = eAcceptSave;
		break;

	}

	return result;
}
