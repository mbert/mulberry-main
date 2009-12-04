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
#include "CAdminLock.h"

#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAPOptions::OnCreate()
{
// begin JXLayout1

    mLDAPSearch =
        new JXTextCheckbox("Use when Searching", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 155,20);
    assert( mLDAPSearch != NULL );

    mLDAPUseExpansion =
        new JXTextCheckbox("Use with Address Expansion", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 190,20);
    assert( mLDAPUseExpansion != NULL );

// end JXLayout1

}

// Set prefs
void CPrefsAddressLDAPOptions::SetData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	mLDAPSearch->SetState(JBoolean(account->GetLDAPSearch()));
	mLDAPUseExpansion->SetState(JBoolean(account->GetLDAPUseExpansion()));
}

// Force update of prefs
bool CPrefsAddressLDAPOptions::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPSearch(mLDAPSearch->IsChecked());
	account->SetLDAPUseExpansion(mLDAPUseExpansion->IsChecked());
	
	return true;
}
