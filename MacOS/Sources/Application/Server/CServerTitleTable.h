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


// Header for CServerTitleTable class

#ifndef __CSERVERTITLETABLE__MULBERRY__
#define __CSERVERTITLETABLE__MULBERRY__

#include "CTitleTableView.h"

// Constants

// Resources
const	ResIDT		MENU_ServerColumnChanger = 4000;
enum {
	cColumnServerFlags = 1,
	cColumnServerTotal,
	cColumnServerNew,
	cColumnServerUnseen,
	cColumnServerAutoCheck,
	cColumnServerSize,
	cColumnServerInsertAfter = cColumnServerSize + 2,
	cColumnServerInsertBefore,
	cColumnServerDelete
};

// Classes
class	CServerTitleTable : public CTitleTableView
{
public:
	enum { class_ID = 'SeTi' };

					CServerTitleTable();
					CServerTitleTable(LStream *inStream);
	virtual 		~CServerTitleTable();

	virtual	void	GetTooltipText(cdstring& txt, const STableCell &inCell);				// Get text for current tooltip cell

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitColumnChanger(void);				// Init column changer

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Stop clicks
	virtual void	MenuChoice(short col, bool sort_col, short menu_item);
	virtual void	DrawItem(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw title

	virtual bool	RightJustify(int col_type);									// Check for right justification

private:
	static MenuHandle	sColumnChanger;		// Column changer menu
};

#endif
