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


// Source for CPrefsAddressLDAPOptions class

#include "CPrefsAddressLDAPOptions.h"

#include "CAddressAccount.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressLDAPOptions::CPrefsAddressLDAPOptions()
{
}

// Constructor from stream
CPrefsAddressLDAPOptions::CPrefsAddressLDAPOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressLDAPOptions::~CPrefsAddressLDAPOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAPOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLDAPUseExpansion = (LCheckBox*) FindPaneByID(paneid_LDAPAccountLDAPUseExpansion);
	mLDAPSearch = (LCheckBox*) FindPaneByID(paneid_LDAPAccountLDAPSearch);

}

// Set prefs
void CPrefsAddressLDAPOptions::SetData(void* data)
{
	LStr255 copyStr;

	CAddressAccount* account = (CAddressAccount*) data;

	mLDAPUseExpansion->SetValue(account->GetLDAPUseExpansion());
	mLDAPSearch->SetValue(account->GetLDAPSearch());
}

// Force update of prefs
void CPrefsAddressLDAPOptions::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPUseExpansion(mLDAPUseExpansion->GetValue()==1);
	account->SetLDAPSearch(mLDAPSearch->GetValue()==1);
}
