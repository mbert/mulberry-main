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


// Header for CNewAttendeeDialog class

#ifndef __CNewAttendeeDialog__MULBERRY__
#define __CNewAttendeeDialog__MULBERRY__

#include "CHelpDialog.h"

#include "CAddressDisplay.h"
#include "CPopupButton.h"

#include "CICalendarDefinitions.h"
#include "CICalendarProperty.h"

// Constants

// Type

class CNewAttendeeDialog : public CHelpDialog
{
public:
	static bool		PoseDialog(iCal::CICalendarProperty& prop);
	static bool		PoseDialog(iCal::CICalendarPropertyList& proplist);

					CNewAttendeeDialog(CWnd* pParent = NULL);
	virtual 		~CNewAttendeeDialog();

protected:
// Dialog Data
	//{{AFX_DATA(CNewAttendeeDialog)
	enum { IDD = IDD_CALENDAR_NEWATTENDEE };
	CAddressDisplay	mNames;
	CPopupButton	mRolePopup;
	CPopupButton	mStatusPopup;
	CButton			mRSVP;
	//}}AFX_DATA
	bool			mITIPSent;
	bool			mNewItem;

	iCal::CICalendarProperty*		mPropData;
	iCal::CICalendarPropertyList*	mPropListData;

// Overrides
			void	InitControls();

			void	SetDetails(const iCal::CICalendarProperty& prop);		// Set the dialogs info
			void	GetDetails(iCal::CICalendarProperty& prop);		// Get the dialogs return info
			void	GetDetails(iCal::CICalendarPropertyList& proplist);		// Get the dialogs return info

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewAttendeeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CNewAttendeeDialog)
	afx_msg	void	OnPartStat();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

#endif
