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


// Source for CEventSemaphore class

#include "CEventSemaphore.h"

#include <LThread.h>
#include <PP_Constants.h>
#include <UException.h>
#include <UThread.h>

//	Default constructor.  The semaphore is created in the unposted state.
CEventSemaphore::CEventSemaphore() : LSemaphore()
{
	// there's nothing to do
}


//	Destructor.
CEventSemaphore::~CEventSemaphore()
{
	// there's nothing to do
}


// ---------------------------------------------------------------------------
//	¥ Reset
// ---------------------------------------------------------------------------
//	Make the semaphore unavailable to any thread until the next call
//	to Signal().  Returns the number of times that Signal() was called
//	since the last call to Reset().

void CEventSemaphore::Reset()
{
	StCritical	critical;	// disable preemption

	if (mExcessSignals > 0)
	{
		mExcessSignals	= 0;
	}
	else
	{
		//Throw_(errSemaphoreAlreadyReset);
	}
}
