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

#include "CWindowStatesFwd.h"

#include "cdstring.h"

// Classes
class CTableWindow;

class	CTitleTable : public CTable
{
public:
					CTitleTable();
	virtual 		~CTitleTable();

	virtual CColumnInfoArray& GetColumns();					// Get column info

	virtual BOOL 		SubclassDlgItem(UINT nID, CWnd* pParent);

	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void OnLButtonUp(UINT nFlags, CPoint point);				// Clicked somewhere

	virtual void	SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, UINT max_str, UINT menu_id);

	virtual void	SetCanSort(bool can_sort)
		{ mCanSort = can_sort; }
	virtual void	SetValue(long value);

	void SyncTable(const CTable* table, bool reposition = false);
			
protected:
	cdstrvect			mTitles;			// Array of titles
	bool				mCanSort;			// Can sort by column
	bool				mCanSize;			// Can size by column
	long				mValue;				// Sort column
	TableIndexT			mTrackingCol;		// Column being sort tracked
	bool				mSortTracking;			// Tracking sort mouse down
	bool				mSortTrackingInside;	// Inside column while tracking sort
	bool				mMouseOverDivider;		// Mouse over column divider
	bool				mSizeTracking;			// Tracking mouse down
	CRect				mSizeTrackLimits;		// Limit of tracking
	CRect				mSizeTrackRect;			// Rect of divider during tracking
	CPoint				mSizeTrackPt;			// Track point
	CPoint				mSizeTrackMovePt;		// Track point
	UINT				mMenuID;			// ID for its menu
	CTableWindow*		mTableWindow;		// Its owning window

	virtual void	DrawBackground(CDC* pDC, const CRect& inLocalRect);
	virtual void	DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);
	virtual void	DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect);
	virtual void	DrawText(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect);
	virtual void	DrawIcon(CDC* pDC, SColumnInfo& col_info, UINT iconID, const CRect &cellRect);

	virtual bool	RightJustify(int col_type)									// Check for right justification
		{ return false; }

	void	OnInvertTracker(const CRect& rect);
			
protected:
	static HCURSOR sVertCursor;

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	DECLARE_MESSAGE_MAP()
};

#endif
