/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CEditIdentities.cpp : implementation file
//

#include "CEditIdentities.h"

#include "CAdminLock.h"
#include "CEditIdentityAddress.h"
#include "CEditIdentityOptions.h"
#include "CEditIdentityOutgoing.h"
#include "CEditIdentitySecurity.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities dialog


CEditIdentities::CEditIdentities(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CEditIdentities::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditIdentities)
	//}}AFX_DATA_INIT
}


void CEditIdentities::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentities)
	DDX_UTF8Text(pDX, IDC_IDENTITY_NAME, mName);
	DDX_Check(pDX, IDC_SMTPACCOUNTSINHERIT, mAccountInherit);
	//}}AFX_DATA_MAP
	
	// Update items now
	if (pDX->m_bSaveAndValidate)
	{
		mAccountName = CUnicodeUtils::GetWindowTextUTF8(&mAccountPopup);
		mInheritName = CUnicodeUtils::GetWindowTextUTF8(&mInheritPopup);
		mTabs.DoDataExchange(pDX);
	}
}


BEGIN_MESSAGE_MAP(CEditIdentities, CHelpDialog)
	//{{AFX_MSG_MAP(CEditIdentities)
	ON_COMMAND_RANGE(IDM_IDENTITY_INHERIT_DEFAULT, IDM_IDENTITY_INHERIT_End, OnInherit)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnSMTPAccount)
	ON_BN_CLICKED(IDC_SMTPACCOUNTSINHERIT, OnSMTPAccountInherit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_IDENTITY_TABS, OnSelChangeIdentityTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities message handlers

BOOL CEditIdentities::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mInheritPopup.SubclassDlgItem(IDC_IDENTITY_INHERIT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mInheritPopup.SetMenu(IDR_POPUP_IDENTITY_INHERIT);
	InitInheritPopup(mCopyPrefs, &mCopyIdentity);

	mAccountPopup.SubclassDlgItem(IDC_SMTPACCOUNTS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_IDENTITY_SMTP);
	InitServerPopup(mCopyPrefs);
	if (mAccountInherit)
		mAccountPopup.EnableWindow(false);

	// Disable locked out items
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mAccountPopup.EnableWindow(false);
		GetDlgItem(IDC_SMTPACCOUNTSINHERIT)->EnableWindow(false);
	}

	mTabs.SubclassDlgItem(IDC_IDENTITY_TABS, this);

	// Create tab panels
	int index = 0;
	mFromEnabled = !CAdminLock::sAdminLock.mLockIdentityFrom;
	if (mFromEnabled)
	{
		CEditIdentityAddress* panel_from = new CEditIdentityAddress;
		panel_from->SetAddressType(true, false, false, false, false, false, false);
		mTabs.AddPanel(panel_from);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_FROM);
		CUnicodeUtils::SetWindowTextUTF8(panel_from, title);
		mTabs.SetPanelTitle(index++, title);
	}

	mReplyToEnabled = !CAdminLock::sAdminLock.mLockIdentityReplyTo;
	if (mReplyToEnabled)
	{
		CEditIdentityAddress* panel_reply_to = new CEditIdentityAddress;
		panel_reply_to->SetAddressType(false, true, false, false, false, false, false);
		mTabs.AddPanel(panel_reply_to);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_REPLYTO);
		CUnicodeUtils::SetWindowTextUTF8(panel_reply_to, title);
		mTabs.SetPanelTitle(index++, title);
	}

	mSenderEnabled = !CAdminLock::sAdminLock.mLockIdentitySender;
	if (mSenderEnabled)
	{
		CEditIdentityAddress* panel_sender = new CEditIdentityAddress;
		panel_sender->SetAddressType(false, false, true, false, false, false, false);
		mTabs.AddPanel(panel_sender);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_SENDER);
		CUnicodeUtils::SetWindowTextUTF8(panel_sender, title);
		mTabs.SetPanelTitle(index++, title);
	}

	CEditIdentityOptions* panel_options = new CEditIdentityOptions;
	mTabs.AddPanel(panel_options);
	index++;

	CEditIdentityOutgoing* panel_outgoing = new CEditIdentityOutgoing;
	mTabs.AddPanel(panel_outgoing);
	index++;

	if (CPluginManager::sPluginManager.HasSecurity())
	{
		CEditIdentitySecurity* panel_security = new CEditIdentitySecurity;
		mTabs.AddPanel(panel_security);
		index++;
	}

	mCalendarEnabled = !CAdminLock::sAdminLock.mLockIdentityFrom;
	if (mCalendarEnabled)
	{
		CEditIdentityAddress* panel_calendar = new CEditIdentityAddress;
		panel_calendar->SetAddressType(false, false, false, false, false, false, true);
		mTabs.AddPanel(panel_calendar);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_CALENDAR);
		CUnicodeUtils::SetWindowTextUTF8(panel_calendar, title);
		mTabs.SetPanelTitle(index++, title);
	}

	// Give data to tab panels
	mTabs.SetContent(&mCopyIdentity);
	mTabs.SetPanel(0);
	return true;
}

void CEditIdentities::OnInherit(UINT nID) 
{
	mInheritPopup.SetValue(nID);
}

void CEditIdentities::OnSMTPAccount(UINT nID) 
{
	mAccountPopup.SetValue(nID);
}

void CEditIdentities::OnSMTPAccountInherit() 
{
	mAccountPopup.EnableWindow(!static_cast<CButton*>(GetDlgItem(IDC_SMTPACCOUNTSINHERIT))->GetCheck());
}

void CEditIdentities::OnSelChangeIdentityTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}

// Set initial identity fields
void CEditIdentities::SetIdentity(const CPreferences* prefs, const CIdentity* identity)
{
	mCopyIdentity = *identity;
	mCopyPrefs = prefs;

	// Make sure SMTP account is valid irrespective of server locked status
	if (mCopyIdentity.GetSMTPAccount(false).empty() && prefs->mSMTPAccounts.GetValue().size())
		mCopyIdentity.SetSMTPAccount(prefs->mSMTPAccounts.GetValue().front()->GetName(), true);

	// Copy text to edit fields
	mName = identity->GetIdentity();
	mAccountInherit = !identity->UseSMTPAccount();
	
	// Other items will get inited in OnInitDialog
}

// Get final identity fields
void CEditIdentities::GetIdentity(CIdentity* identity)
{
	identity->SetIdentity(mName);

	if (CAdminLock::sAdminLock.mLockServerAddress)
		identity->SetSMTPAccount(mCopyIdentity.GetSMTPAccount(false), true);
	else
		identity->SetSMTPAccount(mAccountName, !mAccountInherit);

	identity->SetInherit(mInheritName);

	// Update existing panel
	mTabs.UpdateContent(identity);

	if (CAdminLock::sAdminLock.mLockIdentityFrom)
		identity->SetFrom(mCopyIdentity.GetFrom(), mCopyIdentity.UseFrom());

	if (CAdminLock::sAdminLock.mLockIdentityReplyTo)
		identity->SetReplyTo(mCopyIdentity.GetReplyTo(), mCopyIdentity.UseReplyTo());

	if (CAdminLock::sAdminLock.mLockIdentitySender)
		identity->SetSender(mCopyIdentity.GetSender(), mCopyIdentity.UseSender());

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		identity->SetHeader(mCopyIdentity.GetHeader(), mCopyIdentity.UseHeader());
}

void CEditIdentities::InitServerPopup(const CPreferences* prefs)
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	int found_match = 0;
	for(CSMTPAccountList::const_iterator iter = prefs->mSMTPAccounts.GetValue().begin();
			iter != prefs->mSMTPAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		
		// Check for match
		if ((*iter)->GetName() == mCopyIdentity.GetSMTPAccount(false))
			found_match = menu_id;
	}

	// Set initial value
	if (!found_match && prefs->mSMTPAccounts.GetValue().size())
		found_match = IDM_AccountStart;
	if (found_match)
		mAccountPopup.SetValue(found_match);
}

void CEditIdentities::InitInheritPopup(const CPreferences* prefs, const CIdentity* identity)
{
	CMenu* pPopup = mInheritPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 2; i < num_menu; i++)
		pPopup->RemoveMenu(2, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_IDENTITY_INHERIT_Start;
	int found_match = 0;

	// Add each identity except this one account
	for(CIdentityList::const_iterator iter = prefs->mIdentities.GetValue().begin();
			iter != prefs->mIdentities.GetValue().end(); iter++, menu_id++)
	{
		// Not if this one!
		if ((*iter).GetIdentity() == identity->GetIdentity())
		{
			menu_id--;
			continue;
		}

		// Insert item
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).GetIdentity());

		// Check for match
		if ((*iter).GetIdentity() == mCopyIdentity.GetInherit())
			found_match = menu_id;
	}

	// Set initial value
	if (!found_match && prefs->mIdentities.GetValue().size())
		found_match = IDM_IDENTITY_INHERIT_DEFAULT;
	if (found_match)
		mInheritPopup.SetValue(found_match);
}
