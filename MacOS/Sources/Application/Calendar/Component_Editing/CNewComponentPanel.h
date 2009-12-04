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

#ifndef H_CNewComponentPanel
#define H_CNewComponentPanel
#pragma once

#include <LView.h>

#include "CICalendarVEvent.h"
#include "CICalendarVToDo.h"

// ===========================================================================
//	CNewComponentPanel

class CNewComponentPanel : public LView
{
public:
	enum
	{
		eDurationPeriod_Days = 1,
		eDurationPeriod_Weeks
	};

						CNewComponentPanel(LStream *inStream) :
							LView(inStream) { mReadOnly = false; }
	virtual				~CNewComponentPanel() {}

	virtual void	SetComponent(const iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);
	
	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded) {}
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent) {}
	
	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded) {}
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo) {}

	virtual void	SetReadOnly(bool read_only) = 0;

protected:
	bool			mReadOnly;
};

typedef std::vector<CNewComponentPanel*> CNewComponentPanelList;

#endif
