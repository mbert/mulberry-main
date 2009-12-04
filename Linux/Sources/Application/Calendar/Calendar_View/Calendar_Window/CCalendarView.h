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

#ifndef H_CCalendarView
#define H_CCalendarView

#include "CBaseView.h"

#include "CWindowStatesFwd.h"

#include "CDayWeekViewTimeRange.h"
#include "CCalendarViewTypes.h"

#include "CICalendarPeriod.h"

#include "CCalendarStoreFreeBusy.h"

#include "cdmutexprotect.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarProperty.h"

namespace iCal
{
class CICalendar;
class CICalendarDateTime;
class CICalendarVEvent;
};

namespace calstore
{
class CCalendarStoreNode;
};

class CCalendarStoreView;
class CCalendarViewBase;
class CCalendarWindow;
class CEventPreview;
class CToolbar;
class JXWidgetSet;

// ===========================================================================
//	CCalendarView

class CCalendarView : public CBaseView
{
public:
	typedef std::set<CCalendarView*>	CCalendarViewList;
	static cdmutexprotect<CCalendarViewList> sCalendarViews;	// List of windows (protected for multi-thread access)

				CCalendarView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual		~CCalendarView();

	static bool	ViewExists(const CCalendarView* view);				// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	static void ResetAll();
	static void EventsChangedAll();
	static void EventChangedAll(iCal::CICalendarVEvent* vevent);
	static void ToDosChangedAll();

	virtual void OnCreate();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	void	InitView();
	void	FullReset();
	void	EventsChanged();
	void	EventChanged(iCal::CICalendarVEvent* vevent);
	void	ToDosChanged();
	
	cdstring GetTitle() const;

	void SetDisplay(uint32_t index, bool show);

	virtual void	MakeToolbars(CToolbarView* parent);

	CCalendarWindow* GetCalendarWindow() const;

	bool IsSingleCalendar() const
	{
		return mSingleCalendar;
	}
	void SetSingleCalendar()
	{
		mSingleCalendar = true;
	}
	iCal::CICalendar* GetCalendar() const
	{
		return mCalendar;
	}
	void SetCalendar(iCal::CICalendar* calendar);

	void SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);

	CEventPreview*	GetPreview() const
		{ return mPreview; }
	void SetPreview(CEventPreview* preview);

	bool	GetUsePreview() const
		{ return mUsePreview; }
	void	SetUsePreview(bool use_view);

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	ResetFont(const SFontInfo& finfo);

	virtual void	ResetState(bool force = false);
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();

protected:
	NCalendarView::EViewType		mViewType;
	bool							mShowToDo;
	NCalendarView::EYearLayout		mYearLayout;
	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	uint32_t						mDayWeekScale;
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	CDayWeekViewTimeRange::ERanges	mFreeBusyRange;
	uint32_t						mFreeBusyScale;
	bool							mSingleCalendar;
	iCal::CICalendar*				mCalendar;
	
	// UI Objects
// begin JXLayout1

    JXWidgetSet* mViewContainer;
    JXWidgetSet* mToDoViewContainer;

// end JXLayout1

	CCalendarStoreView*	mCalendarsView;
	CCalendarViewBase*	mCurrentView;
	CCalendarViewBase*	mToDoView;
	CToolbar*			mToolbar;
	CEventPreview*		mPreview;
	bool				mUsePreview;
	
	void OnUpdateNewToDo(CCmdUI* pCmdUI);

	void OnUpdateDayBtn(CCmdUI* pCmdUI);
	void OnUpdateWorkWeekBtn(CCmdUI* pCmdUI);
	void OnUpdateWeekBtn(CCmdUI* pCmdUI);
	void OnUpdateMonthBtn(CCmdUI* pCmdUI);
	void OnUpdateSummaryBtn(CCmdUI* pCmdUI);
	void OnUpdateYearBtn(CCmdUI* pCmdUI);
	void OnUpdateToDoBtn(CCmdUI* pCmdUI);
	
			void		ResetView(NCalendarView::EViewType type, iCal::CICalendarDateTime* dt = NULL);
			void		ShowToDo(bool show);

			void		OnDayBtn();
			void		OnWorkWeekBtn();
			void		OnWeekBtn();
			void		OnMonthBtn();
			void		OnSummaryBtn();
			void		OnYearBtn();
			void		OnToDoBtn();
			
			void		OnNewToDoBtn();

			void		OnFileSave();

			void		OnCheckCalendar();

private:
	bool mIgnoreValueFieldChanged;
	
	class StIgnoreValueFieldChanged
	{
	public:
		StIgnoreValueFieldChanged(CCalendarView* view)
		{
			mView = view; mPreviousState = mView->mIgnoreValueFieldChanged; mView->mIgnoreValueFieldChanged = true;
		}
		~StIgnoreValueFieldChanged()
		{
			mView->mIgnoreValueFieldChanged = mPreviousState;
		}
	private:
		CCalendarView*	mView;
		bool			mPreviousState;
	};
	friend class StIgnoreValueFieldChanged;
};

#endif
