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

#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxProtocol.h"
//#include "CMulberryApp.h"
//#include "CMulberryCommon.h"
#include "CPropMailboxACL.h"
#include "CPropMailboxGeneral.h"
#include "CPropMailboxOptions.h"
#include "CPropMailboxQuota.h"
#include "CPropMailboxServer.h"

#include "JXIconTextButton.h"

#include <JXCardFile.h>
#include <JXColormap.h>
#include <JXEngravedRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxPropDialog::CMailboxPropDialog(JXDirector* supervisor)
	: CPropDialog(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxPropDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 560,370, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 560,370);
    assert( obj1 != NULL );

    JXRadioGroup* rgroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 80,130);
    assert( rgroup != NULL );
    rgroup->SetBorderWidth(0);

    mOptionsBtn =
        new JXIconTextButton(1, "Options", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 80,25);
    assert( mOptionsBtn != NULL );

    mGeneralBtn =
        new JXIconTextButton(2, "General", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 80,25);
    assert( mGeneralBtn != NULL );

    mServerBtn =
        new JXIconTextButton(3, "Server", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,50, 80,25);
    assert( mServerBtn != NULL );

    mAccessBtn =
        new JXIconTextButton(4, "Access", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,75, 80,25);
    assert( mAccessBtn != NULL );

    mQuotaBtn =
        new JXIconTextButton(5, "Quota", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,100, 80,25);
    assert( mQuotaBtn != NULL );

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,5, 460,320);
    assert( obj2 != NULL );

    mCards =
        new JXCardFile(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 450,310);
    assert( mCards != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 466,331, 80,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Mailbox Properties");
	SetButtons(mOKBtn, NULL);

	// Set icons
	mOptionsBtn->SetImage(IDI_MAILBOXPROP_OPTIONS);
	mGeneralBtn->SetImage(IDI_MAILBOXPROP_GENERAL);
	mServerBtn->SetImage(IDI_MAILBOXPROP_SERVER);
	mAccessBtn->SetImage(IDI_MAILBOXPROP_ACL);
	mQuotaBtn->SetImage(IDI_MAILBOXPROP_QUOTA);
	ListenTo(rgroup);

	mPanelContainer = mCards;

	// Do inherited
	CPropDialog::OnCreate();
}

// Handle buttons
void CMailboxPropDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex item = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		SetPanel(item);
	}

	CPropDialog::Receive(sender, message);
}

// Set up panels
void CMailboxPropDialog::SetUpPanels(void)
{
	CMailboxPropPanel* panel = new CPropMailboxOptions(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropMailboxGeneral(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropMailboxServer(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropMailboxACL(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropMailboxQuota(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);
}

// About to show panel
void CMailboxPropDialog::DoPanelInit(void)
{
}

// Set input panel
void CMailboxPropDialog::SetMboxList(CMboxList* mbox_list)
{
	mMboxList = mbox_list;
	mMboxProtocol = ((CMbox*) mbox_list->front())->GetProtocol();

	// Load string title
	cdstring title;
	if (mMboxList)
	{
		title.FromResource((mMboxList->size() == 1 ? "UI::MailboxProp::Title" : "UI::MailboxProp::MultiTitle"));
		
		// Add name if single mailbox
		if (mMboxList->size() == 1)
			title += mMboxList->front()->GetName();
	}
	else if (mMboxProtocol)
	{
		title.FromResource("UI::MailboxProp::ServerTitle");
		
		// Add name of protocol
		title += mMboxProtocol->GetAccountName();
	}

	// Now set window title
	GetWindow()->SetTitle(title);

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
		mAccessBtn->Deactivate();
	if (only_dirs || !logged_in || !mMboxProtocol->UsingQuotas())
		mQuotaBtn->Deactivate();

	// Give list to each panel
	for(std::vector<JXWidgetSet*>::iterator iter = mPanelList.begin(); iter != mPanelList.end(); iter ++)
	{
		static_cast<CMailboxPropPanel*>(*iter)->SetMboxList(mMboxList);
		static_cast<CMailboxPropPanel*>(*iter)->SetProtocol(mMboxProtocol);
	}

	// Initial panel
	SetPanel(1);
}

void CMailboxPropDialog::SetProtocol(CMboxProtocol* proto)
{
	mMboxList = NULL;
	mMboxProtocol = proto;

	// Load string title
	cdstring title;
	title.FromResource("UI::MailboxProp::ServerTitle");
	
	// Add name if single mailbox
	title += mMboxProtocol->GetAccountName();

	// Now set window title
	GetWindow()->SetTitle(title);

	mOptionsBtn->Deactivate();
	mGeneralBtn->Deactivate();
	mAccessBtn->Deactivate();
	mQuotaBtn->Deactivate();

	// Give server to each panel
	for(std::vector<JXWidgetSet*>::iterator iter = mPanelList.begin(); iter != mPanelList.end(); iter ++)
		static_cast<CMailboxPropPanel*>(*iter)->SetProtocol(mMboxProtocol);

	// Initial panel
	//SetPanel(3);
	mServerBtn->Select();
}

bool CMailboxPropDialog::PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto)
{
	CMailboxPropDialog* dlog = new CMailboxPropDialog(JXGetApplication());
	dlog->OnCreate();
	if (mbox_list)
		dlog->SetMboxList(mbox_list);
	else if (proto)
		dlog->SetProtocol(proto);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
		dlog->Close();

	return true;
}
