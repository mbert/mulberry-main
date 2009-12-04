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


//	CTableRowSelector.h

#ifndef __CTABLEROWSELECTOR__MULBERRY__
#define __CTABLEROWSELECTOR__MULBERRY__

#include <WIN_LTableMultiSelector.h>

// Constants

// Classes

class	CTableRowSelector : public LTableMultiSelector {

public:
						CTableRowSelector(LTableView *inTableView);
	virtual				~CTableRowSelector();

	virtual void		SelectCell(const STableCell &inCell);
	virtual void		SelectOneCell(const STableCell &inCell);
	virtual void		UnselectCell(const STableCell &inCell);

	virtual void		SelectCellBlock(const STableCell &inCellA,
										const STableCell &inCellB);
};

class	CTableSingleRowSelector : public LTableMultiSelector {

public:
						CTableSingleRowSelector(LTableView *inTableView);
	virtual				~CTableSingleRowSelector();

	virtual void		SelectCell(const STableCell &inCell);
	virtual void		SelectOneCell(const STableCell &inCell);
	virtual void		ClickSelect(const STableCell &inCell,
									UINT nFlags, bool one_only = false);
	virtual void		UnselectCell(const STableCell &inCell);

};

#endif
