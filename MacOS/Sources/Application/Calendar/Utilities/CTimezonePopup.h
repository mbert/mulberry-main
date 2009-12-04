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

#ifndef H_CTIMEZONEPOPUP
#define H_CTIMEZONEPOPUP
#pragma once

#include <LPopupButton.h>

#include "CICalendarTimezone.h"

// ===========================================================================
//	CTimezonePopup

class	CTimezonePopup : public LPopupButton {
public:
	enum { class_ID = 'Timz' };

	enum
	{
		eNoTimezone = 1,
		eUTC,
		eSeparator,
		eFirstTimezone
	};
						CTimezonePopup(LStream* inStream);
						
	virtual				~CTimezonePopup();

	void		SetTimezone(const iCal::CICalendarTimezone& tz);
	void		GetTimezone(iCal::CICalendarTimezone& tz) const;
	
	void		NoFloating();

protected:
	bool		mNoFloating;

	virtual void		FinishCreateSelf();

};

#endif
