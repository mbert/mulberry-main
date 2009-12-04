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

#include "CDialogDirector.h"

#include "HPopupMenu.h"

#include "cdstring.h"

// Constants

// Type
class CTextInputField;
class JXRadioGroup;
class JXStaticText;
class JXTextButton;
class JXTextCheckbox;
class JXTextRadioButton;

class CCreateAdbkDialog : public CDialogDirector
{
public:
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

			CCreateAdbkDialog(JXDirector* supervisor);
	virtual ~CCreateAdbkDialog();

	static bool PoseDialog(SCreateAdbk* details);

	enum {eFullPathname, eInHierarchy};

protected:
// begin JXLayout

    JXRadioGroup*      mDirectoryGroup;
    JXTextRadioButton* mMkAdbk;
    JXTextRadioButton* mMkCol;
    CTextInputField*   mAdbkName;
    JXRadioGroup*      mWhichPath;
    JXTextRadioButton* mFullPath;
    JXTextRadioButton* mUseDirectory;
    CTextInputField*   mInHierarchy;
    CTextInputField*   mAccount;
    HPopupMenu*        mAccountPopup;
    JXTextCheckbox*    mOpenOnStartup;
    JXTextCheckbox*    mUseNicknames;
    JXTextCheckbox*    mUseSearch;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;

// end JXLayout

	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual void OnCreate(void);					// Do odds & ends

	void SetDetails(SCreateAdbk* create);
	void GetDetails(SCreateAdbk* result);
	
private:
			void InitAccountMenu(void);
};

#endif
