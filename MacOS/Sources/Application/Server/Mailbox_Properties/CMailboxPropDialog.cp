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


// Source for CMailboxPropDialog class

#include "CMailboxPropDialog.h"

#include "CBalloonDialog.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPropMailboxACL.h"
#include "CPropMailboxGeneral.h"
#include "CPropMailboxOptions.h"
#include "CPropMailboxQuota.h"
#include "CPropMailboxServer.h"

#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P R O P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxPropDialog::CMailboxPropDialog()
{
}

// Constructor from stream
CMailboxPropDialog::CMailboxPropDialog(LStream *inStream)
		: CPropDialog(inStream)
{
}

// Default destructor
CMailboxPropDialog::~CMailboxPropDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxPropDialog::FinishCreateSelf(void)
{
	// Do inherited
	CPropDialog::FinishCreateSelf();
}

// Set up panels
void CMailboxPropDialog::SetUpPanels(void)
{
	// Panel list
	mPanelList.push_back(paneid_PropMailboxOptions);
	mPanelList.push_back(paneid_PropMailboxGeneral);
	mPanelList.push_back(paneid_PropMailboxServer);
	mPanelList.push_back(paneid_PropMailboxACL);
	mPanelList.push_back(paneid_PropMailboxQuota);

	// Message list
	mMsgPanelList.push_back(msg_SetPropOptions);
	mMsgPanelList.push_back(msg_SetPropGeneral);
	mMsgPanelList.push_back(msg_SetPropServer);
	mMsgPanelList.push_back(msg_SetPropACL);
	mMsgPanelList.push_back(msg_SetPropQuota);

	// Ridl for buttons
	mRidl = RidL_CMailboxPropDialogBtns;
}

// About to show panel
void CMailboxPropDialog::DoPanelInit(void)
{
	if (mMboxList)
		((CMailboxPropPanel*) mCurrentPanel)->SetMboxList(mMboxList);
	if (mMboxProtocol)
		((CMailboxPropPanel*) mCurrentPanel)->SetProtocol(mMboxProtocol);
}

// Set input panel
void CMailboxPropDialog::SetMboxList(CMboxList* mbox_list)
{
	mMboxList = mbox_list;
	mMboxProtocol = ((CMbox*) mbox_list->front())->GetProtocol();

	// Load string title
	cdstring title;
	title.FromResource((mMboxList->size() == 1 ? "UI::MailboxProp::Title" : "UI::MailboxProp::MultiTitle"));

	// Add name if single mailbox
	if (mMboxList->size() == 1)
	{
		title += mMboxList->front()->GetName();
	}

	// Now set window title
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// See if only directories present
	bool only_dirs = true;
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		
		only_dirs &= mbox->IsDirectory();
	}

	// Disable unwanted prefs panels
	bool logged_in = mMboxProtocol->IsLoggedOn();
	if (only_dirs || !logged_in || !mMboxProtocol->UsingACLs())
	{
		LPane* btn = FindPaneByID(paneid_MailboxPropACLBtn);
		btn->Disable();
	}
	if (only_dirs || !logged_in || !mMboxProtocol->UsingQuotas())
	{
		LPane* btn = FindPaneByID(paneid_MailboxPropQuotaBtn);
		btn->Disable();
	}

	// Initial panel
	long set = 1;
	ListenToMessage(msg_SetPropOptions, &set);
}

// Set input panel
void CMailboxPropDialog::SetProtocol(CMboxProtocol* proto)
{
	mMboxList = NULL;
	mMboxProtocol = proto;

	// Load string title
	cdstring title;
	title.FromResource("UI::MailboxProp::ServerTitle");

	// Add name if single mailbox
	title += proto->GetAccountName();

	// Now set window title
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// Disable unwanted prefs panels
	LPane* btn = FindPaneByID(paneid_MailboxPropOptionsBtn);
	btn->Disable();
	btn = FindPaneByID(paneid_MailboxPropGeneralBtn);
	btn->Disable();
	btn = FindPaneByID(paneid_MailboxPropACLBtn);
	btn->Disable();
	btn = FindPaneByID(paneid_MailboxPropQuotaBtn);
	btn->Disable();

	// Initial panel
	btn = FindPaneByID(paneid_MailboxPropServerBtn);
	btn->SetValue(1);
}

bool CMailboxPropDialog::PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto)
{
	CBalloonDialog	dlog(paneid_MailboxPropDialog, CMulberryApp::sApp);
	CMailboxPropDialog* prop_dlog = (CMailboxPropDialog*) dlog.GetDialog();

	if (mbox_list)
		prop_dlog->SetMboxList(mbox_list);
	else if (proto)
		prop_dlog->SetProtocol(proto);

	// Let DialogHandler process events
	dlog.DoModal();

	return true;
}