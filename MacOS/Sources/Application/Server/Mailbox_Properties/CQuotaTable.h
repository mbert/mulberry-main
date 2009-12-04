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


//	CQuotaTable.h

#ifndef __CQUOTATABLE__MULBERRY__
#define __CQUOTATABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CQuotas.h"

// Classes

class CQuotaTable : public CHierarchyTableDrag
{

private:
	CQuotaRootList*		 	mQuotas;

public:
	enum { class_ID = 'Qutb' };
	
						CQuotaTable(LStream *inStream);
	virtual				~CQuotaTable();

	virtual void		SetList(CQuotaRootList* aList);

protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void		CalcCellFlagRect(
								const STableCell		&inCell,
								Rect					&outRect);						

public:
	virtual void		ResetTable(void);								// Reset the table from the body
	virtual void		AdaptToNewSurroundings(void);					// Adjust column widths

};

#endif
