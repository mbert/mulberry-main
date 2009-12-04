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


// Header for CSaveDraftDialog class

#ifndef __CSAVEDRAFTDIALOG__MULBERRY__
#define __CSAVEDRAFTDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

// Classes
class CMailboxPopup;
class JXRadioGroup;
class JXTextButton;
class JXTextRadioButton;

class CSaveDraftDialog : public CDialogDirector
{
public:
	struct SSaveDraft
	{
		bool		mFile;
		cdstring	mMailboxName;
		
		SSaveDraft() { mFile = true; mMailboxName = "\1"; }
	};

	enum
	{
		eFileRadio = 1,
		eMailboxRadio
	};

					CSaveDraftDialog(JXDirector* supervisor);

	static bool PoseDialog(SSaveDraft& details, bool allow_file);

	void	SetDetails(bool allow_file);			// Set the dialogs info
	bool	GetDetails();							// Set the dialogs return info

protected:
// begin JXLayout

    JXRadioGroup*      mGroup;
    JXTextRadioButton* mFile;
    JXTextRadioButton* mMailbox;
    CMailboxPopup*     mMailboxPopup;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;

// end JXLayout

	static SSaveDraft sDetails;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
};

#endif
