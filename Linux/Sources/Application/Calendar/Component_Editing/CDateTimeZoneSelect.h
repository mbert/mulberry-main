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

#ifndef H_CDateTimeZoneSelect
#define H_CDateTimeZoneSelect

#include <JXWidgetSet.h>

#include "CICalendarDateTime.h"

class CDateTimeControl;
class CTimezonePopup;

class JXMultiImageButton;

// ===========================================================================
//	CDateTimeZoneSelect

class CDateTimeZoneSelect : public JXWidgetSet
{
public:
	static CDateTimeZoneSelect* CreateInside(JXContainer* parent);

					CDateTimeZoneSelect(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual			~CDateTimeZoneSelect();

			void	OnCreate();					// Do odds & ends

			void	SetAllDay(bool all_day);
			
			void	SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day);
			void	GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day);

			void	SetReadOnly(bool read_only);

protected:
	// UI Objects
// begin JXLayout1

    CDateTimeControl*   mDate;
    JXMultiImageButton* mDateBtn;
    CDateTimeControl*   mTime;
    CTimezonePopup*     mZone;

// end JXLayout1

	bool	mAllDay;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
	void	OnDateBtn();
};

#endif
