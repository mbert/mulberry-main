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

#include "CTableViewWindow.h"

#include "CAddressListFwd.h"
#include "CAddressList.h"
#include "CAddressBookView.h"
#include "CAddressView.h"
#include "CToolbarView.h"
#include "CSplitterView.h"

#include "cdmutexprotect.h"

// Classes

class CAddressBook;
class CAddressBookDoc;
class CAddressBookDocTemplate;

class CAddressBookWindow : public CTableViewWindow
{
	DECLARE_DYNCREATE(CAddressBookWindow)

public:
	static CAddressBookDocTemplate* sAddressBookDocTemplate;
	typedef vector<CAddressBookWindow*>	CAddressBookWindowList;
	static cdmutexprotect<CAddressBookWindowList> sAddressBookWindows;	// List of windows (protected for multi-thread access)


					CAddressBookWindow();
	virtual 		~CAddressBookWindow();
	static CAddressBookDoc* ManualCreate(CAddressBook* adbk, const char* fname);

	static bool OpenWindow(CAddressBook* adbk);
	static CAddressBookWindow*	FindWindow(const CAddressBook* adbk);	// Check for window
	static bool	WindowExists(const CAddressBookWindow* wnd);			// Check for window

	virtual void	OnDraw(CDC* pDC) {}

	CAddressBookView*	GetAddressBookView()
		{ return static_cast<CAddressBookView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return &mToolbarView; }

	virtual CSplitterView* GetSplitter()
		{ return &mSplitterView; }

	virtual void		SetDirty(bool dirty);			// Set dirty state
	virtual bool		IsDirty(void);						// Get dirty state

	CAddressBook*	GetAddressBook();
	void			SetAddressBook(CAddressBook* adbk);		// Set the address book
	void			ResetAddressBook();						// Address book changed

protected:
	CToolbarView		mToolbarView;
	CSplitterView		mSplitterView;
	CAddressBookView	mAddressBookView;
	CAddressView		mAddressView;
	bool				mPreviewVisible;

			void	ShowPreview(bool preview);

	// Command updaters
	afx_msg void	OnUpdateMenuShowPreview(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateShowPreview(CCmdUI* pCmdUI);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowPreview(void);

	DECLARE_MESSAGE_MAP()
};

#endif
