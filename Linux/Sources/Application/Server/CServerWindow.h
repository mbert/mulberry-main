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
	friend class CMulberryApp;
	friend class CServerTable;

public:
	typedef std::vector<CServerWindow*> CServerWindowList;
	static cdmutexprotect<CServerWindowList> sServerWindows;	// List of windows (protected for multi-thread access)

	CServerWindow(JXDirector* owner);
	virtual ~CServerWindow();

	virtual void OnCreate();

	void SetTitle(const JCharacter* title)
		{GetWindow()->SetTitle(title);}

	static CServerWindow* ManualCreate(void);			// Manually create document
	static bool	WindowExists(const CServerWindow* wnd);	// Check for window

	virtual bool AttemptClose();							// Attempt 

	CServerView* GetServerView()
		{ return static_cast<CServerView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

private:
// begin JXLayout


// end JXLayout
	CToolbarView*	mToolbarView;
	CServerView* 	mServerView;
	bool			mDoQuit;

};


#endif
