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


// Header for CWindowOptionsDialog class

#ifndef __CWINDOWOPTIONSDIALOG__MULBERRY__
#define __CWINDOWOPTIONSDIALOG__MULBERRY__

#include "CHelpPropertySheet.h"
#include "CMessagePaneOptions.h"
#include "CAddressPaneOptions.h"
#include "CCalendarPaneOptions.h"

#include "CUserAction.h"
#include "C3PaneOptions.h"
#include "C3PaneWindowFwd.h"

// Constants

// Type
class CUserActionOptions;

class CWindowOptionsDialog : public CHelpPropertySheet
{
public:
					CWindowOptionsDialog(CWnd* pParent);
	virtual 		~CWindowOptionsDialog();

	static bool 	PoseDialog();

	//virtual void	BuildPropPageArray();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMailboxPropDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	CMessagePaneOptions		mMessage;
	CAddressPaneOptions		mAddress;
	CCalendarPaneOptions	mCalendar;

	CUserAction				mUserActions[N3Pane::eView_Total][C3PaneOptions::C3PaneViewOptions::eUserAction_Total];		// User actions to trigger viewing
	CMailViewOptions		mMailOptions;
	CAddressViewOptions		mAddressOptions;
	CCalendarViewOptions	mCalendarOptions;
	bool					mIs3Pane;

private:
	void	SetDetails(C3PaneOptions* options, bool is3pane);
	void	GetDetails(C3PaneOptions* options);

	DECLARE_MESSAGE_MAP()
};

#endif
