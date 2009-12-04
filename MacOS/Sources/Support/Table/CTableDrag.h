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


// Header for CTableDrag class

#ifndef __CTABLEDRAG__MULBERRY__
#define __CTABLEDRAG__MULBERRY__

#include "CTableDragAndDrop.h"
#include "CBroadcaster.h"
#include "CListener.h"
#include "CHelpTags.h"

// Types
class LArray;

// Classes

class CTableDrag : public LTableView,
					public LCommander,
					public CHelpTagPane,
					public CBroadcaster,
					public CTableDragAndDrop
{
	friend class CTableDragAndDrop;

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
		StTableAction(CTableDrag* aTable)
			{ mTable = aTable; mTable->Add_Listener(this); }
		virtual ~StTableAction() {}

		virtual void ListenTo_Message(long msg, void* param)
		{
			if ((msg == eBroadcast_TableDeleted) && (param == mTable))
				mTable = NULL;
		};

	protected:
		CTableDrag* mTable;
	};

	class StDeferSelectionChanged : public StTableAction
	{
	public:
		StDeferSelectionChanged(CTableDrag* aTable) : StTableAction(aTable)
			{ mTable->SetDeferSelectionChanged(true); }
		~StDeferSelectionChanged()
			{ if (mTable) mTable->SetDeferSelectionChanged(false); }
	};
	friend class StDeferSelectionChanged;

	class StNoSelectionChanged : public StTableAction
	{
	public:
		StNoSelectionChanged(CTableDrag* aTable) : StTableAction(aTable)
			{ mTable->SetDeferSelectionChanged(true); }
		~StNoSelectionChanged()
			{ if (mTable) mTable->SetDeferSelectionChanged(false, false); }
	};
	friend class StNoSelectionChanged;

	typedef bool (CTableDrag::*DoToSelectionPP)(TableIndexT);
	typedef bool (CTableDrag::*DoToSelection1PP)(TableIndexT, void*);
	typedef bool (CTableDrag::*DoToSelection2PP)(TableIndexT, void*, void*);
	typedef bool (CTableDrag::*TestSelectionPP)(TableIndexT);
	typedef bool (CTableDrag::*TestSelection1PP)(TableIndexT, void*);

	enum { class_ID = 'Tabv' };
						CTableDrag();
						CTableDrag(const SPaneInfo			&inPaneInfo,
									const SViewInfo			&inViewInfo);
						CTableDrag(LStream *inStream);
	virtual 			~CTableDrag();
	
protected:
	virtual void		FinishCreateSelf(void);					// Setup key navigation

public:
	virtual Boolean	ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void	FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual Boolean	HandleKeyPress(const EventRecord &inKeyEvent);

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

			void	EnableTooltips(bool enable = true)
		{ mUseTooltips = enable; }
	virtual	void	GetTooltipText(cdstring& txt, const STableCell &inCell);				// Get text for current tooltip cell

	virtual unsigned long GetItemCount() const					// Count number of rows
		{ return mRows; }
	virtual bool	IsSelectionValid(void);						// Check for valid selection
	virtual bool	IsSingleSelection(void);					// Is a single item only selected
	virtual void	GetFirstSelection(STableCell& firstCell);	// Return first selected cell
	virtual void	GetLastSelection(STableCell& lastCell);		// Return last selected cell
	virtual void	ShowFirstSelection(void);					// Bring first selected cell into view
	virtual void	ShowLastSelection(void);					// Bring last selected cell into view
	virtual void	GetSelectedRows(ulvector& sels);			// Get list of selected rows
	virtual void	SelectAllCells(void);

	virtual TableIndexT	GetFirstSelectedRow();					// Get first selected row
	virtual TableIndexT	GetLastSelectedRow();					// Get last selected row

	virtual STableCell	GetFirstFullyVisibleCell() const;
	virtual STableCell	GetLastFullyVisibleCell() const;

	virtual void	Click(SMouseDownEvent &inMouseDown);		// Allow background window D&D
	virtual Boolean	ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown);	// Fake a click in a cell to keep anchor correct
	virtual void	FakeClickSelect(const STableCell &inCell, bool extend);	// Fake a click in a cell to keep anchor correct
	virtual void	NudgeSelection(short direction, bool extend, bool primary = true);
	virtual void	SelectRow(TableIndexT row);
	virtual void	UnselectRow(TableIndexT row);
	virtual bool	IsRowSelected(TableIndexT row) const;				// Test row for selection

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

	virtual void	SetDeferSelectionChanged(bool defer, bool do_change = true);
	virtual bool	DoToRows(DoToSelectionPP proc,
									bool forward = true);		// Do something to each row
	virtual bool	DoToRows1(DoToSelection1PP proc,			// Do something to each row
									void* data,
									bool forward = true);		// with some data
	virtual bool	DoToSelection(DoToSelectionPP proc,
									bool forward = true);		// Do something to each cell in the selection
	virtual bool	DoToSelection1(DoToSelection1PP proc,		// Do something to each cell in the selection
									void* data,
									bool forward = true);		// with some data
	virtual bool	DoToSelection2(DoToSelection2PP proc,		// Do something to each cell in the selection
									void* data1,
									void* data2,
									bool forward = true);		// with some data
	virtual bool	TestSelectionAnd(TestSelectionPP proc);		// Test each cell in the selection using logical AND
	virtual bool	TestSelectionAnd1(TestSelection1PP proc,
										void* data);					// Test each row in the selection using logical AND
	virtual bool	TestSelectionOr(TestSelectionPP proc);		// Test each cell in the selection using logical OR
	virtual bool	TestSelectionOr1(TestSelection1PP proc,
										void* data);					// Test row cell in the selection using logical OR

	virtual bool	CanScrollImageBy(SInt32 inLeftDelta,
										SInt32 inTopDelta);	// Determine whether pinned scroll will occur

	virtual void	DrawRow(TableIndexT row);				// Draw whole row
	virtual void	DrawColumn(TableIndexT col);			// Draw whole column

	virtual void	HiliteRow(TableIndexT row,
								Boolean inHilite);			// Hilite whole row
	virtual void	HiliteColumn(TableIndexT col,
								Boolean inHilite);			// Hilite whole column

	virtual void	GetLocalRowRect(TableIndexT row, Rect& theRect);	// Get rect of row
	virtual void	GetLocalColRect(TableIndexT col, Rect& theRect);	// Get rect of column

	virtual void	RefreshSelection(void);					// Refresh selection
	virtual void	RefreshRow(TableIndexT row);			// Refresh row
	virtual void	RefreshCol(TableIndexT col);			// Refresh column
	
	virtual void	SetTextTraits(ResIDT txtrID);						// Reset text traits - recal row height
	virtual void	SetTextTraits(const TextTraitsRecord& aTextTrait);	// Reset text traits - recal row height

protected:
	TextTraitsRecord	mTextTraits;								// Text style for table
	short				mTextDescent;								// Descent for chosen font
	short				mIconDescent;								// Descent of icon for chosen font
	unsigned long		mDeferSelectionChanged;						// Defer selection changed
	bool				mSelectionChanged;							// Selection changed
	bool				mUseTooltips;								// Use tooltips
	bool				mDrawSelection;								// Draw a selection
	TableIndexT			mSelectColumn;								// Only this column is selected
	STableCell			mHiliteCell;								// Unselected cell hilite

	virtual void	SelectionChanged(void);
	virtual void	DoSelectionChanged(void);
			void	SelectAllCommandStatus(Boolean &outEnabled, Str255 outName);
			bool	AllCellsSelected() const;

	virtual bool	SetupHelp(LPane* pane, Point inPortMouse, HMHelpContentPtr ioHelpContent);

			void	GetLocalCellRectAlways(const STableCell& inCell, Rect& outCellRect) const;

	virtual void	BeTarget();
	virtual void	DontBeTarget();

#if PP_Target_Carbon
	virtual void	DrawSelf();
	virtual bool	DrawCellSelection(const STableCell& inCell);
	virtual void	HiliteCellActively(
								const STableCell&		inCell,
								Boolean					inHilite);
								
	virtual void	HiliteCellInactively(
								const STableCell&		inCell,
								Boolean					inHilite);
								
#endif

private:
	bool			mRowSelect;
	bool			mColSelect;
	
	void			InitTableDrag();
};

#endif
