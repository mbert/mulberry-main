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


// Header for CMailboxToolbarPopup class

#ifndef __CMAILBOXTOOLBARPOPUP__MULBERRY__
#define __CMAILBOXTOOLBARPOPUP__MULBERRY__

#include "CToolbarPopupButton.h"

#include "CCopyToMenu.h"
#include "cdstring.h"

// Classes

class CMbox;

class CMailboxToolbarPopup : public CToolbarPopupButton
{

	DECLARE_DYNCREATE(CMailboxToolbarPopup)

public:
						CMailboxToolbarPopup();
	virtual 			~CMailboxToolbarPopup();

	virtual void		SetCopyTo(bool copy_to)
		{ mCopyTo = copy_to; }

	virtual	CMenu*		GetPopupMenu()
							{ return CCopyToMenu::GetPopupMenu(mCopyTo)->GetSubMenu(0); }				// Use the common Menu handle

	static	void 	ResetMenuList();								// Synch. menu to mailbox list
	static	void 	ChangeMenuList();								// Mailbox list changed

	virtual void		SetDefault();									// Set to default state
	virtual void		SetSelectedMbox(const cdstring& mbox_name, bool choose);			// Set selected mailbox
	virtual bool		GetSelectedMbox(CMbox*& found, bool do_choice = true);				// Get selected mailbox
	virtual bool		GetSelectedMboxName(cdstring& found, bool do_choice = true);		// Get selected mailbox name

	virtual void	SetValue(UINT value);

	virtual bool	IsMissingMailbox()
						{ return mMissingMailbox; }
	virtual const char*	GetMissingName()
						{ return mMissingName.c_str(); }

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

private:
	typedef vector<CMailboxToolbarPopup*> CMailboxToolbarPopupList;
	static CMailboxToolbarPopupList	sMailboxToolbarPopupMenus;

	bool			mCopyTo;											// Copy to or append to?
	cdstring		mChosenName;										// Name of currently selected mailbox
	bool			mMissingMailbox;						// Indicates that initial mailbox is missing
	cdstring		mMissingName;							// Name of missing mailbox
	CMbox*			mCachedMbox;

	virtual void	SyncMenu();										// Sync control to menu

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
