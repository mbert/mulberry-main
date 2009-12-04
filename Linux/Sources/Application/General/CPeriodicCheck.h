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


// Header for CPeriodicCheck class

#ifndef __CPERIODICCHECK__MULBERRY__
#define __CPERIODICCHECK__MULBERRY__


#include <time.h>
#include <JXIdleTask.h>

// Classes
class CPeriodicCheck : public JXIdleTask
{
public:

	CPeriodicCheck() : JXIdleTask(minWaitTime)
		{ mLastSpendTime = 0; mBlock = false; mPwrSignal = 0; mCallDepth = 0; };
	
	virtual	void	Perform(const Time delta, Time* maxSleepTime);

	// Sleep handling
	void			PwrSignal();
	void			Suspend();
	void			Resume(bool silent = false);

private:
	static const Time minWaitTime = 100;
	static const Time maxWaitTime = 500;

	time_t			mLastSpendTime;
	bool			mBlock;
	unsigned long	mPwrSignal;
	unsigned long	mCallDepth;
	
	void			CheckAppIdleTooLong();
};

#endif
