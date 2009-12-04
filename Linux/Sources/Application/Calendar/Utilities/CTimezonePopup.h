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

#include "TPopupMenu.h"
#include "HPopupMenu.h"

#include "CICalendarTimezone.h"

// ===========================================================================
//	CTimezonePopup

class	CTimezonePopup : public HPopupMenu {
public:
	enum
	{
		eNoTimezone = 1,
		eUTC,
		//eSeparator,
		eFirstTimezone
	};

			CTimezonePopup(const JCharacter*	title,
						   JXContainer*			enclosure,
						   const HSizingOption	hSizing,
						   const VSizingOption	vSizing,
						   const JCoordinate	x,
						   const JCoordinate	y,
						   const JCoordinate	w,
						   const JCoordinate	h);
						
	virtual	~CTimezonePopup();

	virtual void		OnCreate();

	void		SetTimezone(const iCal::CICalendarTimezone& tz);
	void		GetTimezone(iCal::CICalendarTimezone& tz) const;
	
	void		NoFloating();

protected:
	bool		mNoFloating;

};

#endif
