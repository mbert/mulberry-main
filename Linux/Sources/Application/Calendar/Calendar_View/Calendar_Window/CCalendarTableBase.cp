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

#include "CCalendarTableBase.h"

#include "CCalendarViewBase.h"
#include "CMulberryApp.h"

// ---------------------------------------------------------------------------
//	CCalendarTableBase														  [public]
/**
	Default constructor */

CCalendarTableBase::CCalendarTableBase(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	JXIdleTask(minWaitTime)
{
	SetBorderWidth(0);

	mCalendarView = NULL;
	mLastIdleTime = iCal::CICalendarDateTime::GetNowUTC();

	//	install an event loop timer for idling
	CMulberryApp::sApp->InstallPermanentTask(this);
}


// ---------------------------------------------------------------------------
//	~CCalendarTableBase														  [public]
/**
	Destructor */

CCalendarTableBase::~CCalendarTableBase()
{
	// remove our timer, if any
	CMulberryApp::sApp->RemovePermanentTask(this);
}

#pragma mark -

void CCalendarTableBase::Perform(const Time delta, Time* maxSleepTime) 
{
	// Check for periodic check
	*maxSleepTime = maxWaitTime;
	Time bogus;
	if (!TimeToPerform(delta, &bogus))
		return;

	Idle();
}

void CCalendarTableBase::Idle()
{
	// Derived classes will override
}

void CCalendarTableBase::OnCreateNew()
{
	Broadcast_Message(eBroadcast_CreateNew, NULL);
}
