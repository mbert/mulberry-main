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


// Source for CACLTitleTable class


#include "CACLTitleTable.h"

#include "CMulberryCommon.h"
#include "CXStringResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C A C L T I T L E T A B L E
// __________________________________________________________________________________________________



// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CACLTitleTable::CACLTitleTable(JXScrollbarSet* scrollbarSet,
	      JXContainer* enclosure,
	      const HSizingOption hSizing, 
	      const VSizingOption vSizing,
	      const JCoordinate x, const JCoordinate y,
	      const JCoordinate w, const JCoordinate h)
  : CSimpleTitleTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}

// Default destructor
CACLTitleTable::~CACLTitleTable()
{
}

void CACLTitleTable::DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &cellRect)
{
	// Only name is drawn as text
	if (col_info.column_type == 1)
	{
		::DrawClippedStringUTF8(p, rsrc::GetString("UI::Titles::ACL"), JPoint(cellRect.left + 6, cellRect.top + 1), cellRect, eDrawString_Left);
	}
	else
	{
		TableIndexT col = col_info.column_type;
	
		// Adjust for adbk/calendar ACLs
		if (!mMbox)
		{
			if (col > 4) col += 3;
			else if (col > 3) col += 1;
		}
		DrawIcon(p, col_info, IDI_ACL_LOOKUP + col - 2, cellRect);
	}
}
