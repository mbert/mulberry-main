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


// Header for CNamespaceDialog class

#ifndef __CNAMESPACEDIALOG__MULBERRY__
#define __CNAMESPACEDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CSimpleTitleTable.h"
#include "CTable.h"

#include "CMboxProtocol.h"

// Constants

// Classes

class CNamespaceTable : public CTable
{
public:
	enum ENamespaceSelect
	{
		eNamespace_Personal,
		eNamespace_Shared,
		eNamespace_Public,
		eNamespace_All,
		eNamespace_None
	};

					CNamespaceTable();
	virtual 		~CNamespaceTable();

			void	InitTable();
			void	SetServerList(CMboxProtocol::SNamespace& servs, boolvector& servitems);		// Set namespaces
			void	ChangeSelection(ENamespaceSelect select);

protected:
	virtual	void	LClickCell(const STableCell& inCell, UINT nFlags);				// Clicked somewhere

private:
	CMboxProtocol::SNamespace*	mData;
	boolvector*					mDataOn;
	cdstrvect					mTypeItems;

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNamespaceDialog dialog

class CNamespaceDialog : public CHelpDialog
{
// Construction
public:
	CNamespaceDialog(CWnd* pParent = NULL);   // standard constructor
	~CNamespaceDialog();   					// standard destructor

// Dialog Data
	//{{AFX_DATA(CNamespaceDialog)
	enum { IDD = IDD_NAMESPACE };
	CSimpleTitleTable	mTitles;
	CNamespaceTable		mTable;
	cdstring			mHelpText;
	BOOL				mDoAuto;
	cdstring			mUserItems;
	//}}AFX_DATA

	static  bool	PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto);

			void	SetItems(const cdstrvect& items,						// Set namespaces
								CMboxProtocol::SNamespace& servs,
								boolvector& servitems, bool do_auto);
			void	GetItems(cdstrvect& items, bool& do_auto);				// Get selected items
	CNamespaceTable* GetTable(void)
			{ return &mTable; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNamespaceDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMboxProtocol::SNamespace* mServs;
	boolvector* mServitems;

			void HideServerItems();


	// Generated message map functions
	//{{AFX_MSG(CNamespaceDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectPersonalBtn();
	afx_msg void OnSelectSharedBtn();
	afx_msg void OnSelectPublicBtn();
	afx_msg void OnSelectAllBtn();
	afx_msg void OnSelectNoneBtn();
	//}}AFX_MSG

private:

	DECLARE_MESSAGE_MAP()
};

#endif
