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


// Header for CCreateCalendarDialog class

#ifndef __CCREATECALENDARDIALOG__MULBERRY__
#define __CCREATECALENDARDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CINETAccount.h"

#include "cdstring.h"

// Constants

// Type
class	CStaticText;
class	LCheckBox;
class	LPopupButton;
class	LRadioButton;
class	CTextFieldX;

class CCreateCalendarDialog : public LDialogBox
{
public:
	enum { class_ID = 'CrCa', pane_ID = 1821 };

	struct SCreateCalendar
	{
		cdstring	new_name;
		cdstring	parent;
		cdstring	account;
		CINETAccount::EINETServerType	account_type;
		bool		use_wd;
		bool		subscribe;
		bool		directory;
	};

	static bool		PoseDialog(SCreateCalendar& details);

					CCreateCalendarDialog(LStream *inStream);
	virtual 		~CCreateCalendarDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	
protected:
	enum
	{
		eCalendar_ID = 'CALS',
		eDirectory_ID = 'CADR',
		eName_ID = 'NAME',
		eSubscribe_ID = 'SUBS',
		eFullPath_ID = 'FULL',
		eUseDirectory_ID = 'CDIR',
		eHierarchy_ID = 'HIER',
		eAccountBox_ID = 'ACCB',
		eAccount_ID = 'ACCT',
		eAccountPopup_ID = 'ACCP'
	};

	enum
	{
		msg_Calendar = 'CALS',
		msg_Directory = 'CADR',
		msg_SetFullPath = 7000,
		msg_SetUseDirectory = 7001
	};

	// UI Objects
	LRadioButton*		mCalendar;
	LRadioButton*		mDirectory;
	CTextFieldX*		mCalendarName;
	LCheckBox*			mSubscribe;
	LRadioButton*		mFullPath;
	LRadioButton*		mUseDirectory;
	CStaticText*		mHierarchy;
	LView*				mAccountBox;
	CStaticText*		mAccount;
	LPopupButton*		mAccountPopup;
	bool				mUseSubscribe;

	bool				mHasLocal;
	bool				mHasRemote;

	virtual void FinishCreateSelf(void);					// Do odds & ends
			void InitAccountMenu(void);

			void	SetDetails(SCreateCalendar* create);		// Set the dialogs info
			void	GetDetails(SCreateCalendar* result);		// Get the dialogs return info
};

#endif
