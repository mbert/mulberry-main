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


// Header for CMailboxPopup class

#ifndef __CMAILBOXPOPUP__MULBERRY__
#define __CMAILBOXPOPUP__MULBERRY__

#include <LPopupButton.h>

#include "CCopyToMenu.h"

#include "cdstring.h"

const ResIDT MENU_CopyMailboxPopupID = 2000;
const ResIDT MENU_AppendMailboxPopupID = 3000;

// Classes

class CMbox;

class CMailboxPopup : public LPopupButton {

private:
	static LArray		sMailboxPopupMenus;

public:
	enum { class_ID = 'MbPU' };

					CMailboxPopup(LStream *inStream);
	virtual 		~CMailboxPopup();

	virtual void		SetCopyTo(bool copy_to);
	virtual void		SetPickPopup(bool picker)
		{ mPickPopup = picker; }

	static	void 	ResetMenuList(void);								// Synch. menu to mailbox list
	static	void 	ChangeMenuList(void);								// Mailbox list changed

	virtual void	SetDefault(void);														// Set to default state
	virtual void	SetSelectedMbox(const cdstring& mbox_name, bool none, bool choose);		// Set selected mailbox
	virtual bool	GetSelectedMbox(CMbox*& found, bool do_choice = true);					// Get selected mailbox
	virtual bool	GetSelectedMboxSend(CMbox*& found, bool& set_as_default);				// Get selected mailbox when sending
	virtual bool	GetSelectedMboxName(cdstring& found, bool do_choice = true);			// Get selected mailbox name

	virtual	void	SetValue(SInt32 inValue);

	virtual bool	IsMissingMailbox(void)
						{ return mMissingMailbox; }
	virtual const char*	GetMissingName(void)
						{ return mMissingName.c_str(); }

protected:
	virtual void	FinishCreateSelf(void);

	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);

private:
	virtual void	SyncMenu(void);										// Sync control to menu

	bool			mCopyTo;											// Copy to or append to?
	bool			mPickPopup;											// Picker does not remember value
	cdstring		mChosenName;										// Name of currently selected mailbox
	bool			mMissingMailbox;									// Indicates that initial mailbox is missing
	cdstring		mMissingName;										// Name of missing mailbox
	CMbox*			mCachedMbox;
};

#endif
