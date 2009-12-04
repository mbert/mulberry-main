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


// Source for CPropCalendarGeneral class

#include "CPropCalendarGeneral.h"

#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
#include "CConnectionManager.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CRFC822.h"
#include "CTextFieldX.h"
#include "CXstringResources.h"

#include <LIconControl.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CPropCalendarGeneral::CPropCalendarGeneral(LStream *inStream)
		: CCalendarPropPanel(inStream)
{
}

// Default destructor
CPropCalendarGeneral::~CPropCalendarGeneral()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarGeneral::FinishCreateSelf(void)
{
	// Do inherited
	CCalendarPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_CalendarGeneralState);

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralName);
	mServer = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralServer);
	mHierarchy = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralHierarchy);
	mSeparator = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralSeparator);
	mSize = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralSize);
	mStatus = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralStatus);
	mLastSync = (CTextFieldX*) FindPaneByID(paneid_CalendarGeneralLastSync);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropCalendarGeneralBtns);
}

// Handle buttons
void CPropCalendarGeneral::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_CalendarGeneralCalculate:
		CheckSize();
		break;
	}
}

// Set mbox list
void CPropCalendarGeneral::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// For now just use first item
	SetCalendar(mCalList->front());
}


// Set mbox list
void CPropCalendarGeneral::SetCalendar(calstore::CCalendarStoreNode* node)
{
	// Do icon state
	if (node->GetProtocol()->IsLocalCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
		mLastSync->Disable();
	}
	else if (node->GetProtocol()->IsWebCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateDisconnected);
	}
	else if (node->GetProtocol()->CanDisconnect())
	{
		mIconState->SetResourceID(node->GetProtocol()->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
	}
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
		mLastSync->Disable();
	}

	// Copy text to edit fields
	mName->SetText(node->GetDisplayShortName());

	mServer->SetText(node->GetProtocol()->GetDescriptor());

	mHierarchy->SetText(node->GetParent()->GetName());

	cdstring temp(node->GetProtocol()->GetDirDelim());
	mSeparator->SetText(temp);

	// Only set these if not a directory
	if (!node->IsDirectory())
	{
		{
			unsigned long size = node->GetSize();
			if ((size == ULONG_MAX) && (node->GetProtocol()->IsOffline() || node->GetProtocol()->IsWebCalendar()))
			{
				node->CheckSize();
				size = node->GetSize();
			}
			if (size != ULONG_MAX)
			{
				mSize->SetText(::GetNumericFormat(size));
			}
			else
				mSize->SetText(cdstring::null_str);
		}

		{
			// Depends on account type
			if (node->GetProtocol()->IsLocalCalendar())
			{
				mLastSync->SetText(rsrc::GetString("UI::CalendarProp::LocalSync"));
			}
			else if (node->GetProtocol()->IsWebCalendar() || node->GetProtocol()->CanDisconnect())
			{
				unsigned long utc_time = node->GetLastSync();
				
				if (utc_time != 0)
				{
					// Determine timezone offset
					time_t t1 = ::time(NULL);
					time_t t2 = ::mktime(::gmtime(&t1));
					long offset = ::difftime(t1, t2);
					long current_zone = ::difftime(t1, t2) / 3600.0;	// Zone in hundreds e.g. -0500
					current_zone *= 100;
					utc_time += offset;

					temp = CRFC822::GetTextDate(utc_time, current_zone, true);
				}
				else
					temp = rsrc::GetString("UI::CalendarProp::NoSync");
				mLastSync->SetText(temp);
			}
			else
			{
				mLastSync->SetText(rsrc::GetString("UI::CalendarProp::CannotSync"));
			}
		}
	}

	// Disable size calculate if a directory
	if (node->IsDirectory())
		FindPaneByID(paneid_CalendarGeneralCalculate)->Disable();

	temp = cdstring::null_str;
	if (node->IsActive())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::CalendarProp::StateOpen");
		temp += rsrc;
	}
	if (node->IsSubscribed())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::CalendarProp::StateSubs");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	mStatus->SetText(temp);
}

// Set protocol
void CPropCalendarGeneral::SetProtocol(calstore::CCalendarProtocol* proto)
{
	// Does nothing
}

// Force update of items
void CPropCalendarGeneral::ApplyChanges(void)
{
	// Nothing to update
}

// Check sizes of all mailboxes
void CPropCalendarGeneral::CheckSize()
{
	// Iterate over all mailboxes
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if (!(*iter)->IsDirectory())
			(*iter)->CheckSize();
	}

	{
		uint32_t size = mCalList->front()->GetSize();
		if (size != ULONG_MAX)
		{
			mSize->SetText(::GetNumericFormat(size));
		}
		else
			mSize->SetText(cdstring::null_str);
	}
}
