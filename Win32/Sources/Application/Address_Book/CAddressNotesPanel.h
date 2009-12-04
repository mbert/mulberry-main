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


// Header for CAddressNotesPanel class

#ifndef __CADDRESSNOTESPANEL__MULBERRY__
#define __CADDRESSNOTESPANEL__MULBERRY__

#include "CAddressPanelBase.h"

// Classes
class CAddressNotesPanel : public CAddressPanelBase
{
public:
					CAddressNotesPanel();
	virtual 		~CAddressNotesPanel() {}

	virtual void	Focus();

// Dialog Data
	//{{AFX_DATA(CEditAddressDialog)
	enum { IDD = IDD_ADDRESSNOTES };
	cdstring	mNotes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditAddressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog

	// Generated message map functions
	//{{AFX_MSG(CEditAddressDialog)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
