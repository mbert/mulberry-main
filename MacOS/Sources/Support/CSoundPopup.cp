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


// Source for CSoundPopup class

#include "CSoundPopup.h"

#include "CMulberryCommon.h"
#include "CSoundManager.h"

// __________________________________________________________________________________________________
// C L A S S __ C F O N T P O P U P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSoundPopup::CSoundPopup(LStream *inStream)
		: LPopupButton(inStream)
{
}

// Default destructor
CSoundPopup::~CSoundPopup()
{
}

void CSoundPopup::FinishCreateSelf()
{
	LPopupButton::FinishCreateSelf();
	
	// Add each named sound to list
	// Remove any existing items from main menu
	MenuHandle menuH = GetMacMenuH();
	while(::CountMenuItems(menuH))
		::DeleteMenuItem(menuH, 1);

	short index = 1;
	for(cdstrvect::const_iterator iter = CSoundManager::sSoundManager.GetSounds().begin();
			iter != CSoundManager::sSoundManager.GetSounds().end(); iter++, index++)
		::AppendItemToMenu(menuH, index, (*iter).c_str());

	// Force max/min update
	SetMenuMinMax();
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CSoundPopup::SetName(const cdstring& snd)
{
	MenuHandle	menuH = GetMacMenuH();
	short		numItems = ::CountMenuItems(menuH);

	for (short i = 1; i <= numItems; i++)
	{
		Str255	menuTxt;
		GetMenuItemText(i, menuTxt);

		if (snd == menuTxt)
		{
			SetValue(i);
			return;
		}
	}

	SetValue(0);
}

void CSoundPopup::GetName(cdstring& snd)
{
	Str255 mname;
	GetMenuItemText(GetValue(), mname);
	snd = mname;
}
