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

#ifndef H_CCALENDARSTORETITLETABLE
#define H_CCALENDARSTORETITLETABLE
#pragma once

#include "CTitleTableView.h"

const	ResIDT		MENU_CalendarStoreColumnChanger = 1850;
enum {
	cColumnCalendarStoreSubscribe = 1,
	cColumnCalendarStoreColour,
	cColumnCalendarStoreInsertAfter = cColumnCalendarStoreColour + 2,
	cColumnCalendarStoreInsertBefore,
	cColumnCalendarStoreDelete
};

// ===========================================================================
//	CalendarStoreTitleTable

class	CCalendarStoreTitleTable : public CTitleTableView {
public:
	enum { class_ID = 'CSTi' };

						CCalendarStoreTitleTable(LStream *inStream);
						
	virtual				~CCalendarStoreTitleTable();
	
protected:
	virtual void	FinishCreateSelf();					// Do odds & ends
	virtual void	InitColumnChanger();				// Init column changer menu

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Stop clicks
	virtual void	MenuChoice(short col, bool sort_col, short menu_item);
	virtual void	DrawItem(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw title

private:
	static MenuHandle	sColumnChanger;		// Column changer menu

};

#endif
