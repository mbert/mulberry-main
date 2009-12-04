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


// Header for CAddressBookWindow class

#ifndef __CADDRESSBOOKWINDOW__MULBERRY__
#define __CADDRESSBOOKWINDOW__MULBERRY__

#include "CAddressBookDoc.h"

#include "CAddressBookView.h"
#include "CAddressView.h"
#include "CToolbarView.h"
#include "CSplitterView.h"

#include "cdmutexprotect.h"

// Classes

class CAddressBook;

class CAddressBookWindow : public CAddressBookDoc
{
public:
	typedef std::vector<CAddressBookWindow*>	CAddressBookWindowList;
	static cdmutexprotect<CAddressBookWindowList> sAddressBookWindows;	// List of windows (protected for multi-thread access)

			CAddressBookWindow(JXDirector* owner);
	virtual	~CAddressBookWindow();

	static CAddressBookWindow* ManualCreate(CAddressBook* adbk, const char* fname);

	static bool OpenWindow(CAddressBook* adbk);
	static CAddressBookWindow*	FindWindow(const CAddressBook* adbk);	// Check for window
	static bool	WindowExists(const CAddressBookWindow* wnd);	// Check for window

	virtual void OnCreate();

	CAddressBookView*	GetAddressBookView()
		{ return mAddressBookView; }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

	virtual CSplitterView* GetSplitter()
		{ return mSplitterView; }

	virtual void		SetDirty(bool dirty);			// Set dirty state
	virtual bool		IsDirty();						// Get dirty state

	virtual CAddressBook* GetAddressBook();
	virtual void	SetAddressBook(CAddressBook* adbk);		// Set the address book

	// Common updaters
			void	OnUpdateShowPreview(CCmdUI* pCmdUI);
			void	OnUpdateShowPreviewBtn(CCmdUI* pCmdUI);

	// Command handlers
			void OnShowPreview();

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveDefaultState(void);						// Save current state as default

private:
// begin JXLayout
// end JXLayout

	static unsigned long	s_nUntitledCount;
	CToolbarView*		mToolbarView;
	CSplitterView*		mSplitterView;
	CAddressBookView*	mAddressBookView;
	CAddressView*		mAddressView;
	bool				mPreviewVisible;

			void	ShowPreview(bool preview);

	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

};

#endif
