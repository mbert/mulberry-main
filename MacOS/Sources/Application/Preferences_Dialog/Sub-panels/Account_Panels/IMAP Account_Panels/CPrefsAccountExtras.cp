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


// Source for CPrefsAccountExtras class

#include "CPrefsAccountExtras.h"

#include "CCalendarAccount.h"
#include "CIdentityPopup.h"
#include "CMailAccount.h"
#include "CPreferencesDialog.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountExtras::CPrefsAccountExtras()
{
}

// Constructor from stream
CPrefsAccountExtras::CPrefsAccountExtras(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAccountExtras::~CPrefsAccountExtras()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountExtras::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mTieIdentity = (LCheckBox*) FindPaneByID(paneid_AccountExtrasTieIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_AccountExtrasIdentityPopup);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAccountExtrasBtns);
}

// Handle buttons
void CPrefsAccountExtras::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_AccountExtrasTieIdentity:
		if (*(long*) ioParam)
			mIdentityPopup->Enable();
		else
			mIdentityPopup->Disable();
		break;

	case msg_AccountExtrasIdentityPopup:
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
		while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
			prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
		CPreferences* new_prefs = prefs_dlog->GetNewPrefs();

		switch(*(long*) ioParam)
		{
		// New identity wanted
		case eIdentityPopup_New:
			mIdentityPopup->DoNewIdentity(new_prefs);
			break;

		// New identity wanted
		case eIdentityPopup_Edit:
			mIdentityPopup->DoEditIdentity(new_prefs);
			break;

		// Delete existing identity
		case eIdentityPopup_Delete:
			mIdentityPopup->DoDeleteIdentity(new_prefs);
			break;

		// Select an identity
		default:
			// We do nothing here
			break;
		}
		break;
	}
}

// Set prefs
void CPrefsAccountExtras::SetData(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	bool tie = false;
	cdstring id;
	if (maccount != NULL)
	{
		tie = maccount->GetTieIdentity();
		id = maccount->GetTiedIdentity();
	}
	else if (caccount != NULL)
	{
		tie = caccount->GetTieIdentity();
		id = caccount->GetTiedIdentity();
	}

	// Get new prefs
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	CPreferences* new_prefs = prefs_dlog->GetNewPrefs();
	mIdentityPopup->Reset(new_prefs->mIdentities.GetValue());

	// Set first identity
	mTieIdentity->SetValue(tie);

	// Set it in popup
	mIdentityPopup->SetIdentity(new_prefs, id);

	// Disable if not in use
	if (!tie)
		mIdentityPopup->Disable();
}

// Force update of prefs
void CPrefsAccountExtras::UpdateData(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	// Get new prefs
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	CPreferences* new_prefs = prefs_dlog->GetNewPrefs();

	if (maccount != NULL)
	{
		maccount->SetTieIdentity(mTieIdentity->GetValue());
		maccount->SetTiedIdentity(mIdentityPopup->GetIdentity(new_prefs).GetIdentity());
	}
	else if (caccount != NULL)
	{
		caccount->SetTieIdentity(mTieIdentity->GetValue());
		caccount->SetTiedIdentity(mIdentityPopup->GetIdentity(new_prefs).GetIdentity());
	}
}
