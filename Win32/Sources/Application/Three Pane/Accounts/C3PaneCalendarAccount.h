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


// Header for C3PaneCalendarAccount class

#ifndef __C3PANECALENDARACCOUNT__MULBERRY__
#define __C3PANECALENDARACCOUNT__MULBERRY__

#include "C3PaneViewPanel.h"

#include "CCalendarStoreView.h"

// Classes
class C3PaneCalendarAccount : public C3PaneViewPanel
{
public:
					C3PaneCalendarAccount();
	virtual 		~C3PaneCalendarAccount();

	virtual CBaseView*	GetBaseView() const;
	CCalendarStoreView*	GetCalendarStoreView()
		{ return &mCalendarStoreView; }

protected:
	CCalendarStoreView	mCalendarStoreView;

	// Generated message map functions
	//{{AFX_MSG()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
