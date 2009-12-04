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


// Header for CPropCalendarOptions class

#ifndef __CPropCalendarOptions__MULBERRY__
#define __CPropCalendarOptions__MULBERRY__

#include "CCalendarPropPanel.h"

#include "HPopupMenu.h"

#include "cdstring.h"

// Classes
class CIdentityPopup;
class JXImageWidget;
class JXTextButton;
class JXTextCheckbox3;

class CPropCalendarOptions : public CCalendarPropPanel
{
public:
		CPropCalendarOptions(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CCalendarPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	ApplyChanges(void);											// Force update of calendars

protected:
// begin JXLayout1

    JXImageWidget*   mIconState;
    JXTextCheckbox3* mSubscribe;
    JXTextCheckbox3* mFreeBusySet;
    JXTextCheckbox3* mTieIdentity;
    CIdentityPopup*  mIdentityPopup;

// end JXLayout1
	cdstring		mCurrentIdentity;

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	
private:
			void	SetTiedIdentity(bool add);				// Add/remove tied identity
};

#endif
