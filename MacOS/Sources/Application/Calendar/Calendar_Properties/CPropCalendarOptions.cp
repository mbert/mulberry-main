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

#include "CIdentityPopup.h"
#include "CCalendarPropDialog.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"

#include <LCheckBox.h>
#include <LIconControl.h>
#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CPropCalendarOptions::CPropCalendarOptions(LStream *inStream)
		: CCalendarPropPanel(inStream)
{
}

// Default destructor
CPropCalendarOptions::~CPropCalendarOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarOptions::FinishCreateSelf(void)
{
	// Do inherited
	CCalendarPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_CalendarOptionsState);

	// Get items
	mSubscribe = (LCheckBox*) FindPaneByID(paneid_CalendarOptionsSubscribe);
	mFreeBusySet = (LCheckBox*) FindPaneByID(paneid_CalendarOptionsFreeBusySet);
	mTieIdentity = (LCheckBox*) FindPaneByID(paneid_CalendarOptionsTieIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_CalendarOptionsIdentityPopup);
	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropCalendarOptionsBtns);
}

// Handle buttons
void CPropCalendarOptions::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_COSubscribe:
		{
			// Iterate over all Calendars
			for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
			{
				calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(*iter, !(*iter)->IsSubscribed());
			}
		}
			break;
			
	case msg_COFreeBusySet:
		{
			// Iterate over all Calendars
			for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
			{
				//calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(*iter, !(*iter)->IsSubscribed());
			}
		}
		break;

	case msg_COTieIdentity:
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

	case msg_COIdentityPopup:
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
	}
}

// Set cal list
void CPropCalendarOptions::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// Do icon state
	if (cal_list->front()->GetProtocol()->IsLocalCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
	}
	else if (cal_list->front()->GetProtocol()->IsWebCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateDisconnected);
	}
	else if (cal_list->front()->GetProtocol()->CanDisconnect())
	{
		mIconState->SetResourceID(cal_list->front()->GetProtocol()->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
	}
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
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
			freebusyset++;

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

	StopListening();

	if (!all_directory)
	{
		mSubscribe->SetValue(subscribe);
	}
	else
	{
		mSubscribe->Disable();
	}
	
	if (freebusyset == 0)
		mFreeBusySet->Disable();

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

	StartListening();
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
