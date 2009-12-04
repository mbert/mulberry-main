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


//	CTableMultiRowSelector.h

#ifndef __CMULTIROWSELECTOR__MULBERRY__
#define __CMULTIROWSELECTOR__MULBERRY__

#include <UTableHelpers.h>
#include <vector>

class LTableSelState;

class	CTableMultiRowSelector : public LTableSelector {
public:
						CTableMultiRowSelector(
								LTableView				*inTableView);
	virtual				~CTableMultiRowSelector();

	virtual Boolean		CellIsSelected(
								const STableCell		&inCell) const;

	LTableSelState*	GetSavedSelection() const;
	void	SetSavedSelection(const LTableSelState*);

	virtual	TableIndexT	GetFirstSelectedRow() const;
	virtual	STableCell	GetFirstSelectedCell() const;
	virtual	TableIndexT	GetLastSelectedRow() const;
	virtual	STableCell	GetLastSelectedCell() const;

	unsigned long		CountSelection() const;

	virtual void		SelectCell(
								const STableCell		&inCell);
	virtual void		SelectAllCells();
	virtual void		SelectCellBlock(
								const STableCell		&inCellA,
								const STableCell		&inCellB);

	virtual void		UnselectCell(
								const STableCell		&inCell);
	virtual void		UnselectAllCells();

	virtual void		ClickSelect(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);
	virtual Boolean		DragSelect(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);

	virtual void		InsertRows(
								UInt32					inHowMany,
								TableIndexT				inAfterRow);
	virtual void		InsertCols(
								UInt32					inHowMany,
								TableIndexT				inAfterCol);
	virtual void		RemoveRows(
								UInt32					inHowMany,
								TableIndexT				inFromRow);
	virtual void		RemoveCols(
								UInt32					inHowMany,
								TableIndexT				inFromCol);

protected:
	virtual void		HiliteRow(
								TableIndexT			inRow,
								Boolean inSelected);

	std::vector<bool>	mSelection;
	unsigned long	mFirstSelection;
	unsigned long	mLastSelection;
	STableCell		mAnchorCell;
};

#endif
