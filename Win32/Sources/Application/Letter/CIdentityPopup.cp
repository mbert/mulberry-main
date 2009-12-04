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
#include "CDeleteIdentityDialog.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

IMPLEMENT_DYNCREATE(CIdentityPopup, CPopupButton)

BEGIN_MESSAGE_MAP(CIdentityPopup, CPopupButton)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C I D E N T I T Y P O P U P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIdentityPopup::CIdentityPopup()
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

void CIdentityPopup::SetValue(UINT value)
{
	// Cache previous value
	mOldValue = mValue;
	CPopupButton::SetValue(value);
	
	// Make sure custom is set
	if (mHasCustom && (GetValue() == IDM_IDENTITY_CUSTOM))
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
		GetPopupMenu()->EnableMenuItem(IDM_IDENTITY_EDIT, ((GetValue() == IDM_IDENTITY_CUSTOM) ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		GetPopupMenu()->EnableMenuItem(IDM_IDENTITY_DELETE, ((GetValue() == IDM_IDENTITY_CUSTOM) ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
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
	SetValue(IDM_IDENTITYStart + found_pos);
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
	SetValue(IDM_IDENTITYStart + found_pos);
}

const CIdentity& CIdentityPopup::GetIdentity(CPreferences* prefs) const
{
	// Check for custom
	if (mHasCustom && (GetValue() == IDM_IDENTITY_CUSTOM))
		return (mCustomIdentity != NULL) ? *mCustomIdentity : prefs->mIdentities.GetValue()[0];
	else
		return prefs->mIdentities.GetValue()[GetValue() - IDM_IDENTITYStart];
}

void CIdentityPopup::Reset(const CIdentityList& ids, bool custom)
{
	CMenu* pPopup = GetPopupMenu();

	if (custom ^ mHasCustom)
	{
		if (!custom)
		{
			pPopup->RemoveMenu(eIdentityPopup_Custom, MF_BYPOSITION);
		}
		mHasCustom = custom;
	}

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = FirstIndex(); i < num_menu; i++)
		pPopup->RemoveMenu(FirstIndex(), MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_IDENTITYStart;
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

		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, txt);
	}
	
	// Disable delete if only one
	if (ids.size() > 1)
		pPopup->EnableMenuItem(IDM_IDENTITY_DELETE, MF_ENABLED | MF_BYCOMMAND);
	else
		pPopup->EnableMenuItem(IDM_IDENTITY_DELETE, MF_GRAYED | MF_BYCOMMAND);
}

// Add new style
void CIdentityPopup::DoNewIdentity(CPreferences* prefs)
{
	while(true)
	{
		// Create the dialog
		CEditIdentities dlog(CSDIFrame::GetAppTopWindow());
		CIdentity new_identity;
		dlog.SetIdentity(prefs, &new_identity);

		// Let Dialog process events
		if (dlog.DoModal() == IDOK)
		{
			// Copy changed identity
			dlog.GetIdentity(&new_identity);
			
			// Check validity
			if (new_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Add to prefs and popup
			((CIdentityList&) prefs->mIdentities.GetValue()).push_back(new_identity);
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
			CMenu* pPopup = GetPopupMenu();
			int menu_id = IDM_IDENTITYStart + pPopup->GetMenuItemCount() - FirstIndex();
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, txt);

			// Always enable delete as there must now be more than one
			pPopup->EnableMenuItem(IDM_IDENTITY_DELETE, MF_ENABLED | MF_BYCOMMAND);
			
			GetParent()->SendMessage(WM_COMMAND, menu_id);
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
	if (mHasCustom && (GetValue() == IDM_IDENTITY_CUSTOM))
		return EditCustomIdentity();

	bool result = false;

	while(true)
	{
		// Create the dialog
		CEditIdentities dlog(CSDIFrame::GetAppTopWindow());
		CIdentity& old_identity = ((CIdentityList&) prefs->mIdentities.GetValue())[GetValue() - IDM_IDENTITYStart];
		dlog.SetIdentity(prefs, &old_identity);

		// Let Dialog process events
		if (dlog.DoModal() == IDOK)
		{
			// Copy changed identity - init to old one to copy over fields not edited by the dialog
			CIdentity edit_identity(old_identity);
			dlog.GetIdentity(&edit_identity);

			// Check validity
			if (edit_identity.GetIdentity().empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoIdentityName");
				continue;
			}

			// Carry on
			prefs->ReplaceIdentity(old_identity, edit_identity);

			cdstring txt = edit_identity.GetIdentity();
			if (mHasCustom && CPreferences::sPrefs->mDisplayIdentityFrom.GetValue())
			{
				// In custom mode we are in the draft window so display the identity name
				// and the email address
				txt += ":   ";
				txt += edit_identity.GetFrom(true);
			}

			// Reset menu title
			CMenu* pPopup = GetPopupMenu();
			CUnicodeUtils::ModifyMenuUTF8(pPopup, GetValue(), MF_BYCOMMAND | MF_STRING, GetValue(), txt);
			RefreshValue();

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

	// Create the dialog
	CDeleteIdentityDialog dlog(CSDIFrame::GetAppTopWindow());

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		CMenu* pPopup = GetPopupMenu();
		UINT new_value = mValue;

		// Get selection from list
		ulvector& ids = dlog.mIdentitySelect;

		// Remove in reverse order to stay in sync
		for(ulvector::reverse_iterator iter = ids.rbegin(); iter != ids.rend(); iter++)
		{
			// Only delete if more than one left
			if (prefs->mIdentities.GetValue().size() == 1)
				break;

			prefs->DeleteIdentity(&(*(prefs->mIdentities.Value().begin() + *iter)));
			pPopup->RemoveMenu(*iter + IDM_IDENTITYStart, MF_BYCOMMAND);

			if (*iter + IDM_IDENTITYStart <= new_value)
				new_value--;
		}
		
		// Only delete if more than one left
		if (prefs->mIdentities.GetValue().size() == 1)
			pPopup->EnableMenuItem(IDM_IDENTITY_DELETE, MF_GRAYED | MF_BYCOMMAND);

		// Must sync first
		Reset(prefs->mIdentities.GetValue(), mHasCustom);
		SetValue(new_value);
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
		CEditIdentities dlog(CSDIFrame::GetAppTopWindow());
		dlog.SetIdentity(CPreferences::sPrefs, mCustomIdentity);

		// Let Dialog process events
		if (dlog.DoModal() == IDOK)
		{
			// Copy changed identity - init to old one to copy over fields not edited by the dialog
			dlog.GetIdentity(mCustomIdentity);

			result = true;
		}
	}
	
	return result;
}

unsigned long CIdentityPopup::GetCount() const
{
	// Set custom state
	const CMenu* pPopup = GetPopupMenu();
	short num_menu = pPopup->GetMenuItemCount();
	return num_menu - FirstIndex();
}
