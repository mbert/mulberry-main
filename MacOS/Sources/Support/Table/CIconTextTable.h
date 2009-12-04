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


// Header for CIconTextTable class

#ifndef __CICONTEXTTABLE__MULBERRY__
#define __CICONTEXTTABLE__MULBERRY__

#include "CTextTable.h"

// Classes

class CIconTextTable : public CTextTable
{
public:
	enum { class_ID = 'TiTb' };

					CIconTextTable();
					CIconTextTable(LStream *inStream);
	virtual 		~CIconTextTable();

			void	SetTabSelection(bool tab_select, ResIDT select_item)
		{ mSelectWithTab = tab_select; mSelectItem = select_item; }

			void	SetIcon(TableIndexT row, ResIDT index);
			void	SetAllIcons(ResIDT index);
	virtual void	AddItem(const cdstring& item);
	virtual void	SetContents(const cdstrvect& items, const ulvector& states);

protected:
	bool	mSelectWithTab;
	ResIDT	mSelectItem;

	virtual void		FinishCreateSelf(void);							// Get details of sub-panes

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void		DrawSelf();
	virtual void		DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);		// Draw the address
	virtual void	HiliteCellActively(const STableCell &inCell,
										Boolean inHilite);
	virtual void	HiliteCellInactively(const STableCell &inCell,
										Boolean inHilite);

private:
			void		InitIconTextTable(void);
};

#endif
