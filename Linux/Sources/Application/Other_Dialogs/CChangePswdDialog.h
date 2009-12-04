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


// Header for CChangePswdDialog class

#ifndef __CCHANGEPSWDDIALOG__MULBERRY__
#define __CCHANGEPSWDDIALOG__MULBERRY__

#include <LGADialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_ChangePswdDialog = 6003;
const	PaneIDT		paneid_ChangePswdUID = 'USER';
const	PaneIDT		paneid_ChangePswdOldPswd = 'OLDP';
const	PaneIDT		paneid_ChangePswdNewPswd1 = 'NEW1';
const	PaneIDT		paneid_ChangePswdNewPswd2 = 'NEW2';
const	PaneIDT		paneid_ChangePswdIcon = 'ICON';
const	PaneIDT		paneid_ChangePswdDescription = 'DNAM';
const	PaneIDT		paneid_ChangePswdServerName = 'SNAM';
const	PaneIDT		paneid_ChangePswdCapsLock = 'CAPS';

// Classes
class CINETAccount;
class LGAIconSuite;

class CChangePswdDialog : public LGADialogBox
{
private:
	LGAIconSuite*	mIcon;
	LPane*			mDescription;
	LPane*			mServerName;
	LPane*			mUserName;
	LPane*			mOldPassword;
	LPane*			mNew1Password;
	LPane*			mNew2Password;
	LPane*			mCapsLock;

public:
	enum { class_ID = 'ChPa' };

					CChangePswdDialog();
					CChangePswdDialog(LStream *inStream);
	virtual 		~CChangePswdDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	void	CapsLockChange(bool caps_lock_down); // Deal with change in caps lock
	void	SetDetails(const cdstring& uid,
									const cdstring& description,
									const cdstring& server,
									ResIDT icon); 			// Select the user or password text
	void	GetDetails(cdstring& old_pass, cdstring& new1_pass, cdstring& new2_pass); 					// Get details from dialog
	void	ResetNewPasswords();

	static bool		PromptPasswordChange(const CINETAccount* acct,
											cdstring& old_pass,
											cdstring& new_pass,
											LCommander* cmdr);
};

#endif
