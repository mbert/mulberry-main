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
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRemoteAddressBook.h"
#include "CStaticText.h"

#include "StValueChanger.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropAdbkGeneral::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("General", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 55,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mIconState =
        new JXImageWidget(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 414,4, 32,32);
    assert( mIconState != NULL );
    mIconState->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj2 =
        new JXStaticText("Name:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,22, 45,20);
    assert( obj2 != NULL );

    mName =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,20, 315,20);
    assert( mName != NULL );
    mName->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj3 =
        new JXStaticText("Server:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,47, 60,20);
    assert( obj3 != NULL );

    mServer =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,45, 315,20);
    assert( mServer != NULL );
    mServer->SetBorderWidth(kJXDefaultBorderWidth);

    JXDownRect* obj4 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,85, 430,50);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Addresses:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,75, 70,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Total:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,17, 40,20);
    assert( obj6 != NULL );

    mTotal =
        new CStaticText("", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 45,15, 50,20);
    assert( mTotal != NULL );
    mTotal->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj7 =
        new JXStaticText("Single:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,17, 50,20);
    assert( obj7 != NULL );

    mSingle =
        new CStaticText("", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,15, 50,20);
    assert( mSingle != NULL );
    mSingle->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj8 =
        new JXStaticText("Group:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,17, 50,20);
    assert( obj8 != NULL );

    mGroup =
        new CStaticText("", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 260,15, 50,20);
    assert( mGroup != NULL );
    mGroup->SetBorderWidth(kJXDefaultBorderWidth);

    mOpen =
        new JXTextCheckbox("Open on Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 190,20);
    assert( mOpen != NULL );

    mNickName =
        new JXTextCheckbox("Use for Nick-names", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,170, 190,20);
    assert( mNickName != NULL );

    mSearch =
        new JXTextCheckbox("Use for Searching and Expansion", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,190, 220,20);
    assert( mSearch != NULL );

    mAutoSync =
        new JXTextCheckbox("Synchronise on Disconnect", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,210, 195,20);
    assert( mAutoSync != NULL );

// end JXLayout1

	ListenTo(mOpen);
	ListenTo(mNickName);
	ListenTo(mSearch);
	ListenTo(mAutoSync);
}

// Handle buttons
void CPropAdbkGeneral::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mOpen)
		{
			OnCheckOpenAtStart(mOpen->IsChecked());
			return;
		}
		else if (sender == mNickName)
		{
			OnCheckNickName(mNickName->IsChecked());
			return;
		}
		else if (sender == mSearch)
		{
			OnCheckSearch(mSearch->IsChecked());
			return;
		}
		else if (sender == mAutoSync)
		{
			OnCheckAutoSync(mAutoSync->IsChecked());
			return;
		}
	}

	CAdbkPropPanel::Receive(sender, message);
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
	if (adbk->GetProtocol()->CanDisconnect())
		mIconState->SetImage(CIconLoader::GetIcon(adbk->GetProtocol()->IsDisconnected() ?
								IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
#ifdef _TODO
	else if (adbk->IsLocalAdbk())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC), kFalse);
		mAutoSync->Deactivate();
	}
#endif
	else
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
		mAutoSync->Deactivate();
	}

	cdstring copyStr;

	// Copy text to edit fields
	mName->SetText(adbk->GetName());

	mServer->SetText(adbk->GetProtocol()->GetDescriptor());

	if (adbk->IsOpen())
	{
		copyStr = (long) (adbk->GetAddressList()->size() + adbk->GetGroupList()->size());
		mTotal->SetText(copyStr);

		copyStr = (long) adbk->GetAddressList()->size();
		mSingle->SetText(copyStr);

		copyStr = (long) adbk->GetGroupList()->size();
		mGroup->SetText(copyStr);
	}

	// Don't act when these are set
	SetListening(kFalse);

	mOpen->SetState(JBoolean(adbk->IsOpenOnStart()));
	mNickName->SetState(JBoolean(adbk->IsLookup()));
	mSearch->SetState(JBoolean(adbk->IsSearch()));
	mAutoSync->SetState(JBoolean(adbk->IsAutoSync()));

	SetListening(kTrue);
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
