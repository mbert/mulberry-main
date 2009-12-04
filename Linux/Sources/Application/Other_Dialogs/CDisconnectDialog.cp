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

// CDisconnectDialog.cpp : implementation file
//

#include "CDisconnectDialog.h"

#include "CConnectionManager.h"
#include "CDisconnectListPanel.h"
#include "CErrorHandler.h"
#include "CIconTextTable.h"
#include "CMailAccountManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CResizeNotifier.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CTwister.h"

#include "TPopupMenu.h"

#include <JXFlatRect.h>
#include <JXIntegerInput.h>
#include <JXProgressIndicator.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog dialog

bool CDisconnectDialog::sIsTwisted = false;

CDisconnectDialog* CDisconnectDialog::sDisconnectDialog = NULL;

CDisconnectDialog::CDisconnectDialog(JXDirector* supervisor)
  : CDialogDirector(supervisor), mSyncTargets(true)
{
	sDisconnectDialog = this;
}

// Default destructor
CDisconnectDialog::~CDisconnectDialog()
{
	sDisconnectDialog = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog message handlers

void CDisconnectDialog::OnCreate()
{
	
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,455, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,455);
    assert( container != NULL );

    mMailboxListPanel =
        new CDisconnectListPanel(container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 2,253, 496,200);
    assert( mMailboxListPanel != NULL );

    mOKBtn =
        new JXTextButton("OK", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 410,250, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 320,250, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	{
// begin JXLayout2

    mConnectingPanel =
        new JXFlatRect(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,160);
    assert( mConnectingPanel != NULL );

    mConnectingGroup =
        new JXRadioGroup(mConnectingPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 480,135);
    assert( mConnectingGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("When Connecting:", mConnectingPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,10, 110,15);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(eStayConnected, "Stay Connected", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 170,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(eUpdateDisconnect, "Update then Disconnect", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 170,20);
    assert( obj3 != NULL );

    mPlayback =
        new JXTextCheckbox("Synchronise Offline Changes", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 195,20);
    assert( mPlayback != NULL );

    mPOP3 =
        new JXTextCheckbox("Update POP3 Accounts", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 195,20);
    assert( mPOP3 != NULL );

    mSend =
        new JXTextCheckbox("Send Queued Messages", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 195,20);
    assert( mSend != NULL );

    mAdbk1 =
        new JXTextCheckbox("Synchronise Remote Address Books", mConnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 235,20);
    assert( mAdbk1 != NULL );

    mMessagePanel =
        new JXFlatRect(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,160, 500,90);
    assert( mMessagePanel != NULL );

    mMessageGroup =
        new JXRadioGroup(mMessagePanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 480,70);
    assert( mMessageGroup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Message Synchronisation:", mMessagePanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 160,15);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(eFull, "Entire Message", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 115,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(eBelow, "Entire Message Below:", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 160,20);
    assert( obj6 != NULL );

    mSize =
        new CInputField<JXIntegerInput>(mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,25, 60,20);
    assert( mSize != NULL );

    JXStaticText* obj7 =
        new JXStaticText("KB", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,27, 25,20);
    assert( obj7 != NULL );

    JXTextRadioButton* obj8 =
        new JXTextRadioButton(ePartial, "First Displayable Part Only", mMessageGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,45, 180,20);
    assert( obj8 != NULL );

// end JXLayout2
	}
	{
// begin JXLayout3

    mDisconnectingPanel =
        new JXFlatRect(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,160);
    assert( mDisconnectingPanel != NULL );

    mDisconnectingGroup =
        new JXRadioGroup(mDisconnectingPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 480,135);
    assert( mDisconnectingGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("When Disconnecting:", mDisconnectingPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,10, 130,15);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(eAll, "Synchronise All Messages", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 180,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(eNew, "Synchronise New Messages", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 190,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(eNone, "No Automatic Synchronisation", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,45, 200,20);
    assert( obj4 != NULL );

    mList =
        new JXTextCheckbox("Synchronise Mailbox List", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 195,20);
    assert( mList != NULL );

    mWait =
        new JXTextCheckbox("Send Queued Messages First", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 200,20);
    assert( mWait != NULL );

    mAdbk2 =
        new JXTextCheckbox("Synchronise Remote Address Books", mDisconnectingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 235,20);
    assert( mAdbk2 != NULL );

    mProgressPanel =
        new JXFlatRect(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,160, 500,90);
    assert( mProgressPanel != NULL );

    mProgressGroup =
        new JXRadioGroup(mProgressPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 480,70);
    assert( mProgressGroup != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Playback Progress:", mProgressPanel,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 120,15);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Accounts:", mProgressGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 65,20);
    assert( obj6 != NULL );

    mProgress1 =
        new CBarPane(mProgressGroup,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 80,12, 210,16);
    assert( mProgress1 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Items:", mProgressGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,35, 65,20);
    assert( obj7 != NULL );

    mProgress2 =
        new CBarPane(mProgressGroup,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 80,37, 210,16);
    assert( mProgress2 != NULL );

// end JXLayout3
	}

	SetButtons(mOKBtn, mCancelBtn);

	// Create mailbox list area
	mMailboxListPanel->OnCreate(this, NULL, NULL);
	mMailboxListPanel->OnCabinet(CMailAccountManager::eFavouriteAutoSync + 1);

	// Force mailbox sets to dissappear
	mMailboxListPanel->OnTwist();

	// Listen to UI items
	ListenTo(container);
	ListenTo(mConnectingGroup);
	ListenTo(mDisconnectingGroup);
	ListenTo(mMessageGroup);
	ListenTo(mMailboxListPanel->mTwister);
	ListenTo(mMailboxListPanel->mCabinets);
	ListenTo(mMailboxListPanel->mAddListBtn);
	ListenTo(mMailboxListPanel->mClearListBtn);
	ListenTo(mMailboxListPanel->mOptionsBtn);
	ListenTo(mMailboxListPanel->mMailboxList);

	// Reset to last twist state after starting to listen to twister
	if (!sIsTwisted)
		mMailboxListPanel->mTwister->SetState(kTrue);
}

// Handle OK button
void CDisconnectDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kNeedsUpdate) && (sender == mMailboxListPanel->mCabinets))
	{
		// Iterate over every item in the menu
		JXTextMenu* menu = dynamic_cast<JXTextMenu*>(sender);
  		menu->EnableAll();
	}
	else if(message.Is(JXMenu::kItemSelected) && (sender == mMailboxListPanel->mCabinets))
	{
		JIndex item = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		mMailboxListPanel->OnCabinet(item);
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);

		if (sender == mDisconnectingGroup)
		{
			switch (info->GetID())
			{
			case eAll:
			case eNew:
				mMessageGroup->Activate();
				return;
			case eNone:
				mMessageGroup->Deactivate();
				return;
			}				
		}
		else if (sender == mConnectingGroup)
		{
			switch (info->GetID())
			{
			case eStayConnected:
				mMessageGroup->Deactivate();
				mPOP3->Deactivate();

				ShowProgressPanel(false);
				return;
			case eUpdateDisconnect:
				mMessageGroup->Activate();
				mPOP3->Activate();

				ShowProgressPanel(true);
				return;
			}				
		}
		else if (sender == mMessageGroup)
		{
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
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mMailboxListPanel->mAddListBtn)
		{
			mMailboxListPanel->OnAddMailboxList();
			return;
		}
		else if (sender == mMailboxListPanel->mClearListBtn)
		{
			mMailboxListPanel->OnClearMailboxList();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mMailboxListPanel->mTwister)
		{
			// Allow resize of fixed-size dialog window
			GetWindow()->SetMinSize(1, 1);
			GetWindow()->SetMaxSize(5000, 5000);
			mMailboxListPanel->OnTwist();
			mPendingResize = true;
			return;
		}
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

// Show or hide the progress panel
void CDisconnectDialog::ShowProgressPanel(bool show)
{
	const int cShrinkOffset = 24;
	int moveby = mMailboxListPanel->mTwisted ?
					mMailboxListPanel->mMailboxArea->GetFrameHeight() : 0;
	moveby += cShrinkOffset;

	// Allow resize of fixed-size dialog window
	GetWindow()->SetMinSize(1, 1);
	GetWindow()->SetMaxSize(5000, 5000);

	if (show)
	{
		mMailboxListPanel->Show();
		AdjustSize(0, moveby);
	}
	else
	{
		mMailboxListPanel->Hide();
		AdjustSize(0, -moveby);
	}

	// Set flag to force LockWindow when resized event handle
	mPendingResize = true;
}

// Set the dialogs info
void CDisconnectDialog::SetDetails(bool connecting)
{
	// Get dis/connection options from prefs
	const CConnectionManager::CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.GetValue();
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	// Disable unwanted options
	if (connecting)
	{
		mDisconnectingPanel->Hide();

		mConnectingGroup->SelectItem(connect.mStayConnected ? eStayConnected : eUpdateDisconnect);
		mPlayback->SetState(JBoolean(connect.mMboxPlayback));
		mPOP3->SetState(JBoolean(connect.mUpdatePOP3));
		mSend->SetState(JBoolean(connect.mSMTPSend));

		if (connect.mStayConnected)
		{
			mMessageGroup->Deactivate();
			mPOP3->Deactivate();
			
			ShowProgressPanel(false);
		}

		mAdbk1->SetState(JBoolean(connect.mAdbkPlayback));
		
		cdstring title;
		title.FromResource("UI::Disconnect::Connect");
		GetWindow()->SetTitle(title);
	}
	else
	{
		mConnectingPanel->Hide();

		mDisconnectingGroup->SelectItem(disconnect.mMboxSync);
		mList->SetState(JBoolean(disconnect.mListSync));
		mWait->SetState(JBoolean(disconnect.mSMTPWait));
		
		mAdbk2->SetState(JBoolean(disconnect.mAdbkSync));
		
		cdstring title;
		title.FromResource("UI::Disconnect::Disconnect");
		GetWindow()->SetTitle(title);
	}

	mMessageGroup->SelectItem(disconnect.mMsgSync);
	mSize->SetValue(disconnect.mMsgSyncSize/1024);
	if (disconnect.mMsgSync != CConnectionManager::eMessageBelow)
		mSize->Deactivate();

	// Hide address books if no remote accounts
	if (!CPreferences::sPrefs->mAddressAccounts.GetValue().size())
	{
		mAdbk1->Hide();
		mAdbk2->Hide();
	}
	
	// Always hide playback progress
	mProgressPanel->Hide();
}

// Get the details
void CDisconnectDialog::GetDetailsDisconnect()
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	// Sync if requested
	disconnect.mMboxSync = static_cast<CConnectionManager::EMboxSync>(mDisconnectingGroup->GetSelectedItem());

	// Mailbox list sync
	disconnect.mListSync = mList->IsChecked();

	// Wait for sends to complete
	disconnect.mSMTPWait = mWait->IsChecked();

	// Message options
	disconnect.mMsgSync = static_cast<CConnectionManager::EMessageSync>(mMessageGroup->GetSelectedItem());
	JInteger value;
	mSize->GetValue(&value);
	disconnect.mMsgSyncSize = value * 1024;

	// Adbk sync
	disconnect.mAdbkSync = mAdbk2->IsChecked();
	
	// Now set up progress items
	if (disconnect.mMboxSync != CConnectionManager::eNoMessages)
	{
		mMailboxListPanel->SetFound(0);
		mMailboxListPanel->SetMessages(0);
		mMailboxListPanel->mProgress->SetMaxValue(mSyncTargets.size() ? mSyncTargets.size() : 1);
		mMailboxListPanel->mProgress->Show();
	}
	
	// Mark prefs as dirty if needed
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

// Get the details
void CDisconnectDialog::GetDetailsConnect()
{
	// Get dis/connection options from prefs
	CConnectionManager::CConnectOptions orig_connect = CPreferences::sPrefs->mConnectOptions.Value();
	CConnectionManager::CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.Value();
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	// Update if requested
	connect.mStayConnected = (mConnectingGroup->GetSelectedItem() == eStayConnected);

	// Do playback
	connect.mMboxPlayback = mPlayback->IsChecked();

	// POP3 update
	connect.mUpdatePOP3 = mPOP3->IsChecked();

	// Enable SMTP
	connect.mSMTPSend = mSend->IsChecked();

	// Message options
	disconnect.mMsgSync = static_cast<CConnectionManager::EMessageSync>(mMessageGroup->GetSelectedItem());
	JInteger value;
	mSize->GetValue(&value);
	disconnect.mMsgSyncSize = value * 1024;

	// Adbk sync
	connect.mAdbkPlayback = mAdbk1->IsChecked();

	// Now switch in progress items (must force redraw to refresh background properly)
	mMessagePanel->Hide();
	mProgressPanel->Show();
	GetWindow()->Redraw();
	
	// Now set up progress items
	if (!connect.mStayConnected)
	{
		mMailboxListPanel->SetFound(0);
		mMailboxListPanel->SetMessages(0);
		mMailboxListPanel->mProgress->SetMaxValue(mSyncTargets.size() ? mSyncTargets.size() : 1);
		mMailboxListPanel->mProgress->Show();
	}
	
	// Mark prefs as dirty if needed
	if (!(orig_connect == connect))
		CPreferences::sPrefs->mConnectOptions.SetDirty();
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

void CDisconnectDialog::NextItem(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel->mMailboxList->SetIcon(item + 1, IDI_SEARCH_POINTER);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel->mMailboxList->ScrollCellIntoFrame(aCell);
}

void CDisconnectDialog::DoneItem(unsigned long item, bool hit)
{
	// Set hit on previous
	mMailboxListPanel->SetHitState(item, hit, false);
	mMailboxListPanel->SetFound(item + 1);
	mMailboxListPanel->SetProgress(item + 1);
}

JBoolean CDisconnectDialog::OKToDeactivate()
{
	// Get twist state
	sIsTwisted = mMailboxListPanel->mTwisted;

	if (!Cancelled())
		OnOK();

	return kTrue;
}

void CDisconnectDialog::OnOK()
{
	// Disable buttons
	mOKBtn->Deactivate();
	mCancelBtn->Deactivate();
	mMailboxListPanel->SetInProgress(true);

	if (CConnectionManager::sConnectionManager.IsConnected())
	{
		GetDetailsDisconnect();
		
		// Do sync here while dialog is still open
		CConnectionManager::sConnectionManager.Disconnect(false, false, NULL);
	}
	else
	{
		// Preserve disconnect options
		CConnectionManager::CDisconnectOptions disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
		
		GetDetailsConnect();
		
		// Do sync here while dialog is still open
		CConnectionManager::sConnectionManager.Connect(mProgress1, mProgress2);

		// Recover disconnect options
		CPreferences::sPrefs->mDisconnectOptions.Value() = disconnect;
	}
}

void CDisconnectDialog::PoseDialog()
{
	bool disconnecting = CConnectionManager::sConnectionManager.IsConnected();

	// Check for some disconnected mail accounts
	if (disconnecting && !CMailAccountManager::sMailAccountManager->HasDisconnectedProtocols() &&
		(CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Disconnect::NoDisconnectedCaution") != CErrorHandler::Ok))
		return;
	
	bool result = false;

	// Create the dialog
	CDisconnectDialog* dlog = new CDisconnectDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(!disconnecting);
	
	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		result = true;
		dlog->Close();
	}
	else
		return;

	CPreferences::sPrefs->mDisconnected.SetValue(!CConnectionManager::sConnectionManager.IsConnected());

	// Check to see whether there are messages available and force queue window on screen
	if (CSMTPAccountManager::sSMTPAccountManager->ItemsHeld())
		CSMTPWindow::OpenSMTPWindow();
}
