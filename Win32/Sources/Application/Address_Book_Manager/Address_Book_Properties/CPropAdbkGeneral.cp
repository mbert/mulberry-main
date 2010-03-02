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


// CPropAdbkGeneral.cpp : implementation file
//

#include "CPropAdbkGeneral.h"

#include "CAdbkProtocol.h"
#include "CAddressBookManager.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkGeneral dialog

IMPLEMENT_DYNCREATE(CPropAdbkGeneral, CHelpPropertyPage)

CPropAdbkGeneral::CPropAdbkGeneral() : CHelpPropertyPage(CPropAdbkGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPropAdbkGeneral)
	mIconState = 0;
	mTotal = 0;
	mSingle = 0;
	mGroup = 0;
	mOpenOnStart = FALSE;
	mNickName = FALSE;
	mSearch = FALSE;
	mAutoSync = FALSE;
	//mAdd = FALSE;
	//}}AFX_DATA_INIT
	mDisableAutoSync = false;
}


void CPropAdbkGeneral::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropAdbkGeneral)
	DDX_UTF8Text(pDX, IDC_ADBKGENERAL_NAME, mName);
	DDX_UTF8Text(pDX, IDC_ADBKGENERAL_SERVER, mServer);
	DDX_UTF8Text(pDX, IDC_ADBKGENERAL_TOTAL, mTotal);
	DDX_UTF8Text(pDX, IDC_ADBKGENERAL_SINGLE, mSingle);
	DDX_UTF8Text(pDX, IDC_ADBKGENERAL_GROUP, mGroup);
	DDX_Control(pDX, IDC_ADBKGENERAL_START, mOpenOnStartCtrl);
	DDX_Check(pDX, IDC_ADBKGENERAL_START, mOpenOnStart);
	DDX_Control(pDX, IDC_ADBKGENERAL_NICK, mNickNameCtrl);
	DDX_Check(pDX, IDC_ADBKGENERAL_NICK, mNickName);
	DDX_Control(pDX, IDC_ADBKGENERAL_SEARCH, mSearchCtrl);
	DDX_Check(pDX, IDC_ADBKGENERAL_SEARCH, mSearch);
	DDX_Control(pDX, IDC_ADBKGENERAL_AUTOSYNC, mAutoSyncCtrl);
	DDX_Check(pDX, IDC_ADBKGENERAL_AUTOSYNC, mAutoSync);
	//DDX_Control(pDX, IDC_ADBKGENERAL_ADD, mAddCtrl);
	//DDX_Check(pDX, IDC_ADBKGENERAL_ADD, mAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropAdbkGeneral, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropAdbkGeneral)
	ON_BN_CLICKED(IDC_ADBKGENERAL_START, OnCheckOpenAtStart)
	ON_BN_CLICKED(IDC_ADBKGENERAL_NICK, OnCheckNickName)
	ON_BN_CLICKED(IDC_ADBKGENERAL_SEARCH, OnCheckSearch)
	ON_BN_CLICKED(IDC_ADBKGENERAL_AUTOSYNC, OnCheckAutoSync)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkGeneral message handlers

BOOL CPropAdbkGeneral::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_ADBKGENERAL_STATE, this, mIconState);

	if (mDisableAutoSync)
		mAutoSyncCtrl.EnableWindow(false);
		
	return true;
}

// Set mbox list
void CPropAdbkGeneral::SetAdbkList(CAddressBookList* adbk_list)
{
	// Save list
	mAdbkList = adbk_list;

	// For now just use first item
	SetAdbk(mAdbkList->front());
}


// Set adbk list
void CPropAdbkGeneral::SetAdbk(CAddressBook* adbk)
{
	// Do icon state
	if (adbk->GetProtocol()->CanDisconnect())
		mIconState = adbk->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
#ifdef _TODO
	else if (adbk->IsLocalAdbk())
	{
		mIconState = IDI_MAILBOX_LOCAL;
		mDisableAutoSync = true;
	}
#endif
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
		mDisableAutoSync = true;
	}

	// Copy text to edit fields
	mName = adbk->GetName();

	mServer = adbk->GetProtocol()->GetDescriptor();

	if (adbk->IsOpen())
	{
		mTotal = adbk->GetAddressList()->size() + adbk->GetGroupList()->size();
		mSingle = adbk->GetAddressList()->size();
		mGroup = adbk->GetGroupList()->size();
	}

	mOpenOnStart = adbk->IsOpenOnStart();
	mNickName = adbk->IsLookup();
	mSearch = adbk->IsSearch();
	//mAdd = adbk->IsAdd();
	mAutoSync = adbk->IsAutoSync();
}

// Handle check open at start
void CPropAdbkGeneral::OnCheckOpenAtStart(void)
{
	bool set = mOpenOnStartCtrl.GetCheck();
	
	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eOpenOnStart, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);
		
		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookOpenOnStart(*iter, set);
	}
}

// Handle check nick name
void CPropAdbkGeneral::OnCheckNickName(void)
{
	bool set = mNickNameCtrl.GetCheck();

	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eLookup, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);
		
		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookLookup(*iter, set);
	}
}

// Handle check search
void CPropAdbkGeneral::OnCheckSearch(void)
{
	bool set = mSearchCtrl.GetCheck();

	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eSearch, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);
		
		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookSearch(*iter, set);
	}
}

// Handle check search
void CPropAdbkGeneral::OnCheckAutoSync(void)
{
	bool set = mAutoSyncCtrl.GetCheck();

	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eAutoSync, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);
		
		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookAutoSync(*iter, set);
	}
}
