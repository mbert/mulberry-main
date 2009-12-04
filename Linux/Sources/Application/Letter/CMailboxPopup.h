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

#include "TPopupMenu.h"
#include "HPopupMenu.h"

#include "CCopyToMenu.h"
#include "cdstring.h"

// Classes

class CMbox;

class CMailboxPopup : public HPopupMenu
{

public:
	CMailboxPopup(JXContainer* enclosure,
								const HSizingOption hSizing,
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h,
								bool copy_to = true);
	//This version is really just for use with jxlayout which ones
	//extra arguements at the front
	CMailboxPopup(bool copy_to,
								JXContainer* enclosure,
								const HSizingOption hSizing,
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
		

	virtual 			~CMailboxPopup();

	static	void 	ResetMenuList(void);								// Synch. menu to mailbox list
	static	void 	ChangeMenuList(void);								// Mailbox list changed

	virtual void	SetDefault();																// Set to default state
	virtual void	SetSelectedMbox(const cdstring& mbox_name, bool none, bool choose);			// Set selected mailbox
	virtual bool	GetSelectedMbox(CMbox*& found, bool do_choice = true);						// Get selected mailbox
	virtual bool	GetSelectedMboxSend(CMbox*& found, bool& set_as_default);					// Get selected mailbox when sending
	virtual bool	GetSelectedMboxName(cdstring& found, bool do_choice = true);				// Get selected mailbox name

	virtual void	SetValue(JIndex value);

	virtual bool	IsMissingMailbox(void)
						{ return mMissingMailbox; }
	virtual const char*	GetMissingName(void)
						{ return mMissingName.c_str(); }

 private:
	typedef std::vector<CMailboxPopup*> CMailboxPopupList;
	static CMailboxPopupList	sMailboxPopupMenus;

	bool			mCopyTo;											// Copy to or append to?
	cdstring		mChosenName;										// Name of currently selected mailbox
	bool			mMissingMailbox;						// Indicates that initial mailbox is missing
	cdstring		mMissingName;							// Name of missing mailbox
	CMbox*			mCachedMbox;
	bool			mNeedsSync;

	void	SyncMenu(void);										// Sync control to menu
	void	SetNeedsSync()
		{ mNeedsSync = true; }
	bool	NeedsSync() const
		{ return mNeedsSync; }

protected:
	void Receive(JBroadcaster* sender, const Message& message);
	void CommonConstruct(JXContainer* enclosure,
											 const HSizingOption hSizing,
											 const VSizingOption vSizing,
											 const JCoordinate x, const JCoordinate y,
											 const JCoordinate w, const JCoordinate h);
};

class CMailboxPopupButton : public CMailboxPopup
{
public:
	CMailboxPopupButton(bool copy_to,
								 JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
		: CMailboxPopup(copy_to, enclosure, hSizing, vSizing, x, y, w, h)
		{}

protected:
	void AdjustPopupChoiceTitle(const JIndex index) {
		//set the value but don't actually change the title
		SetValue(index);
	};
};

#endif
