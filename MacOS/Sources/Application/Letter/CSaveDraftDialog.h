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

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_SaveDraftDialog = 3001;
const	PaneIDT		paneid_SaveDraftFile = 'FILE';
const	PaneIDT		paneid_SaveDraftMailbox = 'MBOX';
const	PaneIDT		paneid_SaveDraftMailboxPopup = 'LIST';

// Messages
const	MessageT	msg_SaveDraftFile = 'FILE';
const	MessageT	msg_SaveDraftMailbox = 'MBOX';

// Resources
const	ResIDT		RidL_CSaveDraftDialogBtns = 3001;

// Classes
class CMailboxPopup;
class LRadioButton;

class CSaveDraftDialog : public LDialogBox
{
public:
	struct SSaveDraft
	{
		bool		mFile;
		cdstring	mMailboxName;
		
		SSaveDraft() { mFile = true; mMailboxName = "\1"; }
	};

	enum { class_ID = 'SaDr' };

					CSaveDraftDialog();
					CSaveDraftDialog(LStream *inStream);
	virtual 		~CSaveDraftDialog();

	static bool PoseDialog(SSaveDraft& details, bool allow_file);

	void	SetDetails(bool allow_file);			// Set the dialogs info
	bool	GetDetails();							// Set the dialogs return info

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	static SSaveDraft sDetails;

	LRadioButton*	mFile;
	LRadioButton*	mMailbox;
	CMailboxPopup*	mMailboxPopup;
};

#endif
