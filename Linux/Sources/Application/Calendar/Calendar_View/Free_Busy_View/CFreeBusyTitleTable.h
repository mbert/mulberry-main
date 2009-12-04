/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#ifndef H_CFreeBusyTitleTable
#define H_CFreeBusyTitleTable

#include "CSimpleTitleTable.h"

#include "cdstring.h"

class CFreeBusyTable;

// ===========================================================================
//	MonthTitleTable

class	CFreeBusyTitleTable : public CSimpleTitleTable
{
public:
	enum
	{
		eBroadcast_DblClick = 'DWcl'		// Data: iCal::CICalendarDateTime*
	};

			CFreeBusyTitleTable(JXScrollbarSet* scrollbarSet,
			  JXContainer* enclosure,
			  const HSizingOption hSizing, const VSizingOption vSizing,
			  const JCoordinate x, const JCoordinate y,
			  const JCoordinate w, const JCoordinate h);
	virtual	~CFreeBusyTitleTable();
	
	virtual void	OnCreate();					// Do odds & ends

	void SetTable(CFreeBusyTable* table)
	{
		mTable = table;
		TableChanged();
	}

	void TableChanged();

protected:
	CFreeBusyTable*				mTable;
	cdstrvect					mTitles;

	virtual void	DrawCell(JPainter* p, const STableCell& cell, const JRect &cellRect);					// Draw cell

private:	
			void	GenerateTitles(JPainter* p);

};

#endif
