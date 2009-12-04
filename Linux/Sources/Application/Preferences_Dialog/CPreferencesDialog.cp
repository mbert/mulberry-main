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


// CPreferencesDialog.cpp : implementation file
//


#include "CPreferencesDialog.h"

#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CCalendarView.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CIdentityPopup.h"
#include "CLetterWindow.h"
#include "CMailboxWindow.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPluginManager.h"
#include "CPreferencesFile.h"
#include "CPreferenceKeys.h"
#include "CRemotePrefsSets.h"
#include "CResizeNotifier.h"
#include "CRulesWindow.h"
#include "CServerView.h"
#include "CServerWindow.h"
#include "CStatusWindow.h"
#include "CTextListChoice.h"
#include "CTextTable.h"

#include "CPrefsSimple.h"
#include "CPrefsAccount.h"
#include "CPrefsAlerts.h"
#include "CPrefsDisplay.h"
#include "CPrefsFormatting.h"
#include "CPrefsMailbox.h"
#include "CPrefsMessage.h"
#include "CPrefsLetter.h"
#include "CPrefsSecurity.h"
#include "CPrefsIdentities.h"
#include "CPrefsAddress.h"
#include "CPrefsCalendar.h"
#include "CPrefsAttachments.h"
#include "CPrefsSpelling.h"

#include "JXIconTextButton.h"
#include "JXSecondaryRadioGroup.h"

#include <JXCardFile.h>
#include <JXChooseSaveFile.h>
#include <JXDisplay.h>
#include <JXTextButton.h>
#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextRadioButton.h>
#include <JXColormap.h>
#include <jXGlobals.h>

const char* cPanelNames[] = {"", "Account", "Alerts", "Display", "Styled", "Mailbox",
							"Message", "Letter", "Security", "Identities", "Address",
							"Calendar", "Attachments", "Spelling", NULL};

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

CPreferencesDialog* CPreferencesDialog::sPrefsDlog = NULL;

CPreferencesDialog::CPreferencesDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue),
	  mCopyPrefs(*CPreferences::sPrefs)
{
	mCurrentPrefsFile = NULL;
	mLoadedNewPrefs = false;
	mWindowsReset = false;
	mAccountNew = false;
	mAccountRename = false;
	mLocal = true;

	mCurrentCard = 0;

	sPrefsDlog = this;
}

void CPreferencesDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 650,430, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 650,430);
    assert( container != NULL );

    mPrefsButtonsGroup =
        new JXRadioGroup(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,10, 105,335);
    assert( mPrefsButtonsGroup != NULL );
    mPrefsButtonsGroup->SetBorderWidth(0);

    mAccountsBtn =
        new JXIconTextButton(1, "Accounts", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 95,25);
    assert( mAccountsBtn != NULL );

    mAlertsBtn =
        new JXIconTextButton(2, "Alerts", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,30, 95,25);
    assert( mAlertsBtn != NULL );

    mDisplayBtn =
        new JXIconTextButton(3, "Display", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,55, 95,25);
    assert( mDisplayBtn != NULL );

    mFontsBtn =
        new JXIconTextButton(4, "Fonts", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,80, 95,25);
    assert( mFontsBtn != NULL );

    mMailboxBtn =
        new JXIconTextButton(5, "Mailbox", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,105, 95,25);
    assert( mMailboxBtn != NULL );

    mMessageBtn =
        new JXIconTextButton(6, "Message", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,130, 95,25);
    assert( mMessageBtn != NULL );

    mOutgoingBtn =
        new JXIconTextButton(7, "Outgoing", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,155, 95,25);
    assert( mOutgoingBtn != NULL );

    mSecurityBtn =
        new JXIconTextButton(8, "Security", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,180, 95,25);
    assert( mSecurityBtn != NULL );

    mIdentitiesBtn =
        new JXIconTextButton(9, "Identities", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,205, 95,25);
    assert( mIdentitiesBtn != NULL );

    mAddressesBtn =
        new JXIconTextButton(10, "Addresses", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,230, 95,25);
    assert( mAddressesBtn != NULL );

    mCalendarBtn =
        new JXIconTextButton(11, "Calendar", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,255, 95,25);
    assert( mCalendarBtn != NULL );

    mAttachmentBtn =
        new JXIconTextButton(12, "Attachment", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,280, 95,25);
    assert( mAttachmentBtn != NULL );

    mSpellingBtn =
        new JXIconTextButton(13, "Spelling", mPrefsButtonsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,305, 95,25);
    assert( mSpellingBtn != NULL );

    JXRadioGroup* obj1 =
        new JXRadioGroup(container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 115,10, 410,410);
    assert( obj1 != NULL );

    mCards =
        new JXCardFile(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 400,400);
    assert( mCards != NULL );

    mPrefsGroup =
        new JXSecondaryRadioGroup(container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 530,60, 110,50);
    assert( mPrefsGroup != NULL );

    mPrefsSimpleBtn =
        new JXTextRadioButton(1, "Simple", mPrefsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 90,20);
    assert( mPrefsSimpleBtn != NULL );

    mPrefsAdvancedBtn =
        new JXTextRadioButton(2, "Advanced", mPrefsGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,25, 90,20);
    assert( mPrefsAdvancedBtn != NULL );

    mStorageGroup =
        new JXSecondaryRadioGroup(container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 530,150, 110,150);
    assert( mStorageGroup != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Storage", container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 535,140, 55,20);
    assert( obj2 != NULL );

    mPrefsLocalCaption =
        new JXStaticText("Local", mStorageGroup,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 23,20, 70,20);
    assert( mPrefsLocalCaption != NULL );

    mPrefsRemoteCaption =
        new JXStaticText("Remote", mStorageGroup,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 23,20, 70,20);
    assert( mPrefsRemoteCaption != NULL );

    mPrefsLocalBtn =
        new JXTextRadioButton(1, "Local", mStorageGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,10, 90,20);
    assert( mPrefsLocalBtn != NULL );

    mPrefsRemoteBtn =
        new JXTextRadioButton(2, "Remote", mStorageGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,30, 90,20);
    assert( mPrefsRemoteBtn != NULL );

    mPrefsOpenBtn =
        new JXTextButton("Open", mStorageGroup,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 10,55, 90,25);
    assert( mPrefsOpenBtn != NULL );

    mPrefsSaveAsBtn =
        new JXTextButton("Save As...", mStorageGroup,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 10,85, 90,25);
    assert( mPrefsSaveAsBtn != NULL );

    mPrefsSaveDefaultBtn =
        new JXTextButton("Save Default", mStorageGroup,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 10,115, 90,25);
    assert( mPrefsSaveDefaultBtn != NULL );

    mOkBtn =
        new JXTextButton("OK", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 545,380, 80,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 545,340, 80,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	SetButtons(mOkBtn, mCancelBtn);

	// Set icons
	mAccountsBtn->SetImage(IDI_PREFS_ACCOUNTS);
	mAlertsBtn->SetImage(IDI_PREFS_ALERTS);
	mDisplayBtn->SetImage(IDI_PREFS_DISPLAY);
	mFontsBtn->SetImage(IDI_PREFS_FONTS);
	mMailboxBtn->SetImage(IDI_PREFS_MAILBOX);
	mMessageBtn->SetImage(IDI_PREFS_MESSAGE);
	mOutgoingBtn->SetImage(IDI_PREFS_OUTGOING);
	mSecurityBtn->SetImage(IDI_PREFS_SECURITY);
	mIdentitiesBtn->SetImage(IDI_PREFS_IDENTITY);
	mAddressesBtn->SetImage(IDI_PREFS_ADDRESSES);
	mCalendarBtn->SetImage(IDI_PREFS_CALENDAR);
	mAttachmentBtn->SetImage(IDI_PREFS_ATTACHMENT);
	mSpellingBtn->SetImage(IDI_PREFS_SPELLING);
	
	// Simple
	AddPanel(new CPrefsSimple(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[0]);

	// Accounts
	AddPanel(new CPrefsAccount(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[1]);

	// Alerts
	AddPanel(new CPrefsAlerts(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[2]);

	// Display
	AddPanel(new CPrefsDisplay(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[3]);

	// Fonts
	AddPanel(new CPrefsFormatting(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[4]);

	// Mailboxes
	AddPanel(new CPrefsMailbox(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[5]);

	// Messages
	AddPanel(new CPrefsMessage(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[6]);

	// Outgoing
	AddPanel(new CPrefsLetter(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[7]);

	// Security
	if (CPluginManager::sPluginManager.HasSecurity())
	{
		AddPanel(new CPrefsSecurity(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
		mPanelNames.push_back(cPanelNames[8]);
	}

	// Identity
	AddPanel(new CPrefsIdentities(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[9]);

	// Addresses
	AddPanel(new CPrefsAddress(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[10]);

	// Calendar
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		AddPanel(new CPrefsCalendar(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
		mPanelNames.push_back(cPanelNames[11]);
	}

	// Attachment
	AddPanel(new CPrefsAttachments(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
	mPanelNames.push_back(cPanelNames[12]);

	// Spelling
	if (CPluginManager::sPluginManager.HasSpelling())
	{
		AddPanel(new CPrefsSpelling(mCards, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 400, 400));
		mPanelNames.push_back(cPanelNames[13]);
	}

	// Set to intial panel
	static_cast<CPrefsPanel*>(mCards->GetCard(1))->AboutToShow();
	mCards->ShowCard(1);

	// Set prefs file to current app file and change title
	mCurrentPrefsFile = CMulberryApp::sCurrentPrefsFile;
	SetWindowTitle();

	if (CAdminLock::sAdminLock.mNoLocalPrefs)
	{
		mPrefsLocalBtn->Deactivate();
		mStorageGroup->SelectItem(2);
		mLocal = false;
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mPrefsRemoteBtn->Deactivate();
		mStorageGroup->SelectItem(1);
		mLocal = true;
	}
	if (mCurrentPrefsFile)
	{
		mStorageGroup->SelectItem(mCurrentPrefsFile->IsRemote() ? 2 : 1);
		mLocal = !mCurrentPrefsFile->IsRemote();
	}
	mPrefsLocalCaption->Hide();
	mPrefsRemoteCaption->Hide();

	// Hide default button if not allowed
	if (!CAdminLock::sAdminLock.mAllowDefault && mPrefsLocalBtn->IsChecked())
		mPrefsSaveDefaultBtn->Deactivate();

	// Disable Save and open buttons button if not allowed
	if (CAdminLock::sAdminLock.mNoLocalPrefs && CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mPrefsOpenBtn->Deactivate();
		mPrefsSaveAsBtn->Deactivate();
	}

	// Remove unwanted prefs panels by hiding buttons
	if (!CPluginManager::sPluginManager.HasSecurity())
	{
		mSecurityBtn->Hide();
		JCoordinate moveby = mSecurityBtn->GetFrameHeight();

		mIdentitiesBtn->Move(0, -moveby);
		mIdentitiesBtn->SetID(mIdentitiesBtn->GetID() - 1);
		mAddressesBtn->Move(0, -moveby);
		mAddressesBtn->SetID(mAddressesBtn->GetID() - 1);
		mCalendarBtn->Move(0, -moveby);
		mCalendarBtn->SetID(mCalendarBtn->GetID() - 1);
		mAttachmentBtn->Move(0, -moveby);
		mAttachmentBtn->SetID(mAttachmentBtn->GetID() - 1);
		mSpellingBtn->Move(0, -moveby);
		mSpellingBtn->SetID(mSpellingBtn->GetID() - 1);
	}

	// Remove unwanted prefs panels by hiding buttons
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		mCalendarBtn->Hide();
		JCoordinate moveby = mSecurityBtn->GetFrameHeight();

		mAttachmentBtn->Move(0, -moveby);
		mAttachmentBtn->SetID(mAttachmentBtn->GetID() - 1);
		mSpellingBtn->Move(0, -moveby);
		mSpellingBtn->SetID(mSpellingBtn->GetID() - 1);
	}

	// Remove unwanted prefs panels by hiding buttons
	if (!CPluginManager::sPluginManager.HasSpelling())
	{
		mSpellingBtn->Hide();
	}

	// Now check simple/advanced state
	SetSimple(mCopyPrefs.mSimple.GetValue(), true);
	mPrefsGroup->SelectItem(mCopyPrefs.mSimple.GetValue() ? 1 : 2);

	// Now start listening AFTER the intial item setup
	ListenTo(container);
	ListenTo(mPrefsButtonsGroup);
	ListenTo(mPrefsGroup);
	ListenTo(mStorageGroup);
	ListenTo(mPrefsOpenBtn);
	ListenTo(mPrefsSaveAsBtn);
	ListenTo(mPrefsSaveDefaultBtn);
}

void CPreferencesDialog::AddPanel(CPrefsPanel* panel)
{
	panel->OnCreate();
	panel->SetPrefs(&mCopyPrefs);
	mCards->AppendCard(panel);
}

// Set input panel
void CPreferencesDialog::SetWindowTitle(void)
{
	// Load string title
	cdstring title;
	title.FromResource("UI::Preferences::Title");

	// If no current file say no file or default if allowed
	if (!mCurrentPrefsFile)
	{
		cdstring subtitle;
		subtitle.FromResource(CAdminLock::sAdminLock.mAllowDefault ? "UI::Preferences::DefaultTitle" : "UI::Preferences::NoTitle");
		title += subtitle;
	}

	// Otherwise just get its name
	else
	{
		// If remote replace name
		if (mCurrentPrefsFile->IsRemote())
			title.FromResource("UI::Preferences::RemoteTitle");

		title += (char *)(mCurrentPrefsFile->GetName());
	}

	// Now set window title
	GetWindow()->SetTitle(title);
}

void CPreferencesDialog::Receive(JBroadcaster*	sender, const Message&	message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mPrefsButtonsGroup)
		{
			SetPrefsPanel(index + 1);
			return;
		}
		else if (sender == mPrefsGroup)
		{
			SetSimple(index == 1);
			return;
		}
		else if (sender == mStorageGroup)
		{
			switch(index)
			{
			case 1:
				OnPrefsLocalBtn();
				break;
			case 2:
				OnPrefsRemoteBtn();
				break;
			}
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mPrefsOpenBtn)
		{
			OnPrefsOpenBtn();
			return;
		}
		else if (sender == mPrefsSaveAsBtn)
		{
			OnPrefsSaveAsBtn();
			return;
		}
		else if (sender == mPrefsSaveDefaultBtn)
		{
			OnPrefsSaveDefaultBtn();
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

void CPreferencesDialog::ResetPrefs(void)
{
	// Do data exchange
	static_cast<CPrefsPanel*>(mCards->GetCurrentCard())->ResetPrefs(&mCopyPrefs);
}

// Update prefs from panel
void CPreferencesDialog::UpdateCopyPrefs(void)
{
	// Get prefs from the old panel
	if (mCurrentCard)
		static_cast<CPrefsPanel*>(mCards->GetCurrentCard())->UpdatePrefs(&mCopyPrefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

// Set input panel
void CPreferencesDialog::SetPrefsPanel(JIndex panel)
{
	// Only if different
	if (panel != mCurrentCard)
	{
		// Get prefs from the old panel
		UpdateCopyPrefs();

		// Switch to new one
		static_cast<CPrefsPanel*>(mCards->GetCard(panel))->AboutToShow();
		mCards->ShowCard(panel);
		mCurrentCard = panel;

		// Set prefs in the new panel
		ResetPrefs();
	}
}

// Set to simple state
void CPreferencesDialog::SetSimple(bool simple, bool init)
{
	JCoordinate cStorageAdjust = mPrefsButtonsGroup->GetBoundsWidth();

	if (simple)
	{
		// Remember the current panel
		UpdateLastPanel();

		// Hide items
		mPrefsButtonsGroup->Hide();
		mPrefsLocalBtn->Hide();
		mPrefsRemoteBtn->Hide();
		if (mPrefsLocalBtn->IsChecked())
			mPrefsLocalCaption->Show();
		else
			mPrefsRemoteCaption->Show();

		// Reduce width of prefs dialog
		AdjustSize(-cStorageAdjust, 0);

		// Now change to simple panel
		SetPrefsPanel(1);
	}
	else
	{
		// Increase width of prefs dialog
		if (!init)
		{
			AdjustSize(cStorageAdjust, 0);
		}

		// Show items
		mPrefsButtonsGroup->Show();
		mPrefsLocalCaption->Hide();
		mPrefsRemoteCaption->Hide();
		mPrefsLocalBtn->Show();
		mPrefsRemoteBtn->Show();

		// Now change to advanced panel
		SetLastPanel();
	}

	// Now set prefs value
	mCopyPrefs.mSimple.SetValue(simple);
}

JBoolean CPreferencesDialog::OKToDeactivate()
{
	if (Cancelled())
	{
		OnCancel();
		return kTrue;
	}
	else
		return JBoolean(OnOK());
}

bool CPreferencesDialog::OnOK()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before exit
	if (!mCopyPrefs.Valid(true))
		return false;

	// Update current panel name
	UpdateLastPanel();
	
	// Do OK action
	return true;
}

void CPreferencesDialog::UpdateLastPanel()
{
	// Get name of current panel if not simple
	if (mCurrentCard > 1)
	{
		mCopyPrefs.mLastPanel.SetValue(mPanelNames[mCurrentCard - 1]);
		CPreferences::sPrefs->mLastPanel.SetValue(mPanelNames[mCurrentCard - 1]);
	}
}

void CPreferencesDialog::SetLastPanel()
{
	int panel = 1;
	if (!mCopyPrefs.mLastPanel.GetValue().empty())
	{
		for(cdstrvect::const_iterator iter = mPanelNames.begin(); iter != mPanelNames.end(); iter++)
		{
			if (mCopyPrefs.mLastPanel.GetValue() == *iter)
			{
				panel = iter - mPanelNames.begin();
				break;
			}
		}
	}

	// Force change to new card directly
	// Radio group won't broadcast if there is no change
	SetListening(kFalse);
	mPrefsButtonsGroup->SelectItem(panel);
	SetListening(kTrue);
	SetPrefsPanel(panel + 1);
}

void CPreferencesDialog::UpdatePrefs()
{
	// Make a copy of the prefs to copy
	CPreferences* newPrefs = new CPreferences(mCopyPrefs);

	// Always save existing current file
	if (mCurrentPrefsFile)
	{
		// Save new prefs
		mCurrentPrefsFile->SetPrefs(newPrefs);
		mCurrentPrefsFile->SavePrefs(true, false);
	}

	// Try to create default prefs file
	else if (CAdminLock::sAdminLock.mAllowDefault)
	{
		// Create new default prefs file
		mCurrentPrefsFile = new CPreferencesFile(true, false);
		mCurrentPrefsFile->CreateDefault();

		// Save new prefs
		mCurrentPrefsFile->SetPrefs(newPrefs);
		mCurrentPrefsFile->SavePrefs(true, false);
	}

	// Try to save in new file
	else if (!CAdminLock::sAdminLock.mNoLocalPrefs || !CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		// Create new file to save as
		mCurrentPrefsFile = new CPreferencesFile(false, false);
		if (mLocal ? !PrefsLocalSaveAs() : !PrefsRemoteSaveAs())
		{
			// Free unwanted objects and cancel OK action
			delete mCurrentPrefsFile;
			mCurrentPrefsFile = NULL;
			delete newPrefs;

			return;
		}
	}

	// If current file is not previous
	if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
	{
		// Delete old prefs file
		delete CMulberryApp::sCurrentPrefsFile;

		// Make apps current file the new one
		CMulberryApp::sCurrentPrefsFile = mCurrentPrefsFile;
	}

	// Make changes based on prefs - will reset default
	MakeChanges(newPrefs);

	// Reset cabinets if required
	if (mAccountNew && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->UpdateFavourites();

	// Reset windows if required
	if (mWindowsReset)
	{
		// Do status window
		if (CStatusWindow::sStatusWindow)
			CStatusWindow::sStatusWindow->ResetState();

		// Do each server window
		{
			cdmutexprotect<CServerWindow::CServerWindowList>::lock _lock(CServerWindow::sServerWindows);
			for(CServerWindow::CServerWindowList::iterator iter = CServerWindow::sServerWindows->begin(); iter != CServerWindow::sServerWindows->end(); iter++)
				(*iter)->ResetState();
		}

		// Do each mailbox window
		{
			cdmutexprotect<CMailboxWindow::CMailboxWindowList>::lock _lock(CMailboxWindow::sMboxWindows);
			for(CMailboxWindow::CMailboxWindowList::iterator iter = CMailboxWindow::sMboxWindows->begin(); iter != CMailboxWindow::sMboxWindows->end(); iter++)
				(*iter)->ResetState();
		}

		// Do each address book window
		{
			cdmutexprotect<CAddressBookWindow::CAddressBookWindowList>::lock _lock(CAddressBookWindow::sAddressBookWindows);
			for(CAddressBookWindow::CAddressBookWindowList::iterator iter = CAddressBookWindow::sAddressBookWindows->begin(); iter != CAddressBookWindow::sAddressBookWindows->end(); iter++)
				(*iter)->ResetState();
		}

		// Address book manager window
		if (CAdbkManagerWindow::sAdbkManager)
			CAdbkManagerWindow::sAdbkManager->ResetState();

		// Address book search window
		if (CAdbkSearchWindow::sAdbkSearch)
			CAdbkSearchWindow::sAdbkSearch->ResetState();
		
		// Calendar manager window
		if (CCalendarStoreWindow::sCalendarStoreManager)
			CCalendarStoreWindow::sCalendarStoreManager->ResetState();

		// Do each calendar window
		{
			cdmutexprotect<CCalendarWindow::CCalendarWindowList>::lock _lock(CCalendarWindow::sCalendarWindows);
			for(CCalendarWindow::CCalendarWindowList::iterator iter = CCalendarWindow::sCalendarWindows->begin(); iter != CCalendarWindow::sCalendarWindows->end(); iter++)
				(*iter)->ResetState();
		}
	}
}

void CPreferencesDialog::OnCancel()
{
	// Delete prefs file object if not same as existing
	if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
		delete mCurrentPrefsFile;
	mCurrentPrefsFile = NULL;

	// Do menu list reset if it changed
	if (mForceMenuListReset)
	{

		// Force mailbox menu reset
		CCopyToMenu::SetMenuOptions(CPreferences::sPrefs->mUseCopyToMboxes.GetValue(), CPreferences::sPrefs->mUseAppendToMboxes.GetValue());

		CCopyToMenu::DirtyMenuList();
	}

	// Update last panel now
	UpdateLastPanel();
}

// Check for and act on change
void CPreferencesDialog::MakeChanges(CPreferences* newPrefs)
{
	bool refresh_server = false;
	bool refresh_mailbox = false;
	bool refresh_message = false;
	bool refresh_letter = false;
	bool refresh_adbkmanager = false;
	bool refresh_adbksearch = false;
	bool refresh_addressbook = false;
	bool refresh_calendar = false;
	bool refresh_rules = false;
	bool refresh_mrus = false;

	// Check for change of style or colour in mailbox window
	if ((CPreferences::sPrefs->mServerOpenStyle.GetValue() != newPrefs->mServerOpenStyle.GetValue()) ||
		(CPreferences::sPrefs->mServerClosedStyle.GetValue() != newPrefs->mServerClosedStyle.GetValue()) ||
		(CPreferences::sPrefs->mServerBkgndStyle.GetValue() != newPrefs->mServerBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mFavouriteBkgndStyle.GetValue() != newPrefs->mFavouriteBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue() != newPrefs->mHierarchyBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxRecentStyle.GetValue() != newPrefs->mMboxRecentStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxUnseenStyle.GetValue() != newPrefs->mMboxUnseenStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxOpenStyle.GetValue() != newPrefs->mMboxOpenStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxFavouriteStyle.GetValue() != newPrefs->mMboxFavouriteStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxClosedStyle.GetValue() != newPrefs->mMboxClosedStyle.GetValue()))
	{
		refresh_server = true;
	}

	// Check for change of style or colour
	if ((CPreferences::sPrefs->unseen.GetValue() != newPrefs->unseen.GetValue()) ||
		(CPreferences::sPrefs->seen.GetValue() != newPrefs->seen.GetValue()) ||
		(CPreferences::sPrefs->answered.GetValue() != newPrefs->answered.GetValue()) ||
		(CPreferences::sPrefs->important.GetValue() != newPrefs->important.GetValue()) ||
		(CPreferences::sPrefs->deleted.GetValue() != newPrefs->deleted.GetValue()) ||
		(CPreferences::sPrefs->mMultiAddress.GetValue() != newPrefs->mMultiAddress.GetValue()))
	{
		refresh_mailbox = true;
	}

#ifndef USE_FONTMAPPER
	if (CPreferences::sPrefs->mListTextFontInfo.GetValue() != newPrefs->mListTextFontInfo.GetValue())
#else
	if (CPreferences::sPrefs->mListFontMap.GetValue() != newPrefs->mListFontMap.GetValue())
#endif
	{	// Always force change as Font object must be replaced
		refresh_server = true;
		refresh_mailbox = true;
		refresh_message = true;
		refresh_letter = true;
		refresh_adbkmanager = true;
		refresh_adbksearch = true;
		refresh_addressbook = true;
		refresh_calendar = true;
		refresh_rules = true;
	}

	// Always change font as object is static in prefs
#ifndef USE_FONTMAPPER
	if (CPreferences::sPrefs->mDisplayTextFontInfo.GetValue() != newPrefs->mDisplayTextFontInfo.GetValue())
#else
	if (CPreferences::sPrefs->mDisplayFontMap.GetValue() != newPrefs->mDisplayFontMap.GetValue())
#endif
	{
		refresh_message = true;
		refresh_letter = true;
	}

	// Check for change of smart address filters
	bool smart_change = (CPreferences::sPrefs->mSmartAddressList.GetValue().size() != newPrefs->mSmartAddressList.GetValue().size());
	if (!smart_change)
	{
		for(unsigned long i = 0; i < CPreferences::sPrefs->mSmartAddressList.GetValue().size(); i++)
		{
			if (CPreferences::sPrefs->mSmartAddressList.GetValue()[i] != newPrefs->mSmartAddressList.GetValue()[i])
			{
				smart_change = true;
				break;
			}
		}
	}

	// Always resync identities
	if (true)
	{
		// Iterate over all letter windows
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			(*iter)->SyncIdentities(newPrefs->mIdentities.GetValue());
	}
	
	// Check for change in MRU number
	if (CMailAccountManager::sMailAccountManager &&
		(CPreferences::sPrefs->mMRUMaximum.GetValue() != newPrefs->mMRUMaximum.GetValue()))
		refresh_mrus = true;

	// Replace old prefs with copy of new
	newPrefs->SetAsDefault();

	// Now do window refreshing AFTER setting the default prefs to the new set
	// as some of the windows will use the default prefs values

	if (refresh_server)
	{
		// Iterate over all server windows
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::iterator iter = CServerView::sServerViews->begin(); iter != CServerView::sServerViews->end(); iter++)
		{
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
		}
	}

	if (refresh_mailbox)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	if (refresh_message)
	{
		// NB IMPORTANT: message window and message views are separate right now
		// Eventually the message window will contain the message view so only the view
		// will need to be updated

		// Iterate over all message windows
		{
			cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
			for(CMessageWindow::CMessageWindowList::iterator iter = CMessageWindow::sMsgWindows->begin(); iter != CMessageWindow::sMsgWindows->end(); iter++)
#ifndef USE_FONTMAPPER
				(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue(), newPrefs->mDisplayTextFontInfo.GetValue());
#else
				(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits,
										newPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
		}

		// Iterate over all message views
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin(); iter != CMessageView::sMsgViews->end(); iter++)
#ifndef USE_FONTMAPPER
				(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue(), newPrefs->mDisplayTextFontInfo.GetValue());
#else
				(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits,
										newPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
		}
	}

	if (refresh_letter)
	{
		// Iterate over all letter windows
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue(), newPrefs->mDisplayTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits,
										newPrefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	if (refresh_adbkmanager)
	{
		// Iterate over all adbk manager windows
		cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
		for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	if (refresh_adbksearch && CAdbkSearchWindow::sAdbkSearch)
#ifndef USE_FONTMAPPER
		CAdbkSearchWindow::sAdbkSearch->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
		CAdbkSearchWindow::sAdbkSearch->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif

	if (refresh_addressbook)
	{
		// Iterate over all address books
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin(); iter != CAddressBookView::sAddressBookViews->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	if (refresh_calendar)
	{
		// Iterate over all calendars windows
		cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList>::lock _lock1(CCalendarStoreView::sCalendarStoreViews);
		for(CCalendarStoreView::CCalendarStoreViewList::iterator iter = CCalendarStoreView::sCalendarStoreViews->begin(); iter != CCalendarStoreView::sCalendarStoreViews->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif

		// Iterate over all calendars windows
		cdmutexprotect<CCalendarView::CCalendarViewList>::lock _lock2(CCalendarView::sCalendarViews);
		for(CCalendarView::CCalendarViewList::iterator iter = CCalendarView::sCalendarViews->begin(); iter != CCalendarView::sCalendarViews->end(); iter++)
#ifndef USE_FONTMAPPER
			(*iter)->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			(*iter)->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	if (CRulesWindow::sRulesWindow)
	{
		// Always reset table after prefs change
		CRulesWindow::sRulesWindow->ResetTable();

		// Reset font if that has changed
		if (refresh_rules)
#ifndef USE_FONTMAPPER
			CRulesWindow::sRulesWindow->ResetFont(newPrefs->mListTextFontInfo.GetValue());
#else
			CRulesWindow::sRulesWindow->ResetFont(newPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits);
#endif
	}

	// Check for change in MRU number
	if (refresh_mrus && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->CleanMRUHistory(newPrefs->mMRUMaximum.GetValue());

	if (smart_change)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			(*iter)->RefreshSmartAddress();
	}

}

#pragma mark ____________________________Storage Related

void CPreferencesDialog::OnPrefsLocalBtn()
{
	// Only if not local
	if (!mLocal)
	{
		mLocal = true;
		if (CAdminLock::sAdminLock.mAllowDefault)
			mPrefsSaveDefaultBtn->Activate();
		else
			mPrefsSaveDefaultBtn->Deactivate();
		
		// Change state of prefs file
		if (mCurrentPrefsFile)
			mCurrentPrefsFile->SetRemote(false);
	}
}

void CPreferencesDialog::OnPrefsRemoteBtn()
{
	// Only if not already
	if (!mLocal)
		return;

	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	if (!CAdminLock::sAdminLock.mAllowDefault)
		mPrefsSaveDefaultBtn->Activate();

	// Must have valid remote IP
	if (!mCopyPrefs.mRemoteAccounts.GetValue().size())
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoRemoteAccount");

		// Reset buttons
		mStorageGroup->SelectItem(1);

		// Force to remote account panel
		mPrefsButtonsGroup->SelectItem(1);
	}
	else if (mCopyPrefs.mRemoteAccounts.GetValue().front()->GetServerIP().empty())
	{
		const cdstring& name = mCopyPrefs.mRemoteAccounts.GetValue().front()->GetName();
		CErrorHandler::PutStopAlertRsrcStr("Alerts::Preferences::Invalid_RemoteServerIP", name);
		
		// Reset buttons
		mStorageGroup->SelectItem(1);

		// Force to remote account panel
		mPrefsButtonsGroup->SelectItem(1);
	}
	else if (!CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		// Reset buttons
		mStorageGroup->SelectItem(1);
	}
	else
	{
		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();

		mLocal = false;
		mPrefsSaveDefaultBtn->Activate();
	}

	// Change state of prefs file
	if (mCurrentPrefsFile && !mLocal)
		mCurrentPrefsFile->SetRemote(true);
}

void CPreferencesDialog::OnPrefsOpenBtn()
{
	bool opened = false;

	if (mLocal)
		opened = PrefsLocalOpen();
	else
		opened = PrefsRemoteOpen();
	
	// Must force reset of mailbox menu lists and cabinet contents
	if (opened)
	{
		// Indicate new prefs
		SetLoadedNewPrefs(true);
		
		// Reset prefs changed flags
		SetAccountNew(false);
		SetAccountRename(false);
		GetRenames().clear();
		SetForceWindowReset(false);

		// Mark popups as dirty for new prefs set
		CCopyToMenu::DirtyMenuList();
		SetForceMenuListReset(true);
	}
}

void CPreferencesDialog::OnPrefsSaveAsBtn()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Allow save of invalid prefs
	if (mLocal)
		PrefsLocalSaveAs();
	else
		PrefsRemoteSaveAs();
}

void CPreferencesDialog::OnPrefsSaveDefaultBtn()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before save
	if (!mCopyPrefs.Valid(true))
		return;

	if (mLocal)
		PrefsLocalSaveDefault();
	else
		PrefsRemoteSaveDefault();
}

bool CPreferencesDialog::PrefsLocalOpen()
{
	// Do standard open dialog
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Preferences to open:", NULL, &fname))
	{
		// Save copy of existing preferences
		CPreferences temp(mCopyPrefs);

		// Create file object
		CPreferencesFile* new_file = new CPreferencesFile(false, false);

		// Set prefs file to new file and copy prefs
		new_file->SetName(fname.GetCString());
		new_file->SetSpecified(true);
		new_file->SetPrefs(&mCopyPrefs);

		// Read in new prefs (open as local do not resolve to remote)
		if (new_file->VerifyRead(false))
		{
			// Delete prefs file object if not same as existing
			if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
				delete mCurrentPrefsFile;
			mCurrentPrefsFile = new_file;

			// Need to sync new prefs simple/advanced state with old
			mCopyPrefs.mSimple.SetValue(temp.mSimple.GetValue());

			// Set new prefs values in all panels
			ResetPrefs();

			// Reset title
			SetWindowTitle();
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			return false;
		}

		return true;
	}
	
	return false;
}

bool CPreferencesDialog::PrefsLocalSaveAs()
{
	JString saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
		saveAsName = (const char*) mCurrentPrefsFile->GetName();

	else
	{
		// Set default name
		cdstring temp;
		temp.FromResource("UI::Preferences::DefaultPrefsFileName");
		saveAsName = temp;
	}

	// Do standard save as dialog
	if (JXGetChooseSaveFile()->SaveFile("Preferences File", NULL, saveAsName, &saveAsName))
	{

		// Reset prefs file object if same as existing
		if (mCurrentPrefsFile == CMulberryApp::sCurrentPrefsFile)
			mCurrentPrefsFile = NULL;

		// Create file object if required
		if (!mCurrentPrefsFile)
			mCurrentPrefsFile = new CPreferencesFile(false, false);

		// Save current prefs in file
		mCurrentPrefsFile->SetRemote(false);
		mCurrentPrefsFile->SetPrefs(&mCopyPrefs);
		mCurrentPrefsFile->DoSaveAs(saveAsName.GetCString());
		mCurrentPrefsFile->SetSpecified(true);

		// Reset window title
		SetWindowTitle();

		return true;
	}
	else
		return false;
}

void CPreferencesDialog::PrefsLocalSaveDefault()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before save
	if (!mCopyPrefs.Valid(true))
		return;

	// Create new System Folder prefs file
	CPreferencesFile* temp = new CPreferencesFile(true, false);
	temp->CreateDefault();
	
	temp->SetName(CPreferencesFile::GetDefaultLocalFileName());
	// Save current prefs in System Folder
	temp->SetPrefs(&mCopyPrefs);
	temp->SavePrefs(false, true);

	// Finished with file
	delete temp;
}

bool CPreferencesDialog::PrefsRemoteOpen()
{
	// Make sure set is read in
	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
	else
		return false;

	bool result = false;
	cdstrvect items;
	// Add default name first
	items.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		items.push_back(*iter);

	// Let Dialog process events
	ulvector selected;
	if (CTextListChoice::PoseDialog("Alerts::Preferences::OpenRemoteTitle", "Alerts::Preferences::OpenRemoteDesc", NULL, false, true, false, true, items, cdstring::null_str, selected, "Alerts::Preferences::OpenRemoteButton"))
	{
		// Get selection from list
		cdstring remote = items.at(selected.front());

		// Save copy of existing preferences
		CPreferences temp(mCopyPrefs);

		// Create file object
		CPreferencesFile* new_file = new CPreferencesFile(false, true);

		// Set prefs file to new file and copy prefs
		new_file->SetName(remote);
		new_file->SetSpecified(false);
		new_file->SetPrefs(&mCopyPrefs);

		// Read in new prefs
		if (new_file->VerifyRead(false))
		{
			// Delete prefs file object if not same as existing
			if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
				delete mCurrentPrefsFile;
			mCurrentPrefsFile = new_file;

			// Need to sync new prefs simple/advanced state with old
			mCopyPrefs.mSimple.SetValue(temp.mSimple.GetValue());

			// Set new prefs values in all panels
			ResetPrefs();
			
			// Reset title
			SetWindowTitle();
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			return false;
		}
		
		
		return true;
	}
	
	return false;
}

bool CPreferencesDialog::PrefsRemoteSaveAs()
{
	cdstring saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
	{
		// If remote replace name
		saveAsName = mCurrentPrefsFile->GetName();
		if (!mCurrentPrefsFile->IsRemote())
		{
			cdstring temp;
			const char* p = ::strrchr(saveAsName.c_str(), os_dir_delim);
			if (p)
				temp = ++p;
			else
				temp = saveAsName;
			p = ::strrchr(temp.c_str(), '.');
			if (p)
				*const_cast<char*>(p) = 0;
			saveAsName = temp;
		}
	}

	// Make sure set is read in
	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
	else
		return false;
	
	bool result = false;
	cdstrvect items;
	// Add default name first
	items.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		items.push_back(*iter);

	while(true)
	{
		ulvector selected;
		if (CTextListChoice::PoseDialog("Alerts::Preferences::SaveAsRemoteTitle", "Alerts::Preferences::SaveAsRemoteDesc", "Alerts::Preferences::SaveAsRemoteText", true, false, true, false, items, saveAsName, selected, "Alerts::Preferences::SaveAsRemoteButton"))
		{

			// Check for duplicate
			cdstring duplicate;
			if (CMulberryApp::sRemotePrefs->IsDuplicateName(saveAsName, duplicate))
			{
				// Warn replace
				if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Preferences::DuplicateSaveAsSet", duplicate) == CErrorHandler::Cancel)
					// Stop dialog OK
					continue;
			}

			// Login here so that all operations take place on a single connection
			StProtocolLogin login(CMulberryApp::sOptionsProtocol);

			// Delete any duplicate set first
			if (!duplicate.empty())
				CMulberryApp::sRemotePrefs->DeleteSet(duplicate);

			// Reset prefs file object if same as existing
			if (mCurrentPrefsFile == CMulberryApp::sCurrentPrefsFile)
				mCurrentPrefsFile = NULL;

			// Create file object if required
			if (!mCurrentPrefsFile)
				mCurrentPrefsFile = new CPreferencesFile(false, true);
			
			// Save current prefs in file
			mCurrentPrefsFile->SetRemote(true);
			mCurrentPrefsFile->SetName(saveAsName);
			mCurrentPrefsFile->SetSpecified(false);
			mCurrentPrefsFile->SetPrefs(&mCopyPrefs);
			mCurrentPrefsFile->SavePrefs(false, true);
			
			// Add to sets
			CMulberryApp::sRemotePrefs->GetRemoteSets().push_back(saveAsName);
			CMulberryApp::sRemotePrefs->UpdateRemoteSets();
			
			// Reset window title
			SetWindowTitle();
			
			return true;
		}
		else
			return false;
	}
}

void CPreferencesDialog::PrefsRemoteSaveDefault()
{
	// Delete existing set first if required
	if (GetDisplay()->GetLatestKeyModifiers().control())
	{
		// Must be logged in to server
		if (CMulberryApp::sApp->BeginRemote(GetCopyPrefs()))
		{
			// Delete default set
			cdstring set = cDefaultPrefsSetKey_2_0;
			CMulberryApp::sRemotePrefs->DeleteSet(set);

			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
		}
	}

	// Create new System Folder prefs file
	CPreferencesFile* temp = new CPreferencesFile(true, true);
	temp->CreateDefault();

	// Save current prefs in System Folder
	temp->SetPrefs(&mCopyPrefs);
	temp->SavePrefs(false, true);
	
	// Finished with file
	delete temp;
}
