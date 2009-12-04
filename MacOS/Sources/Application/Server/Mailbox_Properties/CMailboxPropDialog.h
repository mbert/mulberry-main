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


// Header for CMailboxPropDialog class

#ifndef __CMAILBOXPROPDIALOG__MULBERRY__
#define __CMAILBOXPROPDIALOG__MULBERRY__

#include "CPropDialog.h"

// Constants

// Panes
const	PaneIDT		paneid_MailboxPropDialog = 4010;
const	PaneIDT		paneid_MailboxPropOptionsBtn = 'BTN1';
const	PaneIDT		paneid_MailboxPropGeneralBtn = 'BTN2';
const	PaneIDT		paneid_MailboxPropServerBtn = 'BTN3';
const	PaneIDT		paneid_MailboxPropACLBtn = 'BTN4';
const	PaneIDT		paneid_MailboxPropQuotaBtn = 'BTN5';

// Resources
const	ResIDT		RidL_CMailboxPropDialogBtns = 4010;

// Mesages
const	MessageT	msg_SetPropOptions = 'BTN1';
const	MessageT	msg_SetPropGeneral = 'BTN2';
const	MessageT	msg_SetPropServer = 'BTN3';
const	MessageT	msg_SetPropACL = 'BTN4';
const	MessageT	msg_SetPropQuota = 'BTN5';

// Classes
class CMboxList;
class CMboxProtocol;

class CMailboxPropDialog : public CPropDialog
{
public:
	enum { class_ID = 'BPrt' };

					CMailboxPropDialog();
					CMailboxPropDialog(LStream *inStream);
	virtual 		~CMailboxPropDialog();

	static bool PoseDialog(CMboxList* mbox_list)
		{ return PoseDialog(mbox_list, NULL); }
	static bool PoseDialog(CMboxProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto);

protected:
	virtual void	FinishCreateSelf(void);								// Do odds & ends

	virtual void	SetUpPanels(void);									// About to start dialog
	virtual void	DoPanelInit(void);									// About to display a panel

private:
	CMboxList*			mMboxList;										// List of selected mboxes
	CMboxProtocol*		mMboxProtocol;									// List of selected mboxes

	void	SetMboxList(CMboxList* mbox_list);							// Set mbox list
	void	SetProtocol(CMboxProtocol* proto);							// Set mbox protocol
};

#endif
