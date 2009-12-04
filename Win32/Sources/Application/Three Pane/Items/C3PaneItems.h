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


// Header for C3PaneItems class

#ifndef __C3PANEITEMS__MULBERRY__
#define __C3PANEITEMS__MULBERRY__

#include "C3PaneParentPanel.h"

#include "CIconWnd.h"
#include "CToolbarButton.h"
#include "C3PaneItemsTabs.h"

// Classes
class CAddressBookView;
class CCalendarView;
class CMailboxInfoView;
class C3PaneMailbox;
class C3PaneAddressBook;
class C3PaneCalendar;

class C3PaneItems : public C3PaneParentPanel
{
public:
		
					C3PaneItems();
	virtual 		~C3PaneItems();

	virtual void	ListenTo_Message(long msg, void* param);

			void	DoneInitMailAccounts();
			void	DoneInitAdbkAccounts();
			void	DoneInitCalendarAccounts();

			void	GetOpenItems(cdstrvect& items) const;
			void	CloseOpenItems();
			void	SetOpenItems(const cdstrvect& items);

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual bool	TestCloseAll();
	virtual void	DoCloseAll();

	virtual bool	TestCloseOne(unsigned long index);
	virtual void	DoCloseOne(unsigned long index);

	virtual bool	TestCloseOthers(unsigned long index);
	virtual void	DoCloseOthers(unsigned long index);

			void	SetUseSubstitute(bool subs);
			bool	IsSubstituteLocked(unsigned long index);
			bool	IsSubstituteDynamic(unsigned long index);

	virtual bool	IsSpecified() const;

	virtual void	ResetState();						// Reset state from prefs
	virtual void	SaveDefaultState();					// Save state in prefs

	virtual void	SetViewType(N3Pane::EViewType view);

			void	SetTitle(const cdstring& title);
			void	SetIcon(UINT icon);

			void	UsePreview(bool preview);

			void	ShowTabs(bool show);
			void	UpdateTabs();

	CMailboxInfoView*	GetMailboxView() const;
	CAddressBookView*	GetAddressBookView() const;
	CCalendarView*		GetCalendarView() const;

protected:
	C3PaneItemsTabs		mTabs;
	bool				mTabDummy;
	CGrayBackground		mHeader;
	CIconWnd			mIcon;
	CStatic				mDescriptor;
	CToolbarButton		mZoom;
	CToolbarButton		mCloseOne;
	bool				mShowTabs;

	C3PaneMailbox*		mMailbox;
	C3PaneAddressBook*	mAddressBook;
	C3PaneCalendar*		mCalendar;

private:
			void	MakeMailbox();
			void	MakeAddressBook();
			void	MakeCalendar();

			void	OnTabsMove(pair<unsigned long, unsigned long> indices);
			void	OnCloseOne(unsigned long index);
			void	OnCloseAll();
			void	OnCloseOthers(unsigned long index);
			void	OnTabsRename(unsigned long index);
			void	OnTabsLock(unsigned long index);
			void	OnTabsLockAll();
			void	OnTabsDynamic(unsigned long index);
			void	OnTabsSave();
			void	OnTabsRestore();

			void	AddSubstitute(const char* name, bool icon);
			void	SelectSubstitute(unsigned long index);
			void	ChangeSubstitute(unsigned long index, const char* name, bool icon);
			void	ChangeIconSubstitute(unsigned long index, bool icon);
			void	MoveSubstitute(unsigned long oldindex, unsigned long newindex);
			void	RemoveSubstitute(unsigned long index);

protected:
	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShiftClickTab(void);
	afx_msg void OnTabContext(UINT item);
	afx_msg void OnZoomPane(void);
	afx_msg void OnCloseTab(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
