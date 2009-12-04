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


// Header for CMultiUserDialog class

#ifndef __CMULTIUSERDIALOG__MULBERRY__
#define __CMULTIUSERDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	ClassIDT	class_MultiUserDialog ='MULT';
const	PaneIDT		paneid_MultiUserDialog = 5200;
const	PaneIDT		paneid_MultiUserRealNameDialog = 5201;
const	PaneIDT		paneid_MUOKBtn = 'OKBT';
const	PaneIDT		paneid_MUBox = 'BOXI';
const	PaneIDT		paneid_MUServerName = 'SNAM';
const	PaneIDT		paneid_MUServerPopup = 'SERP';
const	PaneIDT		paneid_MURealName = 'NAME';
const	PaneIDT		paneid_MURealNameLocked = 'NAMC';
const	PaneIDT		paneid_MUUserId = 'USID';
const	PaneIDT		paneid_MUUserIdLocked = 'USCP';
const	PaneIDT		paneid_MUUserIdCaption = 'UCAP';
const	PaneIDT		paneid_MUPassword = 'PSWD';
const	PaneIDT		paneid_MUPasswordCaption = 'PCAP';
const	PaneIDT		paneid_MUCapsLock = 'CAPS';
const	PaneIDT		paneid_MUAuthenticate = 'AUTH';
const	PaneIDT		paneid_MUConnection = 'CONN';
const	PaneIDT		paneid_MUConnectionCaption = 'CCAP';
const	PaneIDT		paneid_MUVersionNumber = 'VERS';
const	PaneIDT		paneid_MUSerialNumber = 'SERN';
const	PaneIDT		paneid_MULicensee = 'LICE';

// Messages
const	MessageT	msg_MUServerPopup = 'SERP';

// Resources
const	PaneIDT		RidL_CMultiUserDialogBtns = 5200;
const	ResIDT		STRx_MultiUserDialogHelp = 5200;

// Classes

class	CStaticText;
class	CTextFieldX;
class	LPushButton;
class	LPopupButton;

class	CMultiUserDialog : public LDialogBox
{
private:
	LPushButton*		mOKBtn;
	CTextFieldX*		mUserID;
	CStaticText*		mUserIDLocked;
	CTextFieldX*		mRealName;
	CStaticText*		mRealNameLocked;
	CTextFieldX*		mPassword;
	CStaticText*		mCapsLock;
	CStaticText*		mConnection;
	LPopupButton*		mServerPopup;
	bool				mUseUID;
	bool				mUsePswd;
	long				mServerIndex;
	cdstring			mMailServerName;
	cdstring			mSMTPServerName;
	cdstrvect			mMailServers;
	bool				mUseOther;

public:
	enum { class_ID = class_MultiUserDialog };

					CMultiUserDialog();
					CMultiUserDialog(LStream *inStream);
	virtual 		~CMultiUserDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
			void	UpdatePrefs(void);
	
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Check for valid OK

			void	CapsLockChange(bool caps_lock_down); // Deal with change in caps lock

private:
			void	InitServerField();
			void	InitServerMenu();

			void	UpdateConfigPrefs(void);
};

#endif
