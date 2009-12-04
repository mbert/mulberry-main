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


// Source for CDisconnectDialog class

#include "CDisconnectDialog.h"

#include "CBalloonDialog.h"
#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CIconTextTable.h"
#include "CMailAccountManager.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CVisualProgress.h"

#include <LCheckBox.h>
#include <LDisclosureTriangle.h>
#include <LRadioButton.h>
#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

CDisconnectDialog* CDisconnectDialog::sDisconnectDialog = NULL;
bool CDisconnectDialog::sIsTwisted = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CDisconnectDialog::CDisconnectDialog()
	: mSyncTargets(true)
{
	sDisconnectDialog = this;
}

// Constructor from stream
CDisconnectDialog::CDisconnectDialog(LStream *inStream)
		: LDialogBox(inStream), mSyncTargets(true)
{
	sDisconnectDialog = this;
}

// Default destructor
CDisconnectDialog::~CDisconnectDialog()
{
	sDisconnectDialog = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDisconnectDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get radio buttons
	mOKBtn = FindPaneByID(paneid_DisconnectOKBtn);
	mCancelBtn = FindPaneByID(paneid_DisconnectCancelBtn);
	mDisconnectPanel = FindPaneByID(paneid_DisconnectDisconnect);
	mAll = (LRadioButton*) FindPaneByID(paneid_DisconnectAll);
	mNew = (LRadioButton*) FindPaneByID(paneid_DisconnectNew);
	mNone = (LRadioButton*) FindPaneByID(paneid_DisconnectNone);
	mSend = (LCheckBox*) FindPaneByID(paneid_DisconnectSend);
	mConnectPanel = FindPaneByID(paneid_DisconnectConnect);
	mPermanent = (LRadioButton*) FindPaneByID(paneid_DisconnectPermanent);
	mUpdate = (LRadioButton*) FindPaneByID(paneid_DisconnectUpdate);
	mPlayback = (LCheckBox*) FindPaneByID(paneid_DisconnectPlayback);
	mList = (LCheckBox*) FindPaneByID(paneid_DisconnectList);
	mPOP3 = (LCheckBox*) FindPaneByID(paneid_DisconnectPOP3);
	mWait = (LCheckBox*) FindPaneByID(paneid_DisconnectWait);
	mAdbk1 = (LCheckBox*) FindPaneByID(paneid_DisconnectAdbk1);
	mAdbk2 = (LCheckBox*) FindPaneByID(paneid_DisconnectAdbk2);
	mMessagePanel = FindPaneByID(paneid_DisconnectMessage);
	mFull = (LRadioButton*) FindPaneByID(paneid_DisconnectFull);
	mBelow = (LRadioButton*) FindPaneByID(paneid_DisconnectBelow);
	mSize = (CTextFieldX*) FindPaneByID(paneid_DisconnectSize);
	mPartial = (LRadioButton*) FindPaneByID(paneid_DisconnectPartial);
	mPlayProgressPanel = FindPaneByID(paneid_DisconnectPlayProgress);
	mProgress1 = (CBarPane*) FindPaneByID(paneid_DisconnectProgress1);
	mProgress2 = (CBarPane*) FindPaneByID(paneid_DisconnectProgress2);
	mProgressPanel = FindPaneByID(paneid_DisconnectProgress);

	// Check for disconnect
	mMailboxListPanel.FinishCreateSelf(this, mProgressPanel);
	mMailboxListPanel.mMailboxList->AddListener(this);
	mMailboxListPanel.mMailboxList->SetDeleteMsg(msg_DisconnectListDelete);
	mMailboxListPanel.OnCabinet(CMailAccountManager::eFavouriteAutoSync + 1);

	// Force mailbox sets to dissappear
	mMailboxListPanel.OnTwist();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CDisconnectDialogBtns);

}

// Handle OK button
void CDisconnectDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_DisconnectAll:
		case msg_DisconnectNew:
			if (*((long*) ioParam))
				mMessagePanel->Enable();
			break;

		case msg_DisconnectNone:
			if (*((long*) ioParam))
				mMessagePanel->Disable();
			break;

		case msg_DisconnectPermanent:
			if (*((long*) ioParam))
			{
				//mPlayback->SetValue(1);
				//mSend->SetValue(1);
				mMessagePanel->Disable();
				mPOP3->Disable();
				
				ShowProgressPanel(false);
			}
			break;

		case msg_DisconnectUpdate:
			if (*((long*) ioParam))
			{
				//mPlayback->SetValue(0);
				//mSend->SetValue(0);
				mMessagePanel->Enable();
				mPOP3->Enable();
				
				ShowProgressPanel(true);
			}
			break;

		case msg_DisconnectFull:
		case msg_DisconnectPartial:
			if (*((long*) ioParam))
				mSize->Disable();
			break;

		case msg_DisconnectBelow:
			if (*((long*) ioParam))
			{
				mSize->Enable();
				LCommander::SwitchTarget(mSize);
				mSize->SelectAll();
			}
			break;

		case msg_MailboxListTwist:
			mMailboxListPanel.OnTwist();
			break;

		case msg_MailboxListCabinets:
			mMailboxListPanel.OnCabinet(*(long*) ioParam);
			break;

		case msg_MailboxListMailboxAdd:
			mMailboxListPanel.OnAddMailboxList();
			break;

		case msg_MailboxListMailboxClear:
			mMailboxListPanel.OnClearMailboxList();
			break;

		case msg_DisconnectListDelete:
			OnListDelete();
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set the dialogs info
void CDisconnectDialog::SetDetails(bool connecting)
{
	// Get dis/connection options from prefs
	const CConnectionManager::CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.GetValue();
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	StopListening();

	// Disable unwanted options
	if (connecting)
	{
		mDisconnectPanel->Hide();

		mPermanent->SetValue(connect.mStayConnected);
		mUpdate->SetValue(!connect.mStayConnected);
		mPlayback->SetValue(connect.mMboxPlayback);
		mPOP3->SetValue(connect.mUpdatePOP3);
		mSend->SetValue(connect.mSMTPSend);
		
		if (connect.mStayConnected)
		{
			mMessagePanel->Disable();
			mPOP3->Disable();
			
			ShowProgressPanel(false);
		}

		mAdbk1->SetValue(connect.mAdbkPlayback);
		
		cdstring temp;
		temp.FromResource("UI::Disconnect::Connect");
		MyCFString cftemp(temp, kCFStringEncodingUTF8);
		SetCFDescriptor(cftemp);
	}
	else
	{
		mConnectPanel->Hide();

		mAll->SetValue(disconnect.mMboxSync == CConnectionManager::eAllMessages);
		mNew->SetValue(disconnect.mMboxSync == CConnectionManager::eNewMessages);
		mNone->SetValue(disconnect.mMboxSync == CConnectionManager::eNoMessages);
		if (disconnect.mMboxSync == CConnectionManager::eNoMessages)
			mMessagePanel->Disable();

		mList->SetValue(disconnect.mListSync);

		mWait->SetValue(disconnect.mSMTPWait);
		
		mAdbk2->SetValue(disconnect.mAdbkSync);

		cdstring temp;
		temp.FromResource("UI::Disconnect::Disconnect");
		MyCFString cftemp(temp, kCFStringEncodingUTF8);
		SetCFDescriptor(cftemp);
	}
	
	mFull->SetValue(disconnect.mMsgSync == CConnectionManager::eEntireMessage);
	mBelow->SetValue(disconnect.mMsgSync == CConnectionManager::eMessageBelow);
	mPartial->SetValue(disconnect.mMsgSync == CConnectionManager::eFirstDisplayable);
	mSize->SetNumber(disconnect.mMsgSyncSize/1024);
	if (disconnect.mMsgSync != CConnectionManager::eMessageBelow)
		mSize->Disable();

	// Hide address books if no remote accounts
	if (!CPreferences::sPrefs->mAddressAccounts.GetValue().size())
	{
		mAdbk1->Hide();
		mAdbk2->Hide();
	}
	
	// Always hide playback progress
	mPlayProgressPanel->Hide();

	StartListening();
}

// Get the details
void CDisconnectDialog::GetDetailsDisconnect()
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	// Sync if requested
	if (mAll->GetValue())
		disconnect.mMboxSync = CConnectionManager::eAllMessages;

	// Fast if requested
	else if (mNew->GetValue())
		disconnect.mMboxSync = CConnectionManager::eNewMessages;

	// None
	else
		disconnect.mMboxSync = CConnectionManager::eNoMessages;

	// Mailbox list sync
	disconnect.mListSync = mList->GetValue();

	// Wait for sends to complete
	disconnect.mSMTPWait = mWait->GetValue();

	// Message options
	if (mFull->GetValue())
		disconnect.mMsgSync = CConnectionManager::eEntireMessage;
	else if (mBelow->GetValue())
		disconnect.mMsgSync = CConnectionManager::eMessageBelow;
	else
		disconnect.mMsgSync = CConnectionManager::eFirstDisplayable;

	// Get size
	disconnect.mMsgSyncSize = mSize->GetNumber() * 1024;

	// Adbk sync
	disconnect.mAdbkSync = mAdbk2->GetValue();
	
	// Now set up progress items
	if (disconnect.mMboxSync != CConnectionManager::eNoMessages)
	{
		mMailboxListPanel.mFound->SetValue(0);
		mMailboxListPanel.mMessages->SetValue(0);
		mMailboxListPanel.mProgress->SetTotal(mSyncTargets.size());
		mMailboxListPanel.mProgress->Show();
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
	connect.mStayConnected = mPermanent->GetValue();

	// Do playback
	connect.mMboxPlayback = mPlayback->GetValue();

	// POP3 update
	connect.mUpdatePOP3 = mPOP3->GetValue();

	// Enable SMTP
	connect.mSMTPSend = mSend->GetValue();

	// Message options
	if (!connect.mStayConnected)
		disconnect.mMboxSync = CConnectionManager::eNewMessages;
	if (mFull->GetValue())
		disconnect.mMsgSync = CConnectionManager::eEntireMessage;
	else if (mBelow->GetValue())
		disconnect.mMsgSync = CConnectionManager::eMessageBelow;
	else
		disconnect.mMsgSync = CConnectionManager::eFirstDisplayable;

	// Get size
	disconnect.mMsgSyncSize = mSize->GetNumber() * 1024;

	// Adbk sync
	connect.mAdbkPlayback = mAdbk1->GetValue();
	
	// Now switch in progress items
	mMessagePanel->Hide();
	mPlayProgressPanel->Show();
	Draw(NULL);
	
	// Now set up progress items
	if (!connect.mStayConnected)
	{
		mMailboxListPanel.mFound->SetValue(0);
		mMailboxListPanel.mMessages->SetValue(0);
		mMailboxListPanel.mProgress->SetTotal(mSyncTargets.size());
		mMailboxListPanel.mProgress->Show();
	}
	
	// Mark prefs as dirty if needed
	if (!(orig_connect == connect))
		CPreferences::sPrefs->mConnectOptions.SetDirty();
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

// Show or hide the progress panel
void CDisconnectDialog::ShowProgressPanel(bool show)
{
	const int cShrinkOffset = 24;
	if (show)
	{
		SDimension16 size;
		mProgressPanel->GetFrameSize(size);
		mProgressPanel->Show();
		ResizeWindowBy(0, size.height - cShrinkOffset);
	}
	else
	{
		SDimension16 size;
		mProgressPanel->GetFrameSize(size);
		mProgressPanel->Hide();
		ResizeWindowBy(0, -size.height + cShrinkOffset);
	}
}

void CDisconnectDialog::OnListDelete()
{
	// Get selected items
	ulvector selection;
	mMailboxListPanel.mMailboxList->GetSelection(selection);
	
	// Remove from lists
	for(ulvector::reverse_iterator riter = selection.rbegin(); riter != selection.rend(); riter++)
	{
		delete *(mSyncTargets.begin() + *riter);
		mSyncTargets.erase(mSyncTargets.begin() + *riter);
		mSyncTargetHits.erase(mSyncTargetHits.begin() + *riter);
	}
	mMailboxListPanel.ResetMailboxList(mSyncTargets, mSyncTargetHits);
}

void CDisconnectDialog::NextItem(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel.mMailboxList->SetIcon(item + 1, icnx_SearchPointer);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel.mMailboxList->ScrollCellIntoFrame(aCell);
}

void CDisconnectDialog::DoneItem(unsigned long item, bool hit)
{
	// Set hit on previous
	mMailboxListPanel.SetHitState(item, hit, false);
	mMailboxListPanel.SetFound(item + 1);
	mMailboxListPanel.SetProgress(item + 1);
}

void CDisconnectDialog::PoseDialog()
{
	bool disconnecting = CConnectionManager::sConnectionManager.IsConnected();

	// Check to see if any mail accounts are actually marked for disconnect
	if (disconnecting && !CMailAccountManager::sMailAccountManager->HasDisconnectedProtocols())
	{
		// Put up caution alert
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Disconnect::NoDisconnectedCaution") == CErrorHandler::Cancel)
			return;
	}

	{
		// Create the dialog
		CBalloonDialog theHandler(paneid_DisconnectDialog, CMulberryApp::sApp);

		CDisconnectDialog* dlog = (CDisconnectDialog*) theHandler.GetDialog();
		if (sIsTwisted)
			dlog->mMailboxListPanel.mTwister->SetValue(1);
		dlog->SetDetails(!disconnecting);
		theHandler.StartDialog();

		// Let DialogHandler process events
		bool continue_loop = true;
		while (continue_loop)
		{
			MessageT hitMessage = theHandler.DoDialog();

			sIsTwisted = dlog->mMailboxListPanel.mTwisted;

			switch(hitMessage)
			{
			case msg_OK:
				dlog->mOKBtn->Disable();
				dlog->mCancelBtn->Disable();
				dlog->mMailboxListPanel.SetInProgress(true);

				if (disconnecting)
				{
					dlog->GetDetailsDisconnect();
					
					// Do sync here while dialog is still open
					CConnectionManager::sConnectionManager.Disconnect(false, false, NULL);
				}
				else
				{
					// Preserve disconnect options
					CConnectionManager::CDisconnectOptions disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
					
					dlog->GetDetailsConnect();
					
					// Do sync here while dialog is still open
					CConnectionManager::sConnectionManager.Connect(dlog->mProgress1, dlog->mProgress2);

					// Recover disconnect options
					CPreferences::sPrefs->mDisconnectOptions.Value() = disconnect;
				}
				continue_loop = false;
				break;
			case msg_Cancel:
				return;
			}
		}
	}

	CPreferences::sPrefs->mDisconnected.SetValue(!CConnectionManager::sConnectionManager.IsConnected());

	// Check to see whether there are messages available and force queue window on screen
	if (CSMTPAccountManager::sSMTPAccountManager->ItemsHeld())
		CSMTPWindow::OpenSMTPWindow();

}
