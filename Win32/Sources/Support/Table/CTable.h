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


// Header for CTable class

#ifndef __CTABLE__MULBERRY__
#define __CTABLE__MULBERRY__

#include <WIN_LTableView.h>

#include "CBroadcaster.h"
#include "CCommander.h"

//#include "CTableUtils.h"

#include "cdstring.h"
#include "cdustring.h"
#include "templs.h"

// Classes
class CTable : public LTableView,
				public CBroadcaster,
				public CCommander
{
	DECLARE_DYNCREATE(CTable)

public:
	enum
	{
		eBroadcast_Activate = 'TDac',
		eBroadcast_Deactivate = 'TDda',
		eBroadcast_SelectionChanged = 'TDsc',
		eBroadcast_TableDeleted = 'TD~ '
	};

	class StTableAction : public CListener
	{
	public:
		StTableAction(CTable* aTable)
			{ mTable = aTable; mTable->Add_Listener(this); }
		virtual ~StTableAction() {}

		virtual void ListenTo_Message(long msg, void* param)
		{
			if ((msg == eBroadcast_TableDeleted) && (param == mTable))
				mTable = NULL;
		};

	protected:
		CTable* mTable;
	};

	class StDeferTableAdjustment : public StTableAction
	{
	public:
		StDeferTableAdjustment(CTable* aTable) : StTableAction(aTable)
			{ mTable->Changing(true); }
		virtual ~StDeferTableAdjustment()
			{ if (mTable) mTable->Changing(false); }
	};

	class StDeferSelectionChanged : public StTableAction
	{
	public:
		StDeferSelectionChanged(CTable* aTable) : StTableAction(aTable)
			{ mTable->SetDeferSelectionChanged(true); }
		virtual ~StDeferSelectionChanged()
			{ if (mTable) mTable->SetDeferSelectionChanged(false); }
	};

	class StNoSelectionChanged : public StTableAction
	{
	public:
		StNoSelectionChanged(CTable* aTable) : StTableAction(aTable)
			{ mTable->SetDeferSelectionChanged(true); }
		virtual ~StNoSelectionChanged()
			{ if (mTable) mTable->SetDeferSelectionChanged(false, false); }
	};
	friend class StNoSelectionChanged;

	typedef bool (CTable::*DoToSelectionPP)(int row);
	typedef bool (CTable::*DoToSelection1PP)(int row, void* data);
	typedef bool (CTable::*DoToSelection2PP)(int row, void* data1, void* data2);
	typedef bool (CTable::*TestSelectionPP)(int row);
	typedef bool (CTable::*TestSelection1PP)(int row, void* data);

					CTable();
	virtual 		~CTable();

	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	virtual void	SetContextMenuID(UINT contextID)					// Set context menu ID
						{ mContextMenuID = contextID; }
	virtual void	SetContextView(CView* aView)						// Set context view
						{ mContextView = aView; }

	virtual int		GetTextHeight() const								// Get current row text height
						{ return mTextHeight; }

	virtual int		GetSortBy() const;

	virtual	void	GetTooltipText(cdstring& txt);							// Get text for current tooltip cell
	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);	// Get text for current tooltip cell

	virtual void	SetRowSelect(bool rowSelect)
						{ mRowSelect = rowSelect; }
	virtual bool	GetRowSelect(void)
						{ return mRowSelect; }
	virtual void	SetColSelect(bool colSelect)
						{ mColSelect = colSelect; }
	virtual bool	GetColSelect(void)
						{ return mColSelect; }

			void	SetOneColumnSelect(TableIndexT col)
		{ mSelectColumn = col; }
			bool	OKToSelect(TableIndexT col) const
		{ return (mSelectColumn == 0) || (col == mSelectColumn); }

	virtual unsigned long GetItemCount() const					// Count number of rows
		{ return mRows; }
	virtual bool	IsSelectionValid() const;
	virtual bool	IsSingleSelection() const;
	virtual void	GetFirstSelection(STableCell& firstCell) const;		// Return first selected cell
	virtual void	GetLastSelection(STableCell& lastCell) const;		// Return last selected cell
	virtual void	ShowFirstSelection(void);							// Bring first selected cell into view
	virtual void	ShowLastSelection(void);							// Bring last selected cell into view
	virtual void	GetSelectedRows(ulvector& sels) const;				// Get list of selected rows

	virtual void	SelectAllCells(void);								// Select all cells

	virtual bool	ClickSelect(const STableCell &inCell, UINT nFlags);		// Fake a click in a cell to keep anchor correct
	virtual void	FakeClickSelect(const STableCell &inCell, bool extend);	// Fake a click in a cell to keep anchor correct

	virtual void	SetDeferSelectionChanged(bool defer, bool do_change = true);
	virtual void	NudgeSelection(short direction, bool extend, bool primary = true);		// Move selection up or down by specified amount
	virtual void	SelectRow(TableIndexT row);							// Select a row (with focus)
	virtual void	UnselectRow(TableIndexT row);						// Unselect a row
	virtual bool	IsRowSelected(TableIndexT row) const;				// Test row for selection

	virtual TableIndexT		GetFirstSelectedRow() const;					// Get first selected row
	virtual TableIndexT		GetLastSelectedRow() const;						// Get last selected row

	virtual void	HiliteRow(TableIndexT row, bool inHilite);			// Hilite whole row
	virtual void	HiliteColumn(TableIndexT col, bool inHilite);		// Hilite whole column

			void	GetLocalCellRectAlways(const STableCell& inCell, CRect& outCellRect) const;
	virtual void	GetLocalRowRect(TableIndexT row, CRect& theRect);	// Get rect of row
	virtual void	GetLocalColRect(TableIndexT col, CRect& theRect);	// Get rect of column

	virtual void	RefreshSelection(void);								// Update button & captions as well
	virtual void	RefreshRow(TableIndexT row);								// Update button & captions as well
	virtual void	RefreshCol(TableIndexT col);								// Update button & captions as well

	virtual void	DeleteSelection(void);								// Delete all selected rows
	virtual bool	DeleteRow(TableIndexT row);									// DeleteRow

	virtual void	ScrollCellIntoFrame(const STableCell &scrollCell,
											bool middle = false);			// Scroll cell into view
	virtual void	ScrollCellToTop(const STableCell &scrollCell);			// Scroll cell to top of view
	enum EScrollTo
	{
		eScroll_Top = 0,
		eScroll_Center,
		eScroll_Bottom
	};
	virtual void	ScrollToRow(TableIndexT row, bool select, bool clear, EScrollTo scroll_to);

	virtual void	ResetFont(CFont* pFont);							// Change font in each row

	virtual void	Changing(bool changing);							// Indicates that table is changing

	// Common updaters
	afx_msg void	OnUpdateAlways(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateNever(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSelection(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSelectAll(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnEditSelectAll(void);
	afx_msg void	OnEditClear(void);

	// Key/Mouse related
	afx_msg	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);			// Handle character
	
protected:
	UINT			mContextMenuID;										// Context menu ID
	CView*			mContextView;										// View to activate on context
	short			mTextOrigin;										// Vertical draw origin for chosen font
	short			mTextHeight;										// Height of text for a row
	short			mIconOrigin;										// Vertical draw origin of icon for chosen font
	bool			mHasFocus;
	bool			mIsDropTarget;										// Currently a drop target
	bool			mDrawSelection;										// Draw a selection
	TableIndexT		mSelectColumn;										// Only this column is selected
	STableCell		mHiliteCell;										// Unselected cell hilite
	int				mChanging;											// Changing count
	long			mDeferSelectionChanged;								// Defer selection changed
	bool			mSelectionChanged;									// Selection changed
	CCommanderProtect	mCmdProtect;									// Protect commands
	mutable STableCell		mTooltipCell;								// Current cell row for tooltip
#ifdef _UNICODE
	mutable cdustring		mTooltipText;								// Current tooltip text
#else
	mutable cdstring		mTooltipText;								// Current tooltip text
#endif

	bool			mRowSelect;
	bool			mColSelect;

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down

	virtual void	SelectionChanged();
	virtual void	DoSelectionChanged();
			bool	AllCellsSelected() const;

	virtual bool	DrawCellSelection(CDC* pDC,
								const STableCell& inCell);

	virtual void	HiliteCellActively(
								const STableCell&		inCell,
								bool					inHilite);
								
	virtual void	HiliteCellInactively(
								const STableCell&		inCell,
								bool					inHilite);

	virtual bool	DoToRows(DoToSelectionPP proc,
									bool forward = true);				// Do something to each row
	virtual bool	DoToRows1(DoToSelection1PP proc,					// Do something to each row with some data
									void* data,
									bool forward = true);
	virtual bool	DoToSelection(DoToSelectionPP proc,
									bool forward = true);				// Do something to each row in the selection
	virtual bool	DoToSelection1(DoToSelection1PP proc,				// Do something to each row in the selection with some data
									void* data,
									bool forward = true);
	virtual bool	DoToSelection2(DoToSelection2PP proc,				// Do something to each row in the selection with some data
									void* data1,
									void* data2,
									bool forward = true);
	virtual bool	TestSelectionAnd(TestSelectionPP proc);				// Test each row in the selection using logical AND
	virtual bool	TestSelectionAnd1(TestSelection1PP proc,
										void* data);					// Test each row in the selection using logical AND
	virtual bool	TestSelectionOr(TestSelectionPP proc);				// Test row cell in the selection using logical OR
	virtual bool	TestSelectionOr1(TestSelection1PP proc,
										void* data);					// Test row cell in the selection using logical OR

	afx_msg void OnContextMenu(CWnd*, CPoint point);
	virtual void HandleContextMenu(CWnd*, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

typedef CTable CTableDrag;
#endif
