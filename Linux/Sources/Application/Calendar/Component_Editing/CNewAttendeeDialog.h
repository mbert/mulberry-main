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

#include "CDialogDirector.h"

#include "HPopupMenu.h"

#include "CICalendarDefinitions.h"
#include "CICalendarProperty.h"

// Classes
class	CCalendarAddressDisplay;
class	JXTextCheckbox;
class	JXTextButton;

class CNewAttendeeDialog : public CDialogDirector
{
public:
	static bool		PoseDialog(iCal::CICalendarProperty& prop);
	static bool		PoseDialog(iCal::CICalendarPropertyList& proplist);

					CNewAttendeeDialog(JXDirector* supervisor);
	virtual 		~CNewAttendeeDialog();
	
protected:

	// UI Objects
// begin JXLayout

    CCalendarAddressDisplay* mNames;
    HPopupMenu*              mRolePopup;
    HPopupMenu*              mStatusPopup;
    JXTextCheckbox*          mRSVP;
    JXTextButton*            mCancelBtn;
    JXTextButton*            mOKBtn;

// end JXLayout
	bool				mITIPSent;
	bool				mNewItem;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	SetDetails(const iCal::CICalendarProperty& prop);		// Set the dialogs info
			void	GetDetails(iCal::CICalendarProperty& prop);		// Get the dialogs return info
			void	GetDetails(iCal::CICalendarPropertyList& proplist);		// Get the dialogs return info
};

#endif
