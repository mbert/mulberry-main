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


// Header for CPrefsMailboxBasic class

#ifndef __CPREFSMAILBOXBASIC__MULBERRY__
#define __CPREFSMAILBOXBASIC__MULBERRY__

#include "CTabPanel.h"

// Classes
class CTextInputField;
class CMailboxPopupButton;
class JXSecondaryRadioGroup;
class JXTextCheckbox;

class CPrefsMailboxBasic : public CTabPanel
{
public:
	CPrefsMailboxBasic(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    JXSecondaryRadioGroup* mOpenGroup;
    JXSecondaryRadioGroup* mProcessOrder;
    JXTextCheckbox*        mExpungeOnClose;
    JXTextCheckbox*        mWarnOnExpunge;
    JXTextCheckbox*        mMoveFromINBOX;
    CTextInputField*       mClearMailbox;
    JXTextCheckbox*        mWarnOnClear;
    CMailboxPopupButton*   mClearMailboxPopup;
    JXTextCheckbox*        mNoOpenPreview;
    JXTextCheckbox*        mWarnPuntUnseen;
    JXTextCheckbox*        mDoRollover;
    JXTextCheckbox*        mRolloverWarn;
    JXTextCheckbox*        mScrollForUnseen;

// end JXLayout1

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnMoveRead();
			void OnChangeMoveTo(JIndex nID);				// Change move to menu
};

#endif
