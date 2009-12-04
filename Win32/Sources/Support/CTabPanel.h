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


// CTabPanel

// Class that implements a tab panel in a CTabCtrl

#ifndef __CTABPANEL__MULBERRY__
#define __CTABPANEL__MULBERRY__

#include "CHelpDialog.h"

class CTabPanel : public CHelpDialog
{
	DECLARE_DYNAMIC(CTabPanel)

public:
					CTabPanel(LPCTSTR lpszTemplateName);
					CTabPanel(UINT nIDTemplate);
	virtual 		~CTabPanel();

	virtual BOOL Create(const RECT&, CWnd*, UINT, CCreateContext*);

	virtual void	SetState(unsigned long state)
		{ mState = state; }

	virtual void	SetContent(void* data) = 0;			// Set data
	virtual bool	ValidateContent(void* data)			// Validate and update data
		{ UpdateContent(data); return true; }
	virtual bool	UpdateContent(void* data) = 0;		// Force update of data

protected:
	LPCTSTR m_lpszTemplateName;
	CCreateContext* m_pCreateContext;
	unsigned long mState;

	//{{AFX_MSG(CTabPanel)
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
