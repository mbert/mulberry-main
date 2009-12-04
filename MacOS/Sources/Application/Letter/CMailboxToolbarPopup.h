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

#ifndef __CMailboxToolbarPopup__MULBERRY__
#define __CMailboxToolbarPopup__MULBERRY__

#include "CToolbarButton.h"

#include "CCopyToMenu.h"

#include "cdstring.h"

// Classes

class CMbox;

class CMailboxToolbarPopup : public CToolbarButton
{
private:
	static LArray		sMailboxToolbarPopupMenus;

public:
	enum { class_ID = 'MbTP' };

					CMailboxToolbarPopup(LStream *inStream);
					CMailboxToolbarPopup(
							const SPaneInfo	&inPaneInfo,
							MessageT		inValueMessage,
							SInt16			inBevelProc,
							ResIDT			inMenuID,
							SInt16			inMenuPlacement,
							SInt16			inContentType,
							SInt16			inContentResID,
							ResIDT			inTextTraits,
							ConstStringPtr	inTitle,
							SInt16			inInitialValue,
							SInt16			inTitlePlacement,
							SInt16			inTitleAlignment,
							SInt16			inTitleOffset,
							SInt16			inGraphicAlignment,
							Point			inGraphicOffset,
							Boolean			inCenterPopupGlyph);
	virtual 		~CMailboxToolbarPopup();

	virtual void		SetCopyTo(bool copy_to);

	static	void 	ResetMenuList(void);								// Synch. menu to mailbox list
	static	void 	ChangeMenuList(void);								// Mailbox list changed

	virtual void	SetDefault(void);												// Set to default state
	virtual void	SetSelectedMbox(const cdstring& mbox_name, bool choose);		// Set selected mailbox
	virtual bool	GetSelectedMbox(CMbox*& found, bool do_choice = true);			// Get selected mailbox
	virtual bool	GetSelectedMboxName(cdstring& found, bool do_choice = true);	// Get selected mailbox name

	virtual	void	SetMenuValue(SInt32 inValue, bool silent = false);

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
	cdstring		mChosenName;										// Name of currently selected mailbox
	bool			mMissingMailbox;									// Indicates that initial mailbox is missing
	cdstring		mMissingName;										// Name of missing mailbox
	CMbox*			mCachedMbox;
};

#endif
