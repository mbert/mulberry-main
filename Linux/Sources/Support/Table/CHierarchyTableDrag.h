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

#include "UNX_LHierarchyTable.h"

#include "templs.h"

#include <time.h>

// Classes
class CHierarchyTableDrag : public LHierarchyTable
{

public:
  CHierarchyTableDrag(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
  virtual ~CHierarchyTableDrag();

  virtual void	Clear(void);							// Remove everything from table

  virtual const char* GetRowText(UInt32 inWideOpenIndex);	// Get text for row

  virtual bool HasSibling(UInt32 inWideOpenIndex);
  virtual bool HasParent(UInt32 inWideOpenIndex);
  virtual bool HasChildren(UInt32 inWideOpenIndex);
  virtual bool HasTwister(UInt32 inWideOpenIndex)
    { return IsCollapsable(inWideOpenIndex) && HasChildren(inWideOpenIndex); }

 protected:
	virtual void HandleDNDEnter();
	virtual void HandleDNDLeave();
	virtual void HandleDNDHere(const JPoint& pt, const JXWidget* source);

	virtual bool HandleChar(const int key,  const JXKeyModifiers& modifiers);

			void	SetKeySelection(bool key)
		{ mKeySelection = key; }
	virtual void	DoKeySelection();						// Select row after typing

	virtual bool	CanDropExpand(const JArray<Atom>& typeList, int woRow); // Can row expand for drop
	virtual void	DoDropExpand(int woRow); // Do row expand for drop
	virtual void	DrawDropCell(const STableCell& cell); // Draw drag row frame
	virtual void	ClearDropCell(const STableCell& cell); // Clear drag row frame

	TableIndexT		mPreviousDropRow;
 protected:
 	ulvector		mExpandedRows;							// Array of rows expanded during current drag
 	time_t			mTimeInCell;     						// Time in current cell
 	bool			mRowWasSelected;						// Drop row was selected before mouse moved over it
	bool			mKeySelection;							// Select row as user types
	unsigned long	mLastTyping;							// Time of last typed character
	char			mLastChars[32];							// Last characters typed

};

#endif
