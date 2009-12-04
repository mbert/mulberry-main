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


//	CAdbkACLTable.h

#ifndef __CADBKACLTABLE__MULBERRY__
#define __CADBKACLTABLE__MULBERRY__

#include "CTableDrag.h"
#include <LCommander.h>

#include "CMboxACL.h"

// Classes

class CAdbkACLTable : public CTableDrag,
						public LCommander
{
private:
	CMboxACLList*		mACLs;

public:
	enum { class_ID = 'ACtb' };
	static CAdbkACLTable*	CreateFromStream(LStream *inStream);
	
						CAdbkACLTable(LStream *inStream);
	virtual				~CAdbkACLTable();

	virtual void		SetList(CAdbkACLList* aList);

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											Char16 &outMark,
											Str255 outName);
protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void		HiliteCellActively(const STableCell &inCell,
											Boolean inHilite);
	virtual void		HiliteCellInactively(const STableCell &inCell,
											Boolean inHilite);

	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);
	virtual void		ClickCell(const STableCell &inCell,
								const SMouseDownEvent &inMouseDown);		// Click in the cell

public:
	virtual void		AdaptToNewSurroundings(void);						// Adjust column widths

};

#endif
