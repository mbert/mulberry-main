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


// Source for CSizePopup class

#include "CSizePopup.h"


// __________________________________________________________________________________________________
// C L A S S __ C S I Z E P O P U P
// __________________________________________________________________________________________________

MenuHandle CSizePopup::sSizeMenu = NULL;
MenuHandle CSizePopup::sSizePopup = NULL;

const ResIDT MENU_SizeMain = 162;
const ResIDT MENU_SizePopup = 3024;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSizePopup::CSizePopup(LStream *inStream)
		: CMenuPopup(inStream)
{
}

// Default destructor
CSizePopup::~CSizePopup()
{
}


void CSizePopup::FinishCreateSelf()
{
	if (mAttachMenu)
	{
		if (sSizeMenu == NULL)
		{
			sSizeMenu = ::GetMenu(MENU_SizeMain);
		}
		mMenu = sSizeMenu;
	}

	if (sSizePopup == NULL)
	{
		sSizePopup = ::GetMenu(MENU_SizePopup);
	}
	SetMacMenuH(sSizePopup, false);

	SetMenuMinMax();
	CMenuPopup::FinishCreateSelf();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set size
void CSizePopup::SetSize(SInt16 size)
{
	switch(size)
	{
	case 8:
		SetValue(eSizePopup8);
		break;

	case 9:
		SetValue(eSizePopup9);
		break;

	case 10:
		SetValue(eSizePopup10);
		break;

	case 11:
		SetValue(eSizePopup11);
		break;

	case 12:
		SetValue(eSizePopup12);
		break;

	case 13:
		SetValue(eSizePopup13);
		break;

	case 14:
		SetValue(eSizePopup14);
		break;

	case 16:
		SetValue(eSizePopup16);
		break;

	case 18:
		SetValue(eSizePopup18);
		break;

	case 20:
		SetValue(eSizePopup20);
		break;

	case 24:
		SetValue(eSizePopup24);
		break;

	default:
		SetValue(eSizePopupOther);
		break;
	}
}

// Get size
SInt16 CSizePopup::GetSize(void) const
{
	switch(GetValue())
	{
	case eSizePopup8:
		return 8;

	case eSizePopup9:
		return 9;

	case eSizePopup10:
		return 10;

	case eSizePopup11:
		return 11;

	case eSizePopup12:
		return 12;

	case eSizePopup13:
		return 13;

	case eSizePopup14:
		return 14;

	case eSizePopup16:
		return 16;

	case eSizePopup18:
		return 18;

	case eSizePopup20:
		return 20;

	case eSizePopup24:
		return 24;

	default:
		return 0;
	}
}

