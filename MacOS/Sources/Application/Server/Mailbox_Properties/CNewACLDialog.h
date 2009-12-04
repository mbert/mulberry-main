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


// Header for CNewACLDialog class

#ifndef __CNEWACLDIALOG__MULBERRY__
#define __CNEWACLDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CAdbkACL.h"
#include "CCalendarACL.h"
#include "CMboxACL.h"

// Constants

// Panes
const	PaneIDT		paneid_NewACLDialog = 4100;
const	PaneIDT		paneid_NewACLUIDs = 'UIDS';
const	PaneIDT		paneid_NewACLLookupBtn = 'BTN1';
const	PaneIDT		paneid_NewACLReadBtn = 'BTN2';
const	PaneIDT		paneid_NewACLSeenBtn = 'BTN3';
const	PaneIDT		paneid_NewACLWriteBtn = 'BTN4';
const	PaneIDT		paneid_NewACLInsertBtn = 'BTN5';
const	PaneIDT		paneid_NewACLPostBtn = 'BTN6';
const	PaneIDT		paneid_NewACLCreateBtn = 'BTN7';
const	PaneIDT		paneid_NewACLDeleteBtn = 'BTN8';
const	PaneIDT		paneid_NewACLAdminBtn = 'BTN9';
const	PaneIDT		paneid_NewACLScheduleBtn = 'BTNA';
const	PaneIDT		paneid_NewACLStylePopup = 'STYL';

// Resources
const	ResIDT		RidL_CNewACLDialogBtns = 4100;

// Classes
class CACLStylePopup;
class CTextDisplay;
class LBevelButton;

class	CNewACLDialog : public LDialogBox
{
public:
	enum { class_ID = 'AcNW' };

					CNewACLDialog();
					CNewACLDialog(LStream *inStream);
	virtual 		~CNewACLDialog();

	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void		SetDetails(bool mbox, bool adbk, bool cal);						// Specify mbox or adbk or cal version
	virtual CMboxACLList*		GetDetailsMbox(void);			// Get details from dialog
	virtual CAdbkACLList*		GetDetailsAdbk(void);			// Get details from dialog
	virtual CCalendarACLList*	GetDetailsCal(void);			// Get details from dialog

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	LBevelButton*	mLookupBtn;
	LBevelButton*	mReadBtn;
	LBevelButton*	mSeenBtn;
	LBevelButton*	mWriteBtn;
	LBevelButton*	mInsertBtn;
	LBevelButton*	mScheduleBtn;
	LBevelButton*	mPostBtn;
	LBevelButton*	mCreateBtn;
	LBevelButton*	mDeleteBtn;
	LBevelButton*	mAdminBtn;
	CTextDisplay*	mUIDList;
	CACLStylePopup*	mStylePopup;
	bool			mMbox;
	bool			mAdbk;
	bool			mCalendar;

	virtual void		SetACL(SACLRight rights);				// Set buttons from rights
	virtual SACLRight	GetRights(void);						// Get rights from buttons

	virtual void	DoStylePopup(long index);				// Handle popup command
};

#endif
