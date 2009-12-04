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


// CAdbkManagerWindow.h

#ifndef __CADBKMANAGERWINDOW__MULBERRY__
#define __CADBKMANAGERWINDOW__MULBERRY__

#include "CTableViewWindow.h"

#include "CAdbkManagerView.h"
#include "CToolbarView.h"

// Classes

class CAdbkManagerWindow : public CTableViewWindow
{
	DECLARE_DYNCREATE(CAdbkManagerWindow)

	friend class CAdbkManagerTable;

public:
	static CAdbkManagerWindow*		sAdbkManager;
	static CMultiDocTemplate*		sAdbkManagerDocTemplate;

				CAdbkManagerWindow();
	virtual		~CAdbkManagerWindow();

	static void CreateAdbkManagerWindow();
	static void DestroyAdbkManagerWindow();
	static CAdbkManagerWindow* ManualCreate();				// Manually create document

	CAdbkManagerView*	GetAdbkManagerView()
		{ return static_cast<CAdbkManagerView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return &mToolbarView; }

private:
	CToolbarView		mToolbarView;
	CAdbkManagerView 	mAdbkView;
	bool				mHidden;						// Hide state

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	DECLARE_MESSAGE_MAP()
};

#endif
