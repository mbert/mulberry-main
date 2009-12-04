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


// Header for CNewAttendeeDialog class

#ifndef __CNewAttendeeDialog__MULBERRY__
#define __CNewAttendeeDialog__MULBERRY__

#include <LDialogBox.h>

#include "CICalendarDefinitions.h"
#include "CICalendarProperty.h"

// Constants

// Type
class	CStaticText;
class	LCheckBox;
class	LPopupButton;
class	CCalendarAddressDisplay;

class CNewAttendeeDialog : public LDialogBox
{
public:
	enum { class_ID = 'CAtt', pane_ID = 1822 };

	static bool		PoseDialog(iCal::CICalendarProperty& prop);
	static bool		PoseDialog(iCal::CICalendarPropertyList& proplist);

					CNewAttendeeDialog(LStream *inStream);
	virtual 		~CNewAttendeeDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	
protected:
	enum
	{
		eName_ID = 'ATST',
		eRole_ID = 'ROLE',
		eStatus_ID = 'STAT',
		eRSVP_ID = 'RSVP'
	};

	// UI Objects
	CCalendarAddressDisplay*	mNames;
	LPopupButton*				mRolePopup;
	LPopupButton*				mStatusPopup;
	LCheckBox*					mRSVP;
	bool						mITIPSent;
	bool						mNewItem;

	virtual void FinishCreateSelf(void);					// Do odds & ends

			void	SetDetails(const iCal::CICalendarProperty& prop);		// Set the dialogs info
			void	GetDetails(iCal::CICalendarProperty& prop);		// Get the dialogs return info
			void	GetDetails(iCal::CICalendarPropertyList& proplist);		// Get the dialogs return info
};

#endif
