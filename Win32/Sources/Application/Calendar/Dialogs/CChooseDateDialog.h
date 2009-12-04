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

#ifndef H_CChooseDateDialog
#define H_CChooseDateDialog
#pragma once

#include "CHelpDialog.h"
#include "CListener.h"

#include "CIconButton.h"
#include "CMonthIndicator.h"
#include "CMonthPopup.h"
#include "CYearPopup.h"

#include "CICalendarDateTime.h"

// ===========================================================================
//	CChooseDateDialog

class	CChooseDateDialog : public CHelpDialog,
							public CListener
{
public:
	static bool PoseDialog(iCal::CICalendarDateTime& dt);

						CChooseDateDialog(CWnd* pParent = NULL);
	virtual				~CChooseDateDialog();

	virtual void		ListenTo_Message(long msg, void* param);

protected:
// Dialog Data
	//{{AFX_DATA(CChooseDateDialog)
	enum { IDD = IDD_CALENDAR_CHOOSEDATE };
	CIconButton			mYearMinus;
	CYearPopup			mYearPopup;
	CIconButton			mYearPlus;
	CIconButton			mMonthMinus;
	CMonthPopup			mMonthPopup;
	CIconButton			mMonthPlus;
	CMonthIndicator		mTable;
	//}}AFX_DATA

	iCal::CICalendarDateTime	mDateResult;

			void	InitControls();
			void	SetDate(const iCal::CICalendarDateTime& dt);
			void	GetDate(iCal::CICalendarDateTime& dt);
			
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDateDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CChooseDateDialog)
	afx_msg void OnPrevYear();
	afx_msg void OnYearPopup();
	afx_msg void OnNextYear();
	afx_msg void OnPrevMonth();
	afx_msg void OnMonthPopup();
	afx_msg void OnNextMonth();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

};

#endif
