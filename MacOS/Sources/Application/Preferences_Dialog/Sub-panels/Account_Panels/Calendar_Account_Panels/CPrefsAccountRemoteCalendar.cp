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


// Source for CPrefsAccountRemoteCalendar class

#include "CPrefsAccountRemoteCalendar.h"

#include "CAdminLock.h"
#include "CCalendarAccount.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountRemoteCalendar::CPrefsAccountRemoteCalendar()
{
}

// Constructor from stream
CPrefsAccountRemoteCalendar::CPrefsAccountRemoteCalendar(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAccountRemoteCalendar::~CPrefsAccountRemoteCalendar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountRemoteCalendar::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLogonAtStartup = (LCheckBox*) FindPaneByID(paneid_AccountRemoteCalendarLoginAtStart);
	mDisconnected = (LCheckBox*) FindPaneByID(paneid_AccountRemoteCalendarDisconnected);
	mBaseRURL = (CTextFieldX*) FindPaneByID(paneid_AccountRemoteCalendarBaseRURL);
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Disable();
}

// Set prefs
void CPrefsAccountRemoteCalendar::SetData(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info
	mLogonAtStartup->SetValue(account->GetLogonAtStart());
	mDisconnected->SetValue(account->GetDisconnected());
	mBaseRURL->SetText(account->GetBaseRURL());
}

// Force update of prefs
void CPrefsAccountRemoteCalendar::UpdateData(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->GetValue() == 1);
	account->SetDisconnected(mDisconnected->GetValue() == 1);
	account->SetBaseRURL(mBaseRURL->GetText());
}
