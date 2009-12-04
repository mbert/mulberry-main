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

#ifndef H_CDayWeekView
#define H_CDayWeekView

#include "CCalendarViewBase.h"

#include "CDayWeekViewTimeRange.h"

class CCalendarEventBase;
class CDayWeekTable;
class CDayWeekTitleTable;
class CTimezonePopup;

#include "HPopupMenu.h"

class JXMultiImageButton;

// ===========================================================================
//	CDayWeekView

class CDayWeekView : public CCalendarViewBase
{
public:
	enum EDayWeekType
	{
		eDay,
		eWorkWeek,
		eWeek
	};

			CDayWeekView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CDayWeekView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

			void SetType(EDayWeekType type);

	virtual void ResetFont(const SFontInfo& finfo);			// Reset text traits

	virtual void DoPrint();

	CDayWeekViewTimeRange::ERanges GetRange() const
	{
		return mDayWeekRange;
	}
	void SetRange(CDayWeekViewTimeRange::ERanges range);

	uint32_t GetScale() const
	{
		return mDayWeekScale;
	}
	void SetScale(uint32_t scale);

	virtual CCalendarTableBase* GetTable() const;

protected:

	// UI Objects
// begin JXLayout1

    JXMultiImageButton* mPrevWeek;
    JXMultiImageButton* mNextWeek;
    JXMultiImageButton* mPrevDay;
    JXMultiImageButton* mNextDay;
    CTimezonePopup*     mTimezonePopup;
    HPopupMenu*         mScalePopup;
    HPopupMenu*         mRangePopup;

// end JXLayout1

	CDayWeekTitleTable*	mTitles;
	CDayWeekTable*		mTable;
	CCalendarEventBase*	mSelectedEvent;
	
	EDayWeekType		mType;

	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	uint32_t			mDayWeekScale;	

	virtual void		OnCreate();
	
	virtual	void		ResetDate();

			void		OnUpdateNewEvent(CCmdUI* pCmdUI);

			void		OnPrevWeek();
			void		OnNextWeek();
			void		OnPrevDay();
			void		OnNextDay();
			void		OnToday();
			void		OnNewEvent();
			void		OnNewEvent(const iCal::CICalendarDateTime& dt);
			void		OnTimezone();
			void		OnScale();
			void		OnRange();
};

#endif
