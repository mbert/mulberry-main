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


// CIMAPLabelsDialog.h : header file
//

#ifndef __CIMAPLabelsDialog__MULBERRY__
#define __CIMAPLabelsDialog__MULBERRY__

#include "CHelpDialog.h"

#include "CMessageFwd.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CIMAPLabelsDialog dialog

class CIMAPLabelsDialog : public CHelpDialog
{
// Construction
public:
	CIMAPLabelsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIMAPLabelsDialog)
	enum { IDD = IDD_IMAPLABELS };
	cdstring mNames[NMessage::eMaxLabels];
	cdstring mLabels[NMessage::eMaxLabels];
	//}}AFX_DATA

	static bool		PoseDialog(const cdstrvect& names, cdstrvect& labels);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMAPLabelsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
			void	SetDetails(const cdstrvect& names, const cdstrvect& labels);
			bool	GetDetails(cdstrvect& labels);
			bool	ValidLabel(const cdstring& label);

	// Generated message map functions
	//{{AFX_MSG(CIMAPLabelsDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
