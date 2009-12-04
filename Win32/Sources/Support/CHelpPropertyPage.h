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


// CHelpPropertyPage.h : header file
//

#ifndef __CHELPPROPERTYPAGE__MULBERRY__
#define __CHELPPROPERTYPAGE__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CHelpPropertyPage dialog

class CHelpPropertyPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CHelpPropertyPage)

// Construction
public:
	CHelpPropertyPage();
	CHelpPropertyPage(UINT nID);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHelpPropertyPage)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#endif
