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


// Header for CMailboxWindow class

#ifndef __CMAILBOXWINDOW__MULBERRY__
#define __CMAILBOXWINDOW__MULBERRY__

#include "CTableViewWindow.h"
#include "CMainMenu.h"

#include "CMailboxView.h"

#include "cdmutexprotect.h"

// Classes
class CMailboxTable;
class CMailboxTitleTable;
class CMailboxView;
class CMessage;
class CMbox;
class CSplitterView;

class CMailboxWindow : public CTableViewWindow
{
public:
	typedef std::vector<CMailboxWindow*>	CMailboxWindowList;
	static cdmutexprotect<CMailboxWindowList> sMboxWindows;	// List of windows (protected for multi-thread access)

	CMailboxWindow(JXDirector* supervisor);
	virtual 		~CMailboxWindow();

	static CMailboxWindow*	FindWindow(const CMbox* mbox);	// Find the corresponding window
	static bool	WindowExists(const CMailboxWindow* wnd);	// Check for window

	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	CMailboxView*	GetMailboxView()
		{ return static_cast<CMailboxView*>(GetTableView()); }

	CSplitterView*	GetSplitter()
		{ return mSplitter; }

	// Command updaters
	void	OnUpdateShowPreview(CCmdUI* pCmdUI);
	void	OnUpdateShowPreviewBtn(CCmdUI* pCmdUI);

	void	ShowPreview(bool preview);

protected:
	CSplitterView*		mSplitter;
	bool				mPreviewVisible;
};

#endif
