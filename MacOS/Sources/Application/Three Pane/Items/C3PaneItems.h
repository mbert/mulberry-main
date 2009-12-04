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

// Constants
const	PaneIDT		paneid_3PaneItems = 1720;
const	PaneIDT		paneid_3PaneItemsTabs = 'TABS';
const	PaneIDT		paneid_3PaneItemsHeader = 'MOVE';
const	PaneIDT		paneid_3PaneItemsIcon = 'ICON';
const	PaneIDT		paneid_3PaneItemsDesc = 'DESC';
const	PaneIDT		paneid_3PaneItemsZoom1 = 'ZOM1';
const	PaneIDT		paneid_3PaneItemsZoom2 = 'ZOM2';
const	PaneIDT		paneid_3PaneItemsCloseTab = 'CMBX';

// Messages
const	MessageT	msg_3PaneItemsTabs = 'TABS';
const	MessageT	msg_3PaneItemsTabsMove = 'TABM';
const	MessageT	msg_3PaneItemsTabsClose = 'TABC';
const	MessageT	msg_3PaneItemsTabsCloseAll = 'TABA';
const	MessageT	msg_3PaneItemsTabsCloseOthers = 'TABO';
const	MessageT	msg_3PaneItemsTabsRename = 'TABR';
const	MessageT	msg_3PaneItemsTabsLock = 'TABL';
const	MessageT	msg_3PaneItemsTabsLockAll = 'TABY';
const	MessageT	msg_3PaneItemsTabsDynamic = 'TABD';
const	MessageT	msg_3PaneItemsTabsSave = 'TABW';
const	MessageT	msg_3PaneItemsTabsRestore = 'TABX';
const	MessageT	msg_3PaneItemsZoom = 'ZOOM';
const	MessageT	msg_3PaneItemsCloseTab = 'CMBX';

// Resources

// Classes
class CAddressBookView;
class CCalendarView;
class CMailboxInfoView;
class CToolbarButton;
class C3PaneItemsTabs;
class C3PaneMailbox;
class C3PaneAddressBook;
class C3PaneCalendar;
class LIconControl;
class CStaticText;

class C3PaneItems : public C3PaneParentPanel, public LListener
{
public:
		
	enum { class_ID = '3PNI' };

					C3PaneItems();
					C3PaneItems(LStream *inStream);
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
			void	SetIcon(ResIDT icon);

			void	UsePreview(bool preview);

			void	ShowTabs(bool show);
			void	UpdateTabs();

	CMailboxInfoView*	GetMailboxView() const;
	CAddressBookView*	GetAddressBookView() const;
	CCalendarView*		GetCalendarView() const;

protected:
	C3PaneItemsTabs*	mTabs;
	bool				mTabDummy;
	CToolbarButton*		mCloseOne;
	CToolbarButton*		mZoom1;
	CToolbarButton*		mZoom2;
	LView*				mHeader;
	LIconControl*		mIcon;
	CStaticText*		mDescriptor;
	bool				mShowTabs;

	C3PaneMailbox*		mMailbox;
	C3PaneAddressBook*	mAddressBook;
	C3PaneCalendar*		mCalendar;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

			void	OnTabs(unsigned long index);
			void	OnTabsMove(std::pair<unsigned long, unsigned long> indices);
			void	OnTabsClose(unsigned long index);
			void	OnTabsCloseAll();
			void	OnTabsCloseOthers(unsigned long index);
			void	OnTabsRename(unsigned long index);
			void	OnTabsLock(unsigned long index);
			void	OnTabsLockAll();
			void	OnTabsDynamic(unsigned long index);
			void	OnTabsSave();
			void	OnTabsRestore();
			void	OnZoom();
			void	OnCloseTab();

private:
			void	MakeMailbox();
			void	MakeAddressBook();
			void	MakeCalendar();

			void	OnCloseOne(unsigned long index);
			void	OnCloseAll();
			void	OnCloseOthers(unsigned long index);

			void	AddSubstitute(const char* name, bool icon);
			void	SelectSubstitute(unsigned long index);
			void	ChangeSubstitute(unsigned long index, const char* name, bool icon);
			void	ChangeIconSubstitute(unsigned long index, bool icon);
			void	MoveSubstitute(unsigned long oldindex, unsigned long newindex);
			void	RemoveSubstitute(unsigned long index);
};

#endif
