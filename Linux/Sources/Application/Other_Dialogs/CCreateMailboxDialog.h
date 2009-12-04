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

#include "CDialogDirector.h"

#include "CINETAccount.h"

#include "HPopupMenu.h"

#include "cdstring.h"

// Constants


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

class JXTextButton;
class CTextInputField;
class JXTextRadioButton;
class JXRadioGroup;
class JXTextCheckbox;

class	CCreateMailboxDialog : public CDialogDirector
{
public:
	enum {eFullPathname, eInHierarchy};

	CCreateMailboxDialog(JXDirector* supervisor);
	virtual ~CCreateMailboxDialog();

	static bool PoseDialog(SCreateMailbox& details);

protected:
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void	OnAccountPopup(JIndex index);

	void	SetDetails(SCreateMailbox* create);		// Set the dialogs info
	void	GetDetails(SCreateMailbox* results);	// Get the dialogs info

	void InitAccountMenu();
	void OnCreate();
// begin JXLayout

    CTextInputField*   mMailboxName;
    JXRadioGroup*      mWhichPath;
    JXTextRadioButton* mFullPathnameBtn;
    JXTextRadioButton* mInHierBtn;
    CTextInputField*   mInHierarchy;
    CTextInputField*   mAccount;
    HPopupMenu*        mAccountMenu;
    JXTextCheckbox*    mSubscribe;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;
    JXRadioGroup*      mDirectory;
    JXTextRadioButton* mCreateMailbox;
    JXTextRadioButton* mCreateDirectory;

// end JXLayout
	bool				mUseSubscribe;
};

#endif
