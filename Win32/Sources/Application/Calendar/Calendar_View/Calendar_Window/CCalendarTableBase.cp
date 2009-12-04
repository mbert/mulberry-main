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

BEGIN_MESSAGE_MAP(CCalendarTableBase, CTable)
	//{{AFX_MSG_MAP(CCalendarTableBase)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CCalendarTableBase														  [public]
/**
	Default constructor */

CCalendarTableBase::CCalendarTableBase()
{
	mCalendarView = NULL;
	mLastIdleTime = iCal::CICalendarDateTime::GetNowUTC();
}


// ---------------------------------------------------------------------------
//	~CCalendarTableBase														  [public]
/**
	Destructor */

CCalendarTableBase::~CCalendarTableBase()
{
}

#pragma mark -

int CCalendarTableBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTable::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Start a timer once hWnd is setup
	mTimerID = SetTimer('CalT', 1000, NULL);
	
	return 0;
}

// Destroy
void CCalendarTableBase::OnDestroy()
{
	if (mTimerID != 0)
		KillTimer(mTimerID);
	CTable::OnDestroy();
}

void CCalendarTableBase::OnCreateNew()
{
	Broadcast_Message(eBroadcast_CreateNew, NULL);
}

void CCalendarTableBase::OnTimer(UINT nIDEvent)
{
	Idle();
}

void CCalendarTableBase::Idle()
{
	// Derived classes will override
}

