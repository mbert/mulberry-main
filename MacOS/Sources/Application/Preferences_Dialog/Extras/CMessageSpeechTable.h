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


//	CMessageSpeechTable.h

#ifndef __CMESSAGESPEECHTABLE__MULBERRY__
#define __CMESSAGESPEECHTABLE__MULBERRY__

#include "CTableDrag.h"

#include "CSpeechSynthesis.h"


#include <vector>

// Classes
class CMIMEMap;
class LFocusBox;

class CMessageSpeechTable : public CTableDrag
{

private:
	TextTraitsH				mTextTraits;
	CMessageSpeakVector*	mList;

public:
	enum { class_ID = 'MStb' };
	
						CMessageSpeechTable(LStream *inStream);
	virtual				~CMessageSpeechTable();

protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:
	virtual void		ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown);
	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);
	virtual void		DeleteSelection(void);
	virtual bool		RemoveFromList(TableIndexT row, CMessageSpeakVector* list);

public:
	virtual void		EditEntry(const STableCell& aCell);
	virtual void		SetList(CMessageSpeakVector* list);
	virtual CMessageSpeakVector*	GetList(void)
								{ return mList; }
	virtual void		ResetTable(void);							// Reset the table from the body

};

#endif
