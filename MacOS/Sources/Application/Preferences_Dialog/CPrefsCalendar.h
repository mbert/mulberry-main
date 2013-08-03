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


// Header for CPrefsCalendar class

#ifndef __CPREFSCALENDAR__MULBERRY__
#define __CPREFSCALENDAR__MULBERRY__

#include "CPrefsPanel.h"

#include "CClockEventHandler.h"

#include <stdint.h>

// Constants

// Panes
const	PaneIDT		paneid_PrefsCalendar = 5014;
const	PaneIDT		paneid_CalendarWeekStart = 'WSTR';
const	PaneIDT		paneid_CalendarWorkSunday = 'BYD0';
const	PaneIDT		paneid_CalendarWorkMonday = 'BYD1';
const	PaneIDT		paneid_CalendarWorkTuesday = 'BYD2';
const	PaneIDT		paneid_CalendarWorkWednesday = 'BYD3';
const	PaneIDT		paneid_CalendarWorkThursday = 'BYD4';
const	PaneIDT		paneid_CalendarWorkFriday = 'BYD5';
const	PaneIDT		paneid_CalendarWorkSaturday = 'BYD6';
const	PaneIDT		paneid_CalendarMorningStart = 'RST0';
const	PaneIDT		paneid_CalendarMorningEnd = 'REN0';
const	PaneIDT		paneid_CalendarAfternoonStart = 'RST1';
const	PaneIDT		paneid_CalendarAfternoonEnd = 'REN1';
const	PaneIDT		paneid_CalendarEveningStart = 'RST2';
const	PaneIDT		paneid_CalendarEveningEnd = 'REN2';
const	PaneIDT		paneid_CalendarAwakeStart = 'RST3';
const	PaneIDT		paneid_CalendarAwakeEnd = 'REN3';
const	PaneIDT		paneid_CalendarWorkStart = 'RST4';
const	PaneIDT		paneid_CalendarWorkEnd = 'REN4';
const	PaneIDT		paneid_CalendarDisplayTime = 'DTIM';
const	PaneIDT		paneid_CalendarHandleICS = 'HICS';
const	PaneIDT		paneid_AutomaticIMIP = 'IMIP';
const	PaneIDT		paneid_AutomaticEDST = 'EDST';
const	PaneIDT		paneid_ShowUID = 'UID ';

// Mesages

// Resources
const	ResIDT		RidL_CPrefsCalendarBtns = 5014;

// Classes
class LBevelButton;
class LCheckBox;
class LClock;
class LPopupButton;

class	CPrefsCalendar : public CPrefsPanel
{
private:
	CClockEventHandler	mClockHandler;
	LPopupButton*		mWeekStart;
	LBevelButton*		mWorkDay[7];
	LClock*				mRangeStarts[5];
	LClock*				mRangeEnds[5];
	LCheckBox*			mDisplayTime;
	LCheckBox*			mHandleICS;
	LCheckBox*			mAutomaticIMIP;
	LCheckBox*			mAutomaticEDST;
	LCheckBox*			mShowUID;

public:
	enum { class_ID = 'Pcal' };

					CPrefsCalendar();
					CPrefsCalendar(LStream *inStream);
	virtual 		~CPrefsCalendar();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

private:
			void SetHours(LClock* ctrl, uint32_t hours);
			uint32_t GetHours(LClock* ctrl);

};

#endif
