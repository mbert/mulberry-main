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


// Source for CMenuPopup class

#include "CMenuPopup.h"


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMenuPopup::CMenuPopup(LStream *inStream)
		: LPopupButton(inStream)
{
	*inStream >> mAttachMenu;

	savedValue = -1;
	defaultValue = 1;

	mMenu = NULL;
}

// Default destructor
CMenuPopup::~CMenuPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CMenuPopup::Ambiguate()
{
		// ¥ Get the current item setup in the menu
		if (mMenu)
		{
			SetCurrentMenuItem(0);
		}

		// ¥ Call our superclass to handle the setting of the value
		mValue = 0;

		// ¥ Now we need to get the popup redrawn so that the change
		// will be seen
		Draw ( NULL );

		ClearMarks();
}




// Delete mark after popup
void CMenuPopup::SetValue(SInt32 inValue)
{
	bool different = (mValue != inValue);

	LPopupButton::SetValue(inValue);

	SInt16 mark = checkMark;

	if(mMenu)
	{
		SInt32 max = ::CountMenuItems(mMenu);

		for(int i = 1; i <= max; i++){
			::SetItemMark (mMenu, i, 0);
		}

		::SetItemMark(mMenu, GetValue(), mark);
	}
}


void CMenuPopup::SaveState()
{
	savedValue = mValue;
}

void CMenuPopup::RestoreState()
{
	if (savedValue >= 0)
	{
		SInt32 max = ::CountMenuItems(GetMacMenuH());

		for(int i = 1; i <= max; i++)
		{
			if(mMenu)
				::SetItemMark (mMenu, i, 0);
			::SetItemMark (GetMacMenuH(), i, 0);
		}
		mValue = savedValue;
		SetCurrentMenuItem(savedValue);
		SInt16 mark = checkMark;
		if (mMenu)
			::SetItemMark(mMenu, mValue, mark);
		::SetItemMark(GetMacMenuH(), mValue, mark);

	}
}

void CMenuPopup::ClearMarks()
{
	SInt32 max = ::CountMenuItems(GetMacMenuH());

	for(int i = 1; i <= max; i++)
	{
		if (mMenu)
			::SetItemMark (mMenu, i, 0);
	}
}

void CMenuPopup::FinishCreateSelf()
{
	SInt32 max = ::CountMenuItems(GetMacMenuH());

	for(int i = 1; i <= max; i++)
	{
		if (mMenu)
			::SetItemMark(mMenu, i, 0);
		::SetItemMark(GetMacMenuH(), i, 0);
	}
}