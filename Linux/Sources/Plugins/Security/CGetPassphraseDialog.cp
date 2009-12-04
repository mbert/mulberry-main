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


// CGetPassphraseDialog.cp : implementation file
//


#include "CGetPassphraseDialog.h"

#include "CXStringResources.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXInputField.h>
#include <JXPasswordInput.h>
#include "CInputField.h"
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CGetPassphraseDialog::CGetPassphraseDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CGetPassphraseDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 360,170, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 360,170);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Enter the Passphrase:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 140,20);
    assert( obj2 != NULL );

    mPassphrase1 =
        new CInputField<JXPasswordInput>(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 325,40);
    assert( mPassphrase1 != NULL );

    mPassphrase2 =
        new CInputField<JXInputField>(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 325,40);
    assert( mPassphrase2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("For Key:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,85, 60,20);
    assert( obj3 != NULL );

    mKeyPopup =
        new HPopupMenu( "", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,83, 255,20);
    assert( mKeyPopup != NULL );

    mHideTyping =
        new JXTextCheckbox("Hide Typing", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,110, 120,20);
    assert( mHideTyping != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 270,135, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,135, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout

	window->SetTitle("Passphrase");
	SetButtons(mOKBtn, mCancelBtn);

	// Don't require input in password field so it can be switched when empty
	mPassphrase1->SetIsRequired(kFalse);

	// Need to update for every character typed
	mPassphrase1->ShouldBroadcastAllTextChanged(kTrue);
	mPassphrase2->ShouldBroadcastAllTextChanged(kTrue);

	ListenTo(mPassphrase1);
	ListenTo(mPassphrase2);
	mOKBtn->Deactivate();

	mHideTyping->SetState(kTrue);
	mPassphrase2->Hide();
	ListenTo(mHideTyping);

	InitKeysMenu();
}

/////////////////////////////////////////////////////////////////////////////
// CGetPassphraseDialog message handlers

void CGetPassphraseDialog::InitKeysMenu(void) 
{
	if (mKeys)
	{
		mKeyPopup->RemoveAllItems();

		// Now add current items
		const char** p = mKeys;
		while(*p)
			//Append it as a radio button
			mKeyPopup->AppendItem(*p++, kFalse, kTrue);

		mKeyPopup->SetToPopupChoice(kTrue, 1);
	}
	else
		mKeyPopup->Hide();
}

void CGetPassphraseDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JTextEditor::kTextChanged))
	{
		if (sender == mPassphrase1)
		{
			if (mPassphrase1->GetTextLength())
				mOKBtn->Activate();
			else
				mOKBtn->Deactivate();
			return;
		}
		else if (sender == mPassphrase2)
		{
			if (mPassphrase2->GetTextLength())
				mOKBtn->Activate();
			else
				mOKBtn->Deactivate();
			return;
		}
	}
	else if ((sender == mHideTyping) && message.Is(JXCheckbox::kPushed))
	{
		OnHideTyping();
		return;
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

void CGetPassphraseDialog::OnHideTyping() 
{
	JXInputField* hide_it;
	JXInputField* show_it;
	if (mHideTyping->IsChecked())
	{
		hide_it = mPassphrase2;
		show_it = mPassphrase1;
	}
	else
	{
		hide_it = mPassphrase1;
		show_it = mPassphrase2;
	}

	// Get current selection/caret pos
	JIndex sel_start;
	JIndex sel_end;
	if (hide_it->HasSelection())
	{
		hide_it->GetSelection(&sel_start, &sel_end);
		sel_start--;
	}
	else
	{
		hide_it->GetCaretLocation(&sel_start);
		sel_end = --sel_start;
	}

	// Do show/hide
	show_it->SetText(hide_it->GetText());
	hide_it->Hide();
	show_it->Show();
	show_it->Focus();

	// Restore selection
	if (sel_start == sel_end)
		show_it->SetCaretLocation(sel_start + 1);
	else
		show_it->SetSelection(sel_start + 1, sel_end);
}

bool CGetPassphraseDialog::PoseDialog(cdstring& passphrase, const char* title)
{
	cdstring dummy1;
	unsigned long dummy2;
	return PoseDialog(passphrase, NULL, dummy1, dummy2, title);
}

bool CGetPassphraseDialog::PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index, const char* title)
{
	CGetPassphraseDialog* dlog = new CGetPassphraseDialog(JXGetApplication());
	dlog->mKeys = keys;
	dlog->OnCreate();
	if (title != NULL)
		dlog->GetWindow()->SetTitle(rsrc::GetString(title));
	dlog->mPassphrase1->SetText(change);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		change = dlog->mHideTyping->IsChecked() ?
					dlog->mPassphrase1->GetText() :
					dlog->mPassphrase2->GetText();
		change.ConvertFromOS();
		//change.ToUTF8();

		if (keys)
		{
			index = dlog->mKeyPopup->GetValue() - 1;
			chosen_key = keys[index];
		}
		else
		{
			index = 1;
			chosen_key = cdstring::null_str;
		}	
		dlog->Close();
		return true;
	}
	else
		return false;
}
