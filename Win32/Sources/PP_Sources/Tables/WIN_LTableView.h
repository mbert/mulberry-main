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


#ifndef _H_WIN_LTableView
#define _H_WIN_LTableView

#include <WIN_LScrollable.h>

#include <WIN_UTables.h>

class	LTableGeometry;
class	LTableSelector;
class	LTableStorage;

// ---------------------------------------------------------------------------

class	LTableView : public LScrollable {

	DECLARE_DYNCREATE(LTableView)

public:
		// ¥ Constructors & Destructor

						LTableView();
						
	virtual				~LTableView();

		// ¥ Setters/Getters

	void				GetTableSize(
								TableIndexT&			outRows,
								TableIndexT&			outCols) const;

	void				SetUseDragSelect( bool inUseIt )
							{
								mUseDragSelect = inUseIt;
							}

	void				SetDeferAdjustment( bool inDefer );

	bool				GetDeferAdjustment()	{ return mDeferAdjustment; }

		// ¥ Row, Col, and Cell Verification

	bool				IsValidRow( TableIndexT inRow ) const;
								
	bool				IsValidCol( TableIndexT inCol ) const;
								
	bool				IsValidCell( const STableCell & inCell ) const;

		// ¥ Row & Col Management

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

		// ¥ Accessing Cells

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

		//  ¥ Cell Geometry

	virtual void		SetTableGeometry( LTableGeometry* inTableGeometry );

	LTableGeometry*		GetTableGeometry()		{ return mTableGeometry; }

	virtual void		GetImageCellBounds(
								const STableCell&		inCell,
								CRect&					outRect) const;
	virtual void		GetImageCellBounds(
								const STableCell&		inCell,
								SInt32&					outLeft,
								SInt32&					outTop,
								SInt32&					outRight,
								SInt32&					outBottom) const;
								
	virtual bool		GetLocalCellRect(
								const STableCell&		inCell,
								CRect&					outCellFrame) const;
								
	virtual bool		GetCellHitBy(
								const CPoint&			inImagePt,
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
								const CRect&			inLocalRect,
								STableCell&				outTopLeft,
								STableCell&				outBotRight) const;

			UInt16		GetVisibleRows() const;

		// ¥ Selecting Cells

	virtual void		SetTableSelector( LTableSelector* inTableSelector );

	LTableSelector*		GetTableSelector()		{ return mTableSelector; }

	virtual bool		CellIsSelected( const STableCell& inCell ) const;
	
	virtual void		SelectCell( const STableCell& inCell );
	
	virtual void		SelectAllCells();
	
	virtual void		UnselectCell( const STableCell& inCell );
								
	virtual void		UnselectAllCells();

	virtual bool		ClickSelect(
								const STableCell&		inCell,
								UINT	nFlags);

	virtual void		HiliteSelection(
								bool					inActively,
								bool					inHilite);
								
	virtual void		HiliteCell(
								const STableCell&		inCell,
								bool					inHilite);

	virtual void		SelectionChanged();
	
		// ¥ Storing Data

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

		// ¥ Refreshing

	virtual void		RefreshCell( const STableCell& inCell );
								
	virtual void		RefreshCellRange(
								const STableCell&		inTopLeft,
								const STableCell&		inBotRight);
								
protected:
	TableIndexT		mRows;
	TableIndexT		mCols;
	LTableGeometry	*mTableGeometry;
	LTableSelector	*mTableSelector;
	LTableStorage	*mTableStorage;
	bool			mUseDragSelect;
	bool			mDeferAdjustment;
	CPoint			mLDownPoint;

		// ¥ Hiliting

	virtual void		HiliteCellActively(
								const STableCell&		inCell,
								bool					inHilite);
								
	virtual void		HiliteCellInactively(
								const STableCell&		inCell,
								bool					inHilite);
								
	//virtual void		ActivateSelf();
	
	//virtual void		DeactivateSelf();

		// ¥ Clicking

	afx_msg	void		OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void 		OnLButtonDblClk(UINT nFlags, CPoint point);				// Double-clicked item
	afx_msg	void 		OnMButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void 		OnRButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

	virtual void		LClickCell(const STableCell& inCell, UINT nFlags);
	virtual void		LDblClickCell(const STableCell& inCell, UINT nFlags);
	virtual void		MClickCell(const STableCell& inCell, UINT nFlags);
	virtual void		RClickCell(const STableCell& inCell, UINT nFlags);

		// ¥ Key board

	afx_msg UINT OnGetDlgCode();
	afx_msg	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		// ¥ Drawing

	afx_msg void		OnPaint();
	
	virtual void		DrawCellRange(CDC* pDC,
								const STableCell&	inTopLeftCell,
								const STableCell&	inBottomRightCell);
	
	virtual void		DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);
	
	virtual bool		DrawCellSelection(CDC* pDC,
								const STableCell& inCell);

	virtual void		DrawBackground(CDC* pDC, const CRect& inLocalRect);
	

private:
	void				InitTable();

	DECLARE_MESSAGE_MAP()
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
