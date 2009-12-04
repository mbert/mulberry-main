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


// CAdbkPropDialog.h : header file
//

#ifndef __CADBKPROPDIALOG__MULBERRY__
#define __CADBKPROPDIALOG__MULBERRY__

#include "CHelpPropertySheet.h"
#include "CPropAdbkGeneral.h"
#include "CPropAdbkACL.h"

#include "CAdbkList.h"

/////////////////////////////////////////////////////////////////////////////
// CAdbkPropDialog dialog

class CAdbkPropDialog : public CHelpPropertySheet
{
	DECLARE_DYNAMIC(CAdbkPropDialog)

// Construction
public:
	CAdbkPropDialog(CWnd* pParent = NULL);   // standard constructor

	void	SetAdbkList(CFlatAdbkList* adbk_list);								// Set mbox list

	//virtual void BuildPropPageArray();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdbkPropDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG

private:
	CFlatAdbkList*		mAdbkList;											// List of selected mboxes

	CPropAdbkGeneral	mPropAdbkGeneral;
	CPropAdbkACL		mPropAdbkACL;

	DECLARE_MESSAGE_MAP()
};

#endif
