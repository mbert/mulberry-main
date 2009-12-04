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

// CMDNPromptDialog.cpp : implementation file
//

#include "CMDNPromptDialog.h"

#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMDNPromptDialog dialog

CMDNPromptDialog::CMDNPromptDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CMDNPromptDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMDNPromptDialog)
	mAddress = "";
	mSave = FALSE;
	//}}AFX_DATA_INIT
}


void CMDNPromptDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMDNPromptDialog)
	DDX_UTF8Text(pDX, IDC_MDNPROMPT_ADDRESS, mAddress);
	DDX_Check(pDX, IDC_MDNPROMPT_SAVE, mSave);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMDNPromptDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CMDNPromptDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDNPromptDialog message handlers

// Set the dialogs info
void CMDNPromptDialog::SetDetails(const cdstring& addr)
{
	mAddress = addr;
	mSave = false;
}

// Set the dialogs info
void CMDNPromptDialog::GetDetails(bool& save)
{
	save = mSave;
}

void CMDNPromptDialog::OnCancel()
{
	// Do data update for Cancel as well as OK
	UpdateData(TRUE);

	// Do default cancel action
	EndDialog(IDCANCEL);
}

bool CMDNPromptDialog::PoseDialog(const cdstring& addr)
{
	bool result = false;

	CMDNPromptDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(addr);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		bool save;
		dlog.GetDetails(save);

		// Set preferences if required
		if (save)
			CPreferences::sPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);

		result = true;
	}
	else
	{
		bool save;
		dlog.GetDetails(save);

		// Set preferences if required
		if (save)
			CPreferences::sPrefs->mMDNOptions.SetValue(eMDNNeverSend);

		result = false;
	}

	return result;
}
