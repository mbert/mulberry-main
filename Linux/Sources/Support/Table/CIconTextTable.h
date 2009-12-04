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
	CIconTextTable(JXScrollbarSet* scrollbarSet,
					  JXContainer* enclosure,
					  const HSizingOption hSizing, 
					  const VSizingOption vSizing,
					  const JCoordinate x, const JCoordinate y,
					  const JCoordinate w, const JCoordinate h);
	virtual	~CIconTextTable();

			void	SetTabSelection(bool tab_select, unsigned long select_item)
		{ mSelectWithTab = tab_select; mSelectItem = select_item; }

			void	SetIcon(TableIndexT row, unsigned long index);
			void	SetAllIcons(unsigned long index);
	virtual void	AddItem(const cdstring& item);
	virtual void	SetContents(const cdstrvect& items, const ulvector& states);

	virtual bool	HandleChar(const int key, const JXKeyModifiers& modifiers);

protected:
	bool			mSelectWithTab;
	unsigned long	mSelectItem;

	virtual void		DrawCell(JPainter* pDC, const STableCell& cell,
									const JRect& inLocalRect);					// Draw the string

	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);
};

#endif
