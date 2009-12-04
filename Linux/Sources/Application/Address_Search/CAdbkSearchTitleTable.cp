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

#include "CTableWindow.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTitleTable::CAdbkSearchTitleTable(JXScrollbarSet* scrollbarSet,	
										 JXContainer* enclosure,	
										 const HSizingOption hSizing, 
										 const VSizingOption vSizing,
										 const JCoordinate x, 
										 const JCoordinate y,
										 const JCoordinate w, 
										 const JCoordinate h)
  : CTitleTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mWindow = NULL;
}

// Default destructor
CAdbkSearchTitleTable::~CAdbkSearchTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAdbkSearchTitleTable::OnCreate(void)
{
	// Do inherited
	CTitleTable::OnCreate();

	cdstring menu = "Full Name (Smart Sort) %r | Full Name (Simple Sort) %r | Nick-Name %r | Email Address %r | Company %r | Address %r |"
				" Phone Work %r | Phone Home %r | Fax %r | URLs %r | Notes %r %l |"
				" Insert Column after | Insert Column before | Delete Column";
	SetTitleInfo(false, true, "UI::Titles::AddrSearch", eAddrColumnMax, menu);
}

// Draw the titles
void CAdbkSearchTitleTable::MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item)
{
	CTableWindow* window = 
		dynamic_cast<CTableWindow*>(GetWindow()->GetDirector());
	assert(window);

	// Handle menu result
	switch (menu_item)
	{
	case kPopupName:
	case kPopupNameLastFirst:
	case kPopupNickName:
	case kPopupEmail:
	case kPopupCompany:
	case kPopupAddress:
	case kPopupPhoneWork:
	case kPopupPhoneHome:
	case kPopupFax:
	case kPopupURLs:
	case kPopupNotes:
		window->SetColumnType(col, eAddrColumnName + menu_item - kPopupName);
		break;
	
	case kPopupInsertAfter:
		window->InsertColumn(col + 1);
		break;
	
	case kPopupInsertBefore:
		window->InsertColumn(col);
		break;
	
	case kPopupDelete:
		window->DeleteColumn(col);
		break;			
	}
}
