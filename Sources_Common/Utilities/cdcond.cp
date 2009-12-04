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


// cdcond.cp - handles thread event blocking

#include "cdcond.h"

#pragma mark ____________________________cdmutex

cdcond::cdcond()
{
#if __dest_os == __win32_os
	mEvent = ::CreateEvent(NULL, false, false, NULL);
#endif
}

cdcond::~cdcond()
{
#if __dest_os == __win32_os
	if (mEvent)
		::CloseHandle(mEvent);
	mEvent = NULL;
#endif
}

int cdcond::wait(long timeout_ms)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	// Map to windows timeout
	SInt32 time_out;
	switch(timeout_ms)
	{
	case eTimeoutNone:
		time_out = semaphore_NoWait;
		break;
	case eTimeoutInfinite:
		time_out = semaphore_WaitForever;
		break;
	default:
		time_out = timeout_ms;
		break;
	}

	// Wait for event with timeout
	ExceptionCode macresult = mEvent.Wait(time_out);

	// Map windows result code
	switch(macresult)
	{
	case noErr:
		return eCondOK;
	case errSemaphoreTimedOut:
		return eCondTimeout;
	default:
		return eCondFailed;
	}

#elif __dest_os == __win32_os

	// Map to windows timeout
	DWORD time_out;
	switch(timeout_ms)
	{
	case eTimeoutNone:
		time_out = 0;
		break;
	case eTimeoutInfinite:
		time_out = INFINITE;
		break;
	default:
		time_out = timeout_ms;
		break;
	}

	// Wait for event with timeout
	DWORD winresult = mEvent ? ::WaitForSingleObject(mEvent, time_out) : -1;

	// Map windows result code
	switch(winresult)
	{
	case WAIT_OBJECT_0:
		return eCondOK;
	case WAIT_TIMEOUT:
		return eCondTimeout;
	default:
		return eCondFailed;
	}

#elif __dest_os == __linux_os

	// Wait for the appropriate amount of time
	ACE_Time_Value time;
	int result;
	switch(timeout_ms)
	{
	case eTimeoutNone:
		time = 0;
		result = mEvent.wait(&time);
		break;
	case eTimeoutInfinite:
		result = mEvent.wait();
		break;
	default:
		time.set(0, 1000 * timeout_ms);
		time += ACE_OS::gettimeofday();
		result = mEvent.wait(&time);
		break;
	}

	// result == 0 => OK result
	if (!result)
		return eCondOK;

	// check for timeout
	else if (errno == ETIME)
		return eCondTimeout;

	// Generic error
	else
		return eCondFailed;
#endif
}

int cdcond::signal()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mEvent.Signal();
#elif __dest_os == __win32_os
	if (mEvent)
		::SetEvent(mEvent);
#elif __dest_os == __linux_os
	mEvent.signal();
#endif

	return eCondOK;
}

int cdcond::clear()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mEvent.Reset();
#elif __dest_os == __win32_os
	if (mEvent)
		::ResetEvent(mEvent);
#elif __dest_os == __linux_os
	mEvent.reset();
#endif

	return eCondOK;
}
