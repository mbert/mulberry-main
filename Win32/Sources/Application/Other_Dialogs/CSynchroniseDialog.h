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


// CSynchroniseDialog.h : header file
//

#ifndef __CSYNCHRONISEDIALOG__MULBERRY__
#define __CSYNCHRONISEDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSynchroniseDialog dialog


class CMbox;
class CMboxList;

class CSynchroniseDialog : public CHelpDialog
{
// Construction
public:
	CSynchroniseDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSynchroniseDialog)
	enum { IDD = IDD_SYNCHRONISE };
	int		mAll;
	int		mFull;
	UINT	mSize;
	bool	mHasSelection;
	//}}AFX_DATA

	static  void	PoseDialog(CMboxList* mbox_list);
	static  void	PoseDialog(CMbox* mbox, ulvector& selection);
	static  bool	PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSynchroniseDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void	SetDetails(bool has_selection);								// Set the dialogs info
	virtual void	GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CSynchroniseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSynchroniseFull();
	afx_msg void OnSynchroniseBelow();
	afx_msg void OnSynchronisePartial();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif