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

/* 
	CCalendarStoreFreeBusy.h

	Author:			
	Description:	Represents a node in a calendar store calendar hierarchy
*/

#ifndef CCalendarStoreFreeBusy_H
#define CCalendarStoreFreeBusy_H

#include "CICalendarFreeBusy.h"

#include "CICalendarRef.h"

#include "cdstring.h"

namespace calstore {

class CCalendarStoreFreeBusy
{
public:
	CCalendarStoreFreeBusy(const cdstring& name)
	{
		mName = name;
	}
	CCalendarStoreFreeBusy(const cdstring& name, const iCal::CICalendarFreeBusyList& periods)
	{
		mName = name;
		mPeriods = periods;
	}
	CCalendarStoreFreeBusy(const CCalendarStoreFreeBusy& copy)
	{
		mName = copy.mName;
		mPeriods = copy.mPeriods;
	}
	~CCalendarStoreFreeBusy() {}

	void SetName(const cdstring& name)							// Set name
		{ mName = name; }
	const cdstring&	GetName() const								// Get full name
		{ return mName; }

	void	SetPeriods(const iCal::CICalendarFreeBusyList& periods)		// Set periods
		{ mPeriods = periods; }
	const iCal::CICalendarFreeBusyList& GetPeriods() const		// Get periods
		{ return mPeriods; }
	iCal::CICalendarFreeBusyList& GetPeriods()					// Get periods
		{ return mPeriods; }

protected:
	cdstring						mName;						// Full path name of item
	iCal::CICalendarFreeBusyList	mPeriods;					// Periods for busy time
};

typedef std::vector<CCalendarStoreFreeBusy> CCalendarStoreFreeBusyList;

}	// namespace calstore

#endif	// CCalendarStoreFreeBusy_H
