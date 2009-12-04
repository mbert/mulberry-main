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


// Header for CRulesTable class

#ifndef __CRULESTABLE__MULBERRY__
#define __CRULESTABLE__MULBERRY__

#include "CTableDragAndDrop.h"

#include "CFilterItem.h"
#include "CWindowStatesFwd.h"

#include "cdstring.h"

// Classes
class CMessageList;
class CRulesWindow;
class CScriptsTable;

class CRulesTable : public CTableDragAndDrop
{
	friend class CRulesWindow;

	enum
	{
		eRulesIconRule = 0
	};

private:
	CRulesWindow*			mWindow;				// Owner window
	CColumnInfoArray*		mColumnInfo;
	ulvector				mDragged;
	unsigned long			mDropRow;
	CFilterItem::EType		mType;
	CScriptsTable*			mScriptsTable;

public:
					CRulesTable();
	virtual 		~CRulesTable();

	virtual void	ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

			void		SetFilterType(CFilterItem::EType type);
			void		SetScriptsTable(CScriptsTable* table)
				{ mScriptsTable = table; }

	bool	AddSelectionToList(TableIndexT row, CFilterItemList* list);		// Add selected rules to list

protected:

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	void	OnNewRules(void);

	void	OnEditRules(void);
	bool	EditRules(TableIndexT row);						// Display a specified address book

	void	OnDeleteRules(void);
	bool	DeleteRules(TableIndexT row);						// Delete a specified address book

	void	OnApplyRules(void);

	void	MakeRule(const CMessageList& msgs);						// Make rule from example messages

public:
	virtual void	DoSelectionChanged(void);
	virtual void	UpdateButtons(void);							// Update buttons
	virtual void	ResetTable(void);								// Reset the table from the mboxList

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

// Drag methods
	virtual void	GetSelection(ulvector& selection) const;

	virtual bool	AddSelectionToDrag(TableIndexT row, CFilterItems* list);		// Add row to list
	virtual BOOL 	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual bool	DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size, const STableCell& cell);	// Drop data at cell
};

#endif
