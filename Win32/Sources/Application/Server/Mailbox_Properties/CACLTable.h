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

// Classes

class CACLTable : public CTableDragAndDrop
{

	DECLARE_DYNCREATE(CACLTable)

public:
						CACLTable();
	virtual				~CACLTable();

	virtual BOOL 		SubclassDlgItem(UINT nID, CWnd* pParent);
	virtual void		SetList(CMboxACLList* aList, bool read_write);
	virtual void		SetList(CAdbkACLList* aList, bool read_write);
	virtual void		SetList(CCalendarACLList* aList, bool read_write);
	virtual void		SetMyRights(SACLRight rights);					// Set indicators from rights

	// Key/Mouse related
	virtual bool	ClickSelect(const STableCell &inCell, UINT	nFlags);
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);	// Clicked item

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
	SACLRight 			mMyRights;
	static cdstring		sCurrentUser;

protected:

	// message handlers
	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

#endif
