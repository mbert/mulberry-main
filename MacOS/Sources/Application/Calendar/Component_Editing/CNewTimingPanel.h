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

#ifndef H_CNewTimingPanel
#define H_CNewTimingPanel
#pragma once

#include "CNewComponentPanel.h"

// ===========================================================================
//	CNewTimingPanel

class CNewTimingPanel : public CNewComponentPanel
{
public:
						CNewTimingPanel(LStream *inStream) :
							CNewComponentPanel(inStream) {}
	virtual				~CNewTimingPanel() {}

	virtual bool	GetAllDay() const = 0;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const = 0;
			
};

#endif
