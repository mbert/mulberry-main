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


// Source for CPrefsLDAPAccount class

#include "CPrefsLDAPAccount.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsAddressLDAPOptions.h"
#include "CPrefsAddressLDAP1.h"
#include "CPrefsAddressLDAP2.h"
#include "CPrefsSubTab.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsLDAPAccount::CPrefsLDAPAccount()
{
}

// Constructor from stream
CPrefsLDAPAccount::CPrefsLDAPAccount(LStream *inStream)
		: CPrefsAccountPanel(inStream)
{
}

// Default destructor
CPrefsLDAPAccount::~CPrefsLDAPAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLDAPAccount::InitTabs()
{
	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAccountAuth);
	mTabs->AddPanel(paneid_PrefsAddressLDAPOptions);
	mTabs->AddPanel(paneid_PrefsAddressLDAP1);
	mTabs->AddPanel(paneid_PrefsAddressLDAP2);
}
