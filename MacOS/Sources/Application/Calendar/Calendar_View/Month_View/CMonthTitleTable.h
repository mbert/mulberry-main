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

#ifndef H_CMONTHTITLETABLE
#define H_CMONTHTITLETABLE
#pragma once

#include "CSimpleTitleTable.h"

// ===========================================================================
//	MonthTitleTable

class	CMonthTitleTable : public CSimpleTitleTable {
public:
	enum { class_ID = 'CtiM' };

						CMonthTitleTable(LStream *inStream);
						
	virtual				~CMonthTitleTable();

	virtual void		AdaptToNewSurroundings();					// Adjust column widths
	
protected:
	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);					// Draw cell

};

#endif
