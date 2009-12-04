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

#include "CDialogDirector.h"

#include "CAdbkACL.h"
#include "CCalendarACL.h"
#include "CMboxACL.h"

// Classes
class CACLStylePopup;
class CTextInputDisplay;
class JXMultiImageCheckbox;
class JXTextButton;

class	CNewACLDialog : public CDialogDirector
{
public:
	CNewACLDialog(JXDirector* supervisor);

	static bool PoseDialog(CMboxACLList*& details);
	static bool PoseDialog(CAdbkACLList*& details);
	static bool PoseDialog(CCalendarACLList*& details);

protected:
// begin JXLayout

    JXTextButton*         mOKBtn;
    JXTextButton*         mCancelBtn;
    CTextInputDisplay*    mUIDList;
    JXMultiImageCheckbox* mLookupBtn;
    JXMultiImageCheckbox* mReadBtn;
    JXMultiImageCheckbox* mSeenBtn;
    JXMultiImageCheckbox* mWriteBtn;
    JXMultiImageCheckbox* mInsertBtn;
    JXMultiImageCheckbox* mScheduleBtn;
    JXMultiImageCheckbox* mPostBtn;
    JXMultiImageCheckbox* mCreateBtn;
    JXMultiImageCheckbox* mDeleteBtn;
    JXMultiImageCheckbox* mAdminBtn;
    CACLStylePopup*       mStylePopup;

// end JXLayout
	bool			mMbox;
	bool			mAdbk;
	bool			mCalendar;

	virtual void	OnCreate();					// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

private:
			void		SetDetails(bool mbox, bool adbk, bool cal);		// Specify mbox or adbk version
			CMboxACLList*		GetDetailsMbox(void);			// Get details from dialog
			CAdbkACLList*		GetDetailsAdbk(void);			// Get details from dialog
			CCalendarACLList*	GetDetailsCal(void);			// Get details from dialog

			void		SetACL(SACLRight rights);				// Set buttons from rights
			SACLRight	GetRights();							// Get rights from buttons

			void	DoStylePopup(JIndex index);					// Handle popup command
};

#endif
