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


// Header for CAddressPaneOptions class

#ifndef __CADDRESSPANEOPTIONS__MULBERRY__
#define __CADDRESSPANEOPTIONS__MULBERRY__

#include "CCommonViewOptions.h"

#include "C3PaneOptions.h"

class CAddressPaneOptions : public CCommonViewOptions
{
	DECLARE_DYNCREATE(CAddressPaneOptions)

// Construction
public:
	CAddressPaneOptions();
	~CAddressPaneOptions();

	void	SetData(const CUserAction& listPreview,
					const CUserAction& listFullView,
					const CUserAction& itemsPreview,
					const CUserAction& itemsFullView,
					const CAddressViewOptions& options,
					bool is3pane);
	void	GetData(CUserAction& listPreview,
					CUserAction& listFullView,
					CUserAction& itemsPreview,
					CUserAction& itemsFullView,
					CAddressViewOptions& options);

// Dialog Data
	//{{AFX_DATA(CAddressPaneOptions)
	enum { IDD = IDD_3PANE_ADDRESSOPTIONS };
	BOOL	mAddressSelect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAddressPaneOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAddressPaneOptions)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
