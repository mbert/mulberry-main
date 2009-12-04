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


//	CEditMappingsTable.h

#ifndef __CEDITMAPPINGSTABLE__MULBERRY__
#define __CEDITMAPPINGSTABLE__MULBERRY__

#include "CTableDrag.h"
#include "CMIMEMap.h"

// Classes
class CPrefsEditMappings;

class CEditMappingsTable : public CTableDrag
{

private:
	CMIMEMapVector*			mMappings;
	CPrefsEditMappings*		mWindow;
	TextTraitsH				mTextTraits;
	SInt32					mRowShow;

public:
	enum { class_ID = 'MPtb' };
	
						CEditMappingsTable(LStream *inStream);
	virtual				~CEditMappingsTable();

	virtual void		SetMappings(CMIMEMapVector* mapping);

	virtual void		SetRowShow(SInt32 rowShow)
							{ mRowShow = rowShow; Refresh(); }

protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

public:
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		SelectCell(const STableCell &inCell);		// Select the cell

protected:
	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual Boolean		ClickSelect(const STableCell &inCell,
									const SMouseDownEvent &inMouseDown);		// Select the cell
	virtual void		UnselectAllCells(void);						// Unselect all cells

public:
	virtual void		ResetTable(void);							// Reset the table from the body

};

#endif
