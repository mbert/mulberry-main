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


//	CACLTable.h

#ifndef __CACLTABLE__MULBERRY__
#define __CACLTABLE__MULBERRY__

#include "CTableDragAndDrop.h"

#include "CAdbkACL.h"
#include "CCalendarACL.h"
#include "CMboxACL.h"

#include <vector>
class JXImage;
typedef std::vector<JXImage*> CIconArray;

// Classes

class CACLTable : public CTableDragAndDrop
{
	enum
	{
		eACLDiamondTickedIcon,
		eACLDiamondIcon,
		eACLSetIcon,
		eACLUnsetIcon
	};

public:
		CACLTable(JXScrollbarSet* scrollbarSet,
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
		virtual ~CACLTable();

	virtual void		OnCreate();				// Get details of sub-panes
	virtual void		SetList(CMboxACLList* aList, bool read_write);
	virtual void		SetList(CAdbkACLList* aList, bool read_write);
	virtual void		SetList(CCalendarACLList* aList, bool read_write);
	virtual void		SetMyRights(SACLRight rights);					// Set indicators from rights

protected:
	virtual bool		ClickSelect(const STableCell &inCell, const JXKeyModifiers& modifiers);
	virtual void		LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);				// Clicked somewhere
	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell,
									const JRect& inLocalRect);					// Draw the string
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

private:
	union
	{
		CMboxACLList*		mMboxACLs;
		CAdbkACLList*		mAdbkACLs;
		CCalendarACLList*	mCalACLs;
	};
	bool		mMbox;
	bool		mAdbk;
	bool		mCalendar;
	bool		mReadWrite;
	SACLRight 	mMyRights;
	CIconArray 	mIcons;
	cdstring	mCurrentUser;

};

#endif
