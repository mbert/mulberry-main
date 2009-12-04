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

// Constants
const	PaneIDT		paneid_MailboxHeader = 'MHDR';
const	PaneIDT		paneid_MailboxSplitter = 'SPLT';
const	PaneIDT		paneid_MailboxPreview = 'PRVU';

// Classes
class CMessage;
class CMessageView;
class CSplitterView;
class LBevelButton;

class CMailboxWindow : public CTableViewWindow
{
public:
	typedef std::vector<CMailboxWindow*>	CMailboxWindowList;
	static cdmutexprotect<CMailboxWindowList> sMboxWindows;	// List of windows (protected for multi-thread access)

public:
					CMailboxWindow();
					CMailboxWindow(LStream *inStream);
	virtual 		~CMailboxWindow();

	CMailboxView*	GetMailboxView()
		{ return static_cast<CMailboxView*>(GetTableView()); }

	CMessageView*	GetPreview()
		{ return mPreview; }

	CSplitterView*	GetSplitter()
		{ return mSplitter; }

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:
	CSplitterView*		mSplitter;
	CMessageView*		mPreview;
	bool				mPreviewVisible;

	virtual void	FinishCreateSelf(void);
	virtual void	BeTarget(void);
	virtual Boolean	AttemptQuitSelf(SInt32 inSaveOption);

			void	ShowPreview(bool preview);

public:
	static CMailboxWindow*	FindWindow(const CMbox* mbox);	// Find the corresponding window
	static bool	WindowExists(const CMailboxWindow* wnd);	// Check for window
};

#endif
