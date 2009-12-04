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

#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CCalendarStoreWebcal.h"
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include "JXTextCheckbox3.h"
#include "JXSecondaryRadioGroup.h"

#include "CInputField.h"
#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXIntegerInput.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarWebcal::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Webcal", this,
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
        new JXStaticText("URL:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,32, 45,20);
    assert( obj2 != NULL );

    mAllowChanges =
        new JXTextCheckbox3("Allow Local Changes", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 205,20);
    assert( mAllowChanges != NULL );

    mAutoPublish =
        new JXTextCheckbox3("Automatically Publish to Server", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 215,20);
    assert( mAutoPublish != NULL );

    mRefreshGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,170, 420,64);
    assert( mRefreshGroup != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Refresh Calendar from Server:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,160, 180,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(1, "Never", mRefreshGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 70,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(2, "Every:", mRefreshGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,30, 65,20);
    assert( obj5 != NULL );

    mRefreshInterval =
        new CInputField<JXIntegerInput>(mRefreshGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,31, 45,20);
    assert( mRefreshInterval != NULL );

    JXStaticText* obj6 =
        new JXStaticText("mins.", mRefreshGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 130,33, 35,20);
    assert( obj6 != NULL );

    mRevertBtn =
        new JXTextButton("Revert", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,75, 85,20);
    assert( mRevertBtn != NULL );
    mRevertBtn->SetFontSize(10);

    mRemoteURL =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 315,40);
    assert( mRemoteURL != NULL );

// end JXLayout1

	mRemoteURL->SetBreakCROnly(false);

	ListenTo(mRevertBtn);
	ListenTo(mAllowChanges);
	ListenTo(mAutoPublish);
	ListenTo(mRefreshGroup);
}

void CPropCalendarWebcal::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mRevertBtn)
		{
			OnRevertURL();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mAllowChanges)
		{
			OnAllowChanges();
			return;
		}
		else if (sender == mAutoPublish)
		{
			OnAutoPublish();
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mRefreshGroup)
		{
			switch(index)
			{
			case 1:
				OnRefreshNever();
				break;
			case 2:
				OnRefreshEvery();
				break;
			}
			return;
		}
	}

	CCalendarPropPanel::Receive(sender, message);
}

// Set cal list
void CPropCalendarWebcal::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// NB This panel is only active when all items are webcals

	// Save list
	mCalList = cal_list;

	// Do icon state
	mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_DISCONNECTED, mIconState, 32, 0x00CCCCCC),  kFalse);

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

	SetListening(kFalse);

	// Disable URL if more than one
	if (webcal > 1)
	{
		mRemoteURL->SetText(rsrc::GetString("UI::CalendarProp::MultipleWebcals"));
		mRemoteURL->SetActive(false);
		mRevertBtn->SetActive(false);
	}
	else
	{
		mRemoteURL->SetText(mCalList->front()->GetRemoteURL());
		mRevertURL = mCalList->front()->GetRemoteURL();
	}

	mAllowChanges->SetValue(allow_changes);
	if (allow_changes == 0)
		mAutoPublish->SetActive(false);
	mAutoPublish->SetValue(auto_publish);
	
	// Set refresh on if all are on, else off
	mRefreshGroup->SelectItem(periodic_refresh ? 2 : 1);

	// Leave refresh interval empty if more than one
	if (refresh_intervals_equal)
	{
		cdstring temp(common_refresh_interval);
		mRefreshInterval->SetText(temp);
	}
	else
		mRefreshInterval->SetText(cdstring::null_str);
	if (!periodic_refresh)
		mRefreshInterval->SetActive(false);

	SetListening(kTrue);
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
		cdstring url = mRemoteURL->GetText();
		if ((mCalList->front()->GetWebcal() != NULL) && (mCalList->front()->GetWebcal()->GetURL() != url))
		{
			changed = true;
			mCalList->front()->GetWebcal()->SetURL(url);
		}
	}

	// Only when turned on
	if (mRefreshGroup->GetSelectedItem() == 2)
	{
		// Only do when text is present: empty text means indeterminate value
		cdstring temp = mRefreshInterval->GetText().GetCString();
		if (!temp.empty())
		{
			long refresh_interval = ::atoi((char*) temp);

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

void CPropCalendarWebcal::OnAllowChanges()
{
	// Enable/disable publish
	mAutoPublish->SetActive(mAllowChanges->GetValue());
	
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

void CPropCalendarWebcal::OnRevertURL()
{
	mRemoteURL->SetText(mRevertURL);
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
	
	mRefreshInterval->SetActive(false);
}

void CPropCalendarWebcal::OnRefreshEvery()
{
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
	
	mRefreshInterval->SetActive(true);
}
