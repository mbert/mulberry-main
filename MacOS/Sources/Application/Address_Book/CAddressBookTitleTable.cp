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


// Source for CAddressBookTitleTable class

#include "CAddressBookTitleTable.h"

#include "CAdbkSearchTitleTable.h"
#include "CTableView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

MenuHandle CAddressBookTitleTable::sColumnChanger = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressBookTitleTable::CAddressBookTitleTable()
{
}

// Constructor from stream
CAddressBookTitleTable::CAddressBookTitleTable(LStream *inStream)
	: CTitleTableView(inStream)
{
}

// Default destructor
CAddressBookTitleTable::~CAddressBookTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressBookTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(true, true, "UI::Titles::AddrSearch", eAddrColumnMax, MENU_AdbkSearchColumnChanger);
	CTitleTableView::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);
	InsertRows(1, 1, nil, 0, false);
	AdaptToNewSurroundings();
}

// Get menu
void CAddressBookTitleTable::InitColumnChanger(void)
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
void CAddressBookTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
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
		mTableView->SetColumnType(col, eAddrColumnName + menu_item - 1);
		break;

	case cColumnAddrTitleInsertAfter:
		mTableView->InsertColumn(col + 1);
		break;

	case cColumnAddrTitleInsertBefore:
		mTableView->InsertColumn(col);
		break;

	case cColumnAddrTitleDelete:
		mTableView->DeleteColumn(col);
			break;
	}
}
