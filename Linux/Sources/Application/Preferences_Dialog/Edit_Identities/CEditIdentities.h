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


// CEditIdentities.h : header file
//

#ifndef __CEDITIDENTITIES__MULBERRY__
#define __CEDITIDENTITIES__MULBERRY__

#include "CDialogDirector.h"

#include "CIdentity.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities dialog

class CPreferences;
class CTabController;
class CTextInputField;
class JXTextButton;
class JXTextCheckbox;

class CEditIdentities : public CDialogDirector
{
// Construction
public:
	CEditIdentities(JXDirector* supervisor);

	static bool PoseDialog(const CPreferences* prefs, CIdentity* id);

// Implementation
protected:
// begin JXLayout

    CTextInputField* mName;
    HPopupMenu*      mInheritPopup;
    HPopupMenu*      mAccountPopup;
    JXTextCheckbox*  mUseSMTPAccount;
    CTabController*  mTabs;
    JXTextButton*    mOKBtn;
    JXTextButton*    mCancelBtn;

// end JXLayout
	CIdentity			mCopyIdentity;
	const CPreferences*	mCopyPrefs;

	bool				mFromEnabled;
	bool				mReplyToEnabled;
	bool				mSenderEnabled;
	bool				mCalendarEnabled;

			void OnCreate();

	virtual void Receive(JBroadcaster* sender, const Message& message);


			void InitServerPopup();
			void InitInheritPopup();
			void SetIdentity(const CPreferences* prefs, const CIdentity* identity);
			void GetIdentity(CIdentity* identity);
};

#endif
