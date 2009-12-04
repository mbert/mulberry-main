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


// Header for C3PaneAccounts class

#ifndef __C3PANEACCOUNTS__MULBERRY__
#define __C3PANEACCOUNTS__MULBERRY__

#include "C3PaneParentPanel.h"

// Constants
const	PaneIDT		paneid_3PaneAccounts = 1710;
const	PaneIDT		paneid_3PaneAccountsTabs = 'TABS';
const	PaneIDT		paneid_3PaneAccountsZoom = 'ZOOM';

// Messages
const	MessageT	msg_3PaneAccountsTabs = 'TABS';
const	MessageT	msg_3PaneAccountsZoom = 'ZOOM';

// Resources

// Classes
class CAdbkManagerView;
class CCalendarStoreView;
class CServerView;
class C3PaneMailAccount;
class C3PaneContactsAccount;
class C3PaneCalendarAccount;
class LBevelButton;
class LTabsControl;

class C3PaneAccounts : public C3PaneParentPanel, public LListener
{
public:
		
	enum { class_ID = '3PNA' };

					C3PaneAccounts();
					C3PaneAccounts(LStream *inStream);
	virtual 		~C3PaneAccounts();

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual void	ResetState();						// Reset state from prefs
	virtual void	SaveDefaultState();					// Save state in prefs
			
	virtual void	SetViewType(N3Pane::EViewType view);

	CServerView*		GetServerView() const;
	CAdbkManagerView*	GetContactsView() const;
	CCalendarStoreView*	GetCalendarStoreView() const;

protected:
	LTabsControl*			mTabs;
	C3PaneMailAccount*		mMailAccounts;
	C3PaneContactsAccount*	mContactsAccounts;
	C3PaneCalendarAccount*	mCalendarAccounts;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

			void	OnTabs(unsigned long index);
			void	OnZoom();

private:
			void	MakeMail();
			void	MakeContacts();
			void	MakeCalendar();
};

#endif
