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


// CIconWnd.h : header file
//

#ifndef __CICONWND__MULBERRY__
#define __CICONWND__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CIconWnd window

class CIconWnd : public CStatic
{
	DECLARE_DYNAMIC(CIconWnd)

// Construction
public:
	CIconWnd() { mIconID = 0; };
	virtual ~CIconWnd() { }

	virtual BOOL SubclassDlgItem(UINT nID, CWnd* pParent, UINT nIDIcon);

	void	SetIconID(UINT nID);

	// Generated message map functions
protected:
	UINT mIconID;

	//{{AFX_MSG(CIconWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
