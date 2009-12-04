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


#ifndef __CTABLE_H
#define __CTABLE_H

#include <UNX_LTableView.h>
#include "templs.h"

#include "CBroadcaster.h"
#include "CCommander.h"
#include "CContextMenu.h"
#include "CListener.h"
#include "CWindowStatesFwd.h"

class JXTextMenu;

class CTable : public CCommander,		// Commander must be first so it gets destroyed last
				public LTableView,
				public CBroadcaster,
				public CContextMenu
{
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

	CTable(JXScrollbarSet* scrollbarSet, 
				 JXContainer* enclosure,
				 const HSizingOption hSizing, 
				 const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);
  
	virtual ~CTable();
	virtual void OnCreate();
	
	virtual void 		Activate();
	virtual void		Deactivate();

	// Font stuff
	virtual void ResetFont(const SFontInfo& finfo);

			void	EnableTooltips(bool enable = true)
		{ mUseTooltips = enable; }
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

	virtual unsigned long GetItemCount()
		{ return mRows; }

	void SetDeferSelectionChanged(bool defer, bool do_change = true);

	virtual void	ScrollCellIntoFrame(const STableCell &scrollCell,
											bool middle = false);			// Scroll cell into view
	virtual void	ScrollCellToTop(const STableCell &scrollCell);			// Scroll cell to top of view

	enum EScrollTo
	{
		eScroll_Top = 0,
		eScroll_Center,
		eScroll_Bottom
	};
	void ScrollToRow(TableIndexT row, bool select, bool clear, EScrollTo scroll_to);

protected:
	JXScrollbarSet*		mSbs;
	SFontInfo			mFontInfo;
	int					mIconOrigin;
	int					mTextOrigin;

	bool				mIsDropTarget;										// Currently a drop target
	bool				mDrawSelection;										// Draw a selection
	TableIndexT			mSelectColumn;										// Only this column is selected
	STableCell			mHiliteCell;										// Unselected cell hilite
	int					mChanging;											// Changing count
	long				mDeferSelectionChanged;
	bool				mSelectionChanged;							// Selection changed
	STableCell			mTooltipCell;								// Current cell for tooltip
	bool				mUseTooltips;								// Use tooltips
	bool				mHasTooltip;								// Has tooltips
	bool				mRowSelect;
	bool				mColSelect;

	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

	virtual void 		HandleFocusEvent();
	virtual void 		HandleUnfocusEvent();
	virtual void		HandleWindowFocusEvent();
	virtual void		HandleWindowUnfocusEvent();

	virtual void		HandleKeyPress(const int key, const JXKeyModifiers& modifiers);
	virtual bool		HandleChar(const int key, const JXKeyModifiers& modifiers);

	//The JX way of getting mouse clicks
	virtual void HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	virtual void HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers);
	virtual void HandleMouseLeave();

	virtual void Receive (JBroadcaster* sender, const Message& message);

	virtual void SelectionChanged(void);
	virtual void DoSelectionChanged(void);

	virtual void DrawBorder(JXWindowPainter& p, const JRect& frame);

public:
	//Selection functions to emulate the Win32 interface
	virtual unsigned long GetItemCount() const					// Count number of rows
		{ return mRows; }
	bool IsSelectionValid() const;
	bool IsSingleSelection() const;
	bool AllCellsSelected() const;

	TableIndexT	GetFirstSelectedRow() const;
	TableIndexT	GetLastSelectedRow() const;

	virtual void	GetFirstSelection(STableCell& firstCell) const;		// Return first selected cell
	virtual void	GetLastSelection(STableCell& lastCell) const;		// Return last selected cell

	void	ShowFirstSelection();							// Bring first selected cell into view
	void	ShowLastSelection();							// Bring last selected cell into view

	void	GetSelectedRows(ulvector &nums) const;

	virtual bool	ClickSelect(const STableCell &inCell, const JXKeyModifiers& modifiers);		// Fake a click in a cell to keep anchor correct
	virtual void	FakeClickSelect(const STableCell &inCell, bool extend);	// Fake a click in a cell to keep anchor correct

	void SelectAllCells();
	void UnselectAllCells();
	void NudgeSelection(short direction, bool extend, bool primary = true);		// Move selection up or down by specified amount

	virtual void	SelectRow(TableIndexT row);							// Select a row (with focus)
	virtual void	UnselectRow(TableIndexT row);						// Unselect a row
	virtual bool	IsRowSelected(TableIndexT row) const;				// Test row for selection

	virtual void	DeleteSelection(void);								// Delete all selected rows
	virtual bool	DeleteRow(TableIndexT row);									// DeleteRow

	typedef bool (CTable::*TestSelectionPP)(int);
	bool TestSelectionAnd(TestSelectionPP pp);
	bool TestSelectionOr(TestSelectionPP pp);

	typedef bool (CTable::*TestSelection1PP)(int, void* data);
	bool TestSelectionAnd1(TestSelection1PP pp, void* data);
	bool TestSelectionOr1(TestSelection1PP pp, void* data);

	typedef bool (CTable::*DoToSelectionPP)(int);
	bool DoToSelection(DoToSelectionPP pp, bool forward = true);
	bool DoToRows(DoToSelectionPP pp, bool forward = true);

	typedef bool (CTable::*DoToSelection1PP)(int, void*);
	bool DoToSelection1(DoToSelection1PP pp, void* arg, bool forward = true);
	bool DoToRows1(DoToSelection1PP pp, void* arg, bool forward = true);

	typedef bool (CTable::*DoToSelection2PP)(int, void*, void*);
	bool DoToSelection2(DoToSelection2PP pp, void* arg1, void* arg2, bool forward = true);

			void	GetLocalCellRectAlways(const STableCell& inCell, JRect& outCellRect) const;
	virtual void	GetLocalRowRect(TableIndexT row, JRect& theRect);	// Get rect of row
	virtual void	GetLocalColRect(TableIndexT col, JRect& theRect);	// Get rect of column

	virtual void RefreshRow(TableIndexT row);
	virtual void RefreshCol(TableIndexT col);
	virtual void RefreshSelection();

	void OnUpdateSelection(CCmdUI* cmdui);
	void OnUpdateSelectAll(CCmdUI* cmdui);

	void OnEditSelectAll();
	void OnEditClear();

public:
	virtual void Draw(JXWindowPainter& p, const JRect& rect);
	virtual bool DrawCellSelection(JPainter* pDC,
								const STableCell& inCell);
	virtual void	HiliteCellActively(
								const STableCell&		inCell,
								bool					inHilite);
								
	virtual void	HiliteCellInactively(
								const STableCell&		inCell,
								bool					inHilite);
	virtual unsigned long	GetCellBackground(const STableCell& inCell) const;

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

};

typedef CTable CTableDrag;

#endif
