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


// Header for CUploadScriptDialog class

#ifndef __CUPLOADSCRIPTDIALOG__MULBERRY__
#define __CUPLOADSCRIPTDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CINETAccount.h"
#include "cdstring.h"

#include "HPopupMenu.h"

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
class JXTextRadioButton;
class JXRadioGroup;

class CUploadScriptDialog : public CDialogDirector
{
public:
	struct SUploadScript
	{
		bool			mFile;
		unsigned long	mAccountIndex;
		bool			mUpload;
		bool			mActivate;
	};

	enum {eSaveFile, eSaveServer};
	enum {eUploadOnly, eUploadActivate, eDelete};

	CUploadScriptDialog(JXDirector* supervisor);
	virtual ~CUploadScriptDialog();

	static bool PoseDialog(SUploadScript& details);

protected:
	static SUploadScript sLastChoice;

// begin JXLayout

    JXRadioGroup*      mSaveGroup;
    JXTextRadioButton* mFileBtn;
    JXRadioGroup*      mUploadGroup;
    JXTextRadioButton* mServerBtn;
    HPopupMenu*        mAccountPopup;
    JXTextRadioButton* mUploadOnlyBtn;
    JXTextRadioButton* mUploadActivateBtn;
    JXTextRadioButton* mDeleteBtn;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;

// end JXLayout

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void SetDetails(SUploadScript& create);
	void GetDetails(SUploadScript& result);

	void InitAccountMenu();
};

#endif
