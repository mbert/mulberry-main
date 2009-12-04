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


// Header for CPrefsLetterGeneral class

#ifndef __CPREFSLETTERGENERAL__MULBERRY__
#define __CPREFSLETTERGENERAL__MULBERRY__

#include "CTabPanel.h"

#include "cdstring.h"

// Classes
class CPreferences;
template<class T> class CPreferenceValueMap;
class CMailboxPopupButton;
class CTextInputField;
class JXIntegerInput;
class JXSecondaryRadioGroup;
class JXTextButton;
class JXTextCheckbox;
class JXTextRadioButton;
template <class T> class CInputField;

class CPrefsLetterGeneral : public CTabPanel
{
public:
	CPrefsLetterGeneral(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    CTextInputField*             mMailDomain;
    CInputField<JXIntegerInput>* mSpacesPerTab;
    JXTextCheckbox*              mTabSpace;
    CInputField<JXIntegerInput>* mWrapLength;
    CTextInputField*             mReplyQuote;
    JXTextButton*                mReplySetBtn;
    CTextInputField*             mForwardQuote;
    JXTextButton*                mForwardSetBtn;
    CTextInputField*             mForwardSubject;
    JXTextButton*                mPrintSetBtn;
    JXTextCheckbox*              mSeparateBCC;
    JXTextButton*                mBCCCaption;
    JXSecondaryRadioGroup*       mSaveGroup;
    JXTextRadioButton*           mSaveToFile;
    JXTextRadioButton*           mSaveToMailbox;
    CTextInputField*             mSaveToMailboxName;
    CMailboxPopupButton*         mSaveToMailboxPopup;
    JXTextRadioButton*           mSaveChoose;
    JXTextCheckbox*              mAutoSaveDrafts;
    CInputField<JXIntegerInput>* mAutoSaveDraftsInterval;

// end JXLayout1
	CPreferences*	mCopyPrefs;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void EditCaption(CPreferenceValueMap<cdstring>* text1,
								CPreferenceValueMap<cdstring>* text2,
								CPreferenceValueMap<cdstring>* text3,
								CPreferenceValueMap<cdstring>* text4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* box1,
								CPreferenceValueMap<bool>* box2,
								CPreferenceValueMap<bool>* summary);	// Edit a caption
			void EditBCCCaption();

			void OnChangeSaveTo(JIndex nID);				// Change save to menu
};

#endif
