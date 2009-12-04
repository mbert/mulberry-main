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
#pragma once

#include "CBroadcaster.h"

#include <vector>

#include "CICalendarPeriod.h"
#include "CICalendarVEvent.h"
#include "CITIPProcessor.h"

class CCalendarTableBase;

#include "CICalendarComponentExpanded.h"

// ===========================================================================
//	CCalendarEventBase

class	CCalendarEventBase : public CWnd,
								public CBroadcaster
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

						CCalendarEventBase();
	virtual				~CCalendarEventBase();

	void SetDetails(iCal::CICalendarComponentExpandedShared& event, CCalendarTableBase* table, const char* title, bool all_day, bool start_col, bool end_col, bool horiz);


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

	const iCal::CICalendarComponentExpandedShared& GetVEvent() const
		{ return mVEvent; }

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
	cdstring				mTooltipText;
#ifdef _UNICODE
	cdustring				mTooltipTextUTF16;
#endif

	afx_msg	void		OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void		OnLButtonDblClk(UINT nFlags, CPoint point);			// Double-clicked item
	afx_msg void		OnContextMenu(CWnd*, CPoint point);
	afx_msg void		OnPaint();

			bool		IsNow() const;
	virtual void		SetupTagText();

private:
			void		DrawHorizFrame(CDC* pDC, CRect& rect);
			void		DrawVertFrame(CDC* pDC, CRect& rect);

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

typedef std::vector<CCalendarEventBase*> CCalendarEventBaseList;

#endif
