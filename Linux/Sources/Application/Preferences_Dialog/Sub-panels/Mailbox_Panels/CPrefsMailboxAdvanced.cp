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


// Source for CPrefsMailboxAdvanced class

#include "CPrefsMailboxAdvanced.h"

#include "CCopyToMenu.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"

#include "JXSecondaryRadioGroup.h"

#include <JXDownRect.h>
#include <JXEngravedRect.h>
#include <JXIntegerInput.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include "CInputField.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxAdvanced::OnCreate()
{
// begin JXLayout1

    mNewGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 360,67);
    assert( mNewGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("New Messages:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,0, 100,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "are Unseen", mNewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 95,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "are Recent", mNewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 90,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(3, "are Recent and Unseen", mNewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,45, 165,20);
    assert( obj4 != NULL );

    JXDownRect* obj5 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 360,75);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Cabinets:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,80, 60,20);
    assert( obj6 != NULL );

    mFavouriteCopyTo =
        new JXTextCheckbox("Use 'Copy Messages To' Cabinet", obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 215,20);
    assert( mFavouriteCopyTo != NULL );

    mFavouriteAppendTo =
        new JXTextCheckbox("Use 'Copy Drafts To' Cabinet", obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 195,20);
    assert( mFavouriteAppendTo != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Maximum History:", obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,47, 110,20);
    assert( obj7 != NULL );

    mMaximumMRU =
        new CInputField<JXIntegerInput>(obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,45, 50,20);
    assert( mMaximumMRU != NULL );

    JXDownRect* obj8 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,175, 360,163);
    assert( obj8 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Message Display Cache:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,165, 150,20);
    assert( obj9 != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Remote Mailboxes:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,8, 135,20);
    assert( obj10 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Initial Download:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,28, 110,20);
    assert( obj11 != NULL );

    mRLoCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,26, 50,20);
    assert( mRLoCache != NULL );

    mRCacheGroup =
        new JXRadioGroup(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,49, 110,40);
    assert( mRCacheGroup != NULL );
    mRCacheGroup->SetBorderWidth(0);

    JXTextRadioButton* obj12 =
        new JXTextRadioButton(1, "Unlimited Total", mRCacheGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 110,20);
    assert( obj12 != NULL );

    JXTextRadioButton* obj13 =
        new JXTextRadioButton(2, "Limit to:", mRCacheGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 70,20);
    assert( obj13 != NULL );

    mRHiCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,69, 50,20);
    assert( mRHiCache != NULL );

    mRIncrementGroup =
        new JXRadioGroup(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,92, 110,40);
    assert( mRIncrementGroup != NULL );
    mRIncrementGroup->SetBorderWidth(0);

    JXTextRadioButton* obj14 =
        new JXTextRadioButton(1, "Auto Increment", mRIncrementGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 110,20);
    assert( obj14 != NULL );

    JXTextRadioButton* obj15 =
        new JXTextRadioButton(2, "Increment:", mRIncrementGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 85,20);
    assert( obj15 != NULL );

    mRCacheIncrement =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,112, 50,20);
    assert( mRCacheIncrement != NULL );

    JXStaticText* obj16 =
        new JXStaticText("Sort Warning:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,137, 110,20);
    assert( obj16 != NULL );

    mRSortCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,135, 50,20);
    assert( mRSortCache != NULL );

    JXEngravedRect* obj17 =
        new JXEngravedRect(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,13, 3,142);
    assert( obj17 != NULL );

    JXStaticText* obj18 =
        new JXStaticText("Local Mailboxes:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,8, 135,20);
    assert( obj18 != NULL );

    JXStaticText* obj19 =
        new JXStaticText("Initial Download:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,28, 110,20);
    assert( obj19 != NULL );

    mLLoCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,26, 50,20);
    assert( mLLoCache != NULL );

    mLCacheGroup =
        new JXRadioGroup(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 185,49, 110,40);
    assert( mLCacheGroup != NULL );
    mLCacheGroup->SetBorderWidth(0);

    JXTextRadioButton* obj20 =
        new JXTextRadioButton(1, "Unlimited Total", mLCacheGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 110,20);
    assert( obj20 != NULL );

    JXTextRadioButton* obj21 =
        new JXTextRadioButton(2, "Limit to:", mLCacheGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 70,20);
    assert( obj21 != NULL );

    mLHiCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,69, 50,20);
    assert( mLHiCache != NULL );

    mLIncrementGroup =
        new JXRadioGroup(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 185,92, 110,40);
    assert( mLIncrementGroup != NULL );
    mLIncrementGroup->SetBorderWidth(0);

    JXTextRadioButton* obj22 =
        new JXTextRadioButton(1, "Auto Increment", mLIncrementGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 110,20);
    assert( obj22 != NULL );

    JXTextRadioButton* obj23 =
        new JXTextRadioButton(2, "Increment:", mLIncrementGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 85,20);
    assert( obj23 != NULL );

    mLCacheIncrement =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,112, 50,20);
    assert( mLCacheIncrement != NULL );

    JXStaticText* obj24 =
        new JXStaticText("Sort Warning:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,137, 110,20);
    assert( obj24 != NULL );

    mLSortCache =
        new CInputField<JXIntegerInput>(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,135, 50,20);
    assert( mLSortCache != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mFavouriteCopyTo);
	ListenTo(mFavouriteAppendTo);
	ListenTo(mRCacheGroup);
	ListenTo(mRIncrementGroup);
	ListenTo(mLCacheGroup);
	ListenTo(mLIncrementGroup);
}

// Handle buttons
void CPrefsMailboxAdvanced::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if ((sender == mFavouriteCopyTo) || (sender == mFavouriteAppendTo))
		{
			OnForceMailboxListReset();
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mRCacheGroup)
		{
			OnRCacheGroup();
			return;
		}
		else if (sender == mRIncrementGroup)
		{
			OnRIncrementGroup();
			return;
		}
		else if (sender == mLCacheGroup)
		{
			OnLCacheGroup();
			return;
		}
		else if (sender == mLIncrementGroup)
		{
			OnLIncrementGroup();
			return;
		}
	}
}

// Set prefs
void CPrefsMailboxAdvanced::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	if (copyPrefs->mUnseenNew.GetValue() && !copyPrefs->mRecentNew.GetValue())
		mNewGroup->SelectItem(1);
	else if (!copyPrefs->mUnseenNew.GetValue() && copyPrefs->mRecentNew.GetValue())
		mNewGroup->SelectItem(2);
	else
		mNewGroup->SelectItem(3);

	mFavouriteCopyTo->SetState(JBoolean(copyPrefs->mUseCopyToMboxes.GetValue()));
	mFavouriteAppendTo->SetState(JBoolean(copyPrefs->mUseAppendToMboxes.GetValue()));
	mMaximumMRU->SetValue(copyPrefs->mMRUMaximum.GetValue());

	mRLoCache->SetValue(copyPrefs->mRLoCache.GetValue());

	mRHiCache->SetValue(copyPrefs->mRHiCache.GetValue());
	if (copyPrefs->mRUseHiCache.GetValue())
	{
		mRCacheGroup->SelectItem(2);
		mRHiCache->Activate();
	}
	else
	{
		mRCacheGroup->SelectItem(1);
		mRHiCache->Deactivate();
	}

	mRCacheIncrement->SetValue(copyPrefs->mRCacheIncrement.GetValue());
	if (copyPrefs->mRAutoCacheIncrement.GetValue())
	{
		mRIncrementGroup->SelectItem(1);
		mRCacheIncrement->Deactivate();
	}
	else
	{
		mRIncrementGroup->SelectItem(2);
		mRCacheIncrement->Activate();
	}

	mRSortCache->SetValue(copyPrefs->mRSortCache.GetValue());

	mLLoCache->SetValue(copyPrefs->mLLoCache.GetValue());

	mLHiCache->SetValue(copyPrefs->mLHiCache.GetValue());
	if (copyPrefs->mLUseHiCache.GetValue())
	{
		mLCacheGroup->SelectItem(2);
		mLHiCache->Activate();
	}
	else
	{
		mLCacheGroup->SelectItem(1);
		mLHiCache->Deactivate();
	}

	mLCacheIncrement->SetValue(copyPrefs->mLCacheIncrement.GetValue());
	if (copyPrefs->mLAutoCacheIncrement.GetValue())
	{
		mLIncrementGroup->SelectItem(1);
		mLCacheIncrement->Deactivate();
	}
	else
	{
		mLIncrementGroup->SelectItem(2);
		mLCacheIncrement->Activate();
	}

	mLSortCache->SetValue(copyPrefs->mLSortCache.GetValue());
}

// Force update of prefs
bool CPrefsMailboxAdvanced::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mUnseenNew.SetValue((mNewGroup->GetSelectedItem() == 1) || (mNewGroup->GetSelectedItem() == 3));
	copyPrefs->mRecentNew.SetValue((mNewGroup->GetSelectedItem() == 2) || (mNewGroup->GetSelectedItem() == 3));

	copyPrefs->mUseCopyToMboxes.SetValue(mFavouriteCopyTo->IsChecked());
	copyPrefs->mUseAppendToMboxes.SetValue(mFavouriteAppendTo->IsChecked());
	JInteger value;
	mMaximumMRU->GetValue(&value);
	copyPrefs->mMRUMaximum.SetValue(value);

	mRLoCache->GetValue(&value);
	copyPrefs->mRLoCache.SetValue(value);

	copyPrefs->mRUseHiCache.SetValue(mRCacheGroup->GetSelectedItem() == 2);
	mRHiCache->GetValue(&value);
	copyPrefs->mRHiCache.SetValue(value);

	copyPrefs->mRAutoCacheIncrement.SetValue(mRIncrementGroup->GetSelectedItem() == 1);
	mRCacheIncrement->GetValue(&value);
	copyPrefs->mRCacheIncrement.SetValue(value);

	mRSortCache->GetValue(&value);
	copyPrefs->mRSortCache.SetValue(value);

	mLLoCache->GetValue(&value);
	copyPrefs->mLLoCache.SetValue(value);

	copyPrefs->mLUseHiCache.SetValue(mLCacheGroup->GetSelectedItem() == 2);
	mLHiCache->GetValue(&value);
	copyPrefs->mLHiCache.SetValue(value);

	copyPrefs->mLAutoCacheIncrement.SetValue(mLIncrementGroup->GetSelectedItem() == 1);
	mLCacheIncrement->GetValue(&value);
	copyPrefs->mLCacheIncrement.SetValue(value);

	mLSortCache->GetValue(&value);
	copyPrefs->mLSortCache.SetValue(value);
	
	return true;
}

void CPrefsMailboxAdvanced::OnForceMailboxListReset()
{
	// Must force reset of menu lists
	CCopyToMenu::SetMenuOptions(mFavouriteCopyTo->IsChecked(),
								mFavouriteAppendTo->IsChecked());

	// Make sure menus are rest when dialog is exited (OK or Cancel)
	CPreferencesDialog* prefs_dlog = CPreferencesDialog::sPrefsDlog;
	prefs_dlog->SetForceMenuListReset(true);
}

void CPrefsMailboxAdvanced::OnRCacheGroup()
{
	if (mRCacheGroup->GetSelectedItem() == 1)
		mRHiCache->Deactivate();
	else
	{
		mRHiCache->Activate();
		mRHiCache->SelectAll();
		mRHiCache->Focus();
	}
}

void CPrefsMailboxAdvanced::OnRIncrementGroup()
{
	if (mRIncrementGroup->GetSelectedItem() == 1)
		mRCacheIncrement->Deactivate();
	else
	{
		mRCacheIncrement->Activate();
		mRCacheIncrement->SelectAll();
		mRCacheIncrement->Focus();
	}
}

void CPrefsMailboxAdvanced::OnLCacheGroup()
{
	if (mLCacheGroup->GetSelectedItem() == 1)
		mLHiCache->Deactivate();
	else
	{
		mLHiCache->Activate();
		mLHiCache->SelectAll();
		mLHiCache->Focus();
	}
}

void CPrefsMailboxAdvanced::OnLIncrementGroup()
{
	if (mLIncrementGroup->GetSelectedItem() == 1)
		mLCacheIncrement->Deactivate();
	else
	{
		mLCacheIncrement->Activate();
		mLCacheIncrement->SelectAll();
		mLCacheIncrement->Focus();
	}
}
