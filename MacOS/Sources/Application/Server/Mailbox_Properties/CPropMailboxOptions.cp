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


// Source for CPropMailboxOptions class

#include "CPropMailboxOptions.h"

#include "CIdentityPopup.h"
#include "CMailAccountManager.h"
#include "CMailboxPropDialog.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"

#include <LCheckBox.h>
#include <LIconControl.h>
#include <LPopupButton.h>
#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropMailboxOptions::CPropMailboxOptions()
{
}

// Constructor from stream
CPropMailboxOptions::CPropMailboxOptions(LStream *inStream)
		: CMailboxPropPanel(inStream)
{
}

// Default destructor
CPropMailboxOptions::~CPropMailboxOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxOptions::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_MailboxOptionsState);

	// Get items
	mCheck = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsCheck);
	mAlertStylePopup = (LPopupButton*) FindPaneByID(paneid_MailboxOptionsAlertStylePopup);
	InitAlertStylePopup();
	mOpen = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsOpen);
	mCopyTo = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsCopyTo);
	mAppendTo = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsAppendTo);
	mPuntOnClose = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsPuntOnClose);
	mAutoSync = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsAutoSync);
	mTieIdentity = (LCheckBox*) FindPaneByID(paneid_MailboxOptionsTieIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_MailboxOptionsIdentityPopup);
	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());
	mRebuildLocal = (LPushButton*) FindPaneByID(paneid_MailboxOptionsRebuildLocal);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropMailboxOptionsBtns);
}

// Handle buttons
void CPropMailboxOptions::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_AutoCheck:
		{
			// Get new type from popup setting
			CMailAccountManager::EFavourite new_type = GetStyleType(mAlertStylePopup->GetValue() - 1);

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
					if (*(long*) ioParam)
						CMailAccountManager::sMailAccountManager->AddFavouriteItem(new_type, mbox);
				}
			}

			if (*(long*) ioParam)
				mAlertStylePopup->Enable();
			else
				mAlertStylePopup->Disable();
		}
		break;

	case msg_AlertStyle:
		{
			// Get new type from popup setting
			CMailAccountManager::EFavourite new_type = GetStyleType(*(long*) ioParam - 1);

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
		}
		break;
	case msg_OpenAtStart:
		SetFavourite(*((long*) ioParam), CMailAccountManager::eFavouriteOpenLogin);
		break;
	case msg_MessageCopyTo:
		SetFavourite(*((long*) ioParam), CMailAccountManager::eFavouriteCopyTo);
		break;
	case msg_DraftCopyTo:
		SetFavourite(*((long*) ioParam), CMailAccountManager::eFavouriteAppendTo);
		break;
	case msg_PuntOnClose:
		SetFavourite(*((long*) ioParam), CMailAccountManager::eFavouritePuntOnClose);
		break;
	case msg_AutoSync:
		SetFavourite(*((long*) ioParam), CMailAccountManager::eFavouriteAutoSync);
		break;

	case msg_MOTieIdentity:
		if (*(long*) ioParam)
		{
			mIdentityPopup->Enable();
			SetTiedIdentity(true);
		}
		else
		{
			mIdentityPopup->Disable();
			SetTiedIdentity(false);
		}
		break;

	case msg_MOIdentityPopup:
		switch(*(long*) ioParam)
		{
		// New identity wanted
		case eIdentityPopup_New:
			mIdentityPopup->DoNewIdentity(CPreferences::sPrefs);
			break;

		// New identity wanted
		case eIdentityPopup_Edit:
			mIdentityPopup->DoEditIdentity(CPreferences::sPrefs);
			break;

		// Delete existing identity
		case eIdentityPopup_Delete:
			mIdentityPopup->DoDeleteIdentity(CPreferences::sPrefs);
			break;

		// Select an identity
		default:
			SetTiedIdentity(false);
			mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue()[*(long*) ioParam - mIdentityPopup->FirstIndex()].GetIdentity();
			SetTiedIdentity(true);
			break;
		}
		break;
	
	case msg_MORebuildLocal:
		OnRebuildLocal();
		break;
	}
}

// Initialise alert styles
void CPropMailboxOptions::InitAlertStylePopup()
{
	// Delete previous items
	MenuHandle menuH = mAlertStylePopup->GetMacMenuH();
	for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		::DeleteMenuItem(menuH, 1);

	// Add each mail account
	short menu_pos = 1;
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter).GetName());

	// Force max/min update
	mAlertStylePopup->SetMenuMinMax();
}

// Set mbox list
void CPropMailboxOptions::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// Do icon state
	if (static_cast<CMbox*>(mbox_list->front())->GetProtocol()->CanDisconnect())
		mIconState->SetResourceID(static_cast<CMbox*>(mbox_list->front())->GetProtocol()->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
	else if (static_cast<CMbox*>(mbox_list->front())->IsLocalMbox())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
		mAutoSync->Disable();
	}
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
		mAutoSync->Disable();
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
	bool all_directory = true;
	bool has_local = false;
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		all_directory &= mbox->IsDirectory();
		has_local |= mbox->IsLocalMbox();

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

	StopListening();

	if (!all_directory)
	{
		mCheck->SetValue(auto_check);
		if (mCheck->GetValue() == 1)
			mAlertStylePopup->Enable();
		else
			mAlertStylePopup->Disable();
			
		// Need to map from favourite index to mail notifier index
		unsigned long notifier = 0;
		if ((first_type >=  0) &&
			CMailAccountManager::sMailAccountManager->GetNotifierIndex(static_cast<CMailAccountManager::EFavourite>(first_type), notifier))
			{}
		mAlertStylePopup->SetValue(notifier + 1);
		mOpen->SetValue(open_login);
		mCopyTo->SetValue(copy_to);
		mAppendTo->SetValue(append_to);
		mPuntOnClose->SetValue(punt);
		mAutoSync->SetValue(sync);
	}
	else
	{
		mCheck->Disable();
		mAlertStylePopup->Disable();
		mOpen->Disable();
		mCopyTo->Disable();
		mAppendTo->Disable();
		mPuntOnClose->Disable();
		mAutoSync->Disable();
	}

	if (multiple_tied)
	{
		mTieIdentity->SetValue(2);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
		mIdentityPopup->SetValue(mIdentityPopup->FirstIndex());
		mIdentityPopup->Disable();
	}
	else if (!mCurrentIdentity.empty())
	{
		// Identity to use
		const CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);
		mTieIdentity->SetValue(1);
		mIdentityPopup->SetIdentity(CPreferences::sPrefs, id);
	}
	else
	{
		mTieIdentity->SetValue(0);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
		mIdentityPopup->SetValue(mIdentityPopup->FirstIndex());
		mIdentityPopup->Disable();
	}

	// Only have rebuild if local exist
	if (!has_local)
		mRebuildLocal->Hide();

	StartListening();
}

// Set protocol
void CPropMailboxOptions::SetProtocol(CMboxProtocol* protocol)
{
	// Does nothing
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

// Force update of items
void CPropMailboxOptions::ApplyChanges(void)
{
	// Nothing to update
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

