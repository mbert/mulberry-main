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

#ifndef H_CNewEventDialog
#define H_CNewEventDialog
#pragma once

#include "CModelessDialog.h"
#include "CListener.h"

#include "CICalendarVEvent.h"

#include "CCalendarPopup.h"
#include "CCmdEdit.h"
#include "CPopupButton.h"
#include "CTabController.h"

class CNewComponentPanel;
typedef std::vector<CNewComponentPanel*> CNewComponentPanelList;

// ===========================================================================
//	CNewEventDialog

class CNewEventDialog : public CModelessDialog, public CListener
{
public:
	enum EModelessAction
	{
		eNew,
		eEdit,
		eDuplicate
	};

	static void StartNew(const iCal::CICalendarDateTime& dtstart, const iCal::CICalendar* calin = NULL);
	static void StartEdit(const iCal::CICalendarVEvent& vevent);
	static void StartDuplicate(const iCal::CICalendarVEvent& vevent);

						CNewEventDialog();
	virtual				~CNewEventDialog();

	virtual void		ListenTo_Message(long msg, void* param);

			cdstring	GetCurrentSummary() const;

protected:

// Dialog Data
	//{{AFX_DATA(CNewEventDialog)
	enum { IDD = IDD_CALENDAR_NEWEVENT };
	CCmdEdit				mSummary;
	CCalendarPopup			mCalendar;
	CPopupButton			mStatus;

	CTabController			mTabs;
	CButton					mOrganiserEdit;
	//}}AFX_DATA

	EModelessAction			mAction;
	CNewComponentPanelList	mPanels;
	iCal::CICalendarVEvent*	mVEvent;
	bool					mReadOnly;

	static void StartModeless(iCal::CICalendarVEvent& vevent, EModelessAction action);

			bool		ContainsEvent(const iCal::CICalendarVEvent& vevent) const;

			void		InitPanels();
	
			void	SetAction(EModelessAction action)
			{
				mAction = action;
			}
			void	SetEvent(iCal::CICalendarVEvent& vevent);
			void	GetEvent(iCal::CICalendarVEvent& vevent);
			void	ChangedSummary();
			void	ChangedCalendar();

			void	SetReadOnly(bool read_only);

	virtual uint32_t&	TitleCounter()
	{
		return sTitleCounter;
	}

			bool	DoNewOK();
			bool	DoEditOK();
			void	DoCancel();
			
			bool	GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal);

	// Generated message map functions
	//{{AFX_MSG(CNewEventDialog)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnChangeSummary();
	afx_msg void	OnChangeCalendar();
	afx_msg void	OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnOrganiserEdit();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static uint32_t					sTitleCounter;
	static set<CNewEventDialog*>	sDialogs;
};

#endif
