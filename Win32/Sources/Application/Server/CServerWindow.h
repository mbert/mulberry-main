/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CServerWindow.h

#ifndef __CSERVERWINDOW__MULBERRY__
#define __CSERVERWINDOW__MULBERRY__

#include "CTableViewWindow.h"

#include "CServerView.h"
#include "CToolbarView.h"

#include "cdmutexprotect.h"

// Classes

class CServerWindow : public CTableViewWindow
{
	friend class CServerTable;

	DECLARE_DYNCREATE(CServerWindow)

public:
	typedef std::vector<CServerWindow*>	CServerWindowList;
	static cdmutexprotect<CServerWindowList> sServerWindows;	// List of windows (protected for multi-thread access)
	static CMultiDocTemplate*	sServerDocTemplate;

				CServerWindow();
	virtual		~CServerWindow();

	static bool	WindowExists(const CServerWindow* wnd);		// Check for window

	static CServerWindow* ManualCreate();					// Manually create document

	virtual bool AttemptClose();							// Attempt close

	CServerView*	GetServerView()
		{ return static_cast<CServerView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return &mToolbarView; }

private:
	CToolbarView	mToolbarView;
	CServerView 	mServerView;

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);

	DECLARE_MESSAGE_MAP()
};

#endif
