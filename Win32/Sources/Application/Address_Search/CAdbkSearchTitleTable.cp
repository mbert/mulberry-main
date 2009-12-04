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

BEGIN_MESSAGE_MAP(CAdbkSearchTitleTable, CTitleTable)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTitleTable::CAdbkSearchTitleTable()
{
	// Load strings if not already
	SetTitleInfo(false, true, "UI::Titles::AddrSearch", eAddrColumnMax, IDR_POPUP_ADBK_HEADER);
}

// Default destructor
CAdbkSearchTitleTable::~CAdbkSearchTitleTable()
{
}
