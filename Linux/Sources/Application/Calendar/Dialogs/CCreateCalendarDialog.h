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

#include "CDialogDirector.h"

#include "CINETAccount.h"

#include "HPopupMenu.h"

#include "cdstring.h"

// Type
class JXTextButton;
class CTextInputField;
class JXDownRect;
class JXInputField;
class JXTextRadioButton;
class JXRadioGroup;
class JXTextCheckbox;

class CCreateCalendarDialog : public CDialogDirector
{
public:
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

	enum {eFullPathname, eInHierarchy};

	CCreateCalendarDialog(JXDirector* supervisor);
	virtual ~CCreateCalendarDialog();

protected:

	// UI Objects
// begin JXLayout

    JXRadioGroup*      mDirectoryGroup;
    JXTextRadioButton* mCalendar;
    JXTextRadioButton* mDirectory;
    CTextInputField*   mCalendarName;
    JXRadioGroup*      mWhichPath;
    JXTextRadioButton* mFullPath;
    JXTextRadioButton* mUseDirectory;
    CTextInputField*   mInHierarchy;
    CTextInputField*   mAccount;
    HPopupMenu*        mAccountPopup;
    JXTextCheckbox*    mSubscribe;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mOKBtn;

// end JXLayout
	bool				mUseSubscribe;

	bool				mHasLocal;
	bool				mHasRemote;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void InitAccountMenu(void);

			void	SetDetails(SCreateCalendar* create);		// Set the dialogs info
			void	GetDetails(SCreateCalendar* result);		// Get the dialogs return info
};

#endif
