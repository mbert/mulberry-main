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
#include "CSoundPopup.h"
#include "CTextFieldX.h"
#include "CXStringResources.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAlertsMessage::CPrefsAlertsMessage()
{
	mCopyPrefs = NULL;
}

// Constructor from stream
CPrefsAlertsMessage::CPrefsAlertsMessage(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
	mCopyPrefs = NULL;
}

// Default destructor
CPrefsAlertsMessage::~CPrefsAlertsMessage()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAlertsMessage::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mAlertStylePopup = (LPopupButton*) FindPaneByID(paneid_AlertStylePopup);
	mStyleValue = -1;

	mCheckNever = (LRadioButton*) FindPaneByID(paneid_CheckNever);
	mCheckOnce = (LRadioButton*) FindPaneByID(paneid_CheckOnce);
	mCheckEvery = (LRadioButton*) FindPaneByID(paneid_CheckEvery);
	mCheckInterval = (CTextFieldX*) FindPaneByID(paneid_CheckInterval);

	mCheckNew = (LRadioButton*) FindPaneByID(paneid_CheckNew);
	mCheckAll = (LRadioButton*) FindPaneByID(paneid_CheckAll);

	mNewMailAlert = (LCheckBox*) FindPaneByID(paneid_NewMailAlert);
	mNewMailBackAlert = (LCheckBox*) FindPaneByID(paneid_NewMailBackAlert);
	mNewMailFlashIcon = (LCheckBox*) FindPaneByID(paneid_NewMailFlashIcon);
	mNewMailPlaySound = (LCheckBox*) FindPaneByID(paneid_NewMailPlaySound);
	mNewMailSound = (CSoundPopup*) FindPaneByID(paneid_NewMailSound);
	mNewMailSpeak = (LCheckBox*) FindPaneByID(paneid_NewMailSpeak);
	mNewMailSpeakText = (CTextFieldX*) FindPaneByID(paneid_NewMailSpeakText);
	mNewMailOpen = (LCheckBox*) FindPaneByID(paneid_NewMailOpen);

	mApplyToCabinet = (LPopupButton*) FindPaneByID(paneid_ApplyToCabinet);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAlertsMessageBtns);
}

// Handle buttons
void CPrefsAlertsMessage::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_AlertStylePopup:
		switch(*(long*) ioParam)
		{
		case eAlertStylePopup_New:
			DoNewStyle();
			break;
		case eAlertStylePopup_Rename:
			DoRenameStyle();
			break;
		case eAlertStylePopup_Delete:
			DoDeleteStyle();
			break;
		default:
			// Update existing style
			if ((mStyleValue >= 0) && (mStyleValue != *(long*) ioParam - eAlertStylePopup))
				UpdateNotifaction();

			// Set new style
			mStyleValue = *(long*) ioParam - eAlertStylePopup;
			if (mStyleValue >= 0)
				SetNotifaction();
			break;
		}
		break;

	case msg_CheckNever:
		if (*((long*) ioParam))
		{
			mCheckInterval->Disable();
			mCheckNew->Disable();
			mCheckAll->Disable();
			Refresh();
		}
		break;

	case msg_CheckOnce:
		if (*((long*) ioParam))
		{
			mCheckInterval->Disable();
			mCheckNew->Enable();
			mCheckAll->Enable();
		}
		break;

	case msg_CheckEvery:
		if (*((long*) ioParam))
		{
			mCheckInterval->Enable();
			mCheckNew->Enable();
			mCheckAll->Enable();
			LCommander::SwitchTarget(mCheckInterval);
			mCheckInterval->SelectAll();
			Refresh();
		}
		break;

	case msg_NewMailPlaySound:
		if (*((long*) ioParam))
		{
			mNewMailSound->Enable();

			cdstring title;
			mNewMailSound->GetName(title);
			::PlayNamedSound(title);
		}
		else
			mNewMailSound->Disable();
		break;

	case msg_NewMailSound:
		{
			cdstring title;
			mNewMailSound->GetName(title);
			::PlayNamedSound(title);
		}
		break;

	case msg_NewMailSpeak:
		if (*((long*) ioParam))
			mNewMailSpeakText->Enable();
		else
			mNewMailSpeakText->Disable();
		break;
	}
}

// Toggle display of IC items
void CPrefsAlertsMessage::ToggleICDisplay(bool IC_on)
{
	if (IC_on && (mStyleValue == 0))
	{
		mNewMailBackAlert->Disable();
		mNewMailFlashIcon->Disable();
		mNewMailPlaySound->Disable();
		mNewMailSound->Disable();
	}
	else
	{
		mNewMailBackAlert->Enable();
		mNewMailFlashIcon->Enable();
		mNewMailPlaySound->Enable();
		if (mNewMailPlaySound->GetValue())
			mNewMailSound->Enable();
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

	// Set initial value
	mStyleValue = 0;
	StopListening();
	mAlertStylePopup->SetValue(eAlertStylePopup_Off);
	StartListening();
	mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);

	// Toggle IC display
	//ToggleICDisplay(mCopyPrefs->mUse_IC.GetValue());
}

// Force update of prefs
void CPrefsAlertsMessage::UpdateData(void* data)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();
}

// Set up style menu
void CPrefsAlertsMessage::InitStyleMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAlertStylePopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= eAlertStylePopup; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = eAlertStylePopup;
	for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
			iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter).GetName());

	// Force max/min update
	mAlertStylePopup->SetMenuMinMax();
}

// Set up cabinet menu
void CPrefsAlertsMessage::InitCabinetMenu(void)
{
	// Remove any existing items from main menu
	MenuHandle menuH = mApplyToCabinet->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i > CMailAccountManager::eFavouriteOthers + 1; i--)
		::DeleteMenuItem(menuH, i);

	short index = CMailAccountManager::eFavouriteOthers + 1;
	for(CFavouriteItemList::const_iterator iter = mCopyPrefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
			iter != mCopyPrefs->mFavourites.GetValue().end(); iter++, index++)
		::AppendItemToMenu(menuH, index, (*iter).GetName());

	// Force max/min update
	mApplyToCabinet->SetMenuMinMax();
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

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::NewAlertStyleName", new_name))
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

				// Insert item
				MenuHandle menuH = mAlertStylePopup->GetMacMenuH();
				::AppendItemToMenu(menuH, mCopyPrefs->mMailNotification.GetValue().size() + eAlertStylePopup_Off, new_name);

				// Force max/min update
				mAlertStylePopup->SetMenuMinMax();

				// Set new values
				mStyleValue = index;
				mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
				break;
			}
			else
				mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
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

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameAlertStyle", new_name))
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
		mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
	}
}

// Delete accoount
void CPrefsAlertsMessage::DoDeleteStyle(void)
{
	// Prevent delete of last alert style
	if (mCopyPrefs->mMailNotification.GetValue().size() == 1)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoDeleteAlertStyle");
		mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
		return;
	}

	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::DeleteAlertStyle") == CErrorHandler::Ok)
	{
		// Get the dialog
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
		while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
			prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

		// Erase from prefs
		mCopyPrefs->mMailNotification.Value().erase(mCopyPrefs->mMailNotification.Value().begin() + mStyleValue);
		mCopyPrefs->mMailNotification.SetDirty();

		// Remove from menu
		MenuHandle menuH = mAlertStylePopup->GetMacMenuH();
		::DeleteMenuItem(menuH, mStyleValue + eAlertStylePopup);

		// Adjust to new value
		mStyleValue--;
		if (mStyleValue < 0)
			mStyleValue = 0;

		// Update items
		if (mStyleValue >= 0)
			mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
	}
	else
		mAlertStylePopup->SetValue(mStyleValue + eAlertStylePopup);
}

// Set alert details
void CPrefsAlertsMessage::SetNotifaction(void)
{
	const CMailNotification& notify = mCopyPrefs->mMailNotification.GetValue().at(mStyleValue);

	StopListening();

	// Copy text to edit fields
	if (notify.IsEnabled())
	{
		mCheckOnce->SetValue(notify.GetCheckOnce() ? 1 : 0);
		mCheckEvery->SetValue(notify.GetCheckOnce() ? 0 : 1);
	}
	else
		mCheckNever->SetValue(1);

	mCheckInterval->SetNumber(notify.GetCheckInterval());
	if (notify.IsEnabled())
		mCheckInterval->Enable();
	else
		mCheckInterval->Disable();

	mCheckNew->SetValue(notify.GetCheckNew() ? 1 : 0);
	mCheckAll->SetValue(notify.GetCheckNew() ? 0 : 1);

	mNewMailAlert->SetValue(notify.DoShowAlertForeground() ? 1 : 0);
	mNewMailBackAlert->SetValue(notify.DoShowAlertBackground() ? 1 : 0);
	mNewMailFlashIcon->SetValue(notify.DoFlashIcon() ? 1 : 0);

	mNewMailPlaySound->SetValue(notify.DoPlaySound());
	mNewMailSound->SetName(notify.GetSoundID());
	if (notify.DoPlaySound())
		mNewMailSound->Enable();
	else
		mNewMailSound->Disable();

	mNewMailSpeak->SetValue(notify.DoSpeakText() ? 1 : 0);
	mNewMailSpeakText->SetText(notify.GetTextToSpeak());
	if (!notify.DoSpeakText())
		mNewMailSpeakText->Disable();

	mNewMailOpen->SetValue(notify.GetOpenMbox() ? 1 : 0);

	// Set the cabinet
	unsigned long index = notify.GetFavouriteIndex(mCopyPrefs);
	mApplyToCabinet->SetValue(index + 1);

	// Toggle IC display
	ToggleICDisplay(mCopyPrefs->mUse_IC.GetValue());

	StartListening();
}

// Update current account
void CPrefsAlertsMessage::UpdateNotifaction(void)
{
	CMailNotification& notify = mCopyPrefs->mMailNotification.Value().at(mStyleValue);

	// Make copy to look for changes
	CMailNotification copy(notify);

	// Copy info from panel into prefs
	notify.Enable(mCheckNever->GetValue() == 0);
	notify.SetCheckOnce(mCheckOnce->GetValue() == 1);
	notify.SetCheckInterval(mCheckInterval->GetNumber());

	notify.SetCheckNew(mCheckNew->GetValue() == 1);

	notify.SetShowAlertForeground(mNewMailAlert->GetValue()==1);
	notify.SetShowAlertBackground(mNewMailBackAlert->GetValue()==1);
	notify.SetFlashIcon(mNewMailFlashIcon->GetValue()==1);

	notify.SetPlaySound(mNewMailPlaySound->GetValue());
	cdstring snd;
	mNewMailSound->GetName(snd);
	notify.SetSoundID(snd);

	notify.SetSpeakText(mNewMailSpeak->GetValue());
	notify.SetTextToSpeak(mNewMailSpeakText->GetText());

	notify.SetOpenMbox(mNewMailOpen->GetValue()==1);

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
