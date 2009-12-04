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


// Header for CNewMailAlertTask class

#ifndef __CNEWMAILTASK__MULBERRY__
#define __CNEWMAILTASK__MULBERRY__

#include "CTaskQueue.h"

#include "CMailNotification.h"

// Note alert - must be sync as a result is required before continuing
class CNewMailAlertTask : public CTaskAsync
{
public:
	static void Make_NewMailAlertTask(const char* rsrcid, const CMailNotification& notifier);

protected:
	static cdmutex			sOnlyOne;
	static unsigned long	sInUse;

	const char*			mRsrcId;
	CMailNotification	mNotify;
	bool				mNoDisplay;

	virtual void Work();

private:
	CNewMailAlertTask(const char* rsrcid, const CMailNotification& notifier, bool no_display)
		: mRsrcId(rsrcid), mNotify(notifier), mNoDisplay(no_display) {}

};

#endif
