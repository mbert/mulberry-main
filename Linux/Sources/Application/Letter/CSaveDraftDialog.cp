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


// Source for CSaveDraftDialog class

#include "CSaveDraftDialog.h"

#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

#include <JXRadioGroup.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

CSaveDraftDialog::SSaveDraft CSaveDraftDialog::sDetails;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSaveDraftDialog::CSaveDraftDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSaveDraftDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 400,130, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 400,130);
    assert( obj1 != NULL );

    mGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 380,70);
    assert( mGroup != NULL );

    mFile =
        new JXTextRadioButton(eFileRadio, "Save to File", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 105,20);
    assert( mFile != NULL );

    mMailbox =
        new JXTextRadioButton(eMailboxRadio, "Save to Mailbox:", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 120,20);
    assert( mMailbox != NULL );

    mMailboxPopup =
        new CMailboxPopup( false, mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,25, 250,25);
    assert( mMailboxPopup != NULL );

    mOKBtn =
        new JXTextButton("Save", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,90, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,90, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout

	window->SetTitle("Save Draft");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mGroup);
}

// Respond to clicks in the icon buttons
void CSaveDraftDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mGroup && message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);
		switch (info->GetID())
		{
		case eFileRadio:
			mMailboxPopup->Deactivate();
			return;
		case eMailboxRadio:
			mMailboxPopup->Activate();
			return;
		}				
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

// Set the dialogs info
void CSaveDraftDialog::SetDetails(bool allow_file)
{
	mGroup->SelectItem((allow_file && sDetails.mFile) ? eFileRadio : eMailboxRadio);
	if (!allow_file)
		mFile->Deactivate();

	if (sDetails.mFile)
		mMailboxPopup->Deactivate();
	mMailboxPopup->SetSelectedMbox(sDetails.mMailboxName, sDetails.mMailboxName.empty(), sDetails.mMailboxName == "\1");
}

// Set the dialogs info
bool CSaveDraftDialog::GetDetails()
{
	bool result = true;

	sDetails.mFile = (mGroup->GetSelectedItem() == eFileRadio);

	// Only resolve mailbox name if saving to a mailbox
	if (!sDetails.mFile)
		mMailboxPopup->GetSelectedMboxName(sDetails.mMailboxName, true);
	
	return result;
}

bool CSaveDraftDialog::PoseDialog(SSaveDraft& details, bool allow_file)
{
	CSaveDraftDialog* dlog = new CSaveDraftDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(allow_file);

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		bool result = dlog->GetDetails();
		details.mFile = sDetails.mFile;
		details.mMailboxName = sDetails.mMailboxName;
		dlog->Close();
		return result;
	}

	return false;
}
