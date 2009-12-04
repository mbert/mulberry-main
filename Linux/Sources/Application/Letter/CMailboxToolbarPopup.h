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
public:
	CMailboxToolbarPopup(bool copy_to, const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual 		~CMailboxToolbarPopup();

	static	void 	ResetMenuList();								// Synch. menu to mailbox list
	static	void 	ChangeMenuList();								// Mailbox list changed

	virtual void	SetDefault();												// Set to default state
	virtual void	SetSelectedMbox(const cdstring& mbox_name, bool choose);		// Set selected mailbox
	virtual bool	GetSelectedMbox(CMbox*& found, bool do_choice = true);			// Get selected mailbox
	virtual bool	GetSelectedMboxName(cdstring& found, bool do_choice = true);	// Get selected mailbox name

	virtual void	UpdateMenu();

	virtual void	SetValue(JIndex value);

	virtual bool	IsMissingMailbox()
						{ return mMissingMailbox; }
	virtual const char*	GetMissingName()
						{ return mMissingName.c_str(); }

private:
	typedef std::vector<CMailboxToolbarPopup*> CMailboxToolbarPopupList;
	static CMailboxToolbarPopupList	sMailboxToolbarPopupMenus;

	bool			mCopyTo;											// Copy to or append to?
	cdstring		mChosenName;										// Name of currently selected mailbox
	bool			mMissingMailbox;									// Indicates that initial mailbox is missing
	cdstring		mMissingName;										// Name of missing mailbox
	CMbox*			mCachedMbox;
	bool			mNeedsSync;

	void	SyncMenu(void);										// Sync control to menu
	void	SetNeedsSync()
		{ mNeedsSync = true; }
	bool	NeedsSync() const
		{ return mNeedsSync; }

};

#endif
