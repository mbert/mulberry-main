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

#ifndef H_CCalendarEventBase
#define H_CCalendarEventBase

#include <JXWidget.h>
#include "CBroadcaster.h"
#include "CContextMenu.h"
#include "CCommander.h"

#include <vector>

#include "CICalendarPeriod.h"
#include "CICalendarVEvent.h"
#include "CICalendarVFreeBusy.h"
#include "CITIPProcessor.h"

class CCalendarTableBase;

#include "CICalendarComponentExpanded.h"

// ===========================================================================
//	CCalendarEventBase

class	CCalendarEventBase : public CCommander,		// Commander must be first so it gets destroyed last
								public JXWidget,
								public CBroadcaster,
								public CContextMenu
{
public:
	enum
	{
		eBroadcast_EditEvent = 'EdiE',			// param = CCalendarEventBase*
		eBroadcast_ClickEvent = 'SelE'			// param = SClickEvent_Message*
	};

	struct SClickEvent_Message
	{
	public:
		SClickEvent_Message(CCalendarEventBase* item, bool shift) :
			mItem(item), mShift(shift) {}
		
		CCalendarEventBase*	GetItem() const
		{
			return mItem;
		}
		
		bool GetShift() const
		{
			return mShift;
		}

	private:
		CCalendarEventBase*	mItem;
		bool				mShift;
	};

			CCalendarEventBase(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual	~CCalendarEventBase();

	void SetDetails(iCal::CICalendarComponentExpandedShared& event, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz);
	void SetDetails(iCal::CICalendarVFreeBusy* freebusy, const iCal::CICalendarPeriod& period, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz);

	void SetPreviousLink(CCalendarEventBase* prev)
		{ mPreviousLink = prev; }
	CCalendarEventBase* GetPreviousLink() const
	{
		return mPreviousLink;
	}

	void SetNextLink(CCalendarEventBase* next)
		{ mNextLink = next; }
	CCalendarEventBase* GetNextLink() const
	{
		return mNextLink;
	}

	bool IsEvent() const
	{
		return mVEvent.get() != NULL;
	}
	bool IsFreeBusy() const
	{
		return mVFreeBusy != NULL;
	}
	const iCal::CICalendarComponentExpandedShared& GetVEvent() const
		{ return mVEvent; }
	const iCal::CICalendarVFreeBusy* GetVFreeBusy() const
		{ return mVFreeBusy; }

	const iCal::CICalendarPeriod& GetInstancePeriod() const
		{ return mPeriod; }

	uint32_t	GetColumnSpan() const
		{ return mColumnSpan; }
	void	SetColumnSpan(uint32_t span)
		{ mColumnSpan = span; }

	void		Select(bool select);

	bool		IsSelected() const
	{
		return mIsSelected;
	}

	void		CheckNow(iCal::CICalendarDateTime& dt);

protected:
	iCal::CICalendarComponentExpandedShared	mVEvent;
	iCal::CICalendarVFreeBusy*				mVFreeBusy;

	CCalendarTableBase*		mTable;
	cdstring				mTitle;
	bool					mAllDay;
	bool					mStartsInCol;
	bool					mEndsInCol;
	bool					mIsSelected;
	bool					mIsCancelled;
	bool					mIsNow;
	bool					mHasAlarm;
	iCal::CITIPProcessor::EAttendeeState	mAttendeeState;
	bool					mHoriz;
	uint32_t				mColumnSpan;
	CCalendarEventBase*		mPreviousLink;
	CCalendarEventBase*		mNextLink;
	uint32_t				mColour;
	iCal::CICalendarPeriod	mPeriod;
	bool					mIsInbox;

	virtual void		OnCreate();
	
	virtual void		Receive(JBroadcaster* sender, const Message& message);

	virtual void		HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void		Draw(JXWindowPainter& p, const JRect& rect);
	virtual void		DrawBorder(JXWindowPainter& p, const JRect& rect) {}
	virtual void		DrawBackground(JXWindowPainter& p, const JRect& frameG) {}

			bool		IsNow() const;
	virtual void		SetupTagText();
			void		SetupTagTextEvent();
			void		SetupTagTextFreeBusy();

private:
			void		DrawHorizFrame(JXWindowPainter* pDC, JRect& rect);
			void		DrawHorizFrameOutline(JXWindowPainter* pDC, JRect& rect);
			void		DrawHorizFrameFill(JXWindowPainter* pDC, JRect& rect);
			void		DrawVertFrame(JXWindowPainter* pDC, JRect& rect);
			void		DrawVertFrameOutline(JXWindowPainter* pDC, JRect& rect);
			void		DrawVertFrameFill(JXWindowPainter* pDC, JRect& rect);
};

typedef std::vector<CCalendarEventBase*> CCalendarEventBaseList;

#endif
