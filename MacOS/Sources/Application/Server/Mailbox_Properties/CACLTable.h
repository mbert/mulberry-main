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

#include "CTableDrag.h"

#include "CAdbkACL.h"
#include "CCalendarACL.h"
#include "CMboxACL.h"

// Classes

class CACLTable : public CTableDrag
{
private:
	union
	{
		CMboxACLList*		mMboxACLs;
		CAdbkACLList*		mAdbkACLs;
		CCalendarACLList*	mCalACLs;
	};
	bool	mMbox;
	bool	mAdbk;
	bool	mCalendar;
	bool	mReadWrite;

public:
	enum { class_ID = 'ACtb' };
	
						CACLTable(LStream *inStream);
	virtual				~CACLTable();

	virtual void		SetList(CMboxACLList* aList, bool read_write);
	virtual void		SetList(CAdbkACLList* aList, bool read_write);
	virtual void		SetList(CCalendarACLList* aList, bool read_write);

protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void	HiliteCellActively(const STableCell &inCell,
										Boolean inHilite);
	virtual void	HiliteCellInactively(const STableCell &inCell,
										Boolean inHilite);


	virtual Boolean		ClickSelect(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);
	virtual void		ClickCell(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);

public:
	virtual void		AdaptToNewSurroundings(void);						// Adjust column widths

};

#endif
