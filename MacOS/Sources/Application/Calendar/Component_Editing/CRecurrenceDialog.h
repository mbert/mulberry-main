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
#pragma once

#include <LDialogBox.h>

#include "CICalendarRecurrence.h"

class CDateTimeZoneSelect;
class CNumberEdit;

class LBevelButton;
class LLittleArrows;
class LPopupButton;
class LRadioButton;

// ===========================================================================
//	CRecurrenceDialog

class	CRecurrenceDialog : public LDialogBox
{
public:
	enum { class_ID = 'EdRe', pane_ID = 1820 };

	static bool PoseDialog(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);

						CRecurrenceDialog(LStream *inStream);
	virtual				~CRecurrenceDialog();

	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	enum
	{
		eOccursItems_ID = 'OCCS',
		eOccursInterval_ID = 'INUM',
		eOccursIntervalSpin_ID = 'INUm',
		eOccursFreq_ID = 'OCCU',
		eOccursGroup_ID = 'OGRP',
		eOccursForEver_ID = 'OEVR',
		eOccursCount_ID = 'OFOR',
		eOccursUntil_ID = 'OUNT',
		eOccursCounter_ID = 'ONUM',
		eOccursCounterSpin_ID = 'ONUm',
		eOccursDateTimeZone_ID = 'OCUS',
		
		eByMonth_ID = 'BYMO',
		eByMonth01_ID = 'BYMA',
		eByMonth02_ID = 'BYMB',
		eByMonth03_ID = 'BYMC',
		eByMonth04_ID = 'BYMD',
		eByMonth05_ID = 'BYME',
		eByMonth06_ID = 'BYMF',
		eByMonth07_ID = 'BYMG',
		eByMonth08_ID = 'BYMH',
		eByMonth09_ID = 'BYMI',
		eByMonth10_ID = 'BYMJ',
		eByMonth11_ID = 'BYMK',
		eByMonth12_ID = 'BYML',

		eByMonthDay_ID = 'BYMO',
		eByMonthDay01_ID = 'BY01',
		eByMonthDay02_ID = 'BY02',
		eByMonthDay03_ID = 'BY03',
		eByMonthDay04_ID = 'BY04',
		eByMonthDay05_ID = 'BY05',
		eByMonthDay06_ID = 'BY06',
		eByMonthDay07_ID = 'BY07',
		eByMonthDay08_ID = 'BY08',
		eByMonthDay09_ID = 'BY09',
		eByMonthDay10_ID = 'BY0:',
		eByMonthDay11_ID = 'BY0;',
		eByMonthDay12_ID = 'BY0<',
		eByMonthDay13_ID = 'BY0=',
		eByMonthDay14_ID = 'BY0>',
		eByMonthDay15_ID = 'BY0?',
		eByMonthDay16_ID = 'BY0@',
		eByMonthDay17_ID = 'BY0A',
		eByMonthDay18_ID = 'BY0B',
		eByMonthDay19_ID = 'BY0C',
		eByMonthDay20_ID = 'BY0D',
		eByMonthDay21_ID = 'BY0E',
		eByMonthDay22_ID = 'BY0F',
		eByMonthDay23_ID = 'BY0G',
		eByMonthDay24_ID = 'BY0H',
		eByMonthDay25_ID = 'BY0I',
		eByMonthDay26_ID = 'BY0J',
		eByMonthDay27_ID = 'BY0K',
		eByMonthDay28_ID = 'BY0L',
		eByMonthDay29_ID = 'BY0M',
		eByMonthDay30_ID = 'BY0N',
		eByMonthDay31_ID = 'BY0O',

		eByMonthDayLast01_ID = 'BY11',
		eByMonthDayLast02_ID = 'BY12',
		eByMonthDayLast03_ID = 'BY13',
		eByMonthDayLast04_ID = 'BY14',
		eByMonthDayLast05_ID = 'BY15',
		eByMonthDayLast06_ID = 'BY16',
		eByMonthDayLast07_ID = 'BY17',
		
		eByDay_ID = 'BYDY',
		eByDayView_ID = 'BYDV',
		eByDay01_ID = 'BYD0',
		eByDay02_ID = 'BYD1',
		eByDay03_ID = 'BYD2',
		eByDay04_ID = 'BYD3',
		eByDay05_ID = 'BYD4',
		eByDay06_ID = 'BYD5',
		eByDay07_ID = 'BYD6',
		
		eByDayPopup = 'BYDP',

		
		eBySetPos_ID = 'BYSP',
		eBySetPosPopup_ID = 'BYPP',
	};

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
	CNumberEdit*			mOccursInterval;
	LLittleArrows*			mOccursIntervalSpin;
	LPopupButton*			mOccursFreq;

	LRadioButton*			mOccursForEver;
	LRadioButton*			mOccursCount;
	LRadioButton*			mOccursUntil;

	CNumberEdit*			mOccursCounter;
	LLittleArrows*			mOccursCounterSpin;

	CDateTimeZoneSelect*	mOccursDateTimeZone;

	LBevelButton*			mByMonth[12];
	LBevelButton*			mByMonthDay[31];
	LBevelButton*			mByMonthDayLast[7];
	LBevelButton*			mByDay[7];
	LView*					mByDayView;
	LPopupButton*			mByDayPopup;

	LPopupButton*			mBySetPosPopup;

	virtual void		FinishCreateSelf();
	
			void	DoOccursFreq(UInt32 value);
			void	DoOccursGroup(UInt32 value);
			
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
