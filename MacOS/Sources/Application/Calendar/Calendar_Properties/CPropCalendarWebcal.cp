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


// Source for CPropCalendarWebcal class

#include "CPropCalendarWebcal.h"

#include "CCalendarPropDialog.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CCalendarStoreWebcal.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTextFieldX.h"
#include "CURL.h"
#include "CXStringResources.h"

#include <LCheckBox.h>
#include <LIconControl.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CPropCalendarWebcal::CPropCalendarWebcal(LStream *inStream)
		: CCalendarPropPanel(inStream)
{
}

// Default destructor
CPropCalendarWebcal::~CPropCalendarWebcal()
{
	// Change details when the pane is removed
	ApplyChanges();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarWebcal::FinishCreateSelf(void)
{
	// Do inherited
	CCalendarPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_CalendarWebcalState);

	// Get items
	mRemoteURL = (CTextFieldX*) FindPaneByID(paneid_CalendarWebcalRemoteURL);
	mAllowChanges = (LCheckBox*) FindPaneByID(paneid_CalendarWebcalAllowChanges);
	mAutoPublish = (LCheckBox*) FindPaneByID(paneid_CalendarWebcalAutoPublish);
	mRefreshNever = (LRadioButton*) FindPaneByID(paneid_CalendarWebcalRefreshNever);
	mRefreshEvery = (LRadioButton*) FindPaneByID(paneid_CalendarWebcalRefreshEvery);
	mRefreshInterval = (CTextFieldX*) FindPaneByID(paneid_CalendarWebcalRefreshInterval);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropCalendarWebcalBtns);
}

// Handle buttons
void CPropCalendarWebcal::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_CWRevertURL:
		OnRevertURL();
		break;
	case msg_CWAllowChanges:
		OnAllowChanges();
		break;
	case msg_CWAutoPublish:
		OnAutoPublish();
		break;
	case msg_CWRefreshNever:
		OnRefreshNever();
		break;
	case msg_CWRefreshEvery:
		OnRefreshEvery();
		break;
	}
}

// Set cal list
void CPropCalendarWebcal::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// NB This panel is only active when all items are webcals

	// Save list
	mCalList = cal_list;

	// Do icon state
	mIconState->SetResourceID(icnx_MailboxStateDisconnected);

	int webcal = 0;
	int allow_changes = 0;
	int auto_publish = 0;
	int periodic_refresh = 0;
	uint32_t common_refresh_interval = 0;
	bool refresh_intervals_equal = true;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		webcal++;
		if ((*iter)->GetWebcal() != NULL)
		{
			if (!(*iter)->GetWebcal()->GetReadOnly())
				allow_changes++;
			if ((*iter)->GetWebcal()->GetAutoPublish())
				auto_publish++;
			if ((*iter)->GetWebcal()->GetPeriodicRefresh())
				periodic_refresh++;
			
			// Check refersh intervals
			if (iter == mCalList->begin())
				common_refresh_interval = (*iter)->GetWebcal()->GetRefreshInterval();
			else if ((*iter)->GetWebcal()->GetPeriodicRefresh() != common_refresh_interval)
				refresh_intervals_equal = false;
		}
	}

	// Now adjust totals
	if (allow_changes != 0)
		allow_changes = ((allow_changes == mCalList->size()) ? 1 : 2);
	if (auto_publish != 0)
		auto_publish = ((auto_publish == mCalList->size()) ? 1 : 2);
	if (periodic_refresh != 0)
		periodic_refresh = ((periodic_refresh == mCalList->size()) ? 1 : 2);

	StopListening();

	// Disable URL if more than one
	if (webcal > 1)
	{
		mRemoteURL->SetText(rsrc::GetString("UI::CalendarProp::MultipleWebcals"));
		mRemoteURL->Disable();
		FindPaneByID(paneid_CalendarWebcalRevertURL)->Disable();
	}
	else
	{
		mRemoteURL->SetText(mCalList->front()->GetRemoteURL());
		mOriginalURL = mCalList->front()->GetRemoteURL();
	}

	mAllowChanges->SetValue(allow_changes);
	if (allow_changes == 0)
		mAutoPublish->Disable();
	mAutoPublish->SetValue(auto_publish);
	
	// Set refresh on if all are on, else off
	mRefreshNever->SetValue(periodic_refresh != 1);
	mRefreshEvery->SetValue(periodic_refresh == 1);

	// Leave refresh interval empty if more than one
	if (refresh_intervals_equal)
		mRefreshInterval->SetNumber(common_refresh_interval);
	else
		mRefreshInterval->SetText(cdstring::null_str);
	if (mRefreshNever->GetValue())
		mRefreshInterval->SetEnabled(false);

	StartListening();
}

// Set protocol
void CPropCalendarWebcal::SetProtocol(calstore::CCalendarProtocol* proto)
{
	// Does nothing
}

// Force update of items
void CPropCalendarWebcal::ApplyChanges(void)
{
	bool changed = false;
	
	// Only set URL when one item in the list
	if (mCalList->size() == 1)
	{
		CURL parsed(mRemoteURL->GetText(), true);
		cdstring url = parsed.ToString();
		if ((mCalList->front()->GetWebcal() != NULL) && (mCalList->front()->GetWebcal()->GetURL() != url))
		{
			changed = true;
			mCalList->front()->GetWebcal()->SetURL(url);
		}
	}

	// Only when turned on
	if (mRefreshEvery->GetValue() == 1)
	{
		// Only do when text is present: empty text means indeterminate value
		if (!mRefreshInterval->GetText().empty())
		{
			long refresh_interval = mRefreshInterval->GetNumber();

			// Iterate over all Calendars and change webcal read-only state
			for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
			{
				if ((*iter)->GetWebcal() != NULL)
				{
					if ((*iter)->GetWebcal()->GetRefreshInterval() != refresh_interval)
					{
						changed = true;
						(*iter)->GetWebcal()->SetRefreshInterval(refresh_interval);
					}
				}
			}
		}
	}
	
	// Flush any changes
	if (changed)
		calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
}

void CPropCalendarWebcal::OnRevertURL()
{
	mRemoteURL->SetText(mOriginalURL);
}

void CPropCalendarWebcal::OnAllowChanges()
{
	// Enable/disable publish
	mAutoPublish->SetEnabled(mAllowChanges->GetValue());
	
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetReadOnly(!mAllowChanges->GetValue());
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
}

void CPropCalendarWebcal::OnAutoPublish()
{
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetAutoPublish(mAutoPublish->GetValue());
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
}

void CPropCalendarWebcal::OnRefreshNever()
{
	// Only when turned on
	if (mRefreshNever->GetValue() != 1)
		return;
	
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetPeriodicRefresh(false);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
	
	mRefreshInterval->SetEnabled(false);
}

void CPropCalendarWebcal::OnRefreshEvery()
{
	// Only when turned on
	if (mRefreshEvery->GetValue() != 1)
		return;
	
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetPeriodicRefresh(true);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
	
	mRefreshInterval->SetEnabled(true);
}
