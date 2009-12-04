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

	DECLARE_DYNCREATE(CQuotaTable)

public:
						CQuotaTable();
	virtual				~CQuotaTable();

	virtual BOOL 		SubclassDlgItem(UINT nID, CWnd* pParent);
	virtual void		SetList(CQuotaRootList* aList);
	virtual void		ResetTable(void);								// Reset the table from the body

private:
	CQuotaRootList*		 	mQuotas;

	// message handlers
	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

#endif
