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


// Source for CPrefsAddressIMSP class

#include "CPrefsAddressIMSP.h"

#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressIMSP::CPrefsAddressIMSP()
{
}

// Constructor from stream
CPrefsAddressIMSP::CPrefsAddressIMSP(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressIMSP::~CPrefsAddressIMSP()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressIMSP::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLogonAtStartup = (LCheckBox*) FindPaneByID(paneid_AddressIMSPLoginAtStart);
	mDisconnected = (LCheckBox*) FindPaneByID(paneid_AddressIMSPDisconnected);
	mBaseRURL = (CTextFieldX*) FindPaneByID(paneid_AddressIMSPBaseRURL);
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Disable();
}

// Set prefs
void CPrefsAddressIMSP::SetData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLogonAtStartup->SetValue(account->GetLogonAtStart());
	mDisconnected->SetValue(account->GetDisconnected());
	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		mBaseRURL->SetText(account->GetBaseRURL());
	else
	{
		FindPaneByID(paneid_AddressIMSPBaseRURLText)->Hide();
		mBaseRURL->Hide();
	}
}

// Force update of prefs
void CPrefsAddressIMSP::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->GetValue()==1);
	account->SetDisconnected(mDisconnected->GetValue()==1);
	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		account->SetBaseRURL(mBaseRURL->GetText());
}
