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

#include "CMailboxView.h"

#include "cdmutexprotect.h"

// Classes
class CMailboxTable;
class CMbox;
class CMessage;
class CSplitterView;

class CMailboxWindow : public CTableViewWindow
{
public:
	typedef vector<CMailboxWindow*>	CMailboxWindowList;
	static cdmutexprotect<CMailboxWindowList> sMboxWindows;	// List of windows (protected for multi-thread access)

					CMailboxWindow();
	virtual 		~CMailboxWindow();

	CMailboxView*	GetMailboxView()
		{ return static_cast<CMailboxView*>(GetTableView()); }

			void	ShowPreview(bool preview);

	CFrameWnd* GetMailboxFrame()
		{ return mMailboxFrame; }

	CSplitterView*	GetSplitter()
		{ return mSplitter; }

	static CMailboxWindow*	FindWindow(const CMbox* mbox);	// Find the corresponding window
	static bool	WindowExists(const CMailboxWindow* wnd);	// Check for window

protected:
	CFrameWnd*			mMailboxFrame;
	CSplitterView*		mSplitter;
	bool				mPreviewVisible;

	// Command updaters
	afx_msg void	OnUpdateMenuShowPreview(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateShowPreview(CCmdUI* pCmdUI);

	// message handlers
	afx_msg void OnDestroy(void);
	afx_msg void OnShowPreview(void);
	DECLARE_MESSAGE_MAP()
};

#endif
