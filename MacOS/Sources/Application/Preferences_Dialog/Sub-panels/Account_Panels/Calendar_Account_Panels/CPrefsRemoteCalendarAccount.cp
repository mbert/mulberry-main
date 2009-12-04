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


// Source for CPrefsRemoteCalendarAccount class

#include "CPrefsRemoteCalendarAccount.h"

#include "CPreferences.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsAccountExtras.h"
#include "CPrefsAccountRemoteCalendar.h"
#include "CPrefsSubTab.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsRemoteCalendarAccount::CPrefsRemoteCalendarAccount()
{
}

// Constructor from stream
CPrefsRemoteCalendarAccount::CPrefsRemoteCalendarAccount(LStream *inStream)
		: CPrefsAccountPanel(inStream)
{
}

// Default destructor
CPrefsRemoteCalendarAccount::~CPrefsRemoteCalendarAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsRemoteCalendarAccount::InitTabs()
{
	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAccountAuth);
	mTabs->AddPanel(paneid_PrefsAccountRemoteCalendar);
	mTabs->AddPanel(paneid_PrefsAccountExtras);
}
