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


// Source for CPrefsAlertsMessage class

#include "CPrefsAlertsMessage.h"

#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CMailAccountManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"
#include "TPopupMenu.h"

#include <JXDownRect.h>
#include <JXIntegerInput.h>
#include <JXInputField.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include "CInputField.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAlertsMessage::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,15, 370,320);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Style:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 40,20);
    assert( obj2 != NULL );

    mAlertStylePopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,3, 155,25);
    assert( mAlertStylePopup != NULL );

    mCheckGroup =
        new JXSecondaryRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,30, 175,75);
    assert( mCheckGroup != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Check:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,20, 50,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(1, "Never", mCheckGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,7, 75,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(2, "Once on Startup", mCheckGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,27, 120,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(3, "Every:", mCheckGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,47, 65,20);
    assert( obj6 != NULL );

    mCheckInterval =
        new CInputField<JXIntegerInput>(mCheckGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,48, 40,20);
    assert( mCheckInterval != NULL );

    JXStaticText* obj7 =
        new JXStaticText("mins.", mCheckGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,50, 40,20);
    assert( obj7 != NULL );

    mNewMailAlert =
        new JXTextCheckbox("Use Foreground Alert", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,110, 150,20);
    assert( mNewMailAlert != NULL );

    mNewMailBackAlert =
        new JXTextCheckbox("Use Background Alert", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,130, 150,20);
    assert( mNewMailBackAlert != NULL );

    mNewMailFlashIcon =
        new JXTextCheckbox("Flash Icon in Background", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,150, 175,20);
    assert( mNewMailFlashIcon != NULL );

    mNewMailPlaySound =
        new JXTextCheckbox("Play Sound:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,170, 105,20);
    assert( mNewMailPlaySound != NULL );

    mNewMailSound =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,171, 175,20);
    assert( mNewMailSound != NULL );

    mNewMailOpen =
        new JXTextCheckbox("Open Mailbox Window", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,190, 155,20);
    assert( mNewMailOpen != NULL );

    mContainsGroup =
        new JXSecondaryRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,225, 255,55);
    assert( mContainsGroup != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Alert when Mailbox Contains:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,215, 175,20);
    assert( obj8 != NULL );

    JXTextRadioButton* obj9 =
        new JXTextRadioButton(1, "New Messages", mContainsGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,7, 115,20);
    assert( obj9 != NULL );

    JXTextRadioButton* obj10 =
        new JXTextRadioButton(2, "Any Messages", mContainsGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,27, 115,20);
    assert( obj10 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Apply to Cabinet:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,292, 105,20);
    assert( obj11 != NULL );

    mApplyToCabinet =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,287, 175,25);
    assert( mApplyToCabinet != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mAlertStylePopup);
	ListenTo(mCheckGroup);
	ListenTo(mNewMailPlaySound);
}

// Handle buttons
void CPrefsAlertsMessage::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mAlertStylePopup)
    	{
			switch(index)
			{
			case eStyleNew:
				DoNewStyle();
				break;
			case eStyleRename:
				DoRenameStyle();
				break;
			case eStyleDelete:
				DoDeleteStyle();
				break;
			default:
				// Update existing style
				if ((mStyleValue >= 0) && (mStyleValue != index - eStyleFirst))
					UpdateNotifaction();

				// Set new style
				mStyleValue = index - eStyleFirst;
				if (mStyleValue >= 0)
					SetNotifaction();
				break;
			}
			return;
		} 
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mCheckGroup)
		{
			switch (index)
			{
			case 1:
			case 2:
				mCheckInterval->Deactivate();
				break;
			case 3:
				mCheckInterval->Activate();
				mCheckInterval->SelectAll();
				mCheckInterval->Focus();
				break;
			}
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mNewMailPlaySound)
		{
			if (mNewMailPlaySound->IsChecked())
			{
				mNewMailSound->Activate();

				cdstring txt = mNewMailSound->GetText();
				::PlayNamedSound(txt);
			}
			else
				mNewMailSound->Deactivate();
			return;
		}
	}
}

// Set prefs
void CPrefsAlertsMessage::SetData(void* data)
{
	// Save ref to prefs
	mCopyPrefs = (CPreferences*) data;

	// Save ref to prefs
	if (!mCopyPrefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		mCopyPrefs->mMailNotification.Value().push_back(notify);
		mCopyPrefs->mMailNotification.SetDirty();
	}

	// Init the style menu
	InitStyleMenu();

	// Init cabinet menu
	InitCabinetMenu();

	// Set initial value - will trigger panel reset
	mStyleValue = 0;
	mAlertStylePopup->SetUpdateAction(JXMenu::kDisableNone);
	mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
	SetNotifaction();
}

// Force update of prefs
bool CPrefsAlertsMessage::UpdateData(void* data)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();
	
	return true;
}

// Set up style menu
void CPrefsAlertsMessage::InitStyleMenu(void)
{
	// Delete previous items
	mAlertStylePopup->RemoveAllItems();

	// Add each style item
	mAlertStylePopup->SetMenuItems("New... | Rename... | Delete... %l"); 
	for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
			iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++)
		mAlertStylePopup->AppendItem((*iter).GetName(), kFalse, kTrue);
}

// Set up cabinet menu
void CPrefsAlertsMessage::InitCabinetMenu(void)
{
	// Delete previous items
	mApplyToCabinet->RemoveAllItems();

	// Add each cabinet item
	for(CFavouriteItemList::const_iterator iter = mCopyPrefs->mFavourites.GetValue().begin();
		iter != mCopyPrefs->mFavourites.GetValue().end(); iter++)
	    mApplyToCabinet->AppendItem((*iter).GetName(), kTrue, kFalse);

	// Force to popup
	mApplyToCabinet->SetUpdateAction(JXMenu::kDisableNone);
	mApplyToCabinet->DisableItem(CMailAccountManager::eFavouriteNew + 1);
	mApplyToCabinet->SetToPopupChoice(kTrue, 1);
}

// Add new account
void CPrefsAlertsMessage::DoNewStyle(void)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Create the dialog
	{
		while (true)
		{
			bool fail = false;
			cdstring new_name;

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::NewAlertStyleName", "Alerts::Preferences::NewAlertStyleName", new_name))
			{
				// Empty name
				if (new_name.empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAlertStyleName");
					continue;
				}

				// Check duplicate name
				for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
						iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++)
				{
					if (new_name == (*iter).GetName())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAlertStyleName");
						fail = true;
						break;
					}
				}
				if (fail)
					continue;

				// Create new alert style
				CMailNotification notify;
				notify.SetName(new_name);
				mCopyPrefs->mMailNotification.Value().push_back(notify);
				mCopyPrefs->mMailNotification.SetDirty();
				unsigned long index = mCopyPrefs->mMailNotification.GetValue().size() - 1;

				// Reset menu
				InitStyleMenu();

				// Set new values
				mStyleValue = index;
				mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
				break;
			}
			else
				mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
				break;
		}
	}
}

// Rename account
void CPrefsAlertsMessage::DoRenameStyle(void)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Create the dialog
	{
		CMailNotification& notify = mCopyPrefs->mMailNotification.Value().at(mStyleValue);

		while (true)
		{
			bool fail = false;
			cdstring new_name = notify.GetName();

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameAlertStyle", "Alerts::Preferences::RenameAlertStyle", new_name))
			{
				// Empty name
				if (new_name.empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAlertStyleName");
					continue;
				}

				// Check duplicate name
				for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
						iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++)
				{
					if (new_name == (*iter).GetName())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAlertStyleName");
						fail = true;
						break;
					}
				}
				if (fail)
					continue;

				// Do style rename
				notify.SetName(new_name);
				mCopyPrefs->mMailNotification.SetDirty();

				// Reset menu
				InitStyleMenu();
				break;
			}
			else
				break;
		}

		// Reset to previous value - will update display
		mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
	}
}

// Delete accoount
void CPrefsAlertsMessage::DoDeleteStyle(void)
{
	// Prevent delete of last alert style
	if (mCopyPrefs->mMailNotification.GetValue().size() == 1)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoDeleteAlertStyle");
		mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
		return;
	}

	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::DeleteAlertStyle") == CErrorHandler::Ok)
	{
		// Erase from prefs
		mCopyPrefs->mMailNotification.Value().erase(mCopyPrefs->mMailNotification.Value().begin() + mStyleValue);
		mCopyPrefs->mMailNotification.SetDirty();

		// Remove from menu
		InitStyleMenu();

		// Adjust to new value
		mStyleValue--;
		if (mStyleValue < 0)
			mStyleValue = 0;

		// Update items
		if (mStyleValue >= 0)
			mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
	}
	else
		mAlertStylePopup->SetValue(mStyleValue + eStyleFirst);
}

// Set alert details
void CPrefsAlertsMessage::SetNotifaction(void)
{
	const CMailNotification& notify = mCopyPrefs->mMailNotification.GetValue().at(mStyleValue);

	// Copy text to edit fields
	mCheckGroup->SelectItem(notify.IsEnabled() ? (notify.GetCheckOnce() ? 2 : 3) : 1);

	mCheckInterval->SetValue(notify.GetCheckInterval());
	if (notify.IsEnabled())
		mCheckInterval->Activate();
	else
		mCheckInterval->Deactivate();

	mContainsGroup->SelectItem(notify.GetCheckNew() ? 1 : 2);

	mNewMailAlert->SetState(JBoolean(notify.DoShowAlertForeground()));
	mNewMailBackAlert->SetState(JBoolean(notify.DoShowAlertBackground()));
	mNewMailFlashIcon->SetState(JBoolean(notify.DoFlashIcon()));

	SetListening(kFalse);
	mNewMailPlaySound->SetState(JBoolean(notify.DoPlaySound()));
	SetListening(kTrue);
	mNewMailSound->SetText(notify.GetSoundID());
	if (notify.DoPlaySound())
		mNewMailSound->Activate();
	else
		mNewMailSound->Deactivate();

	mNewMailOpen->SetState(JBoolean(notify.GetOpenMbox()));

	// Set the cabinet
	unsigned long index = notify.GetFavouriteIndex(mCopyPrefs);
	mApplyToCabinet->SetValue(index + 1);

}

// Update current account
void CPrefsAlertsMessage::UpdateNotifaction(void)
{
	CMailNotification& notify = mCopyPrefs->mMailNotification.Value().at(mStyleValue);

	// Make copy to look for changes
	CMailNotification copy(notify);

	// Copy info from panel into prefs
	notify.Enable(mCheckGroup->GetSelectedItem() != 1);
	notify.SetCheckOnce(mCheckGroup->GetSelectedItem() == 2);
	JInteger value;
	mCheckInterval->GetValue(&value);
	notify.SetCheckInterval(value);

	notify.SetCheckNew(mContainsGroup->GetSelectedItem() == 1);

	notify.SetShowAlertForeground(mNewMailAlert->IsChecked());
	notify.SetShowAlertBackground(mNewMailBackAlert->IsChecked());
	notify.SetFlashIcon(mNewMailFlashIcon->IsChecked());

	notify.SetPlaySound(mNewMailPlaySound->IsChecked());
	notify.SetSoundID(cdstring(mNewMailSound->GetText()));

	notify.SetOpenMbox(mNewMailOpen->IsChecked());

	// Set the cabinet
	CMailAccountManager::EFavourite type = static_cast<CMailAccountManager::EFavourite>(mApplyToCabinet->GetValue() - 1);
	if (type < CMailAccountManager::eFavouriteOthers)
	{
		cdstring temp("#");
		temp += cdstring((unsigned long) type);
		notify.SetFavouriteID(temp);
	}
	else
	{
		cdstring temp("@");
		temp += mCopyPrefs->mFavourites.GetValue().at(type).GetName();
		notify.SetFavouriteID(temp);
	}

	// Set dirty if required
	if (!(copy == notify))
		mCopyPrefs->mMailNotification.SetDirty();
}
