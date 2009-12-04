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


//	Manages the selection for a TableView than can have only one selected cell

#ifndef _H_UNX_LTableSingleSelector
#define _H_UNX_LTableSingleSelector

#include <UNX_UTableHelpers.h>

// ---------------------------------------------------------------------------

class	LTableSingleSelector : public LTableSelector {
public:
						LTableSingleSelector( LTableView* inTableView );
								
	virtual				~LTableSingleSelector();

	virtual bool		CellIsSelected( const STableCell& inCell ) const;
	
	virtual	STableCell	GetFirstSelectedCell() const;
	
	virtual	TableIndexT	GetFirstSelectedRow() const;
	
	virtual	STableCell	GetLastSelectedCell() const;
	
	virtual	TableIndexT	GetLastSelectedRow() const;
	
	virtual STableCell	GetSelectionAnchor() const;
	virtual STableCell	GetSelectionBoat() const;

	virtual void		SelectCell( const STableCell& inCell );
	
	virtual void		SelectAllCells();
	
	virtual void		UnselectCell( const STableCell& inCell );
								
	virtual void		UnselectAllCells();

	virtual void		ClickSelect(
								const STableCell&		inCell,
								const JXKeyModifiers& modifiers);
								
	virtual bool		DragSelect(
								const STableCell&		inCell,
								const JXKeyModifiers& modifiers);

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
	STableCell		mSelectedCell;
};

#endif
