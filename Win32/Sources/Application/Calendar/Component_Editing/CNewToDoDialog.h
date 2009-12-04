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

#ifndef H_CNewToDoDialog
#define H_CNewToDoDialog
#pragma once

#include "CModelessDialog.h"
#include "CListener.h"

#include "CICalendarVToDo.h"

#include "CCalendarPopup.h"
#include "CCmdEdit.h"
#include "CSubPanelController.h"
#include "CTabController.h"

class CDateTimeZoneSelect;
class CNewComponentPanel;
typedef std::vector<CNewComponentPanel*> CNewComponentPanelList;

// ===========================================================================
//	CNewToDoDialog

class CNewToDoDialog : public CModelessDialog, public CListener
{
public:
	enum EModelessAction
	{
		eNew,
		eEdit,
		eDuplicate
	};

	static void StartNew(const iCal::CICalendar* calin = NULL);
	static void StartEdit(const iCal::CICalendarVToDo& vtodo);
	static void StartDuplicate(const iCal::CICalendarVToDo& vtodo);

						CNewToDoDialog();
	virtual				~CNewToDoDialog();

	virtual void		ListenTo_Message(long msg, void* param);

			cdstring	GetCurrentSummary() const;

protected:
// Dialog Data
	//{{AFX_DATA(CNewEventDialog)
	enum { IDD = IDD_CALENDAR_NEWTODO };
	CCmdEdit				mSummary;
	CCalendarPopup			mCalendar;
	CButton					mCompleted;
	CSubPanelController		mCompletedDateTimeZoneItem;
	CButton					mCompletedNow;

	CTabController			mTabs;
	CButton					mOrganiserEdit;
	//}}AFX_DATA
	CDateTimeZoneSelect*	mCompletedDateTimeZone;

	EModelessAction			mAction;
	CNewComponentPanelList	mPanels;
	iCal::CICalendarVToDo*	mVToDo;
	bool					mReadOnly;
	
	iCal::CICalendarDateTime	mActualCompleted;
	bool						mCompletedExists;

	static void StartModeless(iCal::CICalendarVToDo& vtodo, EModelessAction action);

			bool		ContainsToDo(const iCal::CICalendarVToDo& vtodo) const;

			void		InitPanels();
	
			void	SetAction(EModelessAction action)
			{
				mAction = action;
			}
			void	SetToDo(iCal::CICalendarVToDo& vtodo);
			void	GetToDo(iCal::CICalendarVToDo& vtodo);
			void	ChangedSummary();
			void	ChangedCalendar();

			void	SetReadOnly(bool read_only);

			void	DoCompleted(bool completed);
			
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
	afx_msg void	OnCompleted();
	afx_msg void	OnCompletedNow();
	afx_msg void	OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnOrganiserEdit();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static uint32_t				sTitleCounter;
	static set<CNewToDoDialog*>	sDialogs;
};

#endif
