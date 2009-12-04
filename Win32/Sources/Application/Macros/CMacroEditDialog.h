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


// Header for CMacroEditDialog class

#ifndef __CMACROEDITDIALOG__MULBERRY__
#define __CMACROEDITDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CSimpleTitleTable.h"
#include "CTextTable.h"
#include "CTextMacros.h"

// Constants

// Classes

class CMacroEditTable : public CTextTable
{
public:
					CMacroEditTable();
	virtual 		~CMacroEditTable();

			void	SetData(CTextMacros* macros);

private:
	CTextMacros*	mCopy;

			void	InitTable();
	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDialog dialog

class CLetterWindow;

class CMacroEditDialog : public CHelpDialog
{
// Construction
public:
	CMacroEditDialog(CWnd* pParent = NULL);				// standard constructor

	static  void	PoseDialog();

// Dialog Data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacroEditDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_DATA(CMacroEditDialog)
	enum { IDD = IDD_MACROEDIT };
	CSimpleTitleTable	mTitles;
	CMacroEditTable		mTable;
	CButton				mEditBtn;
	CButton				mDeleteBtn;
	//}}AFX_DATA
	cdstring			mKey;
	CStatic				mKeyCtrl;

	unsigned char		mActualKey;
	CKeyModifiers		mActualMods;

	CTextMacros			mCopy;

	// Generated message map functions
	//{{AFX_MSG(CMacroEditDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnNewBtn();
	afx_msg void OnEditBtn();
	afx_msg void OnDeleteBtn();
	afx_msg void OnChooseBtn();
	//}}AFX_MSG

	void	SetDetails();
	void	GetDetails();

	DECLARE_MESSAGE_MAP()
};

#endif
