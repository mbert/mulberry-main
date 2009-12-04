/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#ifndef H_CFreeBusyView
#define H_CFreeBusyView

#include "CCalendarViewBase.h"

#include "CDayWeekViewTimeRange.h"

#include "CCalendarStoreFreeBusy.h"

class CCalendarEventBase;
class CFreeBusyTable;
class CFreeBusyTitleTable;
class CTimezonePopup;

#include "HPopupMenu.h"

class JXMultiImageButton;

// ===========================================================================
//	CFreeBusyView

class CFreeBusyView : public CCalendarViewBase
{
public:
			CFreeBusyView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CFreeBusyView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void ResetFont(const SFontInfo& finfo);			// Reset text traits

	virtual void DoPrint();

	CDayWeekViewTimeRange::ERanges GetRange() const
	{
		return mFreeBusyRange;
	}
	void SetRange(CDayWeekViewTimeRange::ERanges range);

	uint32_t GetScale() const
	{
		return mFreeBusyScale;
	}
	void SetScale(uint32_t scale);

	virtual CCalendarTableBase* GetTable() const;

	void SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);

protected:

	// UI Objects
// begin JXLayout1

    JXMultiImageButton* mPrevDay;
    JXMultiImageButton* mNextDay;
    CTimezonePopup*     mTimezonePopup;
    HPopupMenu*         mScalePopup;
    HPopupMenu*         mRangePopup;

// end JXLayout1

    CFreeBusyTitleTable*	mTitles;
    CFreeBusyTable*			mTable;
	CCalendarEventBase*		mSelectedEvent;
	
	CDayWeekViewTimeRange::ERanges	mFreeBusyRange;
	uint32_t						mFreeBusyScale;

	iCal::CICalendarRef						mCalendarRef;
	cdstring								mIdentityRef;
	iCal::CICalendarProperty				mOrganizer;
	iCal::CICalendarPropertyList			mAttendees;
	calstore::CCalendarStoreFreeBusyList	mDetails;

	virtual void		OnCreate();
	
	virtual	void		ResetDate();

			void		OnUpdateNewEvent(CCmdUI* pCmdUI);

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
