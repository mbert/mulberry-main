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


// Header for CBrowseMailboxDialog class

#ifndef __CBROWSEMAILBOXDIALOG__MULBERRY__
#define __CBROWSEMAILBOXDIALOG__MULBERRY__

#include "CDialogDirector.h"

// Type
class CMbox;
class CMboxList;
class CServerBrowseTable;
class JXTextButton;
class JXTextCheckbox;

class	CBrowseMailboxDialog : public CDialogDirector
{
public:
			CBrowseMailboxDialog(JXDirector* supervisor);
	virtual ~CBrowseMailboxDialog();

	static bool	PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default);
	static bool	PoseDialog(CMboxList& mbox_list);

protected:
	bool					mSending;
	CServerBrowseTable*		mTable;

	void	SetSending(bool sending);
	void	SetOpenMode(bool mode);
	void	SetMultipleSelection();						// Turn on multiple selection
	CMbox*	GetSelectedMbox();							// Get selected mbox
	void	GetSelectedMboxes(CMboxList& mbox_list);	// Get all selected mboxes
// begin JXLayout


// end JXLayout

/* This one is not used
// begin JXLayout1

    JXTextButton* mOKBtn;
    JXTextButton* mCancelBtn;
    JXTextButton* mCreateBtn;

// end JXLayout1
*/

// begin JXLayout2

    JXTextButton*   mOKBtn;
    JXTextButton*   mCancelBtn;
    JXTextButton*   mCreateBtn;
    JXTextButton*   mNoneBtn;
    JXTextCheckbox* mSetAsDefault;

// end JXLayout2

	virtual void OnCreate();					// Do odds & ends
	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
	virtual void	ResetState(void);						// Reset state from prefs
	virtual void	SaveState(void);						// Save state in prefs

};

#endif
