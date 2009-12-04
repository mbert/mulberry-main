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

// Classes

class CPeriodicCheck
{
public:

					CPeriodicCheck()
						{ mLastSpendTime = 0; mBlock = false; mCallDepth = 0; };

	virtual	void	SpendTime(bool do_tasks = true);	// Called during idle

	// Sleep handling
	void			Suspend();
	void			Resume(bool silent = false);

private:
	time_t			mLastSpendTime;
	bool			mBlock;
	unsigned long	mCallDepth;
	
	void			CheckAppIdleTooLong();
};

#endif
