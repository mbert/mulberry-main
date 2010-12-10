/*
    Copyright (c) 2007-2010 Cyrus Daboo. All rights reserved.
    
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


// Source for CBrowseMailboxDialog class

#include "CBrowseMailboxDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CServerBrowseTable.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

#include <jXGlobals.h>

#include <cassert>

CBrowseMailboxDialog::CBrowseMailboxDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	// Make sure it can be sized and placed manually
	UseModalPlacement(kFalse);
	mSending = false;
}

CBrowseMailboxDialog::~CBrowseMailboxDialog()
{
	// Make sure window position is saved
	SaveState();
}

void CBrowseMailboxDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,375, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* objcont =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,375);
    assert( objcont != NULL );

// end JXLayout

	CTableScrollbarSet* sbs = NULL;

	if (!mSending)
	{
// begin JXLayout1

    mOKBtn =
        new JXTextButton("OK", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 250,340, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 160,340, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mCreateBtn =
        new JXTextButton("Create...", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 50,340, 70,25);
    assert( mCreateBtn != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Select a Mailbox:", objcont,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 115,20);
    assert( obj1 != NULL );

    CTableScrollbarSet* obj2 =
        new CTableScrollbarSet(objcont,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 320,290);
    assert( obj2 != NULL );

// end JXLayout1
	mNoneBtn = NULL;
	mSetAsDefault = NULL;
	sbs = obj2;
	}
	else
	{
// begin JXLayout2

    mOKBtn =
        new JXTextButton("Copy", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,340, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 100,340, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mCreateBtn =
        new JXTextButton("Create...", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 10,340, 70,25);
    assert( mCreateBtn != NULL );

    mNoneBtn =
        new JXTextButton("No Copy", objcont,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 180,340, 70,25);
    assert( mNoneBtn != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Select a Mailbox to Save a Copy of the Draft:", objcont,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 265,20);
    assert( obj1 != NULL );

    CTableScrollbarSet* obj2 =
        new CTableScrollbarSet(objcont,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 320,270);
    assert( obj2 != NULL );

    mSetAsDefault =
        new JXTextCheckbox("Use Chosen Mailbox as the Default in Future", objcont,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,305, 285,20);
    assert( mSetAsDefault != NULL );

// end JXLayout2
	sbs = obj2;

	}

	sbs->NoTitles();
	mTable = new CServerBrowseTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kVElastic,
										0,0, 10, 10);
	mTable->OnCreate();
	mTable->SetManager();
	mTable->SetButtons(this, mOKBtn, mCreateBtn);

	ListenTo(mCreateBtn);
	if (mNoneBtn)
		ListenTo(mNoneBtn);
	ListenTo(mTable);

	window->SetTitle(mSending ? "Copy to Mailbox" : "Browse for Mailbox");
	SetButtons(mOKBtn, mCancelBtn);
	mOKBtn->Deactivate();

	// Reset window state
	ResetState();
}

void CBrowseMailboxDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mCreateBtn)
		{
			mTable->DoCreateMailbox();
			return;
		}

		else if (sender == mNoneBtn)
		{
			EndDialog(kDialogClosed_Btn3);
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

// Set sending
void CBrowseMailboxDialog::SetSending(bool sending)
{
	mSending = sending;
}

// Set open mode
void CBrowseMailboxDialog::SetOpenMode(bool mode)
{
	mTable->SetOpenMode(mode);
	mTable->ResetTable();
}

// Set open mode
void CBrowseMailboxDialog::SetMultipleSelection()
{
	mTable->SetMultipleSelection();
}

// Get selected mbox
CMbox* CBrowseMailboxDialog::GetSelectedMbox(void)
{
	return mTable->GetSelectedMbox();
}

// Get selected mboxes
void CBrowseMailboxDialog::GetSelectedMboxes(CMboxList& mbox_list)
{
	mTable->GetSelectedMboxes(mbox_list);
}

// Reset state from prefs
void CBrowseMailboxDialog::ResetState(void)
{
	CWindowState& state = CPreferences::sPrefs->mServerBrowseDefault.Value();

	// Do not set if empty
	JRect set_rect = state.GetBestRect(state);
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset bounds
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}
}

// Save state in prefs
void CBrowseMailboxDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	CWindowState state(name, &bounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mServerBrowseDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerBrowseDefault.SetDirty();
}

bool CBrowseMailboxDialog::PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default)
{
	bool result = false;

	// Do browse
	CBrowseMailboxDialog* dlog = new CBrowseMailboxDialog(JXGetApplication());
	dlog->SetSending(sending);
	dlog->OnCreate();
	dlog->SetOpenMode(open_mode);

	int dlg_result = dlog->DoModal(false);
	switch(dlg_result)
	{
	case kDialogClosed_OK:
		mbox = dlog->GetSelectedMbox();
		set_as_default = sending ? dlog->mSetAsDefault->IsChecked() : false;
		result = true;
		dlog->Close();
		break;
	case kDialogClosed_Btn3:
		mbox = (CMbox*) -1L;
		result = true;
		dlog->Close();
		break;
	default:
		mbox = NULL;
		break;
	}

	return result;
}

bool CBrowseMailboxDialog::PoseDialog(CMboxList& mboxes)
{
	bool result = false;

	// Do browse
	CBrowseMailboxDialog* dlog = new CBrowseMailboxDialog(JXGetApplication());
	dlog->SetSending(false);
	dlog->OnCreate();
	dlog->SetMultipleSelection(); // Do this before resetting the table
	dlog->SetOpenMode(false);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetSelectedMboxes(mboxes);
		result = true;
		dlog->Close();
	}

	return result;
}
