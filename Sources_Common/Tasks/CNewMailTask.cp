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


// Source for CNewMailAlertTask class

#include "CNewMailTask.h"

#include "CErrorHandler.h"

#pragma mark ____________________________CNewMailAlertTask

cdmutex CNewMailAlertTask::sOnlyOne;
unsigned long CNewMailAlertTask::sInUse = 0;

void CNewMailAlertTask::Make_NewMailAlertTask(const char* rsrcid, const CMailNotification& notifier)
{
	// Protect against multiple access
	cdmutex::lock_cdmutex _lock(sOnlyOne);

	// Bump in use counter
	sInUse++;

	// Create new mail task with indication of whether others are pending
	CNewMailAlertTask* task = new CNewMailAlertTask(rsrcid, notifier, sInUse > 1);
	task->Go();
}

void CNewMailAlertTask::Work()
{
	CErrorHandler::PutNoteAlertRsrc(mRsrcId, mNotify, mNoDisplay);

	// Protect against multiple access and decrement in use count
	cdmutex::lock_cdmutex _lock(sOnlyOne);
	sInUse--;
}
