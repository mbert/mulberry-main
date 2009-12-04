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


// Header for CServerWindow class

#ifndef __CSERVERWINDOW__MULBERRY__
#define __CSERVERWINDOW__MULBERRY__

#include "CTableViewWindow.h"

#include "CServerView.h"

#include "cdmutexprotect.h"

// Panes
const	PaneIDT		paneid_ServerWindow = 4000;
const	PaneIDT		paneid_ServerToolbarView = 'TBar';

// Constants

// Resources

// Classes
class CToolbarView;

class	CServerWindow : public CTableViewWindow
{
public:
	typedef std::vector<CServerWindow*>	CServerWindowList;
	static cdmutexprotect<CServerWindowList> sServerWindows;	// List of windows (protected for multi-thread access)

	enum { class_ID = 'SeWi' };

					CServerWindow();
					CServerWindow(LStream *inStream);
	virtual 		~CServerWindow();

	static bool	WindowExists(const CServerWindow* wnd);		// Check for window

	virtual void	AttemptClose(void);						// Hide instead of close

	CServerView*	GetServerView()
		{ return static_cast<CServerView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
	CToolbarView*	mToolbarView;

	virtual void	FinishCreateSelf(void);
};

#endif
