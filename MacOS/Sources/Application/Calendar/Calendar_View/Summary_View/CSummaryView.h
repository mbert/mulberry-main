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

#ifndef H_CSummaryView
#define H_CSummaryView
#pragma once

#include "CCalendarViewBase.h"
#include <LListener.h>

#include "CCalendarViewTypes.h"
#include "CSummaryTable.h"

class CSummaryTitleTable;
class CTimezonePopup;

class LBevelButton;
class LPopupButton;
class LScrollerView;

// ===========================================================================
//	CSummaryView

class CSummaryView : public CCalendarViewBase,
					public LListener
{
public:
	enum { class_ID = 'CalS', pane_ID = 1804 };

						CSummaryView(LStream *inStream);
	virtual				~CSummaryView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	NCalendarView::ESummaryType GetType() const
	{
		return mSummaryType;
	}
	void SetType(NCalendarView::ESummaryType type);

	NCalendarView::ESummaryRanges GetRange() const
	{
		return mSummaryRange;
	}
	void SetRange(NCalendarView::ESummaryRanges range);

	virtual CCalendarTableBase* GetTable() const;

	virtual void ResetTextTraits(const TextTraitsRecord& list_traits);	// Reset text traits

protected:
	enum
	{
		eTimezone_ID = 'TZID',
		eRange_ID = 'SUMM',
		eTitleTable_ID = 'TITL',
		eScroller_ID = 'SCRL',
		eTable_ID = 'TABL'
	};

	// UI Objects
	CTimezonePopup*	mTimezonePopup;
	LPopupButton*	mRangePopup;

	CSummaryTitleTable*	mTitles;
	LScrollerView*		mScroller;
	CSummaryTable*		mTable;
	
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	cdstring						mRangeText;
	
	virtual void		FinishCreateSelf();
	
	virtual	void		ResetDate();

			void		OnToday();
			void		OnNewEvent();
			void		OnTimezone();
			void		OnRange();
};

#endif
