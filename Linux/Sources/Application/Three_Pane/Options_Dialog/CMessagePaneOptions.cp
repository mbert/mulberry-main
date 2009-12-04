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


// Source for CMessagePaneOptions class

#include "CMessagePaneOptions.h"

#include "CDivider.h"
#include "CMailViewOptions.h"
#include "CUserActionOptions.h"

#include <JXColormap.h>
#include <JXIntegerInput.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include "CInputField.h"

#include <cassert>

// Static members

// Do various bits
void CMessagePaneOptions::OnCreate()
{
// begin JXLayout1

    mListPreview1 =
        new CUserActionOptions(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,40, 200,120);
    assert( mListPreview1 != NULL );

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 200,25);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Server Pane", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 148,15);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    JXStaticText* obj3 =
        new JXStaticText("To Preview a Mailbox", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 140,15);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    CDivider* obj4 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 145,30, 50,2);
    assert( obj4 != NULL );

    mListFullView1 =
        new CUserActionOptions(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,180, 200,120);
    assert( mListFullView1 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("To Open a Mailbox", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,165, 150,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    CDivider* obj6 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,170, 40,2);
    assert( obj6 != NULL );

    mItemsPreview1 =
        new CUserActionOptions(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,40, 200,120);
    assert( mItemsPreview1 != NULL );

    JXUpRect* obj7 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,0, 200,25);
    assert( obj7 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Mailbox Pane", obj7,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 148,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    JXStaticText* obj9 =
        new JXStaticText("To Preview a Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,25, 145,15);
    assert( obj9 != NULL );
    obj9->SetFontSize(10);
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    CDivider* obj10 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,30, 45,2);
    assert( obj10 != NULL );

    mItemsFullView1 =
        new CUserActionOptions(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,180, 200,120);
    assert( mItemsFullView1 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("To Open a Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,165, 155,15);
    assert( obj11 != NULL );
    obj11->SetFontSize(10);
    const JFontStyle obj11_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj11->SetFontStyle(obj11_style);

    CDivider* obj12 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 360,170, 35,2);
    assert( obj12 != NULL );

    JXUpRect* obj13 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,0, 200,25);
    assert( obj13 != NULL );

    JXStaticText* obj14 =
        new JXStaticText("Message Preview", obj13,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 148,15);
    assert( obj14 != NULL );
    obj14->SetFontSize(10);
    const JFontStyle obj14_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj14->SetFontStyle(obj14_style);

    JXStaticText* obj15 =
        new JXStaticText("Preview Options", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,25, 110,15);
    assert( obj15 != NULL );
    obj15->SetFontSize(10);
    const JFontStyle obj15_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj15->SetFontStyle(obj15_style);

    CDivider* obj16 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 515,30, 80,2);
    assert( obj16 != NULL );

    JXStaticText* obj17 =
        new JXStaticText("Options", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,305, 55,15);
    assert( obj17 != NULL );
    obj17->SetFontSize(10);
    const JFontStyle obj17_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj17->SetFontStyle(obj17_style);

    CDivider* obj18 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 260,310, 135,2);
    assert( obj18 != NULL );

    mMailboxUseTabs =
        new JXTextCheckbox("Use Tabbed Display", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,320, 180,20);
    assert( mMailboxUseTabs != NULL );

    mMailboxRestoreTabs =
        new JXTextCheckbox("Restore Tabs on Start", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,340, 180,20);
    assert( mMailboxRestoreTabs != NULL );

    mMessageAddress =
        new JXTextCheckbox("Show Address Bar", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,40, 180,20);
    assert( mMessageAddress != NULL );

    mMessageSummary =
        new JXTextCheckbox("Show Summary Headers", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,60, 180,20);
    assert( mMessageSummary != NULL );

    mMessageParts =
        new JXTextCheckbox("Show Parts Toolbar", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,80, 180,20);
    assert( mMessageParts != NULL );

    mMarkSeenGroup =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,120, 190,105);
    assert( mMarkSeenGroup != NULL );

    JXStaticText* obj19 =
        new JXStaticText("When Previewing", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 410,110, 135,15);
    assert( obj19 != NULL );
    obj19->SetFontSize(10);
    const JFontStyle obj19_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj19->SetFontStyle(obj19_style);

    mMarkSeen =
        new JXTextRadioButton(1, "Mark it Seen", mMarkSeenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 180,20);
    assert( mMarkSeen != NULL );

    mMarkSeenAfter =
        new JXTextRadioButton(2, "Mark it Seen After:", mMarkSeenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 180,20);
    assert( mMarkSeenAfter != NULL );

    mSeenDelay =
        new CInputField<JXIntegerInput>(mMarkSeenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 45,51, 70,20);
    assert( mSeenDelay != NULL );

    JXStaticText* obj20 =
        new JXStaticText("seconds", mMarkSeenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,53, 60,20);
    assert( obj20 != NULL );

    mNoMarkSeen =
        new JXTextRadioButton(3, "Don't Mark it Seen", mMarkSeenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,75, 180,20);
    assert( mNoMarkSeen != NULL );

    CDivider* obj21 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,30, 2,335);
    assert( obj21 != NULL );

    CDivider* obj22 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,30, 2,335);
    assert( obj22 != NULL );

// end JXLayout1

    mListPreview1->OnCreate();
    mListFullView1->OnCreate();
    mItemsPreview1->OnCreate();
    mItemsFullView1->OnCreate();

    mListPreview = mListPreview1;
    mListFullView = mListFullView1;
    mItemsPreview = mItemsPreview1;
    mItemsFullView = mItemsFullView1;
    
    ListenTo(mMailboxUseTabs);
    ListenTo(mMarkSeenGroup);
}

void CMessagePaneOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mMailboxUseTabs)
		{
			if (mMailboxUseTabs->IsChecked())
				mMailboxRestoreTabs->Activate();
			else
				mMailboxRestoreTabs->Deactivate();
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mMarkSeenGroup)
		{
			if (mMarkSeenGroup->GetSelectedItem() - 1 == CMailViewOptions::eDelayedSeen)
			{
				mSeenDelay->Activate();
				mSeenDelay->Focus();
			}
			else
				mSeenDelay->Deactivate();
		}
	}

	CCommonViewOptions::Receive(sender, message);
}

void CMessagePaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CMailViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);

	mMailboxUseTabs->SetState(JBoolean(options.GetUseTabs()));
	mMailboxRestoreTabs->SetState(JBoolean(options.GetRestoreTabs()));
	if (!options.GetUseTabs())
		mMailboxRestoreTabs->Deactivate();
	if (!is3pane)
	{
		mMailboxUseTabs->Deactivate();
		mMailboxRestoreTabs->Deactivate();
	}

	mMessageAddress->SetState(JBoolean(options.GetShowAddressPane()));
	mMessageSummary->SetState(JBoolean(options.GetShowSummary()));
	mMessageParts->SetState(JBoolean(options.GetShowParts()));
	
	mMarkSeenGroup->SelectItem(options.GetPreviewFlagging() + 1);

	mSeenDelay->SetValue(options.GetPreviewDelay());
	
	if (options.GetPreviewFlagging() != CMailViewOptions::eDelayedSeen)
		mSeenDelay->Deactivate();
}

void CMessagePaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CMailViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);

	options.SetUseTabs(mMailboxUseTabs->IsChecked());
	options.SetRestoreTabs(mMailboxRestoreTabs->IsChecked());

	options.SetShowAddressPane(mMessageAddress->IsChecked());
	options.SetShowSummary(mMessageSummary->IsChecked());
	options.SetShowParts(mMessageParts->IsChecked());
	
	options.SetPreviewFlagging(static_cast<CMailViewOptions::EPreviewFlagging>(mMarkSeenGroup->GetSelectedItem() - 1));
	
	JInteger value;
	mSeenDelay->GetValue(&value);
	options.SetPreviewDelay(value);
}
