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

EventLoopTimerUPP _CalendarIdleTimerUPP = NULL;

// ---------------------------------------------------------------------------
//	CCalendarTableBase														  [public]
/**
	Default constructor */

CCalendarTableBase::CCalendarTableBase(LStream *inStream) :
	CTableDrag(inStream)
{
	mCalendarView = NULL;
	mLastIdleTime = iCal::CICalendarDateTime::GetNowUTC();

	//	create timer UPP first time through
	if (_CalendarIdleTimerUPP == NULL)
	{
		_CalendarIdleTimerUPP = NewEventLoopTimerUPP(IdleTimer);
	}

	//	install an event loop timer for idling
	InstallEventLoopTimer(GetMainEventLoop(), 0, TicksToEventTime(60), _CalendarIdleTimerUPP, this, &mIdleTimer);
}


// ---------------------------------------------------------------------------
//	~CCalendarTableBase														  [public]
/**
	Destructor */

CCalendarTableBase::~CCalendarTableBase()
{
	// remove our timer, if any
	if (mIdleTimer != NULL)
	{
		RemoveEventLoopTimer(mIdleTimer);
		mIdleTimer = NULL;
	}
}

#pragma mark -

pascal void CCalendarTableBase::IdleTimer(EventLoopTimerRef inTimer, void * inUserData)
{
	static_cast<CCalendarTableBase*>(inUserData)->Idle();
}

void CCalendarTableBase::Idle()
{
	// Derived classes will override
}

// Click
void CCalendarTableBase::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	CTableDrag::ClickSelf(inMouseDown);
}

void CCalendarTableBase::OnCreateNew()
{
	Broadcast_Message(eBroadcast_CreateNew, NULL);
}
