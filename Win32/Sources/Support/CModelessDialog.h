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


// CModelessDialog.h : header file
//

#ifndef __CMODELESSDIALOG__MULBERRY__
#define __CMODELESSDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CModelessDialog dialog

class CModelessDialog : public CHelpDialog
{
	DECLARE_DYNAMIC(CModelessDialog)

// Construction
public:

	CModelessDialog();					   // standard constructor
	~CModelessDialog();

	virtual void PostNcDestroy();

// Implementation
protected:

	virtual uint32_t&	TitleCounter() = 0;

	// Generated message map functions
	//{{AFX_MSG(CModelessDialog)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
