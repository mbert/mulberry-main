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


// Header for CMailboxTitleTable class

#ifndef __CMAILBOXTITLETABLE__MULBERRY__
#define __CMAILBOXTITLETABLE__MULBERRY__

#include "CTitleTableView.h"

// Constants


// Classes
class	CMailboxTitleTable : public CTitleTableView
{
	enum
	{
		eMailboxTitleIconFlags = 0,
		eMailboxTitleIconAttachments,
		eMailboxTitleIconMatch,
		eMailboxTitleIconDisconnected,
		eMailboxTitleIconMax
	};

	enum
	{
		cColumnTo = 1,
		cColumnFrom,
		cColumnReplyTo,
		cColumnSender,
		cColumnCC,
		cColumnSubject,
		cColumnThread,
		cColumnDateSent,
		cColumnDateReceived,
		cColumnSize,
		cColumnFlags,
		cColumnNumber,
		cColumnSmart,
		cColumnAttachments,
		cColumnParts,
		cColumnMatch,
		cColumnDisconnected,
		cColumnInsertAfter,
		cColumnInsertBefore,
		cColumnDelete
	};

public:
	CMailboxTitleTable(JXScrollbarSet* scrollbarSet,	
						 JXContainer* enclosure,	
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, 
						 const JCoordinate y,
						 const JCoordinate w, 
						 const JCoordinate h);
	virtual 		~CMailboxTitleTable();

	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);				// Get text for current tooltip cell

protected:
	virtual void OnCreate();
	virtual void DrawItem(JPainter* pDC, SColumnInfo& col_info, const JRect &cellRect);
	virtual void MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item);
	virtual bool RightJustify(int col_type);									// Check for right justification

};

#endif
