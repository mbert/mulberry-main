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


// CReplyActionDialog.cp : implementation file
//

#include "CReplyActionDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CReplyActionDialog dialog


CReplyActionDialog::CReplyActionDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CReplyActionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReplyActionDialog)
	mReplyTo = -1;
	mQuote = FALSE;
	mUseStandard = -1;
	mCreateDraft = FALSE;
	//}}AFX_DATA_INIT
}


void CReplyActionDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplyActionDialog)
	DDX_Radio(pDX, IDC_REPLYACTION_REPLYTO, mReplyTo);
	DDX_Check(pDX, IDC_REPLYACTION_QUOTE, mQuote);
	DDX_Radio(pDX, IDC_REPLYACTION_STANDARDIDENTITY, mUseStandard);
	DDX_Control(pDX, IDC_REPLYACTION_IDENTITY, mIdentityPopup);
	DDX_Check(pDX, IDC_REPLYACTION_DRAFT, mCreateDraft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReplyActionDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CReplyActionDialog)
	ON_BN_CLICKED(IDC_REPLYACTION_TEXT, OnText)
	ON_BN_CLICKED(IDC_REPLYACTION_STANDARDIDENTITY, OnStandardIdentity)
	ON_BN_CLICKED(IDC_REPLYACTION_USEIDENTITY, OnUseIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplyActionDialog message handlers

BOOL CReplyActionDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass items
	mIdentityPopup.SubclassDlgItem(IDC_REPLYACTION_IDENTITY, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	// Set identity items
	mIdentityPopup.SetIdentity(CPreferences::sPrefs, mCurrentIdentity);
	mIdentityPopup.EnableWindow(mUseStandard == 1);

	return true;
}

void CReplyActionDialog::OnText() 
{
	CPrefsEditHeadFoot::PoseDialog(CPreferences::sPrefs, mText, false);
}

void CReplyActionDialog::OnStandardIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_REPLYACTION_STANDARDIDENTITY));
	mIdentityPopup.EnableWindow(!cb->GetCheck());
}

void CReplyActionDialog::OnUseIdentity() 
{
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_REPLYACTION_USEIDENTITY));
	mIdentityPopup.EnableWindow(cb->GetCheck());
}

void CReplyActionDialog::OnIdentityPopup(UINT nID)
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
void CReplyActionDialog::SetDetails(CActionItem::CActionReply& details)
{
	mReplyTo = details.GetReplyType();

	mQuote = details.Quote();

	mText = details.GetText();

	mUseStandard = details.UseTiedIdentity() ? 0 : 1;
	
	mCurrentIdentity = details.GetIdentity();

	mCreateDraft = details.CreateDraft();
}

// Get the details
void CReplyActionDialog::GetDetails(CActionItem::CActionReply& details)
{
	details.SetReplyType(static_cast<NMessage::EReplyType>(mReplyTo));

	details.SetQuote(mQuote);

	details.SetText(mText);

	details.SetTiedIdentity(mUseStandard == 0);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft);
}

bool CReplyActionDialog::PoseDialog(CActionItem::CActionReply& details)
{
	bool result = false;

	// Create the dialog
	CReplyActionDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if ((dlog.DoModal() == IDOK))
	{
		dlog.GetDetails(details);
		result = true;
	}

	return result;
}
