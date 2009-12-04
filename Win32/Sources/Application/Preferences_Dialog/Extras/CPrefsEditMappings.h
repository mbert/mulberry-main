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


// CPrefsEditMappings.h : header file
//

#ifndef __CPREFSEDITMAPPINGS__MULBERRY__
#define __CPREFSEDITMAPPINGS__MULBERRY__

#include "CHelpDialog.h"
#include "CMIMEMap.h"
#include "CSimpleTitleTable.h"
#include "CTable.h"

#include <vector>

class CMIMEMap;

class CEditMappingsTable : public CTable
{
public:
					CEditMappingsTable();
	virtual 		~CEditMappingsTable();

	virtual	void	InitTable(void);
	virtual	void	SetMap(CMIMEMapVector* theMap);


	virtual	void	NewMap(void);
	virtual	void	EditMap(void);

	virtual void	RemoveRows(UInt32		inHowMany,
								TableIndexT	inFromRow,
								bool refresh);						// Remove rows

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual	void	LDblClickCell(const STableCell& inCell, UINT nFlags);				// Clicked somewhere

private:
	CMIMEMapVector*	mItsMap;

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings dialog

class CPrefsEditMappings : public CHelpDialog
{
// Construction
public:
	CPrefsEditMappings(CWnd* pParent = NULL);   // standard constructor
	~CPrefsEditMappings();

// Dialog Data
	//{{AFX_DATA(CPrefsEditMappings)
	enum { IDD = IDD_PREFS_MAPPINGS };
	CSimpleTitleTable	mTitles;
	CEditMappingsTable	mTable;
	//}}AFX_DATA

	void	SetMap(CMIMEMapVector* theMap)
					{ mItsMap = theMap; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsEditMappings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsEditMappings)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddMap();
	afx_msg void OnChangeMap();
	afx_msg void OnDeleteMap();
	//}}AFX_MSG

private:
	CMIMEMapVector*	mItsMap;

	DECLARE_MESSAGE_MAP()
};

#endif
