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


// CEditIdentityOptions.h : header file
//

#ifndef __CEDITIDENTITYOPTIONS__MULBERRY__
#define __CEDITIDENTITYOPTIONS__MULBERRY__

#include "CTabPanel.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions dialog

class CMailboxPopupButton;
class CPreferences;
class CTextInputField;
class JXImageButton;
class JXSecondaryRadioGroup;
class JXTextCheckbox;

class CEditIdentityOptions : public CTabPanel
{
public:
	CEditIdentityOptions(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

			void SetPrefs(const CPreferences* prefs)
				{ mCopyPrefs = prefs; }
protected:
// begin JXLayout1

    JXSecondaryRadioGroup* mCopyToGroup;
    JXTextCheckbox*        mCopyToActive;
    CTextInputField*       mCopyTo;
    JXTextCheckbox*        mAlsoCopyTo;
    JXTextCheckbox*        mHeaderActive;
    JXImageButton*         mHeaderBtn;
    JXTextCheckbox*        mFooterActive;
    JXImageButton*         mSignatureBtn;
    CMailboxPopupButton*   mCopyToPopup;

// end JXLayout1

	const CPreferences* mCopyPrefs;
	cdstring mHeader;
	cdstring mFooter;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnCopyToGroup(JIndex index);
			void OnCopyToPopup(JIndex nID);
			void OnHeaderBtn();
			void OnSignatureBtn();

			void OnActiveCopy(bool active);
			void OnActiveHeader(bool active);
			void OnActiveSignature(bool active);
};

#endif
