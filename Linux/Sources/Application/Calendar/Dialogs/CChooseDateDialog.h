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

#include "CDialogDirector.h"
#include "CListener.h"

#include "CICalendarDateTime.h"

class CMonthIndicator;
class CMonthPopup;
class CYearPopup;

class JXMultiImageButton;
class JXTextButton;

// ===========================================================================
//	CChooseDateDialog

class	CChooseDateDialog : public CDialogDirector,
							public CListener
{
public:
	static bool PoseDialog(iCal::CICalendarDateTime& dt);

						CChooseDateDialog(JXDirector* supervisor);
	virtual				~CChooseDateDialog();

	virtual void		ListenTo_Message(long msg, void* param);

protected:
	// UI Objects
// begin JXLayout

    JXMultiImageButton* mYearMinus;
    CYearPopup*         mYearPopup;
    JXMultiImageButton* mYearPlus;
    JXMultiImageButton* mMonthMinus;
    CMonthPopup*        mMonthPopup;
    JXMultiImageButton* mMonthPlus;
    CMonthIndicator*    mTable;
    JXTextButton*       mCancelBtn;

// end JXLayout

	iCal::CICalendarDateTime	mDateResult;
	bool						mDone;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

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
