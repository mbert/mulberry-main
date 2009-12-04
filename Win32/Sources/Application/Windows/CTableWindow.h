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


// Header for CTableWindow class

#ifndef __CTABLEWINDOW__MULBERRY__
#define __CTABLEWINDOW__MULBERRY__

#include "CWindowStatus.h"
#include "CWindowStatesFwd.h"

// Classes
class CTitleTable;
class CTable;

class CTableWindow : public CView,
						public CWindowStatus
{
	friend class CTitleTable;
	friend class CTable;

public:
					CTableWindow();
	virtual 		~CTableWindow();

	CColumnInfoArray&	GetColumnInfo()
		{ return mColumnInfo; }

	virtual void	OnInitialUpdate(void);				// Focus
	virtual void	OnDraw(CDC* pDC) {}
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command


	virtual void	ResetTable(void) = 0;					// Reset the table

	virtual void	InitColumns(void) = 0;					// Init columns and text
	virtual void	InsertColumn(short col_pos);			// Insert a column
	virtual void	DeleteColumn(short col_pos);			// Delete a column
	virtual void	SetColumnType(short col_pos,
									int col_type);			// Set type of a column
	virtual void	SetColumnWidth(short col_pos,
									short col_width);		// Set width of column
	
	virtual int		GetSortBy(void);						// Get sort
	virtual void	SetSortBy(int sort);					// Force change of sort

	virtual int		GetShowBy(void);						// Get sort direction
	virtual void	SetShowBy(int sort);					// Force change of sort direction
	virtual void	ToggleShowBy();							// Force toggle of sort direction

	virtual void	ResetState(bool force = false) = 0;			// Reset window state
	virtual void	ResetColumns(const CColumnInfoArray& newCols);	// Reset column state
	virtual void	SaveState(void) {}							// Save current state in prefs
	virtual void	SaveDefaultState(void) = 0;					// Save current state as default

	virtual void	ResetFont(CFont* font);				// Reset list font

protected:
	CColumnInfoArray	mColumnInfo;
	CTitleTable*		mWndTitles;
	CTable*				mWndTable;
	int					mNewColumn;
	short				mNewSize;

	virtual void	PostCreate(CTable* tbl, CTitleTable* titles);
	virtual void	SetWindowInfo(int new_col_type, short new_size)
		{ mNewColumn = new_col_type; mNewSize = new_size; }

	// message handlers
	afx_msg void OnDestroy(void);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSaveDefaultState(void);				// Save state in prefs
	afx_msg void OnResetDefaultState(void);

	DECLARE_MESSAGE_MAP()
};

#endif
