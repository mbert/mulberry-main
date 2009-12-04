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


// Header for CAddressPanelBase class

#ifndef __CADDRESSPANELBASE__MULBERRY__
#define __CADDRESSPANELBASE__MULBERRY__

#include "CTabPanel.h"
#include "CWndAligner.h"

// Classes
class CAdbkAddress;

class CAddressPanelBase : public CTabPanel, public CWndAligner
{
public:
				CAddressPanelBase(UINT nIDTemplate);
	virtual 	~CAddressPanelBase();

	virtual void	SetContent(void* data);					// Set data
	virtual bool	UpdateContent(void* data);				// Force update of data

	virtual void	Focus() = 0;

protected:
	CAdbkAddress* mAddress;

	virtual void	SetFields(const CAdbkAddress* addr) = 0;	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr) = 0;			// Get fields from dialog

	// Generated message map functions
	//{{AFX_MSG(CEditAddressDialog)
		// NOTE: the ClassWizard will add member functions here
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef vector<CAddressPanelBase*> CAddressPanelList;

#endif
