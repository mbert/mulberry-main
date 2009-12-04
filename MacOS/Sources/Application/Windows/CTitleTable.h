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

#include "CTableDrag.h"
#include "CWindowStatesFwd.h"

// Constants

// Classes
class CHierarchyTableDrag;
class CTableWindow;

class CTitleTable : public CTableDrag
{
protected:
	cdstrvect			mTitles;			// List of title strings
	bool				mCanSort;			// Can sort by column
	bool				mCanSize;			// Can size columns
	long				mValue;				// Current sort cell
	ResIDT				mMenuID;			// ID for its menu
	MenuHandle			mColumnChanger;		// Column changer menu
	bool				mTracking;			// Tracking mouse down
	bool				mTrackingInside;	// Inside column while tracking
	CTableWindow*		mTableWindow;		// Its owning window
	CTableDrag*				mTable;
	CHierarchyTableDrag*	mHTable;

public:
					CTitleTable();
					CTitleTable(LStream *inStream);
	virtual 		~CTitleTable();

			void	SetOwner(CTableDrag* table)
			{
				mTable = table;
			}
			void	SetOwner(CHierarchyTableDrag* htable)
			{
				mHTable = htable;
			}
			
	virtual CColumnInfoArray& GetColumns();					// Get column info

	virtual void	SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, unsigned long max_str, ResIDT menu_id);

	virtual void	SetCanSort(bool can_sort)
		{ mCanSort = can_sort; }
	virtual void	SetValue(long value);

	virtual Boolean	ObeyCommand(CommandT inCommand, void *ioParam = nil);

			void SyncTable(const LTableView* table, bool reposition = false);

private:
			void	InitTitleTable(void);					// Do common init

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitColumnChanger(void) = 0;			// Init column changer menu

	virtual 	void				ActivateSelf		();								// ¥ OVERRIDE
	virtual 	void				DeactivateSelf		();								// ¥ OVERRIDE

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Allow column size change
	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Stop clicks
	virtual void	MenuChoice(short col, bool sort_col, short menu_item) = 0;

	virtual void	DrawBackground();
	virtual void	DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);					// Draw cell
	virtual void	DrawItem(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw item
	virtual void	DrawText(SColumnInfo& col_info,
								bool sort_col, const Rect &inLocalRect);		// Draw title
	virtual void	DrawIcon(SColumnInfo& col_info, ResIDT icon,
								bool sort_col, const Rect &inLocalRect);		// Draw icon
	
	virtual bool	RightJustify(int col_type)									// Check for right justification
		{ return false; }

public:
	virtual void	AdjustMouseSelf(Point inPortPt,
									const EventRecord &inMacEvent,
									RgnHandle outMouseRgn);
};

#endif
