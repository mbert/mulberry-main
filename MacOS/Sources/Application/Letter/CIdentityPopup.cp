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


// Source for CIdentityPopup class

#include "CIdentityPopup.h"

#include "CBalloonDialog.h"
#include "CEditIdentities.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextListChoice.h"

// __________________________________________________________________________________________________
// C L A S S __ C A C L S T Y L E P O P U P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CIdentityPopup::CIdentityPopup(LStream *inStream)
		: LPopupButton(inStream)
{
	mHasCustom = true;
	mCustomIdentity = NULL;
}

// Default destructor
CIdentityPopup::~CIdentityPopup()
{
	delete mCustomIdentity;
	mCustomIdentity = NULL;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CIdentityPopup::SetValue(long value)
{
	// Cache previous value
	mOldValue = mValue;
	LPopupButton::SetValue(value);
	
	// Make sure custom is set
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
	{
		// Must have custom item
		if (mCustomIdentity == NULL)
		{
			// Create it and make it temporary and have it inherit from the default
			mCustomIdentity = new CIdentity;
			mCustomIdentity->SetTemp();
			mCustomIdentity->SetInherit(CPreferences::sPrefs->mIdentities.Value().front().GetIdentity());
		}
	}
	
	// Enable/disable edit/delete if custom is used
	if (mHasCustom)
	{
		if (GetValue() == eIdentityPopup_Custom)
		{
			::DisableMenuItem(GetMacMenuH(), eIdentityPopup_Edit);
			::DisableMenuItem(GetMacMenuH(), eIdentityPopup_Delete);
		}
		else
		{
			::EnableMenuItem(GetMacMenuH(), eIdentityPopup_Edit);
			::EnableMenuItem(GetMacMenuH(), eIdentityPopup_Delete);
		}
	}
}

void CIdentityPopup::SetIdentity(CPreferences* prefs, const CIdentity* id)
{
	// Find corresponding identity
	long pos = 0;
	long found_pos = 0;
	for(CIdentityList::const_iterator iter = prefs->mIdentities.GetValue().begin();
			iter != prefs->mIdentities.GetValue().end(); iter++, pos++)
	{
		if (&(*iter) == id)
		{
			found_pos = pos;
			break;
		}
	}

	// Set it in popup
	SetValue(FirstIndex() + found_pos);
}

void CIdentityPopup::SetIdentity(CPreferences* prefs, const cdstring& name)
{
	// Find corresponding identity
	long pos = 0;
	long found_pos = 0;
	for(CIdentityList::const_iterator iter = prefs->mIdentities.GetValue().begin();
			iter != prefs->mIdentities.GetValue().end(); iter++, pos++)
	{
		if ((*iter).GetIdentity() == name)
		{
			found_pos = pos;
			break;
		}
	}

	// Set it in popup
	SetValue(FirstIndex() + found_pos);
}

const CIdentity& CIdentityPopup::GetIdentity(CPreferences* prefs) const
{
	// Check for custom
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
		return (mCustomIdentity != NULL) ? *mCustomIdentity : prefs->mIdentities.GetValue()[0];
	else
		return prefs->mIdentities.GetValue()[GetValue() - FirstIndex()];
}

// Reset items
void CIdentityPopup::Reset(const CIdentityList& ids, bool custom)
{
	// Set custom state
	MenuHandle menuH = GetMacMenuH();
	if (custom ^ mHasCustom)
	{
		if (!custom)
		{
			::DeleteMenuItem(menuH, eIdentityPopup_Custom);
		}
		mHasCustom = custom;
	}

	// Delete previous items
	for(short i = ::CountMenuItems(menuH); i >= FirstIndex(); i--)
		::DeleteMenuItem(menuH, i);

	// Add each style item
	short menu_pos = FirstIndex();
	for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++, menu_pos++)
	{
		cdstring txt = (*iter).GetIdentity();
		if (mHasCustom && CPreferences::sPrefs->mDisplayIdentityFrom.GetValue())
		{
			// In custom mode we are in the draft window so display the identity name
			// and the email address
			txt += ":   ";
			txt += (*iter).GetFrom(true);
		}

		::AppendItemToMenu(menuH, menu_pos, txt);
	}

	// Disable delete if only one
	if (ids.size() > 1)
		::EnableItem(menuH, eIdentityPopup_Delete);
	else
		::DisableItem(menuH, eIdentityPopup_Delete);

	// Force max/min update
	SetMenuMinMax();
}

// Add new identity
void CIdentityPopup::DoNewIdentity(CPreferences* prefs)
{

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsEditIdentity, CMulberryApp::sApp);
		CIdentity new_identity;
		((CEditIdentities*) theHandler.GetDialog())->SetIdentity(prefs, &new_identity);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy changed identity
				((CEditIdentities*) theHandler.GetDialog())->GetIdentity(&new_identity);

				// Check validity
				if (new_identity.GetIdentity().empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
					continue;
				}

				// Add to prefs and popup
				prefs->mIdentities.Value().push_back(new_identity);
				prefs->mIdentities.SetDirty();

				cdstring txt = new_identity.GetIdentity();
				if (mHasCustom && CPreferences::sPrefs->mDisplayIdentityFrom.GetValue())
				{
					// In custom mode we are in the draft window so display the identity name
					// and the email address
					txt += ":   ";
					txt += new_identity.GetFrom(true);
				}

				// Insert item
				MenuHandle menuH = GetMacMenuH();
				::AppendItemToMenu(menuH, ::CountMenuItems(menuH) + 1, txt);

				SetMenuMinMax();

				// Always enable as there must now be more than one
				::EnableItem(menuH, eIdentityPopup_Delete);

				// Now force value change to new identity
				SetValue(::CountMenuItems(menuH));
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				// Reset to old value
				StopBroadcasting();
				SetValue(mOldValue);
				StartBroadcasting();
				break;
			}
		}
	}

}

// Edit identity
bool CIdentityPopup::DoEditIdentity(CPreferences* prefs)
{
	// Check for custom
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
		return EditCustomIdentity();

	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsEditIdentity, CMulberryApp::sApp);
		CIdentity& old_identity = prefs->mIdentities.Value()[mOldValue - FirstIndex()];
		((CEditIdentities*) theHandler.GetDialog())->SetIdentity(prefs, &old_identity);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy changed identity - init to old one to copy over fields not edited by the dialog
				CIdentity edit_identity(old_identity);
				((CEditIdentities*) theHandler.GetDialog())->GetIdentity(&edit_identity);

				// Check validity
				if (edit_identity.GetIdentity().empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
					continue;
				}

				// Carry on
				prefs->ReplaceIdentity(old_identity, edit_identity);

				// Reset menu title
				if (old_identity.GetIdentity().empty())
				{
					::SetMenuItemText(GetMacMenuH(), mOldValue, "\p?");
				}
				else
				{
					cdstring txt = old_identity.GetIdentity();
					if (mHasCustom && CPreferences::sPrefs->mDisplayIdentityFrom.GetValue())
					{
						// In custom mode we are in the draft window so display the identity name
						// and the email address
						txt += ":   ";
						txt += old_identity.GetFrom(true);
					}
					::SetMenuItemTextUTF8(GetMacMenuH(), mOldValue, txt);
				}

				// Reset to old value
				SetValue(mOldValue);

				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				// Reset to old value silently
				StopBroadcasting();
				SetValue(mOldValue);
				StartBroadcasting();

				break;
			}
		}
	}

	return result;
}

// Delete existing identity
void CIdentityPopup::DoDeleteIdentity(CPreferences* prefs)
{
	// Check for custom and ignore
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
		return;

	cdstrvect items;
	CIdentityList& ids = prefs->mIdentities.Value();
	for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
		items.push_back((*iter).GetIdentity());

	// Do the dialog
	ulvector styles;
	if (CTextListChoice::PoseDialog("Alerts::Preferences::DeleteIdsTitle", "Alerts::Preferences::DeleteIdsDesc", NULL, false, false, false, false, items, cdstring::null_str, styles, "Alerts::Preferences::DeleteIdsButton"))
	{
		// Remove in reverse order to stay in sync
		for(ulvector::reverse_iterator iter = styles.rbegin(); iter != styles.rend(); iter++)
		{
			// Only delete if more than one left
			if (ids.size() == 1)
				break;

			prefs->DeleteIdentity(&(*(ids.begin() + *iter)));

			if (*iter + FirstIndex() <= mOldValue)
				mOldValue--;
		}
		
		// Only delete if more than one left
		if (ids.size() == 1)
			::DisableMenuItem(GetMacMenuH(), (MenuItemIndex) eIdentityPopup_Delete);

		// Must sync first
		Reset(ids, mHasCustom);
		if (mOldValue != mValue)
			SetValue(mOldValue);
	}
	else
	{
		// Reset to old value silently
		StopBroadcasting();
		SetValue(mOldValue);
		StartBroadcasting();
	}
}

bool CIdentityPopup::EditCustomIdentity()
{
	// Must have custom item
	if (mCustomIdentity == NULL)
	{
		// Create it and make it temporary and have it inherit from the default
		mCustomIdentity = new CIdentity;
		mCustomIdentity->SetTemp();
		mCustomIdentity->SetInherit(CPreferences::sPrefs->mIdentities.Value().front().GetIdentity());
	}

	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsEditIdentity, CMulberryApp::sApp);
		((CEditIdentities*) theHandler.GetDialog())->SetIdentity(CPreferences::sPrefs, mCustomIdentity);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy changed identity - init to old one to copy over fields not edited by the dialog
				((CEditIdentities*) theHandler.GetDialog())->GetIdentity(mCustomIdentity);

				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
		}
	}

	return result;
}

unsigned long CIdentityPopup::GetCount() const
{
	// Set custom state
	MenuHandle menuH = GetMacMenuH();
	unsigned long i = ::CountMenuItems(menuH);
	return i - FirstIndex() + 1;
}
