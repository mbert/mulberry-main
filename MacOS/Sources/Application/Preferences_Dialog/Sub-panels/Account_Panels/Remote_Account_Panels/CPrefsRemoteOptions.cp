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


// Source for CPrefsRemoteOptions class

#include "CPrefsRemoteOptions.h"

#include "COptionsAccount.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsRemoteOptions::CPrefsRemoteOptions()
{
}

// Constructor from stream
CPrefsRemoteOptions::CPrefsRemoteOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsRemoteOptions::~CPrefsRemoteOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsRemoteOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mUseRemote = (LCheckBox*) FindPaneByID(paneid_PrefsRemoteOptionsUse);
	mBaseRURL = (CTextFieldX*) FindPaneByID(paneid_PrefsRemoteOptionsBaseRURL);
}

// Set prefs
void CPrefsRemoteOptions::SetData(void* data)
{
	COptionsAccount* acct = (COptionsAccount*) data;

	// Copy info
	mUseRemote->SetValue(acct->GetLogonAtStart());
	if (acct->GetServerType() == CINETAccount::eWebDAVPrefs)
		mBaseRURL->SetText(acct->GetBaseRURL());
	else
	{
		FindPaneByID(paneid_PrefsRemoteOptionsBaseRURLText)->Hide();
		mBaseRURL->Hide();
	}
}

// Force update of prefs
void CPrefsRemoteOptions::UpdateData(void* data)
{
	COptionsAccount* acct = (COptionsAccount*) data;

	acct->SetLoginAtStart(mUseRemote->GetValue()==1);
	if (acct->GetServerType() == CINETAccount::eWebDAVPrefs)
		acct->SetBaseRURL(mBaseRURL->GetText());
}
