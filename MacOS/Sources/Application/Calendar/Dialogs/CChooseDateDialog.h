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

#ifndef H_CChooseDateDialog
#define H_CChooseDateDialog
#pragma once

#include <LDialogBox.h>
#include "CListener.h"

#include "CICalendarDateTime.h"

class CMonthIndicator;
class CMonthPopup;
class CYearPopup;

class LBevelButton;

// ===========================================================================
//	CChooseDateDialog

class	CChooseDateDialog : public LDialogBox,
							public CListener
{
public:
	enum { class_ID = 'Cdat', pane_ID = 1850 };

	static bool PoseDialog(iCal::CICalendarDateTime& dt);

						CChooseDateDialog(LStream *inStream);
	virtual				~CChooseDateDialog();

	virtual void		ListenTo_Message(long msg, void* param);

protected:
	enum
	{
		ePrevYear_ID = 'LYEA',
		eYearPopup_ID = 'YPOP',
		eNextYear_ID = 'PYEA',
		ePrevMonth_ID = 'LMNT',
		eMonthPopup_ID = 'MPOP',
		eNextMonth_ID = 'PMNT',
		eTable_ID = 'TABL'
	};

	// UI Objects
	LBevelButton*		mYearMinus;
	CYearPopup*			mYearPopup;
	LBevelButton*		mYearPlus;
	LBevelButton*		mMonthMinus;
	CMonthPopup*		mMonthPopup;
	LBevelButton*		mMonthPlus;
	CMonthIndicator*	mTable;

	iCal::CICalendarDateTime	mDateResult;
	bool						mDone;

	virtual void		FinishCreateSelf();
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);

			void	SetDate(const iCal::CICalendarDateTime& dt);
			void	GetDate(iCal::CICalendarDateTime& dt);
			
			void	OnPrevYear();
			void	OnYearPopup();
			void	OnNextYear();
			void	OnPrevMonth();
			void	OnMonthPopup();
			void	OnNextMonth();
};

#endif
