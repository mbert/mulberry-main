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


// Source for CPrefsAccountIMAP class

#include "CPrefsAccountIMAP.h"

#include "CAdminLock.h"
#include "CMailAccount.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountIMAP::CPrefsAccountIMAP()
{
}

// Constructor from stream
CPrefsAccountIMAP::CPrefsAccountIMAP(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAccountIMAP::~CPrefsAccountIMAP()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountIMAP::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLogonAtStartup = (LCheckBox*) FindPaneByID(paneid_AccountIMAPLoginAtStart);
	mDirectorySeparator = (CTextFieldX*) FindPaneByID(paneid_AccountIMAPDirSeparator);
	mAutoNamespace = (LCheckBox*) FindPaneByID(paneid_AccountIMAPAutoNamespace);
	mDisconnected = (LCheckBox*) FindPaneByID(paneid_AccountIMAPDisconnected);
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Disable();
}

// Set prefs
void CPrefsAccountIMAP::SetData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info
	mLogonAtStartup->SetValue(account->GetLogonAtStart());
	cdstring temp(account->GetDirDelim());
	mDirectorySeparator->SetText(temp);
	mAutoNamespace->SetValue(account->GetAutoNamespace());
	mDisconnected->SetValue(account->GetDisconnected());
}

// Force update of prefs
void CPrefsAccountIMAP::UpdateData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->GetValue()==1);

	account->SetDirDelim(mDirectorySeparator->GetText()[0UL]);

	account->SetAutoNamespace(mAutoNamespace->GetValue()==1);
	account->SetDisconnected(mDisconnected->GetValue()==1);
}
