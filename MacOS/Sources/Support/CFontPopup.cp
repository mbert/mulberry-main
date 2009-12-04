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


// Source for CFontPopup class

#include "CFontPopup.h"
#include "CPreferences.h"



// __________________________________________________________________________________________________
// C L A S S __ C F O N T P O P U P
// __________________________________________________________________________________________________

MenuHandle CFontPopup::sFontMenu = NULL;
MenuHandle CFontPopup::sFontPopup = NULL;

const ResIDT MENU_FontMain = 161;
const ResIDT MENU_FontPopup = 3023;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CFontPopup::CFontPopup(LStream *inStream)
		: CMenuPopup(inStream)
{
}

// Default destructor
CFontPopup::~CFontPopup()
{
}

void CFontPopup::FinishCreateSelf()
{
	if (mAttachMenu)
	{
		if (sFontMenu == NULL)
		{
			sFontMenu = ::GetMenu(MENU_FontMain);
			::AppendResMenu(sFontMenu, 'FONT');
		}
		mMenu = sFontMenu;
	}
	if (sFontPopup == NULL)
	{
		sFontPopup = ::GetMenu(MENU_FontPopup);
		::AppendResMenu(sFontPopup, 'FONT');
	}
	SetMacMenuH(sFontPopup, false);

	SetMenuMinMax();
	defaultValue = 20;
	CMenuPopup::FinishCreateSelf();
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CFontPopup::SetFontName(const Str255 fontName)
{
	MenuHandle	menuH = GetMacMenuH();
	short		numItems = ::CountMenuItems(menuH);

	for (short i = 1; i <= numItems; i++) {
		Str255	menuTxt;

		GetMenuItemText(i, menuTxt);

		if (::PLstrcmp(fontName, menuTxt) == 0)
		{
			SetValue(i);
			return;
		}
	}

	SetValue(0);
}

void CFontPopup::GetFontName(Str255 fontName)
{
	GetMenuItemText(GetValue(), fontName);
}
