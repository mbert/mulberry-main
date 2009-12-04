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
public:
	CQuotaTable(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual				~CQuotaTable();

	virtual void		OnCreate();				// Get details of sub-panes
	virtual void		SetList(CQuotaRootList* aList);
	virtual void		ResetTable();								// Reset the table from the body

protected:
	virtual void		DrawCell(JPainter* pDC, const STableCell& cell,
									const JRect& inLocalRect);					// Draw the string

	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

private:
	CQuotaRootList*		 	mQuotas;

};

#endif
