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


// CUserPswdDialog.h : header file
//

#ifndef __CUSERPSWDDIALOG__MULBERRY__
#define __CUSERPSWDDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog dialog

class CStaticText;
class CTextInputField;
class JXInputField;
template <class T> class CInputField;
class JXImageWidget;
class JXPasswordInput;
class cdstring;

class CUserPswdDialog : public CDialogDirector
{
// Construction
public:
	CUserPswdDialog(JXDirector* supervisor);

	static bool PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
								const cdstring& title, const cdstring& server_ip,
								const cdstring& match, bool secure, int icon);

protected:
	virtual void OnCreate();
	virtual void Continue();

// begin JXLayout

    JXImageWidget*                mIcon;
    CStaticText*                  mDescription;
    CStaticText*                  mServerName;
    CTextInputField*              mUserID;
    CInputField<JXPasswordInput>* mPassword;
    CStaticText*                  mCapsLock;
    CStaticText*                  mMethod;
    CStaticText*                  mSecure;

// end JXLayout
	bool						mCapsLockDown;
};

#endif
