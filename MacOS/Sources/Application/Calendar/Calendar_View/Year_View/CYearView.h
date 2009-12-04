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

#ifndef H_CYearView
#define H_CYearView
#pragma once

#include "CCalendarViewBase.h"
#include <LListener.h>

#include "CYearTable.h"

class LBevelButton;
class LPopupButton;
class CStaticText;

// ===========================================================================
//	CYearView

class	CYearView : public CCalendarViewBase,
					public LListener
{
public:
	enum { class_ID = 'CalY', pane_ID = 1801 };

						CYearView(LStream *inStream);
	virtual				~CYearView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	NCalendarView::EYearLayout GetLayout() const
	{
		return mLayoutValue;
	}
	void SetLayout(NCalendarView::EYearLayout layout);

	virtual CCalendarTableBase* GetTable() const;

protected:
	enum
	{
		eYearMinus_ID = 'LYEA',
		eYear_ID = 'YEAR',
		eYearPlus_ID = 'PYEA',
		eLayout_ID = 'LAYO',
		eTable_ID = 'TABL'
	};

	NCalendarView::EYearLayout	mLayoutValue;

	// UI Objects
	LBevelButton*	mYearMinus;
	LBevelButton*	mYearPlus;

	CStaticText*	mYear;
	
	LPopupButton*	mLayout;
	
	CYearTable*			mTable;	

	virtual void		FinishCreateSelf();
	
	virtual void		ResetDate();
			void		SetCaptions();

			void		OnPreviousYear();
			void		OnNextYear();
			void		OnThisYear();
			void		OnNewEvent();
			void		OnLayout();
};

#endif
