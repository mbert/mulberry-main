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

#include <JXWidgetSet.h>

#include "CICalendarDuration.h"

#include "HPopupMenu.h"

class CNumberEdit;

class JXFlatRect;

// ===========================================================================
//	CDurationSelect

class CDurationSelect : public JXWidgetSet
{
public:
	static CDurationSelect* CreateInside(JXContainer* parent);

					CDurationSelect(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual			~CDurationSelect();

			void	OnCreate();					// Do odds & ends

			void	SetAllDay(bool all_day);
			
			void	SetDuration(const iCal::CICalendarDuration& du, bool all_day);
			void	GetDuration(iCal::CICalendarDuration& du, bool all_day);

			void	SetReadOnly(bool read_only);

protected:

	enum
	{
		eDurationPeriod_Days = 1,
		eDurationPeriod_Weeks
	};

	// UI Objects
// begin JXLayout1

    CNumberEdit* mDurationNumber;
    HPopupMenu*  mDurationPeriod;
    JXFlatRect*  mDurationTime;
    CNumberEdit* mDurationHours;
    CNumberEdit* mDurationMinutes;
    CNumberEdit* mDurationSeconds;

// end JXLayout1

	bool	mAllDay;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
			void		OnDurationPopup(JIndex value);
	
};

#endif
