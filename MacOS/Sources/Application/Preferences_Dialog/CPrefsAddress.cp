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


// Source for CPrefsAddress class

#include "CPrefsAddress.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"
#include "CPrefsAccountLocal.h"
#include "CPrefsAddressExpansion.h"
#include "CPrefsAddressCapture.h"

#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddress::CPrefsAddress()
{
}

// Constructor from stream
CPrefsAddress::CPrefsAddress(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsAddress::~CPrefsAddress()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddress::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	mOptionKeyDown = (LRadioButton*) FindPaneByID(paneid_AddressOptionKeyDown);
	mOptionKeyUp = (LRadioButton*) FindPaneByID(paneid_AddressOptionKeyUp);

	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_AddressTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAddressExpansion);
	mTabs->AddPanel(paneid_PrefsAddressCapture);
	mTabs->AddPanel(paneid_PrefsAccountLocal);
	static_cast<CPrefsAccountLocal*>(mTabs->GetPanel(2))->SetLocalAddress();
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsAddressBtns);
}

// Handle buttons
void CPrefsAddress::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AddressTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

		default:
			break;
	}
}

// Toggle display of IC items
void CPrefsAddress::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsAddress::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	if (mCopyPrefs->mOptionKeyAddressDialog.GetValue())
		mOptionKeyDown->SetValue(1);
	else
		mOptionKeyUp->SetValue(1);

	// Set existing panel
	mTabs->SetData(mCopyPrefs);
}

// Force update of prefs
void CPrefsAddress::UpdatePrefs(void)
{
	// Copy info from panel into prefs
	mCopyPrefs->mOptionKeyAddressDialog.SetValue(mOptionKeyDown->GetValue()==1);

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);
}
