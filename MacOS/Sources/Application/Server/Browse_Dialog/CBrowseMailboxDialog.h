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

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_BrowseMailboxDialog = 7002;
const	PaneIDT		paneid_BrowseCopyMessageDialog = 7003;
const	PaneIDT		paneid_BrMovePane = 'MOVE';
const	PaneIDT		paneid_BrTable = 'LIST';
const	PaneIDT		paneid_BrOKBtn = 'OKBT';
const	PaneIDT		paneid_BrNoCopyBtn = 'NONE';
const	PaneIDT		paneid_BrCreateBtn = 'NEWM';
const	PaneIDT		paneid_BrSaveDefaultBtn = 'DEFT';

// Resources
const	ResIDT		RidL_CBrowseMailboxDialogBtns = 7002;
const	ResIDT		RidL_CBrowseCopyDraftDialogBtns = 7003;

// Message
const	MessageT	msg_BrNoCopy = 'NONE';
const	MessageT	msg_BrCreate = 'NEWM';

// Type
class CMbox;
class CMboxList;
class CServerBrowseTable;

class	CBrowseMailboxDialog : public LDialogBox
{
public:
	enum { class_ID = 'BrMa' };

					CBrowseMailboxDialog();
					CBrowseMailboxDialog(LStream *inStream);
	virtual 		~CBrowseMailboxDialog();

	static bool	PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default);
	static bool	PoseDialog(CMboxList& mbox_list);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	SetOpenMode(bool mode);
	void	SetSending(bool sending);
	void	SetMultipleSelection();						// Turn on multiple selection
	CMbox*	GetSelectedMbox();							// Get selected mbox
	void	GetSelectedMboxes(CMboxList& mbox_list);	// Get all selected mboxes
	bool	GetSetAsDefault();							// Get set as default checkbox state

private:
	CServerBrowseTable*		mTable;

	virtual void	ResetState(void);					// Reset state from prefs
	virtual void	SaveState(void);					// Save state as default
};

#endif
