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

#include "CIconLoader.h"
#include "CIdentityPopup.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CPreferences.h"

#include "JXTextCheckbox3.h"
#include "TPopupMenu.h"
#include "StValueChanger.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXEngravedRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxOptions::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Options", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 55,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mIconState =
        new JXImageWidget(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 414,4, 32,32);
    assert( mIconState != NULL );
    mIconState->SetBorderWidth(kJXDefaultBorderWidth);

    mCheck =
        new JXTextCheckbox3("Check for New Email with Style:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 210,20);
    assert( mCheck != NULL );

    mAlertStylePopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,30, 170,20);
    assert( mAlertStylePopup != NULL );

    mOpen =
        new JXTextCheckbox3("Open after Login", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 170,20);
    assert( mOpen != NULL );

    mCopyTo =
        new JXTextCheckbox3("Use for Copying Messages", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 190,20);
    assert( mCopyTo != NULL );

    mAppendTo =
        new JXTextCheckbox3("Use for Drafts or Sent Mail", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 190,20);
    assert( mAppendTo != NULL );

    mPuntOnClose =
        new JXTextCheckbox3("Mark Seen on Close", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 160,20);
    assert( mPuntOnClose != NULL );

    mAutoSync =
        new JXTextCheckbox3("Synchronise on Disconnect", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 195,20);
    assert( mAutoSync != NULL );

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,165, 420,70);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Identity", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,155, 50,20);
    assert( obj3 != NULL );

    mTieIdentity =
        new JXTextCheckbox3("Tie to Identity", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 170,20);
    assert( mTieIdentity != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Identity:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,37, 55,20);
    assert( obj4 != NULL );

    mIdentityPopup =
        new CIdentityPopup("",obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,35, 200,20);
    assert( mIdentityPopup != NULL );

    mRebuildLocal =
        new JXTextButton("Rebuild Local Mailbox", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,250, 140,25);
    assert( mRebuildLocal != NULL );
    mRebuildLocal->SetFontSize(10);

// end JXLayout1

	// Initialise items
	InitAlertStylePopup();

	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());

	ListenTo(mCheck);
	ListenTo(mAlertStylePopup);
	ListenTo(mOpen);
	ListenTo(mCopyTo);
	ListenTo(mAppendTo);
	ListenTo(mPuntOnClose);
	ListenTo(mAutoSync);
	ListenTo(mTieIdentity);
	ListenTo(mIdentityPopup);
	ListenTo(mRebuildLocal);
}

// Handle buttons
void CPropMailboxOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mCheck)
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
					if (mCheck->IsChecked())
						CMailAccountManager::sMailAccountManager->AddFavouriteItem(new_type, mbox);
				}
			}

			if (mCheck->IsChecked())
				mAlertStylePopup->Activate();
			else
				mAlertStylePopup->Deactivate();
			return;
		}
		else if (sender == mOpen)
		{
			SetFavourite(mOpen->IsChecked(), CMailAccountManager::eFavouriteOpenLogin);
			return;
		}
		else if (sender == mCopyTo)
		{
			SetFavourite(mCopyTo->IsChecked(), CMailAccountManager::eFavouriteCopyTo);
			return;
		}
		else if (sender == mAppendTo)
		{
			SetFavourite(mAppendTo->IsChecked(), CMailAccountManager::eFavouriteAppendTo);
			return;
		}
		else if (sender == mPuntOnClose)
		{
			SetFavourite(mPuntOnClose->IsChecked(), CMailAccountManager::eFavouritePuntOnClose);
			return;
		}
		else if (sender == mAutoSync)
		{
			SetFavourite(mAutoSync->IsChecked(), CMailAccountManager::eFavouriteAutoSync);
			return;
		}
		else if (sender == mTieIdentity)
		{
			if (mTieIdentity->IsChecked())
			{
				mIdentityPopup->Activate();
				SetTiedIdentity(true);
			}
			else
			{
				mIdentityPopup->Deactivate();
				SetTiedIdentity(false);
			}
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		if (sender == mAlertStylePopup)
		{
			// Get new type from popup setting
			JIndex item = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			CMailAccountManager::EFavourite new_type = GetStyleType(item - 1);

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
		else if (sender == mIdentityPopup)
		{
			JIndex item = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			switch(item)
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
				mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue()[item - mIdentityPopup->FirstIndex()].GetIdentity();
				SetTiedIdentity(true);
				break;
			}
		}
	}
	if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mRebuildLocal)
		{
			OnRebuildLocal();
		}
	}

	CMailboxPropPanel::Receive(sender, message);
}

// Initialise alert styles
void CPropMailboxOptions::InitAlertStylePopup()
{
	// Delete previous items
	mAlertStylePopup->RemoveAllItems();

	// Add each alert style
	short menu_pos = 1;
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++, menu_pos++)
		mAlertStylePopup->AppendItem((*iter).GetName(), kFalse, kTrue);

	mAlertStylePopup->SetToPopupChoice(kTrue, 1);
}

// Set mbox list
void CPropMailboxOptions::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// Do icon state
	if (static_cast<CMbox*>(mbox_list->front())->GetProtocol()->CanDisconnect())
		mIconState->SetImage(CIconLoader::GetIcon(static_cast<CMbox*>(mbox_list->front())->GetProtocol()->IsDisconnected() ?
												IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
	else if (static_cast<CMbox*>(mbox_list->front())->IsLocalMbox())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC),  kFalse);
		mAutoSync->Deactivate();
	}
	else
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
		mAutoSync->Deactivate();
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

	SetListening(kFalse);

	if (!mAllDir)
	{
		mCheck->SetValue(auto_check);
		if (mCheck->GetValue() == 1)
			mAlertStylePopup->Activate();
		else
			mAlertStylePopup->Deactivate();
			
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
		mCheck->Deactivate();
		mAlertStylePopup->Deactivate();
		mOpen->Deactivate();
		mCopyTo->Deactivate();
		mAppendTo->Deactivate();
		mPuntOnClose->Deactivate();
		mAutoSync->Deactivate();
	}

	if (multiple_tied)
	{
		mTieIdentity->SetValue(2);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
		mIdentityPopup->SetValue(mIdentityPopup->FirstIndex());
		mIdentityPopup->Deactivate();
	}
	else if (!mCurrentIdentity.empty())
	{
		const CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);
		mTieIdentity->SetValue(1);
		mIdentityPopup->SetIdentity(CPreferences::sPrefs, id);
	}
	else
	{
		mTieIdentity->SetValue(0);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
		mIdentityPopup->SetValue(mIdentityPopup->FirstIndex());
		mIdentityPopup->Deactivate();
	}

	if (!mHasLocal)
		mRebuildLocal->SetVisible(kFalse);

	SetListening(kTrue);
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
