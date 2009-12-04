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


//	Manages the selection for a TableView than can have multiple selections

#ifndef _H_WIN_LTableMultiSelector
#define _H_WIN_LTableMultiSelector
#pragma once

#include <WIN_UTableHelpers.h>

#include <vector>

// ---------------------------------------------------------------------------

class	LTableMultiSelector : public LTableSelector {
public:
						LTableMultiSelector( LTableView* inTableView );
								
	virtual				~LTableMultiSelector();

	virtual bool		CellIsSelected( const STableCell& inCell ) const;
								
	virtual	STableCell	GetFirstSelectedCell() const;
	
	virtual	TableIndexT	GetFirstSelectedRow() const;

	virtual	STableCell	GetLastSelectedCell() const;
	
	virtual	TableIndexT	GetLastSelectedRow() const;

	virtual STableCell	GetSelectionAnchor() const;
	virtual STableCell	GetSelectionBoat() const;

	virtual void		SelectCell( const STableCell& inCell );
	virtual void		SelectOneCell( const STableCell& inCell );
	
	virtual void		SelectAllCells();

	virtual void		UnselectCell( const STableCell& inCell );
	
	virtual void		UnselectAllCells();

	virtual void		ClickSelect(
								const STableCell&		inCell,
								UINT nFlags,
								bool one_only = false);
								
	virtual bool		DragSelect(
								const STableCell&		inCell,
								UINT nFlags);

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
	virtual void		UnselectAllCells( bool inReportChange );

	virtual void		SelectCellBlock(
								const STableCell&		inCellA,
								const STableCell&		inCellB);
protected:
	std::vector<bool>	mSelection;
	STableCell			mFirstSelection;
	STableCell			mLastSelection;
	STableCell			mAnchorCell;
	STableCell			mBoatCell;
	
	std::vector<bool>::size_type	GetSelectionIndex(const STableCell& cell) const;
};

#endif
