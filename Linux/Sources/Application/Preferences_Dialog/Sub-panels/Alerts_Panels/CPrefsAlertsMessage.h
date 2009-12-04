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


// Header for CPrefsAlertsMessage class

#ifndef __CPREFSALERTSMESSAGE__MULBERRY__
#define __CPREFSALERTSMESSAGE__MULBERRY__

#include "CTabPanel.h"

#include "HPopupMenu.h"

// Classes
class JXInputField;
class JXIntegerInput;
class JXSecondaryRadioGroup;
class JXTextCheckbox;
template<class T> class CInputField;
class CPreferences;
class CTextInputField;

class CPrefsAlertsMessage : public CTabPanel
{
	enum
	{
		eStyleNew = 1,
		eStyleRename,
		eStyleDelete,
		eStyleFirst
	};

public:
	CPrefsAlertsMessage(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    HPopupMenu*                  mAlertStylePopup;
    JXSecondaryRadioGroup*       mCheckGroup;
    CInputField<JXIntegerInput>* mCheckInterval;
    JXTextCheckbox*              mNewMailAlert;
    JXTextCheckbox*              mNewMailBackAlert;
    JXTextCheckbox*              mNewMailFlashIcon;
    JXTextCheckbox*              mNewMailPlaySound;
    CTextInputField*             mNewMailSound;
    JXTextCheckbox*              mNewMailOpen;
    JXSecondaryRadioGroup*       mContainsGroup;
    HPopupMenu*                  mApplyToCabinet;

// end JXLayout1
	CPreferences*	mCopyPrefs;
	int				mStyleValue;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	InitStyleMenu();			// Set up styles menu
			void	InitCabinetMenu();			// Set up cabinet menu
			void	DoNewStyle();				// Add new account
			void	DoRenameStyle();			// Rename account
			void	DoDeleteStyle();			// Delete accoount
			void	SetNotifaction();			// Set account details
			void	UpdateNotifaction();		// Update current account
};

#endif
