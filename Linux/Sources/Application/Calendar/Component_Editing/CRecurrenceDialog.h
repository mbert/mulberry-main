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

#ifndef H_CRECURRENCEDIALOG
#define H_CRECURRENCEDIALOG

#include "CDialogDirector.h"

#include "CICalendarRecurrence.h"

#include "HPopupMenu.h"

class CDateTimeZoneSelect;
class CNumberEdit;

class JXFlatRect;
class JXRadioGroup;
class JXRadioGroup;
class JXTextButton;
class JXTextPushButton;
class JXTextRadioButton;

// ===========================================================================
//	CRecurrenceDialog

class	CRecurrenceDialog : public CDialogDirector
{
public:
	static bool PoseDialog(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);

						CRecurrenceDialog(JXDirector* supervisor);
	virtual				~CRecurrenceDialog();

protected:
	enum
	{
		eOccurs_Yearly = 1,
		eOccurs_Monthly,
		eOccurs_Weekly,
		eOccurs_Daily,
		eOccurs_Hourly,
		eOccurs_Minutely,
		eOccurs_Secondly
	};

	enum
	{
		eOccurs_ForEver = 1,
		eOccurs_Count,
		eOccurs_Until
	};

	enum
	{
		eByDay_Any = 1,
		eByDay_1st,
		eByDay_2nd,
		eByDay_3rd,
		eByDay_4th,
		eByDay_2ndLast,
		eByDay_1stLast
	};

	enum
	{
		eBySetPos_All = 1,
		eBySetPos_1st,
		eBySetPos_Last
	};

	// UI Objects
// begin JXLayout

    CNumberEdit*         mOccursInterval;
    HPopupMenu*          mOccursFreq;
    JXRadioGroup*        mOccursGroup;
    JXTextRadioButton*   mOccursForEver;
    JXTextRadioButton*   mOccursCount;
    JXTextRadioButton*   mOccursUntil;
    CNumberEdit*         mOccursCounter;
    CDateTimeZoneSelect* mOccursDateTimeZone;
    JXFlatRect*          mByDayView;
    HPopupMenu*          mByDayPopup;
    HPopupMenu*          mBySetPosPopup;
    JXTextButton*        mCancelBtn;
    JXTextButton*        mOKBtn;

// end JXLayout
	JXTextPushButton*			mByMonth[12];
	JXTextPushButton*			mByMonthDay[31];
	JXTextPushButton*			mByMonthDayLast[7];
	JXTextPushButton*			mByDay[7];

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	
			void	DoOccursFreq(JIndex value);
			void	DoOccursGroup(JIndex value);
			
			void	SetRecurrence(const iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);
			void	SetByMonth(const iCal::CICalendarRecurrence& recur);
			void	SetByMonthDay(const iCal::CICalendarRecurrence& recur);
			void	SetByDay(const iCal::CICalendarRecurrence& recur);
			void	SetBySetPos(const iCal::CICalendarRecurrence& recur);

			void	GetRecurrence(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);
			void	GetByMonth(iCal::CICalendarRecurrence& recur);
			void	GetByMonthDay(iCal::CICalendarRecurrence& recur);
			void	GetByDay(iCal::CICalendarRecurrence& recur);
			void	GetBySetPos(iCal::CICalendarRecurrence& recur);
};

#endif
