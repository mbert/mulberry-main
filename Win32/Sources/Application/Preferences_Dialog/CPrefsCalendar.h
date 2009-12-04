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


// CPrefsCalendar.h : header file
//

#ifndef __CPREFSCALENDAR__MULBERRY__
#define __CPREFSCALENDAR__MULBERRY__

#include "CPrefsPanel.h"

#include "CDateControl.h"
#include "CPopupButton.h"
#include "CTextButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar dialog

class CPreferences;

class CPrefsCalendar : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsCalendar)

// Construction
public:
	CPrefsCalendar();
	~CPrefsCalendar();

// Dialog Data
	//{{AFX_DATA(CPrefsCalendar)
	enum { IDD = IDD_PREFS_CALENDAR };
	CPopupButton	mWeekStartBtn;
	CTextButton		mWorkDay[7];
	CDateControl	mRangeStarts[5];
	CDateControl	mRangeEnds[5];
	BOOL			mDisplayTime;
	BOOL			mHandleICS;
	BOOL			mAutomaticIMIP;
	BOOL			mAutomaticEDST;
	//}}AFX_DATA
	bool			mInitialised;
	

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsCalendar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsCalendar)
	virtual void InitControls(void);
	virtual void SetControls(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
			void SetHours(CDateControl& ctrl, uint32_t hours);
			uint32_t GetHours(CDateControl& ctrl);
};

#endif
