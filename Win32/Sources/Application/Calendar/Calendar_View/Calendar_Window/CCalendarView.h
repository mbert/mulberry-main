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
#pragma once

#include "CBaseView.h"

#include "CGrayBackground.h"
#include "CDayWeekViewTimeRange.h"
#include "CCalendarViewTypes.h"

#include "cdmutexprotect.h"

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

// ===========================================================================
//	CCalendarView

class CCalendarView : public CBaseView
{
public:
	typedef set<CCalendarView*>	CCalendarViewList;
	static cdmutexprotect<CCalendarViewList> sCalendarViews;	// List of windows (protected for multi-thread access)

						CCalendarView();
	virtual				~CCalendarView();

	static bool	ViewExists(const CCalendarView* view);				// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	static void ResetAll();
	static void EventsChangedAll();
	static void EventChangedAll(iCal::CICalendarVEvent* vevent);
	static void ToDosChangedAll();

	void	InitView();
	void	FullReset();
	void	EventsChanged();
	void	EventChanged(iCal::CICalendarVEvent* vevent);
	void	ToDosChanged();

	cdstring GetTitle() const;

	void SetDisplay(uint32_t index, bool show);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

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

	CEventPreview*	GetPreview() const
		{ return mPreview; }
	void SetPreview(CEventPreview* preview);

	bool	GetUsePreview() const
		{ return mUsePreview; }
	void	SetUsePreview(bool use_view);

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	ResetState(bool force = false);
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();

protected:
	enum
	{
		eCalendars_ID = 'CALS',
		eView_ID = 'VIEW',
		eToDo_ID = 'TODO'
	};

	NCalendarView::EViewType		mViewType;
	bool							mShowToDo;
	NCalendarView::EYearLayout		mYearLayout;
	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	uint32_t						mDayWeekScale;
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	bool							mSingleCalendar;
	iCal::CICalendar*				mCalendar;
	
	// UI Objects
	CGrayBackground			mFocusRing;
	CGrayBackground			mViewContainer;
	CGrayBackground			mToDoViewContainer;

	CCalendarStoreView*		mCalendarsView;
	CCalendarViewBase*		mCurrentView;
	CCalendarViewBase*		mToDoView;
	CToolbar*				mToolbar;
	CEventPreview*			mPreview;
	bool					mUsePreview;
	
			void		ResetView(NCalendarView::EViewType type, iCal::CICalendarDateTime* dt = NULL);
			void		ShowToDo(bool show);


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

protected:

	afx_msg void OnUpdateNewToDo(CCmdUI* pCmdUI);

	afx_msg void OnUpdateDayBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWorkWeekBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWeekBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMonthBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSummaryBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateYearBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToDoBtn(CCmdUI* pCmdUI);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDayBtn();
	afx_msg void OnWorkWeekBtn();
	afx_msg void OnWeekBtn();
	afx_msg void OnMonthBtn();
	afx_msg void OnSummaryBtn();
	afx_msg void OnYearBtn();
	afx_msg void OnToDoBtn();
			
	afx_msg void OnNewToDoBtn();

	afx_msg void OnFileSave();

	DECLARE_MESSAGE_MAP()
};

#endif
