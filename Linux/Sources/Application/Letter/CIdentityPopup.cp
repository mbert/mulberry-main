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

#include "CErrorHandler.h"
#include "CEditIdentities.h"
#include "CTextListChoice.h"

#include "CPreferences.h"

// __________________________________________________________________________________________________
// C L A S S __ C I D E N T I T Y P O P U P
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIdentityPopup::CIdentityPopup(const JCharacter* title,
									JXContainer* enclosure,
									 const HSizingOption hSizing,
									 const VSizingOption vSizing,
									 const JCoordinate x, const JCoordinate y,
									 const JCoordinate w, const JCoordinate h)
	: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
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

void CIdentityPopup::SetValue(JIndex value)
{
	// Cache previous value
	HPopupMenu::SetValue(value);
	
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
		SetItemEnable(eIdentityPopup_Edit, ((GetValue() == eIdentityPopup_Custom) ? kFalse : kTrue));
		SetItemEnable(eIdentityPopup_Delete, ((GetValue() == eIdentityPopup_Custom) ? kFalse : kTrue));
	}
}

void CIdentityPopup::SetIdentity(CPreferences* prefs, const CIdentity* id)
{
	// Find corresponding identity
	long pos = 0;
	long found_pos = 0;
	for(CIdentityList::const_iterator iter
				= prefs->mIdentities.GetValue().begin();
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
	return prefs->mIdentities.GetValue()[GetValue() - FirstIndex()];

	// Check for custom
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
		return (mCustomIdentity != NULL) ? *mCustomIdentity : prefs->mIdentities.GetValue()[0];
	else
		return prefs->mIdentities.GetValue()[GetValue() - FirstIndex()];
}

void CIdentityPopup::Reset(const CIdentityList& ids, bool custom)
{
	// Remove any existing items
	RemoveAllItems();

	// Now add current items
	SetMenuItems(custom ? "New Identity... | Edit Identity... | Delete Identity... | Custom Identity %l" : "New Identity... | Edit Identity... | Delete Identity... %l"); 
	mHasCustom = custom;
	for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
	{
		cdstring txt = (*iter).GetIdentity();
		if (mHasCustom && CPreferences::sPrefs->mDisplayIdentityFrom.GetValue())
		{
			// In custom mode we are in the draft window so display the identity name
			// and the email address
			txt += ":   ";
			txt += (*iter).GetFrom(true);
		}
		AppendItem(txt, kFalse, kTrue);
	}

	// Disable delete if only one
	if (ids.size() > 1)
		EnableItem(eIdentityPopup_Delete);
	else
		DisableItem(eIdentityPopup_Delete);
}

// Add new style
void CIdentityPopup::DoNewIdentity(CPreferences* prefs)
{
	CIdentity new_identity;
	while(true)
	{
		if (CEditIdentities::PoseDialog(prefs, &new_identity))
		{
			// Check validity
			if (new_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Add to prefs and popup
			prefs->mIdentities.Value().push_back(new_identity);
			prefs->mIdentities.SetDirty();

			// Insert item and select it and broadcast its change
			Reset(prefs->mIdentities.GetValue(), mHasCustom);
			SetValue(GetItemCount());
			BroadcastSelection(mValue, kJFalse);
			break;
		}
		else
			break;
	}
}

// Delete existing styles
bool CIdentityPopup::DoEditIdentity(CPreferences* prefs)
{
	// Check for custom
	if (mHasCustom && (GetValue() == eIdentityPopup_Custom))
		return EditCustomIdentity();

	bool result = false;

	while(true)
	{
		// Create the dialog
		CIdentity& old_identity = prefs->mIdentities.Value()[mValue - FirstIndex()];
		CIdentity edit_identity(old_identity);
		if (CEditIdentities::PoseDialog(prefs, &edit_identity))
		{
			// Check validity
			if (edit_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Carry on
			prefs->ReplaceIdentity(old_identity, edit_identity);
			Reset(prefs->mIdentities.GetValue(), mHasCustom);

			// Force reset of identity by forcing broadcast of change
			BroadcastSelection(mValue, kJFalse);

			result = true;
			break;
		}
		else
			break;
	}

	return result;
}

// Delete existing styles
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
		JIndex new_value = mValue;

		// Remove in reverse order to stay in sync
		for(ulvector::reverse_iterator iter = styles.rbegin(); iter != styles.rend(); iter++)
		{
			// Only delete if more than one left
			if (prefs->mIdentities.GetValue().size() == 1)
				break;

			prefs->DeleteIdentity(&(*(prefs->mIdentities.Value().begin() + *iter)));

			if (*iter + FirstIndex() <= new_value)
				new_value--;
		}
		
		// Only delete if more than one left
		if (ids.size() == 1)
			DisableItem(eIdentityPopup_Delete);

		// Must sync first
		Reset(ids, mHasCustom);
		if (new_value != mValue)
		{
			SetValue(new_value);
			BroadcastSelection(mValue, &kJFalse);
		}
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

	// Let Dialog process events
	if (CEditIdentities::PoseDialog(CPreferences::sPrefs, mCustomIdentity))
	{
		result = true;
	}
	
	return result;
}

unsigned long CIdentityPopup::GetCount() const
{
	// Set custom state
	return GetItemCount() - FirstIndex();
}

void CIdentityPopup::Receive(JBroadcaster* sender, const Message& message)
{
  if (sender == this && message.Is(JXMenu::kNeedsUpdate))
  {
  		EnableAll();

		// Disable delete if only one
		if (GetItemCount() == FirstIndex()) 
			DisableItem(eIdentityPopup_Delete);

		CheckItem(GetValue());
  }
  else
    HPopupMenu::Receive(sender, message);
}

void
CIdentityPopup::AdjustPopupChoiceTitle
	(
	const JIndex index
	)
{
	//We only want to change what is checked if it is not one of the commands
	//We still want to act on that action (and will elsewhere) but here
	//we don't want to update the title or value, so we don't call the parent
	//AdjustPopupChoiceTitle and it never knows anything was selected.
	if (index >= FirstIndex()) {
		HPopupMenu::AdjustPopupChoiceTitle(index);
	} 
}
