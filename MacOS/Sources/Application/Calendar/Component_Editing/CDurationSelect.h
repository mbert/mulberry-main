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

#ifndef H_CDurationSelect
#define H_CDurationSelect
#pragma once

#include <LView.h>
#include <LListener.h>

#include "CICalendarDuration.h"

class CNumberEdit;

class LLittleArrows;
class LPopupButton;

// ===========================================================================
//	CDurationSelect

class CDurationSelect : public LView,
						public LListener
{
public:
	enum { class_ID = 'Durs', pane_ID = 1812 };

	static CDurationSelect* CreateInside(LView* parent);

						CDurationSelect(LStream *inStream);
	virtual				~CDurationSelect();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

			void	SetAllDay(bool all_day);
			
			void	SetDuration(const iCal::CICalendarDuration& du, bool all_day);
			void	GetDuration(iCal::CICalendarDuration& du, bool all_day);

			void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eDurationNumber_ID = 'DURN',
		eDurationNumberSpin_ID = 'DURn',
		eDurationPeriod_ID = 'DURP',
		eDurationTime_ID = 'DURT',
		eDurationHours_ID = 'DURH',
		eDurationMinutes_ID = 'DURM',
		eDurationSeconds_ID = 'DURS'
	};

	enum
	{
		eDurationPeriod_Days = 1,
		eDurationPeriod_Weeks
	};

	// UI Objects
	CNumberEdit*			mDurationNumber;
	LLittleArrows*			mDurationNumberSpin;
	LPopupButton*			mDurationPeriod;
	LView*					mDurationTime;
	CNumberEdit*			mDurationHours;
	CNumberEdit*			mDurationMinutes;
	CNumberEdit*			mDurationSeconds;

	bool	mAllDay;

	virtual void		FinishCreateSelf();
	
			void		OnDurationPopup(UInt32 value);
	
};

#endif
