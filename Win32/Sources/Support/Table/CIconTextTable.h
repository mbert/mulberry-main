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
					CIconTextTable();
	virtual 		~CIconTextTable();

			void	SetTabSelection(bool tab_select, UINT select_item)
		{ mSelectWithTab = tab_select; mSelectItem = select_item; }

			void	SetIcon(TableIndexT row, UINT index);
			void	SetAllIcons(UINT index);
	virtual void	AddItem(const cdstring& item);
	virtual void	SetContents(const cdstrvect& items, const ulvector& states);

protected:
	bool	mSelectWithTab;
	UINT	mSelectItem;

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);		// Draw the address

	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif
