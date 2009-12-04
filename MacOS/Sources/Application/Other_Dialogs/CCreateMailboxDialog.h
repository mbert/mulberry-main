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


// Header for CCreateMailboxDialog class

#ifndef __CCREATEMAILBOXDIALOG__MULBERRY__
#define __CCREATEMAILBOXDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CINETAccount.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_CreateMailboxDialog = 7001;
const	PaneIDT		paneid_CrMailbox = 'MBOX';
const	PaneIDT		paneid_CrDirectory = 'MDIR';
const	PaneIDT		paneid_CrName = 'NAME';
const	PaneIDT		paneid_CrSubscribe = 'SUBS';
const	PaneIDT		paneid_CrFullPath = 'FULL';
const	PaneIDT		paneid_CrUseDirectory = 'CDIR';
const	PaneIDT		paneid_CrHierarchy = 'HIER';
const	PaneIDT		paneid_CrAccountBox = 'ACCB';
const	PaneIDT		paneid_CrAccount = 'ACCT';
const	PaneIDT		paneid_CrAccountPopup = 'ACCP';

// Mesages
const	MessageT	msg_SetFullPath = 7000;
const	MessageT	msg_SetUseDirectory = 7001;
const	MessageT	msg_CrAccountPopup = 'ACCP';

// Resources
const	ResIDT		RidL_CCreateMailboxDialogBtns = 7001;
const	ResIDT		STRx_CreateMailboxDialogHelp = 7001;

// Type
struct SCreateMailbox
{
	cdstring	new_name;
	cdstring	parent;
	cdstring	account;
	CINETAccount::EINETServerType	account_type;
	bool		use_wd;
	bool		subscribe;
	bool		directory;
};

class	CTextFieldX;
class	CStaticText;
class	LCheckBox;
class	LPopupButton;
class	LRadioButton;

class	CCreateMailboxDialog : public LDialogBox {

private:
	LRadioButton*		mMailbox;
	LRadioButton*		mDirectory;
	CTextFieldX*		mMailboxName;
	LCheckBox*			mSubscribe;
	LRadioButton*		mFullPath;
	LRadioButton*		mUseDirectory;
	CStaticText*		mHierarchy;
	LView*				mAccountBox;
	CStaticText*		mAccount;
	LPopupButton*		mAccountPopup;
	bool				mUseSubscribe;

public:
	enum { class_ID = 'CrMa' };

					CCreateMailboxDialog();
					CCreateMailboxDialog(LStream *inStream);
	virtual 		~CCreateMailboxDialog();

	static bool PoseDialog(SCreateMailbox& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

			void	OnChangeAccount(long index);

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
			void	SetDetails(SCreateMailbox* create);		// Set the dialogs info
			void	GetDetails(SCreateMailbox* result);		// Get the dialogs return info
	
private:
			void InitAccountMenu(void);
};

#endif
