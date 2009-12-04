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


// Source for CUploadScriptDialog class

#include "CUploadScriptDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>
#include <string.h>

CUploadScriptDialog::SUploadScript CUploadScriptDialog::sLastChoice = {false, 0, true, true};

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUploadScriptDialog::CUploadScriptDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}


// Default destructor
CUploadScriptDialog::~CUploadScriptDialog()
{
}

void CUploadScriptDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 365,205, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 365,205);
    assert( obj1 != NULL );

    mSaveGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 345,140);
    assert( mSaveGroup != NULL );
    mSaveGroup->SetBorderWidth(0);

    mFileBtn =
        new JXTextRadioButton(eSaveFile, "Save to a File", mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,0, 115,20);
    assert( mFileBtn != NULL );

    mUploadGroup =
        new JXRadioGroup(mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 345,110);
    assert( mUploadGroup != NULL );

    mServerBtn =
        new JXTextRadioButton(eSaveServer, "Save to Server", mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,20, 115,20);
    assert( mServerBtn != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Account:", mUploadGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,17, 60,20);
    assert( obj2 != NULL );

    mAccountPopup =
        new HPopupMenu( "", mUploadGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,12, 250,25);
    assert( mAccountPopup != NULL );

    mUploadOnlyBtn =
        new JXTextRadioButton(eUploadOnly, "Upload To Server Only", mUploadGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,40, 200,20);
    assert( mUploadOnlyBtn != NULL );

    mUploadActivateBtn =
        new JXTextRadioButton(eUploadActivate, "Upload to Server and Activate", mUploadGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,60, 200,20);
    assert( mUploadActivateBtn != NULL );

    mDeleteBtn =
        new JXTextRadioButton(eDelete, "Delete from Server", mUploadGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,80, 200,20);
    assert( mDeleteBtn != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,165, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 185,165, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	window->SetTitle("Write SIEVE Script");
	SetButtons(mOKBtn, mCancelBtn);

	InitAccountMenu();

	// Start listening
	ListenTo(mSaveGroup);
}

void CUploadScriptDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == mSaveGroup) && message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		switch (info->GetID())
		{
		case eSaveFile:
			mUploadGroup->Deactivate();
			return;
		case eSaveServer:
			mUploadGroup->Activate();
			return;
		}				
	}

	CDialogDirector::Receive(sender, message);
}

// Called during idle
void CUploadScriptDialog::InitAccountMenu()
{
	// Delete previous items
	mAccountPopup->RemoveAllItems();

	// Add each filter account
	for(CManageSIEVEAccountList::const_iterator iter = CPreferences::sPrefs->mSIEVEAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSIEVEAccounts.GetValue().end(); iter++)
		// Append it as a radio button
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);

	mAccountPopup->SetToPopupChoice(kTrue, 1);
}

// Set the details
void CUploadScriptDialog::SetDetails(SUploadScript& create)
{
	// Set file/server group
	if (create.mFile || (mAccountPopup->GetItemCount() == 0))
	{
		mSaveGroup->SelectItem(eSaveFile);
		mUploadGroup->Deactivate();
	}
	else
	{
		mSaveGroup->SelectItem(eSaveServer);
		mUploadGroup->Activate();
	}

	// Disable server if no accounts
	if (mAccountPopup->GetItemCount() == 0)
		mServerBtn->Deactivate();
	else
		// Init acct
		mAccountPopup->SetValue(create.mAccountIndex + 1);

	// Init group
	if (create.mUpload && !create.mActivate)
		mUploadGroup->SelectItem(eUploadOnly);
	else if (create.mUpload && create.mActivate)
		mUploadGroup->SelectItem(eUploadActivate);
	else
		mUploadGroup->SelectItem(eDelete);
}

// Get the details
void CUploadScriptDialog::GetDetails(SUploadScript& result)
{
	// Get items
	result.mFile = (mSaveGroup->GetSelectedItem() == eSaveFile);
	result.mAccountIndex = mAccountPopup->GetValue() - 1;
	result.mUpload = (mUploadGroup->GetSelectedItem() != eDelete);
	result.mActivate = (mUploadGroup->GetSelectedItem() == eUploadActivate);
}

bool CUploadScriptDialog::PoseDialog(SUploadScript& details)
{
	bool result = false;

	CUploadScriptDialog* dlog = new CUploadScriptDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(sLastChoice);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(sLastChoice);
		details = sLastChoice;
		result = true;
		dlog->Close();
	}

	return result;
}
