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


// Source for CAdbkPropDialog class

#include "CAdbkPropDialog.h"

#include "CAddressBook.h"
#include "CAdbkProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPropAdbkACL.h"
#include "CPropAdbkGeneral.h"

#include "JXIconTextButton.h"

#include <JXCardFile.h>
#include <JXEngravedRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkPropDialog::CAdbkPropDialog(JXDirector* supervisor)
	: CPropDialog(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CAdbkPropDialog::OnCreate()
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
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 80,55);
    assert( rgroup != NULL );
    rgroup->SetBorderWidth(0);

    mGeneralBtn =
        new JXIconTextButton(1, "General", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 80,25);
    assert( mGeneralBtn != NULL );

    mAccessBtn =
        new JXIconTextButton(2, "Access", rgroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 80,25);
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

	window->SetTitle("Address Book Properties");
	SetButtons(mOKBtn, NULL);

	// Set icons
	mGeneralBtn->SetImage(IDI_ADDRESSFILE);
	mAccessBtn->SetImage(IDI_MAILBOXPROP_ACL);
	ListenTo(rgroup);

	mPanelContainer = mCards;

	// Do inherited
	CPropDialog::OnCreate();
}

// Handle buttons
void CAdbkPropDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex item = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		SetPanel(item);
	}

	CPropDialog::Receive(sender, message);
}

// Set up panels
void CAdbkPropDialog::SetUpPanels(void)
{
	CAdbkPropPanel* panel = new CPropAdbkGeneral(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);

	panel = new CPropAdbkACL(mCards,
				 				JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450,310);
	panel->OnCreate();
	mPanelList.push_back(panel);
	mCards->AppendCard(panel);
}

// About to show panel
void CAdbkPropDialog::DoPanelInit(void)
{
}

// Set input panel
void CAdbkPropDialog::SetAdbkList(CAddressBookList* adbk_list)
{
	mAdbkList = adbk_list;

	// Load string title
	cdstring title;
	title.FromResource((mAdbkList->size() == 1 ? "UI::AdbkProps::Title" : "UI::AdbkProps::MultiTitle"));

	// Add name if single mailbox
	if (mAdbkList->size() == 1)
		title += mAdbkList->front()->GetName();

	// Now set window title
	GetWindow()->SetTitle(title);

	// Disable unwanted prefs panels
	CAddressBook* adbk = mAdbkList->front();
	bool logged_in = adbk && adbk->GetProtocol()->IsLoggedOn();
	if (!logged_in || !adbk->GetProtocol()->UsingACLs())
		mAccessBtn->Deactivate();

	// Give list to each panel
	for(std::vector<JXWidgetSet*>::iterator iter = mPanelList.begin(); iter != mPanelList.end(); iter ++)
		static_cast<CAdbkPropPanel*>(*iter)->SetAdbkList(mAdbkList);

	// Initial panel
	SetPanel(1);
}

bool CAdbkPropDialog::PoseDialog(CAddressBookList* adbk_list)
{
	CAdbkPropDialog* dlog = new CAdbkPropDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetAdbkList(adbk_list);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
		dlog->Close();

	return true;
}
