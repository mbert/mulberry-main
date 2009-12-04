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


// Header for CUserPswdDialog class

#ifndef __CUSERPSWDDIALOG__MULBERRY__
#define __CUSERPSWDDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	ClassIDT	class_UserPswdDialog ='UsPw';
const	PaneIDT		paneid_UserPswdDialog = 6000;
const	PaneIDT		paneid_UserIDEdit = 'USID';
const	PaneIDT		paneid_UserIDCaption = 'USCP';
const	PaneIDT		paneid_PswdEdit = 'PSWD';
const	PaneIDT		paneid_Icon = 'ICON';
const	PaneIDT		paneid_Description = 'DNAM';
const	PaneIDT		paneid_ServerName = 'SNAM';
const	PaneIDT		paneid_Method = 'METH';
const	PaneIDT		paneid_Connection = 'CONN';
const	PaneIDT		paneid_CapsLock = 'CAPS';

// Resources
const	ResIDT		STRx_UserPswdDialogHelp = 6000;

// Classes

class	LIconControl;
class	CStaticText;
class	CTextFieldX;

class	CUserPswdDialog : public LDialogBox
{
private:
	LIconControl*	mIcon;
	CStaticText*	mDescription;
	CStaticText*	mServerName;
	CTextFieldX*	mUserTextEdit;
	CStaticText*	mUserTextCaption;
	CTextFieldX*	mPswdTextEdit;
	CStaticText*	mMethod;
	CStaticText*	mConnection;
	CStaticText*	mCapsLock;

public:
	enum { class_ID = class_UserPswdDialog };

					CUserPswdDialog();
					CUserPswdDialog(LStream *inStream);
	virtual 		~CUserPswdDialog();

	static bool PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
								const cdstring& title, const cdstring& server_ip, const cdstring& method,
								bool secure, int icon);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	CapsLockChange(bool caps_lock_down); // Deal with change in caps lock
	virtual void	SetUserPass(const cdstring& uid, const cdstring& pswd,
									bool lock_user, bool lock_pswd,
									const cdstring& description, const cdstring& server,
									const cdstring& method, bool secure, ResIDT icon); 			// Select the user or password text
	virtual void	GetUserPass(cdstring& uid, cdstring& pswd); 					// Get details from dialog

};

#endif
