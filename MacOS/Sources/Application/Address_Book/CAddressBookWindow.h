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
#include "CAddressBookView.h"

#include "cdmutexprotect.h"

// Consts
const	PaneIDT		paneid_AddressBookWindow = 9000;
const	PaneIDT		paneid_AddressBookToolbarView = 'TBar';
const	PaneIDT		paneid_AddressBookSplitter = 'TSPL';
const	PaneIDT		paneid_AddressBookPreview = 'PRVU';

// Messages

// Resources
const	ResIDT		STRx_AddressBookHelp = 9000;
enum {
	str_AddressTable = 1,
	str_GroupTable,
	str_SingleBtn,
	str_GroupBtn,
	str_NewLetterBtn,
	str_NewEntryBtn,
	str_DeleteEntryBtn,
	str_SortByMenu,
	str_ShowByMenu
};

// Others

// Types

// Classes
class CAddressBook;
class CAddressBookView;
class CSplitterView;

class CAddressBookWindow : public CTableViewWindow
{
public:
	typedef std::vector<CAddressBookWindow*>	CAddressBookWindowList;
	static cdmutexprotect<CAddressBookWindowList> sAddressBookWindows;	// List of windows (protected for multi-thread access)

	enum { class_ID = 'ABWi' };
					CAddressBookWindow();
					CAddressBookWindow(LStream *inStream);
	virtual 		~CAddressBookWindow();

	static bool OpenWindow(CAddressBook* adbk);
	static CAddressBookWindow*	FindWindow(const CAddressBook* adbk);	// Check for window
	static bool	WindowExists(const CAddressBookWindow* wnd);			// Check for window

	CAddressBookView*	GetAddressBookView()
		{ return static_cast<CAddressBookView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

	virtual CSplitterView* GetSplitter()
		{ return mSplitter; }

	virtual void		SetDirty(bool dirty);			// Set dirty state
	virtual bool		IsDirty(void);						// Get dirty state

	CAddressBook* GetAddressBook();
	void	SetAddressBook(CAddressBook* adbk);		// Set the address book
	void	ResetAddressBook();						// Address book changed
	
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:
	CToolbarView*		mToolbarView;
	CSplitterView*		mSplitter;
	bool				mPreviewVisible;

	virtual void	FinishCreateSelf(void);					// Do odds & ends

			void	ShowPreview(bool preview);

private:
			void	InitAddressBookWindow(void);
};

#endif
