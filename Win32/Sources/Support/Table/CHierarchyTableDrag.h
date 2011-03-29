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


// CHierarchyTableDrag.h

// Header file for hierarchy table class that implements drag and drop extras

#ifndef __CHIERARCHYTABLEDRAG__MULBERRY__
#define __CHIERARCHYTABLEDRAG__MULBERRY__

#include <WIN_LHierarchyTable.h>

#include "templs.h"

#include <time.h>

// Classes
class CHierarchyTableDrag : public LHierarchyTable
{

public:
	DECLARE_DYNCREATE(CHierarchyTableDrag)

					CHierarchyTableDrag();
	virtual			~CHierarchyTableDrag();

	virtual void	Clear(void);							// Remove everything from table
	
	virtual const char* GetRowText(UInt32 inWideOpenIndex);	// Get text for row

	virtual Boolean		HasSibling(
								UInt32			inWideOpenIndex);
	virtual Boolean		HasParent(
								UInt32			inWideOpenIndex);
	virtual Boolean		HasChildren(
								UInt32			inWideOpenIndex);
	virtual bool		HasTwister(
								UInt32			inWideOpenIndex)
		{ return IsCollapsable(inWideOpenIndex) && HasChildren(inWideOpenIndex); }

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual	bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle character

			void	SetKeySelection(bool key)
		{ mKeySelection = key; }
	virtual void	DoKeySelection();						// Select row after typing

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragStartScroll(CWnd* pWnd);
	virtual DROPEFFECT OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

	virtual bool	CanDropExpand(COleDataObject* pDataObject, TableIndexT woRow);				// Can row expand for drop
	virtual void	DoDropExpand(COleDataObject* pDataObject, TableIndexT woRow);				// Do row expand for drop
	virtual void	DrawDropCell(COleDataObject* pDataObject, const STableCell& cell);				// Draw drag row frame

protected:
	ulvector		mExpandedRows;								// Array of rows expanded during current drag
	clock_t			mTimeInCell;								// Time in current cell
	bool			mRowWasSelected;							// Drop row was selected before mouse moved over it
	bool			mKeySelection;								// Select row as user types
	unsigned long	mLastTyping;								// Time of last typed character
	char			mLastChars[32];								// Last characters typed

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
