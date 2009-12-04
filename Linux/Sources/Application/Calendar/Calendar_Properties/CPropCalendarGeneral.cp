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
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CRFC822.h"
#include "CStaticText.h"
#include "CXStringResources.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarGeneral::OnCreate()
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

    JXStaticText* obj4 =
        new JXStaticText("Hierarchy:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,72, 65,20);
    assert( obj4 != NULL );

    mHierarchy =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,70, 315,20);
    assert( mHierarchy != NULL );
    mHierarchy->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj5 =
        new JXStaticText("Hierarchy\nSeparator:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,92, 70,31);
    assert( obj5 != NULL );

    mSeparator =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,103, 20,20);
    assert( mSeparator != NULL );
    mSeparator->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj6 =
        new JXStaticText("Size:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,132, 40,18);
    assert( obj6 != NULL );

    mSize =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,130, 75,20);
    assert( mSize != NULL );
    mSize->SetBorderWidth(kJXDefaultBorderWidth);

    mCalculateBtn =
        new JXTextButton("Calculate", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,130, 70,19);
    assert( mCalculateBtn != NULL );
    mCalculateBtn->SetFontSize(10);

    JXStaticText* obj7 =
        new JXStaticText("Status:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,157, 55,22);
    assert( obj7 != NULL );

    mStatus =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,155, 315,20);
    assert( mStatus != NULL );
    mStatus->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj8 =
        new JXStaticText("Synchronised:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,182, 88,22);
    assert( obj8 != NULL );

    mLastSync =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,180, 315,20);
    assert( mLastSync != NULL );
    mLastSync->SetBorderWidth(kJXDefaultBorderWidth);

// end JXLayout1

	ListenTo(mCalculateBtn);
}

// Handle buttons
void CPropCalendarGeneral::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed) && (sender == mCalculateBtn))
	{
		CheckSize();
	}

	CCalendarPropPanel::Receive(sender, message);
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
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else if (node->GetProtocol()->IsWebCalendar())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_DISCONNECTED, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else if (node->GetProtocol()->CanDisconnect())
	{
		mIconState->SetImage(CIconLoader::GetIcon(node->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC),  kFalse);
	}
	else
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC),  kFalse);
	}

	// Copy text to edit fields
	mName->SetText(node->GetDisplayName());

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
		mCalculateBtn->Deactivate();

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
