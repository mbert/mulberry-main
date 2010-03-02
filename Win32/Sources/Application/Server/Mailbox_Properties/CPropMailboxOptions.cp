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


// CPropMailboxOptions.cp : implementation file
//

#include "CPropMailboxOptions.h"

#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxOptions property page

IMPLEMENT_DYNCREATE(CPropMailboxOptions, CHelpPropertyPage)

CPropMailboxOptions::CPropMailboxOptions() : CHelpPropertyPage(CPropMailboxOptions::IDD)
{
	//{{AFX_DATA_INIT(CPropMailboxOptions)
	mIconState = 0;
	mCheck = 0;
	mOpen = 0;
	mCopyTo = 0;
	mAppendTo = 0;
	mPunt = 0;
	mAutoSync = 0;
	mTieIdentity = 0;
	//}}AFX_DATA_INIT
	
	mMboxList = NULL;
	mDisableAutoSync = false;
	mAllDir = false;
}

CPropMailboxOptions::~CPropMailboxOptions()
{
}

void CPropMailboxOptions::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropMailboxOptions)
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_CHECKNEW, mCheck);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_CHECKNEW, mCheckCtrl);
	//DDX_Control(pDX, IDC_MAILBOXOPTIONS_ALERTSTYLE_POPUP, mAlertStylePopup);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_OPEN, mOpen);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_OPEN, mOpenCtrl);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_COPYTO, mCopyTo);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_COPYTO, mCopyToCtrl);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_APPENDTO, mAppendTo);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_APPENDTO, mAppendToCtrl);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_PUNT, mPunt);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_PUNT, mPuntCtrl);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_AUTOSYNC, mAutoSync);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_AUTOSYNC, mAutoSyncCtrl);
	DDX_Check(pDX, IDC_MAILBOXOPTIONS_TIEIDENTITY, mTieIdentity);
	DDX_Control(pDX, IDC_MAILBOXOPTIONS_TIEIDENTITY, mTieIdentityCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropMailboxOptions, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropMailboxOptions)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_CHECKNEW, OnCheckNew)
	ON_COMMAND_RANGE(IDM_ALERTSTYLE_Start, IDM_ALERTSTYLE_End, OnAlertStylePopup)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_COPYTO, OnCopyTo)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_APPENDTO, OnAppendTo)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_PUNT, OnPunt)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_AUTOSYNC, OnAutoSync)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_TIEIDENTITY, OnTieIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	ON_BN_CLICKED(IDC_MAILBOXOPTIONS_REBUILD, OnRebuildLocal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxOptions message handlers

BOOL CPropMailboxOptions::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_MAILBOXOPTIONS_STATE, this, mIconState);

	// Subclass buttons
	mAlertStylePopup.SubclassDlgItem(IDC_MAILBOXOPTIONS_ALERTSTYLE_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAlertStylePopup.SetMenu(IDR_POPUP_ALERTCHOICE);
	InitAlertStylePopup();
	mAlertStylePopup.SetValue(mAlertStyle + IDM_ALERTSTYLE_Start);
	mAlertStylePopup.EnableWindow(mCheck);

	mIdentityPopup.SubclassDlgItem(IDC_MAILBOXOPTIONS_IDENTITYPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	//  the identity popup
	// Set identity items
	if (mTieIdentity == 2)
		mIdentityPopup.SetValue(IDM_IDENTITYStart);
	else if (!mCurrentIdentity.empty())
	{
		// Identity to use
		const CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);
		mIdentityPopup.SetIdentity(CPreferences::sPrefs, id);
	}
	else
		mIdentityPopup.SetValue(IDM_IDENTITYStart);
	mIdentityPopup.EnableWindow(mTieIdentity == 1);

	if (mDisableAutoSync)
		mAutoSyncCtrl.EnableWindow(false);

	if (mAllDir)
	{
		GetDlgItem(IDC_MAILBOXOPTIONS_CHECKNEW)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_ALERTSTYLE_POPUP)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_OPEN)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_COPYTO)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_APPENDTO)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_PUNT)->EnableWindow(false);
		GetDlgItem(IDC_MAILBOXOPTIONS_AUTOSYNC)->EnableWindow(false);
	}

	if (!mHasLocal)
		GetDlgItem(IDC_MAILBOXOPTIONS_REBUILD)->ShowWindow(SW_HIDE);
	
	return true;
}

// Initialise alert styles
void CPropMailboxOptions::InitAlertStylePopup()
{
	CMenu* pPopup = mAlertStylePopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each mail account
	int menu_id = IDM_ALERTSTYLE_Start;
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter).GetName());
}

// Set mbox list
void CPropMailboxOptions::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// Do icon state
	if (static_cast<CMbox*>(mbox_list->front())->GetProtocol()->CanDisconnect())
		mIconState = static_cast<CMbox*>(mbox_list->front())->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
	else if (static_cast<CMbox*>(mbox_list->front())->IsLocalMbox())
	{
		mIconState = IDI_MAILBOX_LOCAL;
		mDisableAutoSync = true;
	}
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
		mDisableAutoSync = true;
	}

	int auto_check = 0;
	int first_type = -1;
	int open_login = 0;
	int copy_to = 0;
	int append_to = 0;
	int punt = 0;
	int sync = 0;
	int multiple_tied = 0;
	bool first = true;
	mAllDir = true;
	mHasLocal = false;
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		mAllDir &= mbox->IsDirectory();
		mHasLocal |= mbox->IsLocalMbox();

		// Only valid id not a directory
		if (!mbox->IsDirectory())
		{
			auto_check += (mbox->IsAutoCheck() ? 1 : 0);
			open_login+= (CMailAccountManager::sMailAccountManager->IsFavouriteItem(CMailAccountManager::eFavouriteOpenLogin, mbox) ? 1 : 0);
			copy_to += (mbox->IsCopyTo() ? 1 : 0);
			append_to += (mbox->IsAppendTo() ? 1 : 0);
			punt += (mbox->IsPuntOnClose() ? 1 : 0);
			sync += (mbox->IsAutoSync() ? 1 : 0);
			
			// Special for auto check
			CMailAccountManager::EFavourite type;
			if (CMailAccountManager::sMailAccountManager->GetFavouriteAutoCheck(mbox, type) && (first_type == -1))
				first_type = type;
		}

		CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedMailboxes.GetValue().GetTiedMboxIdentity(mbox));
		if (first)
		{
			mCurrentIdentity = (id ? id->GetIdentity() : cdstring::null_str);
			first = false;
		}
		else if (!multiple_tied)
		{
			if (mCurrentIdentity != (id ? id->GetIdentity() : cdstring::null_str))
			{
				// Mark as multiple
				multiple_tied = true;
				
				// Remember the first 'real' identity found
				if (mCurrentIdentity.empty())
					mCurrentIdentity = (id ? id->GetIdentity() : cdstring::null_str);
			}
		}
	}
	
	// Now adjust totals
	if (auto_check)
		auto_check = ((auto_check == mMboxList->size()) ? 1 : 2);
	if (open_login)
		open_login = ((open_login == mMboxList->size()) ? 1 : 2);
	if (copy_to)
		copy_to = ((copy_to == mMboxList->size()) ? 1 : 2);
	if (append_to)
		append_to = ((append_to == mMboxList->size()) ? 1 : 2);
	if (punt)
		punt = ((punt == mMboxList->size()) ? 1 : 2);
	if (sync)
		sync = ((sync == mMboxList->size()) ? 1 : 2);

	if (!mAllDir)
	{
		// Set items
		mCheck = auto_check;
			
		// Need to map from favourite index to mail notifier index
		mAlertStyle = 0;
		if ((first_type >=  0) &&
			CMailAccountManager::sMailAccountManager->GetNotifierIndex(static_cast<CMailAccountManager::EFavourite>(first_type), mAlertStyle))
			{}

		mOpen = open_login;
		mCopyTo = copy_to;
		mAppendTo = append_to;
		mPunt = punt;
		mAutoSync = sync;
	}

	// Set identity items
	if (multiple_tied)
	{
		mTieIdentity = 2;
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	}
	else if (!mCurrentIdentity.empty())
	{
		mTieIdentity = 1;
	}
	else
	{
		mTieIdentity = 0;
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	}

}


// Handle check new
void CPropMailboxOptions::OnCheckNew(void)
{
	UINT result = ::CheckboxToggle(&mCheckCtrl);

	// Get new type from popup setting
	CMailAccountManager::EFavourite new_type = GetStyleType(mAlertStylePopup.GetValue() - IDM_ALERTSTYLE_Start);

	// Iterate over all mailboxes
	for(CMboxList::iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		
		if (!mbox->IsDirectory())
		{
			// Find existing auto check type and remove it
			CMailAccountManager::EFavourite old_type;
			if (CMailAccountManager::sMailAccountManager->GetFavouriteAutoCheck(mbox, old_type))
				CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(old_type, mbox);
			
			// Add in new auto check type if auto check turned on
			if (result)
				CMailAccountManager::sMailAccountManager->AddFavouriteItem(new_type, mbox);
		}
	}

	mAlertStylePopup.EnableWindow(result);
}

// Handle alert style change
void CPropMailboxOptions::OnAlertStylePopup(UINT nID)
{
	// Get new type from popup setting
	CMailAccountManager::EFavourite new_type = GetStyleType(nID - IDM_ALERTSTYLE_Start);

	// Iterate over all mailboxes
	for(CMboxList::iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);

		if (!mbox->IsDirectory())
		{
			// Find existing auto check type and remove it
			CMailAccountManager::EFavourite old_type;
			if (CMailAccountManager::sMailAccountManager->GetFavouriteAutoCheck(mbox, old_type))
				CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(old_type, mbox);
			
			// Add in new auto check type
			CMailAccountManager::sMailAccountManager->AddFavouriteItem(new_type, mbox);
		}
	}
	
	mAlertStylePopup.SetValue(nID);
}

// Handle open after login
void CPropMailboxOptions::OnOpen(void)
{
	SetFavourite(::CheckboxToggle(&mOpenCtrl), CMailAccountManager::eFavouriteOpenLogin);
}

// Handle copy to
void CPropMailboxOptions::OnCopyTo(void)
{
	SetFavourite(::CheckboxToggle(&mCopyToCtrl), CMailAccountManager::eFavouriteCopyTo);
}

// Handle append to
void CPropMailboxOptions::OnAppendTo(void)
{
	SetFavourite(::CheckboxToggle(&mAppendToCtrl), CMailAccountManager::eFavouriteAppendTo);
}

// Handle punt
void CPropMailboxOptions::OnPunt(void)
{
	SetFavourite(::CheckboxToggle(&mPuntCtrl), CMailAccountManager::eFavouritePuntOnClose);
}

// Handle auto sync
void CPropMailboxOptions::OnAutoSync(void)
{
	SetFavourite(::CheckboxToggle(&mAutoSyncCtrl), CMailAccountManager::eFavouriteAutoSync);
}

// Handle tie change
void CPropMailboxOptions::OnTieIdentity(void)
{
	::CheckboxToggle(&mTieIdentityCtrl);
	mIdentityPopup.EnableWindow(mTieIdentityCtrl.GetCheck());
	SetTiedIdentity(mTieIdentityCtrl.GetCheck());
}

void CPropMailboxOptions::OnIdentityPopup(UINT nID)
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
		SetTiedIdentity(false);
		mIdentityPopup.SetValue(nID);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.Value()[nID - IDM_IDENTITYStart].GetIdentity();
		SetTiedIdentity(true);
		break;
	}
}

void CPropMailboxOptions::SetFavourite(bool set, CMailAccountManager::EFavourite fav_type)
{
	// Iterate over all mailboxes
	for(CMboxList::iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		if (!mbox->IsDirectory())
		{
			if (set)
				CMailAccountManager::sMailAccountManager->AddFavouriteItem(fav_type, mbox);
			else
				CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(fav_type, mbox);
		}
	}
}

// Add/remove tied identity
void CPropMailboxOptions::SetTiedIdentity(bool add)
{
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		if (add)
		{
			// Identity to use
			CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);

			id->AddTiedMailbox(mbox);
		}
		else
		{
			// With multiple mailboxes might have multiple ids so resolve each one
			CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedMailboxes.GetValue().GetTiedMboxIdentity(mbox));
			if (id)
				id->RemoveTiedMailbox(mbox);
		}
	}
	
	// Mark it as dirty
	CPreferences::sPrefs->mTiedMailboxes.SetDirty();
}

CMailAccountManager::EFavourite CPropMailboxOptions::GetStyleType(unsigned long index) const
{
	const CMailNotification& notify = CPreferences::sPrefs->mMailNotification.GetValue().at(index);
	return CMailAccountManager::sMailAccountManager->GetFavouriteFromID(notify.GetFavouriteID());
}

// Rebuild local mailboxes
void CPropMailboxOptions::OnRebuildLocal()
{
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		// Only if account is local
		CMbox* mbox = static_cast<CMbox*>(*iter);
		mbox->Rebuild();
	}
}
