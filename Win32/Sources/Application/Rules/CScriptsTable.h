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


// Header for CScriptsTable class

#ifndef __CSCRIPTSTABLE__MULBERRY__
#define __CSCRIPTSTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CFilterItem.h"
#include "CWindowStatesFwd.h"

// Classes
class CRulesWindow;

class CScriptsTable : public CHierarchyTableDrag
{

	DECLARE_DYNCREATE(CScriptsTable)

	enum
	{
		eScriptsIconTarget = 0,
		eScriptsIconScripts,
		eScriptsIconRule
	};


public:
					CScriptsTable();
	virtual 		~CScriptsTable();

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

			void		SetFilterType(CFilterItem::EType type);

	virtual void		DoSelectionChanged();
	virtual void		UpdateButtons();								// Update buttons
			void		ResetTable(void);									// Reset the table from the entry list

protected:
	CRulesWindow*			mWindow;				// Owner window
	CColumnInfoArray*		mColumnInfo;
	CFilterItem::EType		mType;

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);

	void		OnNewScript();									// Create a new address
	void		OnEditScript();									// Edit selected entries
	bool		EditScript(TableIndexT row);					// Edit specified address
	void		OnDeleteScript();								// Delete selected entries
	bool		DeleteScripts(TableIndexT row);					// Delete a specified entry
	void		OnGenerateScript();								// Generate selected entries
	bool		GenerateScript(TableIndexT row, void* data);	// Generate a specified entry

	// Drag & Drop
	virtual bool		IsDropCell(COleDataObject* pDataObject, const STableCell& cell);			// Draw drag row frame
	virtual bool		IsDropAtCell(COleDataObject* pDataObject, STableCell& cell);			// Can cell be dropped at (modify if not)
	virtual DROPEFFECT GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point); // Determine effect
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);							// Drop data into cell
	virtual bool		DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);							// Drop data at cell
private:
	bool				TestSelectionScript(TableIndexT row);				// Test for selected item script
	bool				TestSelectionRule(TableIndexT row);					// Test for selected item rule
	bool				TestCellSameScript(TableIndexT row,
												TableIndexT* parent);		// Test for selected item group

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

};

#endif
