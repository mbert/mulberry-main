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


// Source for CPropAdbkGeneral class

#include "CPropAdbkGeneral.h"

#include "CAdbkProtocol.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LIconControl.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropAdbkGeneral::CPropAdbkGeneral()
{
}

// Constructor from stream
CPropAdbkGeneral::CPropAdbkGeneral(LStream *inStream)
		: CAdbkPropPanel(inStream)
{
}

// Default destructor
CPropAdbkGeneral::~CPropAdbkGeneral()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropAdbkGeneral::FinishCreateSelf(void)
{
	// Do inherited
	CAdbkPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_AdbkGeneralState);

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_AdbkGeneralName);
	mServer = (CTextFieldX*) FindPaneByID(paneid_AdbkGeneralServer);
	mTotal = (CTextFieldX*) FindPaneByID(paneid_AdbkGeneralTotal);
	mSingle = (CTextFieldX*) FindPaneByID(paneid_AdbkGeneralSingle);
	mGroup = (CTextFieldX*) FindPaneByID(paneid_AdbkGeneralGroup);
	mOpen = (LCheckBox*) FindPaneByID(paneid_AdbkGeneralOpenStart);
	mNickName = (LCheckBox*) FindPaneByID(paneid_AdbkGeneralNickName);
	mSearch = (LCheckBox*) FindPaneByID(paneid_AdbkGeneralSearch);
	mAdd = (LCheckBox*) FindPaneByID(paneid_AdbkGeneralAdd);
	mAdd->Hide();
	mAutoSync = (LCheckBox*) FindPaneByID(paneid_AdbkGeneralAutoSync);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropAdbkGeneralBtns);
}

// Handle buttons
void CPropAdbkGeneral::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_AdbkGeneralOpenStart:
		OnCheckOpenAtStart(*((long*) ioParam) != 0);
		break;
	case msg_AdbkGeneralNickName:
		OnCheckNickName(*((long*) ioParam) != 0);
		break;
	case msg_AdbkGeneralSearch:
		OnCheckSearch(*((long*) ioParam) != 0);
		break;
	case msg_AdbkGeneralAutoSync:
		OnCheckAutoSync(*((long*) ioParam) != 0);
		break;
	}
}

// Set adbk list
void CPropAdbkGeneral::SetAdbkList(CAddressBookList* adbk_list)
{
	// Save list
	mAdbkList = adbk_list;

	// For now just use first item
	SetAddressBook(mAdbkList->front());
}


// Set mbox list
void CPropAdbkGeneral::SetAddressBook(CAddressBook* adbk)
{
	// Do icon state
	if (adbk && adbk->GetProtocol()->CanDisconnect())
		mIconState->SetResourceID(adbk->GetProtocol()->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
#ifdef _TODO
	else if (adbk->IsLocalAdbk())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
		mAutoSync->Disable();
	}
#endif
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
		mAutoSync->Disable();
	}

	// Copy text to edit fields
	mName->SetText(adbk->GetName());

	mServer->SetText(adbk->GetProtocol()->GetDescriptor());

	if (adbk->IsOpen())
	{
		mTotal->SetNumber(adbk->GetAddressList()->size() + adbk->GetGroupList()->size());
		mSingle->SetNumber(adbk->GetAddressList()->size());
		mGroup->SetNumber(adbk->GetGroupList()->size());
	}

	// Don't act when these are set
	StopListening();;
	mOpen->SetValue(adbk->IsOpenOnStart());
	mNickName->SetValue(adbk->IsLookup());
	mSearch->SetValue(adbk->IsSearch());
	mAdd->SetValue(adbk->IsAdd());
	mAutoSync->SetValue(adbk->IsAutoSync());
	StartListening();
}

// Force update of items
void CPropAdbkGeneral::ApplyChanges(void)
{
	// Nothing to update
}

void CPropAdbkGeneral::OnCheckOpenAtStart(bool set)
{
	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eOpenOnStart, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);

		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookOpenOnStart(*iter, set);
	}
}

void CPropAdbkGeneral::OnCheckNickName(bool set)
{
	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eLookup, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);

		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookLookup(*iter, set);
	}
}

void CPropAdbkGeneral::OnCheckSearch(bool set)
{
	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eSearch, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);

		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookSearch(*iter, set);
	}
}

void CPropAdbkGeneral::OnCheckAutoSync(bool set)
{
	// Iterate over all adbks
	for(CAddressBookList::iterator iter = mAdbkList->begin(); iter != mAdbkList->end(); iter++)
	{
		(*iter)->SetFlags(CAddressBook::eAutoSync, set);
		CAddressBookManager::sAddressBookManager->SyncAddressBook(*iter, set);

		// Change prefs list
		CPreferences::sPrefs->ChangeAddressBookAutoSync(*iter, set);
	}
}
