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


// CForwardActionDialog.cp : implementation file
//

#include "CForwardActionDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CForwardActionDialog dialog


CForwardActionDialog::CForwardActionDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CForwardActionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForwardActionDialog)
	mQuote = FALSE;
	mAttach = FALSE;
	mUseStandard = -1;
	mCreateDraft = FALSE;
	//}}AFX_DATA_INIT
}


void CForwardActionDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForwardActionDialog)
	DDX_UTF8Text(pDX, IDC_FORWARDACTION_TO, mTo);
	DDX_UTF8Text(pDX, IDC_FORWARDACTION_CC, mCC);
	DDX_UTF8Text(pDX, IDC_FORWARDACTION_BCC, mBcc);
	DDX_Check(pDX, IDC_FORWARDACTION_QUOTE, mQuote);
	DDX_Check(pDX, IDC_FORWARDACTION_ATTACH, mAttach);
	DDX_Radio(pDX, IDC_FORWARDACTION_STANDARDIDENTITY, mUseStandard);
	DDX_Control(pDX, IDC_FORWARDACTION_IDENTITY, mIdentityPopup);
	DDX_Check(pDX, IDC_FORWARDACTION_DRAFT, mCreateDraft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CForwardActionDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CForwardActionDialog)
	ON_BN_CLICKED(IDC_FORWARDACTION_TEXT, OnText)
	ON_BN_CLICKED(IDC_FORWARDACTION_STANDARDIDENTITY, OnStandardIdentity)
	ON_BN_CLICKED(IDC_FORWARDACTION_USEIDENTITY, OnUseIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForwardActionDialog message handlers

BOOL CForwardActionDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass items
	mIdentityPopup.SubclassDlgItem(IDC_FORWARDACTION_IDENTITY, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	// Set identity items
	mIdentityPopup.SetIdentity(CPreferences::sPrefs, mCurrentIdentity);
	mIdentityPopup.EnableWindow(mUseStandard == 1);

	return true;
}

void CForwardActionDialog::OnText() 
{
	CPrefsEditHeadFoot::PoseDialog(CPreferences::sPrefs, mText, false);
}

void CForwardActionDialog::OnStandardIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_FORWARDACTION_STANDARDIDENTITY));
	mIdentityPopup.EnableWindow(!cb->GetCheck());
}

void CForwardActionDialog::OnUseIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_FORWARDACTION_USEIDENTITY));
	mIdentityPopup.EnableWindow(cb->GetCheck());
}

void CForwardActionDialog::OnIdentityPopup(UINT nID)
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
void CForwardActionDialog::SetDetails(CActionItem::CActionForward& details)
{
	mTo = details.Addrs().mTo;

	mCC = details.Addrs().mCC;

	mBcc = details.Addrs().mBcc;

	mQuote = details.Quote();

	mAttach = details.Attach();

	mText = details.GetText();

	mUseStandard = details.UseTiedIdentity() ? 0 : 1;
	
	mCurrentIdentity = details.GetIdentity();

	mCreateDraft = details.CreateDraft();
}

// Get the details
void CForwardActionDialog::GetDetails(CActionItem::CActionForward& details)
{
	details.Addrs().mTo = mTo;

	details.Addrs().mCC = mCC;

	details.Addrs().mBcc = mBcc;

	details.SetQuote(mQuote);

	details.SetAttach(mAttach);

	details.SetText(mText);

	details.SetTiedIdentity(mUseStandard == 0);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft);
}

bool CForwardActionDialog::PoseDialog(CActionItem::CActionForward& details)
{
	bool result = false;

	// Create the dialog
	CForwardActionDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if ((dlog.DoModal() == IDOK))
	{
		dlog.GetDetails(details);
		result = true;
	}

	return result;
}
