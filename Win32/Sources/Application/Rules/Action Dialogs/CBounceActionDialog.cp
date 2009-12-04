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


// BounceActionDialog.cpp : implementation file
//

#include "CBounceActionDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CBounceActionDialog dialog


CBounceActionDialog::CBounceActionDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CBounceActionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBounceActionDialog)
	mUseStandard = -1;
	mCreateDraft = FALSE;
	//}}AFX_DATA_INIT
}

void CBounceActionDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBounceActionDialog)
	DDX_UTF8Text(pDX, IDC_BOUNCEACTION_TO, mTo);
	DDX_UTF8Text(pDX, IDC_BOUNCEACTION_CC, mCC);
	DDX_UTF8Text(pDX, IDC_BOUNCEACTION_BCC, mBcc);
	DDX_Radio(pDX, IDC_BOUNCEACTION_STANDARDIDENTITY, mUseStandard);
	DDX_Control(pDX, IDC_BOUNCEACTION_IDENTITY, mIdentityPopup);
	DDX_Check(pDX, IDC_BOUNCEACTION_DRAFT, mCreateDraft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBounceActionDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CBounceActionDialog)
	ON_BN_CLICKED(IDC_BOUNCEACTION_STANDARDIDENTITY, OnStandardIdentity)
	ON_BN_CLICKED(IDC_BOUNCEACTION_USEIDENTITY, OnUseIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBounceActionDialog message handlers

BOOL CBounceActionDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass items
	mIdentityPopup.SubclassDlgItem(IDC_BOUNCEACTION_IDENTITY, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	// Set identity items
	mIdentityPopup.SetIdentity(CPreferences::sPrefs, mCurrentIdentity);
	mIdentityPopup.EnableWindow(mUseStandard == 1);

	return true;
}

void CBounceActionDialog::OnStandardIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_BOUNCEACTION_STANDARDIDENTITY));
	mIdentityPopup.EnableWindow(!cb->GetCheck());
}

void CBounceActionDialog::OnUseIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_BOUNCEACTION_USEIDENTITY));
	mIdentityPopup.EnableWindow(cb->GetCheck());
}

void CBounceActionDialog::OnIdentityPopup(UINT nID)
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
void CBounceActionDialog::SetDetails(CActionItem::CActionBounce& details)
{
	mTo = details.Addrs().mTo;

	mCC = details.Addrs().mCC;

	mBcc = details.Addrs().mBcc;

	mUseStandard = details.UseTiedIdentity() ? 0 : 1;
	
	mCurrentIdentity = details.GetIdentity();

	mCreateDraft = details.CreateDraft();
}

// Get the details
void CBounceActionDialog::GetDetails(CActionItem::CActionBounce& details)
{
	details.Addrs().mTo = mTo;

	details.Addrs().mCC = mCC;

	details.Addrs().mBcc = mBcc;

	details.SetTiedIdentity(mUseStandard == 0);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft);
}

bool CBounceActionDialog::PoseDialog(CActionItem::CActionBounce& details)
{
	bool result = false;

	// Create the dialog
	CBounceActionDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if ((dlog.DoModal() == IDOK))
	{
		dlog.GetDetails(details);
		result = true;
	}

	return result;
}
