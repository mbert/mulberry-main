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


// CQuickSearchText.h : header file
//

#ifndef __CQUICKSEARCHTEXT__MULBERRY__
#define __CQUICKSEARCHTEXT__MULBERRY__

#include "CCmdEdit.h"
#include "CBroadcaster.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText window

class CQuickSearchText : public CCmdEdit, public CBroadcaster
{
// Construction
public:
	enum
	{
		eBroadcast_Return = 'QSTr',
		eBroadcast_Tab = 'QSTt',
		eBroadcast_Key = 'QSTk'
	};

	CQuickSearchText();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickSearchText)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQuickSearchText();

	// Generated message map functions
protected:
	//{{AFX_MSG(CQuickSearchText)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

private:
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
