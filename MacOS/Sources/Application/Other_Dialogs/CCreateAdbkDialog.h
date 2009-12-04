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


// Header for CCreateAdbkDialog class

#ifndef __CCREATEADBKDIALOG__MULBERRY__
#define __CCREATEADBKDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_CreateAdbkDialog = 9513;
const	PaneIDT		paneid_CrMkAdbk = 'CALS';
const	PaneIDT		paneid_CrMkCol = 'CADR';
const	PaneIDT		paneid_CrAdbk = 'NAME';
const	PaneIDT		paneid_CrAdbkType = 'TYPE';
const	PaneIDT		paneid_CrAdbkFullPath = 'FULL';
const	PaneIDT		paneid_CrAdbkUseDirectory = 'CDIR';
const	PaneIDT		paneid_CrAdbkHierarchy = 'HIER';
const	PaneIDT		paneid_CrAdbkMover = 'MOVE';
const	PaneIDT		paneid_CrAdbkAccountBox = 'ACCB';
const	PaneIDT		paneid_CrAdbkAccount = 'ACCT';
const	PaneIDT		paneid_CrAdbkAccountPopup = 'ACCP';
const	PaneIDT		paneid_CrAdbkOpenOnStartup = 'OPEN';
const	PaneIDT		paneid_CrAdbkUseNicknames = 'NICK';
const	PaneIDT		paneid_CrAdbkUseSearch = 'SRCH';

// Mesages
const	MessageT	msg_CrAdbkAccountPopup = 'ACCP';
const	MessageT	msg_CrAdbkMkAdbk = 'CALS';
const	MessageT	msg_CrAdbkMkCol = 'CADR';
const	MessageT	msg_CrAdbkSetFullPath = 7000;
const	MessageT	msg_CrAdbkSetUseDirectory = 7001;

// Resources
const	ResIDT		RidL_CCreateAdbkDialogBtns = 9513;

// Type
class	CTextFieldX;
class	LCheckBox;
class	LPopupButton;
class	LRadioButton;
class	CStaticText;

class	CCreateAdbkDialog : public LDialogBox
{
private:
	LRadioButton*		mMkAdbk;
	LRadioButton*		mMkCol;
	CTextFieldX*		mAdbkName;
	LRadioButton*		mFullPath;
	LRadioButton*		mUseDirectory;
	CStaticText*		mHierarchy;
	LView*				mMover;
	LView*				mAccountBox;
	CStaticText*		mAccount;
	LPopupButton*		mAccountPopup;
	LCheckBox*			mOpenOnStartup;
	LCheckBox*			mUseNicknames;
	LCheckBox*			mUseSearch;

public:
	enum { class_ID = 'CrAb' };

	struct SCreateAdbk
	{
		cdstring	name;
		cdstring	parent;
		cdstring	account;
		bool		directory;
		bool		use_wd;
		bool		open_on_startup;
		bool		use_nicknames;
		bool		use_search;
	};

					CCreateAdbkDialog();
					CCreateAdbkDialog(LStream *inStream);
	virtual 		~CCreateAdbkDialog();

	static bool PoseDialog(SCreateAdbk* details);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void SetDetails(SCreateAdbk* create);
	void GetDetails(SCreateAdbk* result);
	
private:
			void InitAccountMenu(void);
};

#endif
