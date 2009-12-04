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


// CRejectActionDialog.cp : implementation file
//

#include "CRejectActionDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CRejectActionDialog dialog


CRejectActionDialog::CRejectActionDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CRejectActionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRejectActionDialog)
	mReturn = -1;
	mUseStandard = -1;
	mCreateDraft = FALSE;
	//}}AFX_DATA_INIT
}


void CRejectActionDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRejectActionDialog)
	DDX_Radio(pDX, IDC_REJECTACTION_HEADERS, mReturn);
	DDX_Radio(pDX, IDC_REJECTACTION_STANDARDIDENTITY, mUseStandard);
	DDX_Control(pDX, IDC_REJECTACTION_IDENTITY, mIdentityPopup);
	DDX_Check(pDX, IDC_REJECTACTION_DRAFT, mCreateDraft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRejectActionDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CRejectActionDialog)
	ON_BN_CLICKED(IDC_REJECTACTION_STANDARDIDENTITY, OnStandardIdentity)
	ON_BN_CLICKED(IDC_REJECTACTION_USEIDENTITY, OnUseIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRejectActionDialog message handlers

BOOL CRejectActionDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass items
	mIdentityPopup.SubclassDlgItem(IDC_REJECTACTION_IDENTITY, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	// Set identity items
	mIdentityPopup.SetIdentity(CPreferences::sPrefs, mCurrentIdentity);
	mIdentityPopup.EnableWindow(mUseStandard == 1);

	return true;
}

void CRejectActionDialog::OnStandardIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_REJECTACTION_STANDARDIDENTITY));
	mIdentityPopup.EnableWindow(!cb->GetCheck());
}

void CRejectActionDialog::OnUseIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_REJECTACTION_USEIDENTITY));
	mIdentityPopup.EnableWindow(cb->GetCheck());
}

void CRejectActionDialog::OnIdentityPopup(UINT nID)
{
	switch(nID)
	{
	// New identity wanted
	case IDM_IDENTITY_NEW:
		mIdentityPopup.DoNewIdentity(CPreferences::sPrefs);
		break;
	
	// New identity wanted
	case IDM_IDENTITY_EDIT:
		mIdentityPopup.DoEditIdentity(CPreferences::sPrefs);
		break;
	
	// Delete existing identity
	case IDM_IDENTITY_DELETE:
		mIdentityPopup.DoDeleteIdentity(CPreferences::sPrefs);
		break;
	
	// Select an identity
	default:
		mIdentityPopup.SetValue(nID);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.Value()[nID - IDM_IDENTITYStart].GetIdentity();
		break;
	}
}

// Set the details
void CRejectActionDialog::SetDetails(CActionItem::CActionReject& details)
{
	mReturn = details.ReturnMessage();

	mUseStandard = details.UseTiedIdentity() ? 0 : 1;
	
	mCurrentIdentity = details.GetIdentity();

	mCreateDraft = details.CreateDraft();
}

// Get the details
void CRejectActionDialog::GetDetails(CActionItem::CActionReject& details)
{
	details.SetReturnMessage(mReturn == 1);

	details.SetTiedIdentity(mUseStandard == 0);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft);
}

bool CRejectActionDialog::PoseDialog(CActionItem::CActionReject& details)
{
	bool result = false;

	// Create the dialog
	CRejectActionDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if ((dlog.DoModal() == IDOK))
	{
		dlog.GetDetails(details);
		result = true;
	}

	return result;
}
