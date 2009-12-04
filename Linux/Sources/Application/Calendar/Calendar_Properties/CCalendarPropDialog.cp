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


// Source for CCalendarPropDialog class

#include "CCalendarPropDialog.h"

#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
//#include "CMulberryApp.h"
//#include "CMulberryCommon.h"
#include "CPropCalendarACL.h"
#include "CPropCalendarGeneral.h"
#include "CPropCalendarOptions.h"
#include "CPropCalendarServer.h"
#include "CPropCalendarWebcal.h"

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

// Constructor from stream
CCalendarPropDialog::CCalendarPropDialog(JXDirector* supervisor)
	: CPropDialog(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCalendarPropDialog::OnCreate()
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

    mWebCalBtn =
        new JXIconTextButton(4, "WebCal", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,75, 80,25);
    assert( mWebCalBtn != NULL );

    mAccessBtn =
        new JXIconTextButton(5, "Access", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,100, 80,25);
    assert( mAccessBtn != NULL );

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
	mWebCalBtn->SetImage(IDI_MAILBOXPROP_QUOTA);
	mAccessBtn->SetImage(IDI_MAILBOXPROP_ACL);
	ListenTo(rgroup);

	mPanelContainer = mCards;

	// Do inherited
	CPropDialog::OnCreate();
}

// Handle buttons
void CCalendarPropDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex item = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		SetPanel(item);
	}

	CPropDialog::Receive(sender, message);
}

// Set up panels
void CCalendarPropDialog::SetUpPanels(void)
{
	CCalendarPropPanel* panel = new CPropCalendarOptions(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropCalendarGeneral(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropCalendarServer(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropCalendarWebcal(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropCalendarACL(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);
}

// About to show panel
void CCalendarPropDialog::DoPanelInit(void)
{
}

// Set input panel
void CCalendarPropDialog::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	mCalList = cal_list;
	mCalProtocol = cal_list->front()->GetProtocol();

	// Load string title
	cdstring title;
	title.FromResource((mCalList->size() == 1 ? "UI::CalendarProp::Title" : "UI::CalendarProp::MultiTitle"));

	// Add name if single calendar
	if (mCalList->size() == 1)
	{
		title += mCalList->front()->GetName();
	}

	// Now set window title
	GetWindow()->SetTitle(title);

	// Check webcal state
	int local = 0;
	int webcal = 0;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;

		if (!node->IsDirectory())
		{
			if (node->GetProtocol()->IsLocalCalendar())
				local++;

			if (node->GetProtocol()->IsWebCalendar())
				webcal++;
		}
	}
	
	// Must have server cals in list
	if ((webcal == mCalList->size()) || (local == mCalList->size()))
	{
		mServerBtn->Deactivate();
	}

	// Must have only webcals in the entire list
	if ((webcal == 0) || (webcal != mCalList->size()) || (local == mCalList->size()))
	{
		mWebCalBtn->Deactivate();
	}

	// Disable unwanted prefs panels
	bool logged_in = mCalProtocol->IsLoggedOn();
	if (!logged_in || !mCalProtocol->UsingACLs())
	{
		mAccessBtn->Deactivate();
	}

	// Give list to each panel
	for(std::vector<JXWidgetSet*>::iterator iter = mPanelList.begin(); iter != mPanelList.end(); iter ++)
	{
		static_cast<CCalendarPropPanel*>(*iter)->SetCalList(mCalList);
		static_cast<CCalendarPropPanel*>(*iter)->SetProtocol(mCalProtocol);
	}

	// Initial panel
	SetPanel(1);
}

// Set input panel
void CCalendarPropDialog::SetProtocol(calstore::CCalendarProtocol* proto)
{
	mCalList = NULL;
	mCalProtocol = proto;

	// Load string title
	cdstring title;
	title.FromResource("UI::CalendarProp::ServerTitle");

	// Add name if single calendar
	title += proto->GetAccountName();

	// Now set window title
	GetWindow()->SetTitle(title);

	// Disable unwanted prefs panels
	mOptionsBtn->Deactivate();
	mGeneralBtn->Deactivate();
	mWebCalBtn->Deactivate();
	mAccessBtn->Deactivate();

	// Give server to each panel
	for(std::vector<JXWidgetSet*>::iterator iter = mPanelList.begin(); iter != mPanelList.end(); iter ++)
		static_cast<CCalendarPropPanel*>(*iter)->SetProtocol(mCalProtocol);

	// Initial panel
	//SetPanel(3);
	mServerBtn->Select();
}

bool CCalendarPropDialog::PoseDialog(calstore::CCalendarStoreNodeList* cal_list, calstore::CCalendarProtocol* proto)
{
	CCalendarPropDialog* prop_dlog = new CCalendarPropDialog(JXGetApplication());
	prop_dlog->OnCreate();

	if (cal_list)
		prop_dlog->SetCalList(cal_list);
	else if (proto)
		prop_dlog->SetProtocol(proto);

	// Test for OK
	if (prop_dlog->DoModal(false) == kDialogClosed_OK)
		prop_dlog->Close();

	return true;
}
