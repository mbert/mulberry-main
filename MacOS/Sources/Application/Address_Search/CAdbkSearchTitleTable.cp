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


// Source for CAdbkSearchTitleTable class

#include "CAdbkSearchTitleTable.h"

#include "CAdbkManagerTitleTable.h"
#include "CAdbkSearchWindow.h"


// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

MenuHandle CAdbkSearchTitleTable::sColumnChanger = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTitleTable::CAdbkSearchTitleTable()
{
}

// Constructor from stream
CAdbkSearchTitleTable::CAdbkSearchTitleTable(LStream *inStream)
	: CTitleTable(inStream)
{
}

// Default destructor
CAdbkSearchTitleTable::~CAdbkSearchTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAdbkSearchTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(false, true, "UI::Titles::AddrSearch", eAddrColumnMax, MENU_AdbkSearchColumnChanger);
	CTitleTable::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);
	InsertRows(1, 1, nil, 0, false);
	AdaptToNewSurroundings();
}

// Get menu
void CAdbkSearchTitleTable::InitColumnChanger(void)
{
	// Get menu handle if not already here
	if (!sColumnChanger)
	{
		sColumnChanger = ::GetMenu(mMenuID);
		ThrowIfResFail_(sColumnChanger);
		::InsertMenu(sColumnChanger, hierMenu);
	}

	mColumnChanger = sColumnChanger;
}

// Draw the titles
void CAdbkSearchTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
	case cColumnAddrTitleName:
	case cColumnAddrTitleNameLastFirst:
	case cColumnAddrTitleNickName:
	case cColumnAddrTitleEmail:
	case cColumnAddrTitleCompany:
	case cColumnAddrTitleAddress:
	case cColumnAddrTitlePhoneWork:
	case cColumnAddrTitlePhoneHome:
	case cColumnAddrTitleFax:
	case cColumnAddrTitleURLs:
	case cColumnAddrTitleNotes:
		((CAdbkSearchWindow*) mSuperView)->SetColumnType(col, eAddrColumnName + menu_item - 1);
		break;

	case cColumnAddrTitleInsertAfter:
		((CAdbkSearchWindow*) mSuperView)->InsertColumn(col + 1);
		break;

	case cColumnAddrTitleInsertBefore:
		((CAdbkSearchWindow*) mSuperView)->InsertColumn(col);
		break;

	case cColumnAddrTitleDelete:
		((CAdbkSearchWindow*) mSuperView)->DeleteColumn(col);
		break;
	}
}
