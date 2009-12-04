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

	/*
 * Copyright (c) 2006 Cyrus Daboo.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are not permitted
 * without specific prior written permission from Cyrus Daboo.
 * This software is provided "as is" without express or implied warranty.
 */

// CMultiUserDialog.h : header file
//

#ifndef __CMULTIUSERDIALOG__MULBERRY__
#define __CMULTIUSERDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiUserDialog dialog

class JXImageWidget;
class JXInputField;
template <class T> class CInputField;
class JXPasswordInput;
class JXStaticText;
class JXTextButton;

class CMultiUserDialog : public CDialogDirector
{
// Construction
public:
	CMultiUserDialog(JXDirector* supervisor);   // standard constructor

	static bool PoseDialog();

protected:
	// These ones are ignored
#if 0
// begin JXLayout

    JXImageWidget*                mLogo;
    JXStaticText*                 mVersion;
    JXStaticText*                 mServerTxt;
    HPopupMenu*                   mServerPopup;
    JXStaticText*                 mUserIDCaption;
    CInputField<JXInputField>*    mUserID;
    JXStaticText*                 mPasswordCaption;
    CInputField<JXPasswordInput>* mPassword;
    JXStaticText*                 mCapsLock;
    JXStaticText*                 mAuth;
    JXTextButton*                 mOKBtn;
    JXTextButton*                 mCancelBtn;
    JXStaticText*                 mSSLStateCaption;
    JXStaticText*                 mSSLState;

// end JXLayout
#endif

// begin JXLayout2

    JXImageWidget*                mLogo;
    JXStaticText*                 mVersion;
    JXStaticText*                 mServerTxt;
    HPopupMenu*                   mServerPopup;
    CInputField<JXInputField>*    mRealName;
    JXStaticText*                 mUserIDCaption;
    CInputField<JXInputField>*    mUserID;
    JXStaticText*                 mPasswordCaption;
    CInputField<JXPasswordInput>* mPassword;
    JXStaticText*                 mCapsLock;
    JXStaticText*                 mAuth;
    JXTextButton*                 mOKBtn;
    JXTextButton*                 mCancelBtn;
    JXStaticText*                 mSSLStateCaption;
    JXStaticText*                 mSSLState;

// end JXLayout2
	bool						mCapsLockDown;

	bool			mUseUIDPswd;
	unsigned long	mServerIndex;
	cdstring		mMailServerName;
	cdstring		mSMTPServerName;
	cdstrvect		mMailServers;
	bool			mUseOther;
	bool			mUseRealName;

	virtual void OnCreate();
	virtual void Continue();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void OnServerPopup(JIndex nID);

private:
			void	InitFields();
			void	InitServerField();
			void	InitServerMenu();

			void	UpdatePrefs(void);
			void	UpdateConfigPrefs(void);
};

#endif
