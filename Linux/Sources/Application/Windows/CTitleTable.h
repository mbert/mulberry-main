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


// Header for CTitleTable class

#ifndef __CTITLETABLE__MULBERRY__
#define __CTITLETABLE__MULBERRY__

#include "CTable.h"

#include "HPopupMenu.h"

// Classes
class CTableWindow;
class JXImage;

class CTitleTable : public CTable
{
public:
	static int sClickColumn;

	CTitleTable(JXScrollbarSet* scrollbarSet,
				JXContainer* enclosure,
				const HSizingOption hSizing, 
				const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual ~CTitleTable();
	virtual void OnCreate();

	virtual CColumnInfoArray& GetColumns();					// Get column info

	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
																const JSize clickCount,
																const JXButtonStates& buttonStates,
																const JXKeyModifiers& modifiers);
	virtual void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void	HandleMouseUp(const JPoint& pt, const JXMouseButton button,
								  const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers);

	virtual void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers);
	JBoolean		InDragRegion(const JPoint& pt, STableCell& cell) const;
	virtual void	OnRButtonDown(const JPoint& point,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers);

	virtual void	SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, unsigned long max_str, const cdstring& menu_items);

	virtual void	SetCanSort(bool can_sort)
		{ mCanSort = can_sort; }
	virtual void	SetValue(long value);
	
	void SyncTable(const CTable* table, bool reposition);

protected:
	enum DragTitleType
	{
		kInvalidDrag,
		kDragOneCol,
		kDragAllCols
	};

	cdstrvect			mTitles;			// Array of titles
	bool				mCanSort;			// Can sort by column
	bool				mCanSize;			// Can size by column
	long				mValue;				// Sort column
	cdstring			mMenuBase; 				// ID for its menu
	HPopupMenu*			mPopup;
	CTableWindow*		mTableWindow;			// Its owning window

	// used during drag
	JCursorIndex		mDragLineCursor;
	JCursorIndex		mDragAllLineCursor;

	DragTitleType		mDragType;
	STableCell			mDragCell;
	JRect				mDragCellRect;
	JPoint				mPrevPt;

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	AdjustScrollbars();

	virtual void	MenuChoice(JIndex col, bool sort_col, JIndex menu_item) = 0;

	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);
	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frame);

	virtual void	DrawCell(JPainter* p, const STableCell& cell, const JRect &cellRect);
	virtual void	DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &cellRect);
	virtual void	DrawText(JPainter* p, SColumnInfo& col_info, const JRect &cellRect);
	virtual void	DrawIcon(JPainter* p, SColumnInfo& col_info, unsigned int icon_id, const JRect &cellRect);

	virtual bool	RightJustify(int col_type)									// Check for right justification
		{ return false; }
};

#endif
