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

// CSaveDraftDialog.cpp : implementation file
//

#include "CSaveDraftDialog.h"

#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveDraftDialog dialog

CSaveDraftDialog::SSaveDraft CSaveDraftDialog::sDetails;

CSaveDraftDialog::CSaveDraftDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSaveDraftDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveDraftDialog)
	mFile = -1;
	//}}AFX_DATA_INIT
}


void CSaveDraftDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveDraftDialog)
	DDX_Radio(pDX, IDC_SAVEDRAFT_TOFILE, mFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveDraftDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSaveDraftDialog)
	ON_COMMAND_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnCopyToPopup)
	ON_BN_CLICKED(IDC_SAVEDRAFT_TOFILE, OnSaveDraftToFile)
	ON_BN_CLICKED(IDC_SAVEDRAFT_TOMAILBOX, OnSaveDraftToMailbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveDraftDialog message handlers

BOOL CSaveDraftDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass popup
	mMailboxPopup.SubclassDlgItem(IDC_SAVEDRAFT_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mMailboxPopup.SetCopyTo(false);
	mMailboxPopup.EnableWindow(mFile);
	mMailboxPopup.SetSelectedMbox(mMailboxName, mMailboxName.empty(), mMailboxName == "\1");

	if (!mAllowFile)
		GetDlgItem(IDC_SAVEDRAFT_TOFILE)->EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set the dialogs info
void CSaveDraftDialog::SetDetails(bool allow_file)
{
	mFile = (allow_file && sDetails.mFile ? 0 : 1);
	mAllowFile = allow_file;

	mMailboxName = sDetails.mMailboxName;
}

// Set the dialogs return info
bool CSaveDraftDialog::GetDetails()
{
	bool result = true;

	sDetails.mFile = (mFile == 0);
	
	// Only resolve mailbox name if saving to a mailbox
	if (!sDetails.mFile)
		result = mMailboxPopup.GetSelectedMboxName(sDetails.mMailboxName, true);
	
	return result;
}

void CSaveDraftDialog::OnSaveDraftToFile() 
{
	mMailboxPopup.EnableWindow(false);
}

void CSaveDraftDialog::OnSaveDraftToMailbox() 
{
	mMailboxPopup.EnableWindow(true);
}

void CSaveDraftDialog::OnCopyToPopup(UINT nID) 
{
	// Don't allow None
	if (nID == IDM_AppendToPopupNone)
		nID = IDM_AppendToPopupChoose;
	
	// Make sure menu value is setup
	mMailboxPopup.SetValue(nID);
}

bool CSaveDraftDialog::PoseDialog(SSaveDraft& details, bool allow_file)
{
	bool result = false;

	CSaveDraftDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(allow_file);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		result = dlog.GetDetails();
		details.mFile = sDetails.mFile;
		details.mMailboxName = sDetails.mMailboxName;
	}

	return result;
}
