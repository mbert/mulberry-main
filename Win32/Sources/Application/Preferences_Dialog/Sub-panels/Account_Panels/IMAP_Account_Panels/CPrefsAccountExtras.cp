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


// CPrefsAccountExtras.cpp : implementation file
//

#include "CPrefsAccountExtras.h"

#include "CCalendarAccount.h"
#include "CMailAccount.h"
#include "CPreferencesDialog.h"
#include "CPrefsAuthPlainText.h"
#include "CPrefsAuthKerberos.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountExtras dialog


CPrefsAccountExtras::CPrefsAccountExtras()
	: CTabPanel(CPrefsAccountExtras::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAccountExtras)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrefsAccountExtras::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAccountExtras)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_EXTRAS_TIEIDENTITY, mTieIdentity);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_EXTRAS_TIEIDENTITY, mTieIdentityCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAccountExtras, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAccountExtras)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_EXTRAS_TIEIDENTITY, OnTieIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountExtras message handlers

BOOL CPrefsAccountExtras::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mIdentityPopup.SubclassDlgItem(IDC_PREFS_ACCOUNT_EXTRAS_IDENTITYPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	CPreferences* new_prefs = dlog->GetCopyPrefs();
	mIdentityPopup.Reset(new_prefs->mIdentities.GetValue());

	return true;
}

// Handle tie change
void CPrefsAccountExtras::OnTieIdentity(void)
{
	mIdentityPopup.EnableWindow(mTieIdentityCtrl.GetCheck());
}

void CPrefsAccountExtras::OnIdentityPopup(UINT nID)
{
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	CPreferences* new_prefs = dlog->GetCopyPrefs();

	switch(nID)
	{
	// New identity wanted
	case IDM_IDENTITY_NEW:
		mIdentityPopup.DoNewIdentity(new_prefs);
		break;
	
	// New identity wanted
	case IDM_IDENTITY_EDIT:
		mIdentityPopup.DoEditIdentity(new_prefs);
		break;
	
	// Delete existing identity
	case IDM_IDENTITY_DELETE:
		mIdentityPopup.DoDeleteIdentity(new_prefs);
		break;
	
	// Select an identity
	default:
		mIdentityPopup.SetValue(nID);
		break;
	}
}

// Set data
void CPrefsAccountExtras::SetContent(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	bool tie = false;
	cdstring id;
	if (maccount != NULL)
	{
		tie = maccount->GetTieIdentity();
		id = maccount->GetTiedIdentity();
	}
	else if (caccount != NULL)
	{
		tie = caccount->GetTieIdentity();
		id = caccount->GetTiedIdentity();
	}
	
	// Get new prefs
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	CPreferences* new_prefs = dlog->GetCopyPrefs();
	mIdentityPopup.Reset(new_prefs->mIdentities.GetValue());

	// Set first identity
	mTieIdentity = tie;

	// Set it in popup
	mIdentityPopup.SetIdentity(new_prefs, id);
	
	// Disable if not in use
	mIdentityPopup.EnableWindow(tie);
}

// Force update of data
bool CPrefsAccountExtras::UpdateContent(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	// Get new prefs
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	CPreferences* new_prefs = dlog->GetCopyPrefs();

	if (maccount != NULL)
	{
		maccount->SetTieIdentity(mTieIdentity);
		maccount->SetTiedIdentity(mIdentityPopup.GetIdentity(new_prefs).GetIdentity());
	}
	else if (caccount != NULL)
	{
		caccount->SetTieIdentity(mTieIdentity);
		caccount->SetTiedIdentity(mIdentityPopup.GetIdentity(new_prefs).GetIdentity());
	}
	
	return true;
}