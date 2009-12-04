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

#ifndef _H_UNX_LTableView
#define _H_UNX_LTableView

#include <UNX_LScrollable.h>

#include <UNX_UTables.h>

class	LTableGeometry;
class	LTableSelector;
class	LTableStorage;

class JPainter;

// ---------------------------------------------------------------------------

class	LTableView : public LScrollable {

public:
		//  Constructors & Destructor

	LTableView(JXScrollbarSet* scrollbarSet, 
				 JXContainer* enclosure,
				 const HSizingOption hSizing, 
				 const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);
						
	virtual				~LTableView();

		//  Setters/Getters

	void				GetTableSize(
								TableIndexT&			outRows,
								TableIndexT&			outCols) const;

	void				SetUseDragSelect( bool inUseIt )
							{
								mUseDragSelect = inUseIt;
							}

	void				SetDeferAdjustment( bool inDefer );

	bool				GetDeferAdjustment()	{ return mDeferAdjustment; }

		//  Row, Col, and Cell Verification

	bool				IsValidRow( TableIndexT inRow ) const;
								
	bool				IsValidCol( TableIndexT inCol ) const;
								
	bool				IsValidCell( const STableCell & inCell ) const;

		//  Row & Col Management

	virtual void		InsertRows(
								UInt32					inHowMany,
								TableIndexT				inAfterRow,
								const void*				inDataPtr = nil,
								UInt32					inDataSize = 0,
								bool					inRefresh = false);
								
	virtual void		InsertCols(
								UInt32					inHowMany,
								TableIndexT				inAfterCol,
								const void*				inDataPtr = nil,
								UInt32					inDataSize = 0,
								bool					inRefresh = false);
								
	virtual void		RemoveRows(
								UInt32					inHowMany,
								TableIndexT				inFromRow,
								bool					inRefresh);
								
	virtual void		RemoveAllRows( bool inRefresh );
	
	virtual void		RemoveCols(
								UInt32					inHowMany,
								TableIndexT				inFromCol,
								bool					inRefresh);
								
	virtual void		RemoveAllCols( bool inRefresh );
								
	virtual void		AdjustImageSize( bool inRefresh );

		//  Accessing Cells

	virtual void		CellToIndex(
								const STableCell&		inCell,
								TableIndexT&			outIndex) const;
								
	virtual void		IndexToCell(
								TableIndexT				inIndex,
								STableCell&				outCell) const;

	virtual bool		GetNextCell( STableCell& ioCell ) const;
								
	virtual STableCell	GetFirstSelectedCell() const;
	
	virtual bool		GetNextSelectedCell( STableCell& ioCell ) const;

	virtual bool		GetPreviousCell( STableCell& ioCell ) const;
								
	virtual bool		GetPreviousSelectedCell( STableCell& ioCell ) const;

	virtual void		ScrollCellIntoFrame( const STableCell& inCell );

	virtual STableCell	GetFirstFullyVisibleCell() const;
	virtual STableCell	GetLastFullyVisibleCell() const;


	virtual STableCell	GetSelectionAnchor() const;
	virtual STableCell	GetSelectionBoat() const;

		//   Cell Geometry

	virtual void		SetTableGeometry( LTableGeometry* inTableGeometry );

	LTableGeometry*		GetTableGeometry()		{ return mTableGeometry; }

	virtual void		GetImageCellBounds(
								const STableCell&		inCell,
								JRect&					outRect) const;
	virtual void		GetImageCellBounds(
								const STableCell&		inCell,
								SInt32&					outLeft,
								SInt32&					outTop,
								SInt32&					outRight,
								SInt32&					outBottom) const;
								
	virtual bool		GetLocalCellRect(
								const STableCell&		inCell,
								JRect&					outCellFrame) const;
								
	virtual bool		GetCellHitBy(
								const JPoint&			inImagePt,
								STableCell&				outCell) const;

	virtual UInt16		GetRowHeight( TableIndexT inRow ) const;
								
	virtual void		SetRowHeight(
								UInt16					inHeight,
								TableIndexT				inFromRow,
								TableIndexT				inToRow);

	virtual UInt16		GetColWidth( TableIndexT inCol ) const;
								
	virtual void		SetColWidth(
								UInt16					inWidth,
								TableIndexT				inFromCol,
								TableIndexT				inToCol);

	virtual void		FetchIntersectingCells(
								const JRect&			inLocalRect,
								STableCell&				outTopLeft,
								STableCell&				outBotRight) const;

			UInt16		GetVisibleRows() const;

		//  Selecting Cells

	virtual void		SetTableSelector( LTableSelector* inTableSelector );

	LTableSelector*		GetTableSelector()		{ return mTableSelector; }

	virtual bool		CellIsSelected( const STableCell& inCell ) const;
	
	virtual void		SelectCell( const STableCell& inCell );
	
	virtual void		SelectAllCells();
	
	virtual void		UnselectCell( const STableCell& inCell );
								
	virtual void		UnselectAllCells();

	virtual bool		ClickSelect(
								const STableCell&		inCell,
								const JXKeyModifiers& modifiers);

	virtual void		HiliteSelection(
								bool					inActively,
								bool					inHilite);
								
	virtual void		HiliteCell(
								const STableCell&		inCell,
								bool					inHilite);

	virtual void		SelectionChanged();
	
		//  Storing Data

	virtual void		SetTableStorage( LTableStorage* inTableStorage );

	LTableStorage*		GetTableStorage()		{ return mTableStorage; }

	virtual void		SetCellData(
								const STableCell&		inCell,
								const void*				inDataPtr,
								UInt32					inDataSize);
								
	virtual void		GetCellData(
								const STableCell&		inCell,
								void*					outDataPtr,
								UInt32&					ioDataSize) const;
								
	virtual bool		FindCellData(
								STableCell&				outCell,
								const void*				inDataPtr,
								UInt32					inDataSize) const;

		//  Refreshing

	virtual void		RefreshCell( const STableCell& inCell );
								
	virtual void		RefreshCellRange(
								const STableCell&		inTopLeft,
								const STableCell&		inBotRight);
								
protected:
	enum DragType
	{
		kInvalidDrag,
		kWaitForDNDDrag,
		kSelectDrag
	};

	TableIndexT		mRows;
	TableIndexT		mCols;
	LTableGeometry	*mTableGeometry;
	LTableSelector	*mTableSelector;
	LTableStorage	*mTableStorage;
	bool			mUseDragSelect;
	bool			mDeferAdjustment;
	JPoint			mDragStartPoint;
	DragType		mDragType;

		//  Hiliting

	virtual void		HiliteCellActively(
								const STableCell&		inCell,
								bool					inHilite);
								
	virtual void		HiliteCellInactively(
								const STableCell&		inCell,
								bool					inHilite);
								
		//  Clicking

	virtual void HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void HandleMouseDrag(const JPoint& pt,
								 const JXButtonStates&	buttonStates,
								 const JXKeyModifiers&	modifiers);
	virtual void HandleMouseUp(const JPoint& pt,
								 const JXMouseButton button,
								 const JXButtonStates& buttonStates,
								 const JXKeyModifiers& modifiers);

	virtual void		LClickCell(const STableCell& inCell,
									const JXKeyModifiers& modifiers);
	virtual void		LDblClickCell(const STableCell& inCell,
									const JXKeyModifiers& modifiers);
	virtual void		RClickCell(const STableCell& inCell,
									const JXKeyModifiers& modifiers);

		//  Key board

	virtual void		HandleKeyPress(const int key, const JXKeyModifiers& modifiers);
	virtual bool		HandleChar(const int key, const JXKeyModifiers& modifiers);

		//  Drawing

	virtual void		Draw(JXWindowPainter& p, const JRect& rect);
	
	virtual void		DrawCellRange(JPainter* pDC,
								const STableCell&	inTopLeftCell,
								const STableCell&	inBottomRightCell);
	
	virtual void		DrawCell(JPainter* pDC,
								const STableCell&	inCell,
								const JRect&		inLocalRect);
	
	virtual bool		DrawCellSelection(JPainter* pDC,
								const STableCell& inCell);

	virtual void		DrawBackground(JXWindowPainter& pDC, const JRect& inLocalRect);
	

private:
	void				InitTable();
};


// ---------------------------------------------------------------------------

class	StDeferTableAdjustment {
public:
				StDeferTableAdjustment( LTableView* inTable );
				
				~StDeferTableAdjustment();
private:
	LTableView*		mTable;
	bool			mSaveDefer;
};

#endif
