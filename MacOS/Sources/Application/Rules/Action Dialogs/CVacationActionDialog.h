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


// Header for CVacationActionDialog class

#ifndef __CVACATIONACTIONDIALOG__MULBERRY__
#define __CVACATIONACTIONDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CActionItem.h"

// Constants

// Panes
const	PaneIDT		paneid_VacationActionDialog = 1224;
const	PaneIDT		paneid_VacationActionDays = 'DAYS';
const	PaneIDT		paneid_VacationActionSubject = 'SUBJ';
const	PaneIDT		paneid_VacationActionText = 'TEXT';
const	PaneIDT		paneid_VacationActionAddresses = 'STXT';

// Mesages
const	MessageT	msg_VacationActionText = 'TEXT';

// Resources
const	ResIDT		RidL_CVacationActionDialogBtns = 1224;

// Type
class CTextFieldX;
class CTextDisplay;

class CVacationActionDialog : public LDialogBox
{
private:
	CTextFieldX*		mDays;
	CTextFieldX*		mSubject;
	CTextDisplay*		mAddresses;

	cdstring			mText;

public:
	enum { class_ID = 'Vact' };

					CVacationActionDialog();
					CVacationActionDialog(LStream *inStream);
	virtual 		~CVacationActionDialog();

	static bool PoseDialog(CActionItem::CActionVacation& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
			void	SetDetails(CActionItem::CActionVacation& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionVacation& details);		// Get the dialogs return info

			void	SetText();
};

#endif
