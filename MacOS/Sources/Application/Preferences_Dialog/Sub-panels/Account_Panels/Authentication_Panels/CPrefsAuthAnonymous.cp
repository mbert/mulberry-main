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


// Source for CPrefsAuthAnonymous class

#include "CPrefsAuthAnonymous.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAuthAnonymous::CPrefsAuthAnonymous()
{
}

// Constructor from stream
CPrefsAuthAnonymous::CPrefsAuthAnonymous(LStream *inStream)
		: CPrefsAuthPanel(inStream)
{
}

// Default destructor
CPrefsAuthAnonymous::~CPrefsAuthAnonymous()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAuthAnonymous::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsAuthPanel::FinishCreateSelf();
}

// Toggle display of IC
void CPrefsAuthAnonymous::ToggleICDisplay(bool IC_on)
{
	// Does nothing!
}

// Set prefs
void CPrefsAuthAnonymous::SetAuth(CAuthenticator* auth)
{
	// Does nothing!
}

// Force update of prefs
void CPrefsAuthAnonymous::UpdateAuth(CAuthenticator* auth)
{
	// Does nothing!
}

// Change items states
void CPrefsAuthAnonymous::UpdateItems(bool enable)
{
	// Does nothing!
}
