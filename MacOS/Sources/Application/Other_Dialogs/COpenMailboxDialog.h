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


// Header for COpenMailboxDialog class

#ifndef __COPENMAILBOXDIALOG__MULBERRY__
#define __COPENMAILBOXDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_OpenMailboxDialog = 7000;
const	PaneIDT		paneid_OpenMailboxOKbtn = 'OKBT';
const	PaneIDT		paneid_OpenMailboxAccountPopup = 'ACCT';
const	PaneIDT		paneid_OpenMailboxName = 'NAME';

// Messages
const	MessageT	msg_OpenMailboxBrowse = 'BROW';
const	MessageT	msg_OpenMailbox = 'NAME';

// Resources
const	ResIDT		RidL_COpenMailboxDialogBtns = 7000;

// Classes
class CMbox;
class CTextFieldX;
class LPopupButton;
class LPushButton;

class	COpenMailboxDialog : public LDialogBox
{
private:
	LPushButton*	mOKButton;					// OK button
	LPopupButton*	mAccountPopup;
	CTextFieldX*	mMailboxName;
	CMbox*			mBrowsed;

public:
	enum { class_ID = 'OpMa' };

					COpenMailboxDialog();
					COpenMailboxDialog(LStream *inStream);
	virtual 		~COpenMailboxDialog();

	static bool PoseDialog(CMbox*& mbox);

	CMbox*	GetSelectedMbox(void);					// Set the dialogs return info
	bool	IsDone()
		{ return mBrowsed; }

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	void InitAccountMenu(void);
};

#endif
