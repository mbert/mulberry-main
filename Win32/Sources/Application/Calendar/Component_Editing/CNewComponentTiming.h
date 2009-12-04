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

#ifndef H_CNewComponentTiming
#define H_CNewComponentTiming
#pragma once

#include "CNewComponentPanel.h"
#include "CTabPanel.h"

#include "CICalendarRecurrence.h"

#include "CNumberEdit.h"
#include "CPopupButton.h"
#include "CSubPanelController.h"
#include "CTabController.h"

class CDateTimeZoneSelect;
class CNewTimingPanel;
class CNewComponentRepeatSimple;
class CNewComponentRepeatAdvanced;
class CNewComponentRepeatComplex;

// ===========================================================================
//	CNewComponentTiming

class CNewComponentTiming : public CNewComponentPanel
{
	friend class CNewComponentRepeatSimple;
	friend class CNewComponentRepeatAdvanced;

public:
						CNewComponentTiming();
	virtual				~CNewComponentTiming();

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual	void	SetReadOnly(bool read_only);
			
protected:
	enum
	{
		eOccurs_Simple = 0,
		eOccurs_Advanced,
		eOccurs_Complex
	};

	enum
	{
		eOccurs_Yearly = 0,
		eOccurs_Monthly,
		eOccurs_Weekly,
		eOccurs_Daily,
		eOccurs_Hourly,
		eOccurs_Minutely,
		eOccurs_Secondly
	};

	enum
	{
		eOccurs_ForEver = 0,
		eOccurs_Count,
		eOccurs_Until
	};

	// UI Objects
	enum { IDD = IDD_CALENDAR_NEW_TIMING };
	CSubPanelController		mTimingView;
	CNewTimingPanel*		mTimingPanel;

	CButton					mRepeats;
	CTabController			mRepeatsTabs;

	CNewComponentRepeatSimple*		mRepeatSimpleItems;
	CNewComponentRepeatAdvanced*	mRepeatAdavancedItems;
	CNewComponentRepeatComplex*		mRepeatComplexItems;

	iCal::CICalendarRecurrence	mAdvancedRecur;
	cdstring				mComplexDescription;

			void	DoRepeat(bool repeat);
			void	DoRepeatTab(UInt32 value);
			void	DoOccursGroup(UInt32 value);
			void	DoOccursEdit();
			
			void	SetRecurrence(const iCal::CICalendarRecurrenceSet* recurs);
			
			bool	GetRecurrence(iCal::CICalendarRecurrenceSet& recurs);
			
			void	SyncEnd();
			void	SyncDuration();


	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnRepeat();
	afx_msg void	OnSelChangeRepeatTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CNewComponentRepeatSimple : public CTabPanel
{
public:
						CNewComponentRepeatSimple(CNewComponentTiming* timing);
	virtual				~CNewComponentRepeatSimple();

	virtual void	SetContent(void* data) {}			// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

	// UI Objects
	CNumberEdit				mOccursInterval;
	CSpinButtonCtrl			mOccursIntervalSpin;
	CPopupButton			mOccursFreq;

	CButton					mOccursForEver;
	CButton					mOccursCount;
	CButton					mOccursUntil;

	CNumberEdit				mOccursCounter;
	CSpinButtonCtrl			mOccursCounterSpin;

	CSubPanelController		mOccursDateTimeZoneItem;
	CDateTimeZoneSelect*	mOccursDateTimeZone;

protected:
	CNewComponentTiming*	mTimingPanel;

	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	afx_msg	void	OnOccursForEver();
	afx_msg	void	OnOccursCount();
	afx_msg	void	OnOccursUntil();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CNewComponentRepeatAdvanced : public CTabPanel
{
public:
						CNewComponentRepeatAdvanced(CNewComponentTiming* timing);
	virtual				~CNewComponentRepeatAdvanced();

	virtual void	SetContent(void* data) {}			// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

	// UI Objects
	CStatic					mOccursDescription;
	CButton					mOccursEdit;

protected:
	CNewComponentTiming*	mTimingPanel;

	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	afx_msg	void	OnOccursEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CNewComponentRepeatComplex : public CTabPanel
{
public:
						CNewComponentRepeatComplex();
	virtual				~CNewComponentRepeatComplex();

	virtual void	SetContent(void* data) {}			// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

	// UI Objects
	CStatic					mOccursDescription;

protected:
	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
