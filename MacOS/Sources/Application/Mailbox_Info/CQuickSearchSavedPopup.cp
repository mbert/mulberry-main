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


// Source for CQuickSearchSavedPopup class

#include "CQuickSearchSavedPopup.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CQuickSearchSavedPopup::CQuickSearchSavedPopup(LStream *inStream)
		: LPopupButton(inStream)
{
}

// Default destructor
CQuickSearchSavedPopup::~CQuickSearchSavedPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

Boolean CQuickSearchSavedPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always sync menu state when clicked
	SyncMenu();
	
	// Do default
	return LPopupButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

void CQuickSearchSavedPopup::SyncMenu()
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(GetMacMenuH());
	for(short i = eFirst; i <= num_menu; i++)
		::DeleteMenuItem(GetMacMenuH(), eFirst);

	short index = eFirst;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++, index++)
		::AppendItemToMenu(GetMacMenuH(), index, (*iter)->GetName());

	// Force max/min update
	SetMenuMinMax();
}
