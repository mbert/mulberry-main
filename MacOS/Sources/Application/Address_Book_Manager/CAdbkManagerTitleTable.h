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


// Header for CAdbkManagerTitleTable class

#ifndef __CADBKMANAGERTITLETABLE__MULBERRY__
#define __CADBKMANAGERTITLETABLE__MULBERRY__

#include "CTitleTableView.h"

// Constants

// Resources
const	ResIDT		MENU_AdbkManagerColumnChanger = 9500;
enum {
	cColumnAdbkOpen = 1,
	cColumnAdbkResolve,
	cColumnAdbkSearch,
	cColumnAdbkInsertAfter = cColumnAdbkSearch + 2,
	cColumnAdbkInsertBefore,
	cColumnAdbkDelete
};

// Classes
class	CAdbkManagerTitleTable : public CTitleTableView
{
public:
	enum { class_ID = 'AmTi' };

					CAdbkManagerTitleTable();
					CAdbkManagerTitleTable(LStream *inStream);
	virtual 		~CAdbkManagerTitleTable();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitColumnChanger(void);				// Init column changer

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Stop clicks
	virtual void	MenuChoice(short col, bool sort_col, short menu_item);
	virtual void	DrawItem(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw title

private:
	static MenuHandle	sColumnChanger;		// Column changer menu
};

#endif
