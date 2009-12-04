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


// Source for CSynchroniseDialog class

#include "CSynchroniseDialog.h"

#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CPreferences.h"

#include <JXIntegerInput.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSynchroniseDialog::CSynchroniseDialog(JXDirector* supervisor)
  : CDialogDirector(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSynchroniseDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 280,255, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 280,255);
    assert( obj1 != NULL );

    JXTextButton* okBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,220, 70,25);
    assert( okBtn != NULL );
    okBtn->SetShortcuts("^M");

    JXTextButton* cancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,220, 70,25);
    assert( cancelBtn != NULL );
    cancelBtn->SetShortcuts("^[");

    mMailboxGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 260,80);
    assert( mMailboxGroup != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Mailbox:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 60,15);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(eAll, "All Messages", mMailboxGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 110,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(eNew, "New Messages", mMailboxGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 115,20);
    assert( obj4 != NULL );

    mSelected =
        new JXTextRadioButton(eSelected, "Selected Messages", mMailboxGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 145,20);
    assert( mSelected != NULL );

    mMessageGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,125, 260,80);
    assert( mMessageGroup != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Messages:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,115, 70,15);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(eFull, "Entire Message", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 115,20);
    assert( obj6 != NULL );

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(eBelow, "Entire Message Below:", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 160,20);
    assert( obj7 != NULL );

    mSize =
        new CInputField<JXIntegerInput>(mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,28, 60,20);
    assert( mSize != NULL );

    JXStaticText* obj8 =
        new JXStaticText("KB", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,30, 25,20);
    assert( obj8 != NULL );

    JXTextRadioButton* obj9 =
        new JXTextRadioButton(ePartial, "First Displayable Part Only", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 180,20);
    assert( obj9 != NULL );

// end JXLayout

	window->SetTitle("Synchronise");
	SetButtons(okBtn, cancelBtn);

	ListenTo(mMessageGroup);
}

// Handle OK button
void CSynchroniseDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mMessageGroup && message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);
		switch (info->GetID())
		{
		case eFull:
		case ePartial:
			mSize->Deactivate();
			return;
		case eBelow:
			mSize->Activate();
			return;
		}				
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

// Set the details
void CSynchroniseDialog::SetDetails(bool has_selection)
{
	// Get disconnection options from prefs
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	if (!has_selection)
	{
		if (disconnect.mMboxSync != CConnectionManager::eNewMessages)
			mMailboxGroup->SelectItem(eAll);
		else
			mMailboxGroup->SelectItem(eNew);
		mSelected->Deactivate();
	}
	else
		mMailboxGroup->SelectItem(eSelected);

	switch(disconnect.mMsgSync)
	{
	case CConnectionManager::eEntireMessage:
		mMessageGroup->SelectItem(eFull);
		break;
	case CConnectionManager::eMessageBelow:
		mMessageGroup->SelectItem(eBelow);
		break;
	case CConnectionManager::eFirstDisplayable:
		mMessageGroup->SelectItem(ePartial);
		break;
	}

	mSize->SetValue(disconnect.mMsgSyncSize/1024);
	if (disconnect.mMsgSync != CConnectionManager::eMessageBelow)
		mSize->Deactivate();

}

// Get the details
void CSynchroniseDialog::GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection)
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	fast = (mMailboxGroup->GetSelectedItem() == eNew);
	partial = (mMessageGroup->GetSelectedItem() == ePartial);
	selection = (mMailboxGroup->GetSelectedItem() == eSelected);

	// Sync if requested
	if (mMailboxGroup->GetSelectedItem() == eAll)
		disconnect.mMboxSync = CConnectionManager::eAllMessages;

	// Fast if requested
	else if (mMailboxGroup->GetSelectedItem() == eNew)
		disconnect.mMboxSync = CConnectionManager::eNewMessages;

	// Message options
	if (mMessageGroup->GetSelectedItem() == eFull)
		disconnect.mMsgSync = CConnectionManager::eEntireMessage;
	else if (mMessageGroup->GetSelectedItem() == eBelow)
		disconnect.mMsgSync = CConnectionManager::eMessageBelow;
	else
		disconnect.mMsgSync = CConnectionManager::eFirstDisplayable;

	JInteger value;
	mSize->GetValue(&value);
	size = value * 1024;
	disconnect.mMsgSyncSize = size;

	if (mMessageGroup->GetSelectedItem() != eBelow)
		size = 0;
	
	// Mark prefs as dirty if needed
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

void CSynchroniseDialog::PoseDialog(CMboxList* mbox_list)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = false;

	if (PoseDialog(fast, partial, size, selected))
	{
		// Synchonise each mailbox
		for(CMboxList::iterator iter = mbox_list->begin(); iter != mbox_list->end(); iter++)
			static_cast<CMbox*>(*iter)->Synchronise(fast, partial, size);
	}
}

void CSynchroniseDialog::PoseDialog(CMbox* mbox, ulvector& selection)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = selection.size();

	if (PoseDialog(fast, partial, size, selected))
	{
		if (selected)
			// Synchronise the messages
			mbox->SynchroniseMessage(selection, false, fast, partial, size, true);
		else
			// Full synchonise of mailbox
			mbox->Synchronise(fast, partial, size);
	}
}

bool CSynchroniseDialog::PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected)
{
	bool result = false;

	CSynchroniseDialog* dlog = new CSynchroniseDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(selected);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(fast, partial, size, selected);
		result = true;
		dlog->Close();
	}

	return result;
}
