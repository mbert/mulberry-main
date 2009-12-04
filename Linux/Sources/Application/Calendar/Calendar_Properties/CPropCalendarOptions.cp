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


// Source for CPropCalendarOptions class

#include "CPropCalendarOptions.h"

#include "CIconLoader.h"
#include "CIdentityPopup.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CPreferences.h"

#include "JXTextCheckbox3.h"
#include "TPopupMenu.h"
#include "StValueChanger.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarOptions::OnCreate()
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
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 414,4, 32,32);
    assert( mIconState != NULL );
    mIconState->SetBorderWidth(kJXDefaultBorderWidth);

    mSubscribe =
        new JXTextCheckbox3("Show in Subscribed Views", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,30, 205,20);
    assert( mSubscribe != NULL );

    mFreeBusySet =
        new JXTextCheckbox3("Searched During Freebusy Lookups", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,55, 290,20);
    assert( mFreeBusySet != NULL );

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,100, 420,70);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Identity", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,90, 50,20);
    assert( obj3 != NULL );

    mTieIdentity =
        new JXTextCheckbox3("Tie to Identity", obj2,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 170,20);
    assert( mTieIdentity != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Identity:", obj2,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,35, 55,20);
    assert( obj4 != NULL );

    mIdentityPopup =
        new CIdentityPopup("", obj2,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 70,35, 200,20);
    assert( mIdentityPopup != NULL );

// end JXLayout1

	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());

	ListenTo(mSubscribe);
	ListenTo(mTieIdentity);
	ListenTo(mIdentityPopup);
}

// Handle buttons
void CPropCalendarOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mSubscribe)
		{
			// Iterate over all Calendars
			for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
			{
				calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(*iter, !(*iter)->IsSubscribed());
			}
			return;
		}
		else if (sender == mFreeBusySet)
		{
			// Iterate over all Calendars
			for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
			{
				//calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(*iter, !(*iter)->IsSubscribed());
			}
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
		if (sender == mIdentityPopup)
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
			return;
		}
	}

	CCalendarPropPanel::Receive(sender, message);
}

// Set cal list
void CPropCalendarOptions::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// Do icon state
	if (cal_list->front()->GetProtocol()->IsLocalCalendar())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else if (cal_list->front()->GetProtocol()->IsWebCalendar())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_DISCONNECTED, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else if (cal_list->front()->GetProtocol()->CanDisconnect())
	{
		mIconState->SetImage(CIconLoader::GetIcon(cal_list->front()->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC),  kFalse);
	}

	int first_type = -1;
	int subscribe = 0;
	int freebusyset = 0;
	int multiple_tied = 0;
	bool first = true;
	bool all_directory = true;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;
		all_directory &= node->IsDirectory();

		// Only valid id not a directory
		if (!node->IsDirectory() && node->IsSubscribed())
			subscribe++;

		if (node->IsStandardCalendar() && node->GetProtocol()->IsComponentCalendar())
			subscribe++;

		CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(*iter));
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
	if (subscribe)
		subscribe = ((subscribe == mCalList->size()) ? 1 : 2);

	SetListening(kFalse);

	if (!all_directory)
	{
		mSubscribe->SetValue(subscribe);
	}
	else
	{
		mSubscribe->Deactivate();
	}

	if (freebusyset == 0)
		mFreeBusySet->Deactivate();

	if (multiple_tied)
	{
		mTieIdentity->SetValue(2);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
		mIdentityPopup->SetValue(mIdentityPopup->FirstIndex());
		mIdentityPopup->Deactivate();
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
		mIdentityPopup->Deactivate();
	}

	SetListening(kTrue);
}

// Set protocol
void CPropCalendarOptions::SetProtocol(calstore::CCalendarProtocol* proto)
{
	// Does nothing
}

// Force update of items
void CPropCalendarOptions::ApplyChanges(void)
{
	// Nothing to update
}

// Add/remove tied identity
void CPropCalendarOptions::SetTiedIdentity(bool add)
{
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;
		if (add)
		{
			// Identity to use
			CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);

			id->AddTiedCalendar(node);
		}
		else
		{
			// With multiple Calendars might have multiple ids so resolve each one
			CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(node));
			if (id)
				id->RemoveTiedCalendar(node);
		}
	}

	// Mark it as dirty
	CPreferences::sPrefs->mTiedCalendars.SetDirty();
}
