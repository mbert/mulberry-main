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


// Source for CPrefsLocalCalendarAccount class

#include "CPrefsLocalCalendarAccount.h"

#include "CPrefsAccountExtras.h"
#include "CPrefsAccountLocal.h"
#include "CPrefsSubTab.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsLocalCalendarAccount::CPrefsLocalCalendarAccount()
{
}

// Constructor from stream
CPrefsLocalCalendarAccount::CPrefsLocalCalendarAccount(LStream *inStream)
		: CPrefsAccountPanel(inStream)
{
}

// Default destructor
CPrefsLocalCalendarAccount::~CPrefsLocalCalendarAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLocalCalendarAccount::InitTabs()
{
	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAccountLocal);
	mTabs->AddPanel(paneid_PrefsAccountExtras);
}
