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

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_UploadScriptDialog = 1230;
const	PaneIDT		paneid_UploadScriptFile = 'SAVE';
const	PaneIDT		paneid_UploadScriptServer = 'SERV';
const	PaneIDT		paneid_UploadScriptGroup = 'GRUP';
const	PaneIDT		paneid_UploadScriptAccountPopup = 'ACCP';
const	PaneIDT		paneid_UploadScriptUploadOnly = 'UPLD';
const	PaneIDT		paneid_UploadScriptUploadActvate = 'UPAC';
const	PaneIDT		paneid_UploadScriptDelete = 'DELE';

// Mesages
const	MessageT	msg_UploadScriptAccountPopup = 'ACCP';
const	PaneIDT		msg_UploadScriptFile = 'SAVE';
const	PaneIDT		msg_UploadScriptServer = 'SERV';

// Resources
const	ResIDT		RidL_CUploadScriptDialogBtns = 1230;

// Type
class	LPopupButton;
class	LRadioButton;
class	LTextGroupBox;

class	CUploadScriptDialog : public LDialogBox
{
private:
	LRadioButton*		mFile;
	LRadioButton*		mServer;
	LTextGroupBox*		mGroup;
	LPopupButton*		mAccountPopup;
	LRadioButton*		mUploadOnly;
	LRadioButton*		mUploadActivate;
	LRadioButton*		mDelete;

public:
	enum { class_ID = 'UpSc' };

	struct SUploadScript
	{
		bool			mFile;
		unsigned long	mAccountIndex;
		bool			mUpload;
		bool			mActivate;
	};

					CUploadScriptDialog();
					CUploadScriptDialog(LStream *inStream);
	virtual 		~CUploadScriptDialog();

	static bool PoseDialog(SUploadScript& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void SetDetails(SUploadScript& create);
	void GetDetails(SUploadScript& result);
	
private:
	static SUploadScript sLastChoice;

			void InitAccountMenu();
};

#endif
