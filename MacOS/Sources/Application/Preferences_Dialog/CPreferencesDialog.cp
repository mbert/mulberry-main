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


// Source for CPreferencesDialog class

#include "CPreferencesDialog.h"

#include "CPrefsPanel.h"

#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CAdminLock.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CICSupport.h"
#include "CIMAPClient.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryCommon.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPluginManager.h"
#include "CPreferencesFile.h"
#include "CPreferenceKeys.h"
#include "CPrefsAccount.h"
#include "CPrefsAddress.h"
#include "CPrefsAlerts.h"
#include "CPrefsAttachments.h"
#include "CPrefsCalendar.h"
#include "CPrefsDisplay.h"
#include "CPrefsFormatting.h"
#include "CPrefsIdentities.h"
#include "CPrefsLetter.h"
#include "CPrefsMailbox.h"
#include "CPrefsMessage.h"
#include "CPrefsSimple.h"
#include "CPrefsSecurity.h"
#include "CPrefsSpeech.h"
#include "CPrefsSpelling.h"
#include "CRemotePrefsSets.h"
#include "CServerView.h"
#include "CRulesWindow.h"
#include "CServerWindow.h"
#include "CSpeechSynthesis.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CTextListChoice.h"

#include <LCheckBox.h>
#include <LPushButton.h>
#include <LRadioButton.h>

#include <UStandardDialogs.h>

const char* cPanelNames[] = {"", "Account", "Alerts", "Display", "Styled", "Mailbox",
							"Message", "Letter", "Security", "Identities", "Address",
							"Calendar", "Attachments", "Spelling", "Speech", NULL};

// __________________________________________________________________________________________________
// C L A S S __ C P R E F E N C E S D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPreferencesDialog::CPreferencesDialog()
	: mCopyPrefs(*CPreferences::sPrefs)
{
	InitPreferencesDialog();
}

// Constructor from stream
CPreferencesDialog::CPreferencesDialog(LStream *inStream)
		: LDialogBox(inStream),
	  mCopyPrefs(*CPreferences::sPrefs)
{
	InitPreferencesDialog();
}

// Default destructor
CPreferencesDialog::~CPreferencesDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Standard init
void CPreferencesDialog::InitPreferencesDialog(void)
{
	mCurrentPrefsFile = NULL;
	mPrefsPanel = NULL;
	mCurrentPanel = NULL;
	mCurrentPanelNum = 0;

	mLoadedNewPrefs = false;
	mWindowsReset = false;
	mAccountNew = false;
	mAccountRename = false;
}

// Get details of sub-panes
void CPreferencesDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	mPrefsPanel = (LView*) FindPaneByID(paneid_PrefsPanel);

	// Set prefs file to current app file and change title
	mCurrentPrefsFile = CMulberryApp::sCurrentPrefsFile;
	SetWindowTitle();

	mButtons = (LView*) FindPaneByID(paneid_PrefsButtons);

	mSimpleBtn = (LRadioButton*) FindPaneByID(paneid_PrefsSimpleBtn);
	mAdvancedBtn = (LRadioButton*) FindPaneByID(paneid_PrefsAdvancedBtn);

	// Get local/remote buttons
	mStorage = (LView*) FindPaneByID(paneid_PrefsStorage);
	mLocalCaption = (CStaticText*) FindPaneByID(paneid_PrefsLocalCaption);
	mRemoteCaption = (CStaticText*) FindPaneByID(paneid_PrefsRemoteCaption);
	mLocalBtn = (LRadioButton*) FindPaneByID(paneid_PrefsLocalOnBtn);
	mRemoteBtn = (LRadioButton*) FindPaneByID(paneid_PrefsRemoteOnBtn);

	if (CAdminLock::sAdminLock.mNoLocalPrefs)
	{
		mLocalBtn->Disable();
		mLocalBtn->SetValue(0);
		mRemoteBtn->SetValue(1);
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mRemoteBtn->Disable();
		mLocalBtn->SetValue(1);
		mRemoteBtn->SetValue(0);
	}
	if (mCurrentPrefsFile)
	{
		mLocalBtn->SetValue(!mCurrentPrefsFile->IsRemote());
		mRemoteBtn->SetValue(mCurrentPrefsFile->IsRemote());
	}

	// Hide default button if not allowed
	mSaveDefaultBtn = (LPushButton*) FindPaneByID(paneid_PrefsSaveDefaultBtn);
	if (!CAdminLock::sAdminLock.mAllowDefault && (mLocalBtn->GetValue() == 1))
		mSaveDefaultBtn->Disable();

	// Disable Save and open buttons button if not allowed
	if (CAdminLock::sAdminLock.mNoLocalPrefs && CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		FindPaneByID(paneid_PrefsOpenBtn)->Disable();
		FindPaneByID(paneid_PrefsSaveAsBtn)->Disable();
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDialogBtns);

	// Set to last panel used
	StopListening();
	SetLastPanel();

	// Remove unwanted prefs panels by hiding buttons
	if (!CPluginManager::sPluginManager.HasSecurity())
	{
		LPane* btn = FindPaneByID(paneid_PrefsSecurityBtn);
		btn->Hide();
		SDimension16 moveby;
		btn->GetFrameSize(moveby);

		FindPaneByID(paneid_PrefsIdentitiesBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsAddressBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsCalendarBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsAttachmentsBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsSpellingBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsSpeechBtn)->MoveBy(0, -moveby.height, false);
	}

	// Remove unwanted prefs panels by hiding buttons
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		LPane* btn = FindPaneByID(paneid_PrefsCalendarBtn);
		btn->Hide();
		SDimension16 moveby;
		btn->GetFrameSize(moveby);

		FindPaneByID(paneid_PrefsAttachmentsBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsSpellingBtn)->MoveBy(0, -moveby.height, false);
		FindPaneByID(paneid_PrefsSpeechBtn)->MoveBy(0, -moveby.height, false);
	}

	// Remove unwanted prefs panels by hiding buttons
	if (!CPluginManager::sPluginManager.HasSpelling())
	{
		LPane* btn = FindPaneByID(paneid_PrefsSpellingBtn);
		btn->Hide();
		SDimension16 moveby;
		btn->GetFrameSize(moveby);

		FindPaneByID(paneid_PrefsSpeechBtn)->MoveBy(0, -moveby.height, false);
	}

	if (!CSpeechSynthesis::Available())
	{
		LPane* btn = FindPaneByID(paneid_PrefsSpeechBtn);
		btn->Hide();
	}

	// Now check simple/advanced state
	if (mCopyPrefs.mSimple.GetValue())
		SetSimple(true);
	else
		mAdvancedBtn->SetValue(1);
	StartListening();
}


// Set input panel
void CPreferencesDialog::SetPrefsPanel(short panel)
{
	ResIDT	panel_open;

	// First remove and update any existing panel
	if (mPrefsPanel->GetSubPanes().GetCount() > 1)
	{
		CPrefsPanel* old_panel = static_cast<CPrefsPanel*>(mPrefsPanel->GetSubPanes()[2]);
		old_panel->UpdatePrefs();
		delete old_panel;
	}

	// Update to new panel id
	mCurrentPanelNum = panel;
	const ResIDT panels[] = {paneid_PrefsSimple, paneid_PrefsAccount, paneid_PrefsAlerts,
								paneid_PrefsDisplay, paneid_PrefsStyled,
								paneid_PrefsMailbox, paneid_PrefsMessage, paneid_PrefsLetter,
								paneid_PrefsSecurity, paneid_PrefsIdentities, paneid_PrefsAddress,
								paneid_PrefsCalendar, paneid_PrefsAttachments, paneid_PrefsSpelling, paneid_PrefsSpeech};
	panel_open = panels[panel];

	// Make panel area default so new panel is automatically added to it
	SetDefaultView(mPrefsPanel);
	mPrefsPanel->Hide();
	LCommander* defCommander;
	mSubCommanders.FetchItemAt(1, defCommander);
	SetDefaultCommander(defCommander);
	mCurrentPanel = (LView*) UReanimator::ReadObjects('PPob', panel_open);
	mCurrentPanel->FinishCreate();
	((CPrefsPanel*) mCurrentPanel)->SetPrefs(&mCopyPrefs);
	mPrefsPanel->Show();
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

		title += mCurrentPrefsFile->GetName();
	}

	// Now set window title
	SetDescriptor(LStr255(title));
}

// Handle buttons
void CPreferencesDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_SetAccount:
		case msg_SetAlerts:
		case msg_SetStyled:
		case msg_SetDisplay:
		case msg_SetMailbox:
		case msg_SetMessage:
		case msg_SetLetter:
		case msg_SetIdentities:
		case msg_SetSecurity:
		case msg_SetAddress:
		case msg_SetCalendar:
		case msg_SetAttachments:
		case msg_SetSpelling:
		case msg_SetSpeech:
			// Only change if setting value
			if (*(long*) ioParam)
				SetPrefsPanel(inMessage - msg_SetAccount + 1);
			break;

		case msg_SimplePrefs:
			// Only do if setting
			if (*(long*) ioParam)
				SetSimple(true);
			break;

		case msg_AdvancedPrefs:
			// Only do if setting
			if (*(long*) ioParam)
				SetSimple(false);
			break;

		case msg_LocalPrefs:
			// Only do if setting
			if (*(long*) ioParam)
			{
				mRemoteBtn->SetValue(0);
				if (!CAdminLock::sAdminLock.mAllowDefault)
					mSaveDefaultBtn->Disable();
				
				// Change state of prefs file
				if (mCurrentPrefsFile)
					mCurrentPrefsFile->SetRemote(false);
			}
			break;

		case msg_RemotePrefs:
			// Only do if setting
			if (*(long*) ioParam)
			{
				mLocalBtn->SetValue(0);

				if (!CAdminLock::sAdminLock.mAllowDefault)
					mSaveDefaultBtn->Enable();

				// Force update of prefs
				((CPrefsPanel*) mCurrentPanel)->UpdatePrefs();

				// Must have valid remote IP
				if (!mCopyPrefs.mRemoteAccounts.GetValue().size())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoRemoteAccount");

					// Reset buttons
					mLocalBtn->SetValue(1);

					// Force to accounts panel
					((LControl*) FindPaneByID(paneid_PrefsAccountBtn))->SetValue(1);
				}
				else if (mCopyPrefs.mRemoteAccounts.GetValue().front()->GetServerIP().empty())
				{
					const cdstring& name = mCopyPrefs.mRemoteAccounts.GetValue().front()->GetName();
					CErrorHandler::PutStopAlertRsrcStr("Alerts::Preferences::Invalid_RemoteServerIP", name);

					// Reset buttons
					mLocalBtn->SetValue(1);

					// Force to accounts panel
					((LControl*) FindPaneByID(paneid_PrefsAccountBtn))->SetValue(1);
				}
				else if (!CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
				{
					mLocalBtn->SetValue(1);
					if (!CAdminLock::sAdminLock.mAllowDefault)
						mSaveDefaultBtn->Disable();
				}
				else
					// Do logoff to kill connection
					CMulberryApp::sOptionsProtocol->Logoff();

				// Change state of prefs file
				if (mCurrentPrefsFile && (mRemoteBtn->GetValue() == 1))
					mCurrentPrefsFile->SetRemote(true);
			}
			break;

		case msg_OpenPrefs:
			{
				bool opened = false;
				if (mLocalBtn->GetValue())
					opened = PrefsLocalOpenFile();
				else
					opened = PrefsRemoteOpenFile();

				// May need to reset menus
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
			break;

		case msg_SavePrefsAs:

			// Force update of prefs
			((CPrefsPanel*) mCurrentPanel)->UpdatePrefs();

			// Allow save of invalid preferences
			if (mLocalBtn->GetValue())
				PrefsLocalSaveAs();
			else
				PrefsRemoteSaveAs();
			break;

		case msg_SaveDefaultPrefs:

			// Force update of prefs
			((CPrefsPanel*) mCurrentPanel)->UpdatePrefs();

			// Prefs must be valid before save
			if (!mCopyPrefs.Valid(true))
				return;

			if (mLocalBtn->GetValue())
				PrefsLocalSaveDefault();
			else
				PrefsRemoteSaveDefault();
			break;

		case msg_OK:

			// Force update of prefs
			((CPrefsPanel*) mCurrentPanel)->UpdatePrefs();
			break;

		default:
			break;
	}
}

// Handle success OK
bool CPreferencesDialog::DoOK(void)
{
	// Update last panel now
	UpdateLastPanel();

	// Prefs must be valid before exit
	if (!mCopyPrefs.Valid(true))
		return false;
	else
		return true;
}

void CPreferencesDialog::UpdateLastPanel()
{
	// Record panel choice if not simple
	if (mCurrentPanelNum)
	{
		mCopyPrefs.mLastPanel.SetValue(cPanelNames[mCurrentPanelNum]);
		CPreferences::sPrefs->mLastPanel.SetValue(cPanelNames[mCurrentPanelNum]);
	}
}

void CPreferencesDialog::SetLastPanel()
{
	short panel = 1;
	if (!mCopyPrefs.mLastPanel.GetValue().empty())
	{
		const char** p = cPanelNames;
		while(*++p)
		{
			if (mCopyPrefs.mLastPanel.GetValue() == *p)
				break;
			
		}
		if (*p)
			panel = p - cPanelNames;
	}

	const PaneIDT btns[] = {0, paneid_PrefsAccountBtn, paneid_PrefsAlertsBtn,
								paneid_PrefsDisplayBtn, paneid_PrefsStyledBtn,
								paneid_PrefsMailboxBtn, paneid_PrefsMessageBtn, paneid_PrefsLetterBtn,
								paneid_PrefsSecurityBtn, paneid_PrefsIdentitiesBtn, paneid_PrefsAddressBtn,
								paneid_PrefsCalendarBtn, paneid_PrefsAttachmentsBtn, paneid_PrefsSpellingBtn, paneid_PrefsSpeechBtn};

	LButton* btn = (LButton*) FindPaneByID(btns[panel]);
	btn->SetValue(1);

	SetPrefsPanel(panel);
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
		// Create new System Folder prefs file
		mCurrentPrefsFile = new CPreferencesFile(true, false);
		try
		{
			mCurrentPrefsFile->CreateDefault();
		}
		catch (const PP_PowerPlant::LException& ex)
		{
			CLOG_LOGCATCH(PP_PowerPlant::LException);

			// Ignore duplicate file => file already exists
			if (ex.GetErrorCode() != dupFNErr)
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}

		// Save new prefs
		mCurrentPrefsFile->SetPrefs(newPrefs);
		mCurrentPrefsFile->SavePrefs(true, false);
	}

	// Try to save in new file
	else if (!CAdminLock::sAdminLock.mNoLocalPrefs || !CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		// Create new file to save as
		mCurrentPrefsFile = new CPreferencesFile(false, false);
		if (mLocalBtn->GetValue() ? !PrefsLocalSaveAs() : !PrefsRemoteSaveAs())
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
			for(CServerWindow::CServerWindowList::iterator iter1 = CServerWindow::sServerWindows->begin(); iter1 != CServerWindow::sServerWindows->end(); iter1++)
				(*iter1)->ResetState();
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

		// Address search window
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

	return;
}

// Handle Cancel
void CPreferencesDialog::DoCancel(void)
{
	// Update last panel now
	UpdateLastPanel();

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
}

bool CPreferencesDialog::IsLocal() const
{
	return mLocalBtn->GetValue();
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

	// Check for change of style or colour in mailbox window
	if ((CPreferences::sPrefs->unseen.GetValue() != newPrefs->unseen.GetValue()) ||
		(CPreferences::sPrefs->seen.GetValue() != newPrefs->seen.GetValue()) ||
		(CPreferences::sPrefs->answered.GetValue() != newPrefs->answered.GetValue()) ||
		(CPreferences::sPrefs->important.GetValue() != newPrefs->important.GetValue()) ||
		(CPreferences::sPrefs->deleted.GetValue() != newPrefs->deleted.GetValue()) ||
		(CPreferences::sPrefs->mMultiAddress.GetValue() != newPrefs->mMultiAddress.GetValue()))
	{
		refresh_mailbox = true;
	}

	// Check for change of font or size in list display
	if ((CPreferences::sPrefs->mListTextTraits.GetValue().traits.size != newPrefs->mListTextTraits.GetValue().traits.size) ||
		::PLstrcmp(CPreferences::sPrefs->mListTextTraits.GetValue().traits.fontName, newPrefs->mListTextTraits.GetValue().traits.fontName) != 0)
	{
		refresh_server = true;
		refresh_mailbox = true;
		refresh_message = true;
		refresh_letter = true;
		refresh_adbkmanager = true;
		refresh_adbksearch = true;
		refresh_addressbook = true;
		refresh_rules = true;
	}

	// Check for change of font or size in text display
	if ((CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits.size != newPrefs->mDisplayTextTraits.GetValue().traits.size) ||
		::PLstrcmp(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits.fontName, newPrefs->mDisplayTextTraits.GetValue().traits.fontName) != 0)
	{
		refresh_message = true;
		refresh_letter = true;
	}

	// Check for change of smart address filters
	bool smart_change = (CPreferences::sPrefs->mSmartAddressList.GetValue().size() != newPrefs->mSmartAddressList.GetValue().size());
	if (!smart_change)
	{
		for(int i = 0; i < CPreferences::sPrefs->mSmartAddressList.GetValue().size(); i++)
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
			// Found existing window so sync identities
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
			// Found existing window so refresh
			(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);
	}

	if (refresh_mailbox)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			// Found existing window so refresh
			(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);
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
				// Found existing window so refresh
				(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits, newPrefs->mDisplayTextTraits.GetValue().traits);
		}

		// Iterate over all message views
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin(); iter != CMessageView::sMsgViews->end(); iter++)
				// Found existing window so refresh
				(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits, newPrefs->mDisplayTextTraits.GetValue().traits);
		}
	}

	if (refresh_letter)
	{
		// Iterate over all letter windows
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			// Found existing window so refresh
			(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits, newPrefs->mDisplayTextTraits.GetValue().traits);
	}

	if (refresh_adbkmanager)
	{
		// Iterate over all adbk manager windows
		cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
		for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
			// Found existing window so refresh
			(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);
	}

	if (refresh_adbksearch && CAdbkSearchWindow::sAdbkSearch)
		CAdbkSearchWindow::sAdbkSearch->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);

	if (refresh_addressbook)
	{
		// Iterate over all address book windows
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin(); iter != CAddressBookView::sAddressBookViews->end(); iter++)
			// Found existing window so refresh
			(*iter)->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);
	}

	if (CRulesWindow::sRulesWindow)
	{
		// Always reset the list
		CRulesWindow::sRulesWindow->ResetTable();
		
		// Refresh the font if that changed
		if (refresh_rules)
			CRulesWindow::sRulesWindow->ResetTextTraits(newPrefs->mListTextTraits.GetValue().traits);
	}

	// Check for change in MRU number
	if (refresh_mrus && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->CleanMRUHistory(newPrefs->mMRUMaximum.GetValue());

	if (smart_change)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			// Found existing window so refresh smart address display
			(*iter)->RefreshSmartAddress();
	}
}

// Set to simple state
void CPreferencesDialog::SetSimple(bool simple)
{
	const int cStorageAdjust = 26;

	if (simple)
	{
		// Remember the current panel
		UpdateLastPanel();

		// Hide items
		mButtons->Hide();
		mLocalBtn->Hide();
		mRemoteBtn->Hide();
		mStorage->ResizeFrameBy(0, -cStorageAdjust, true);
		if (mLocalBtn->GetValue())
			mLocalCaption->Show();
		else
			mRemoteCaption->Show();

		// Reduce width of prefs dialog
		SDimension16 frame_size;
		mButtons->GetFrameSize(frame_size);
		ResizeWindowBy(-frame_size.width, 0);

		// Now change to simple panel
		SetPrefsPanel(0);
	}
	else
	{
		// Increase width of prefs dialog
		SDimension16 frame_size;
		mButtons->GetFrameSize(frame_size);
		ResizeWindowBy(frame_size.width, 0);

		// Show items
		mButtons->Show();
		mStorage->ResizeFrameBy(0, cStorageAdjust, true);
		mLocalCaption->Hide();
		mRemoteCaption->Hide();
		mLocalBtn->Show();
		mRemoteBtn->Show();
		UpdatePort();

		// Now change to advanced panel
		SetLastPanel();
	}

	// Now set prefs value
	mCopyPrefs.mSimple.SetValue(simple);
}

// Do open file
bool CPreferencesDialog::PrefsLocalOpenFile(void)
{
	// Check for good reply
	PPx::FSObject fspec;
	bool done = false;
	if (PP_StandardDialogs::AskOpenOneFile(kPrefFileType, fspec, kNavDefaultNavDlogOptions | kNavAllowPreviews | kNavAllFilesInPopup))
	{
		done = true;

		// Save copy of existing preferences
		CPreferences temp(mCopyPrefs);

		// Create file object
		CPreferencesFile* new_file = new CPreferencesFile(false, false);

		// Set prefs file to new file and copy prefs
		new_file->SetSpecifier(fspec);
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
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			return false;
		}

		// Set new prefs values in current panel dialog
		((CPrefsPanel*) mCurrentPanel)->SetPrefs(&mCopyPrefs);

		// Reset title
		SetWindowTitle();
	}

	return done;
}

// Do save as on file
bool CPreferencesDialog::PrefsLocalSaveAs(void)
{
	cdstring			saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
		saveAsName = mCurrentPrefsFile->GetName();

	else
	{
		// Set default name
		saveAsName.FromResource("UI::Preferences::DefaultPrefsFileName");
	}

	// Do standard save as dialog
	PPx::CFString cfstr(saveAsName.c_str(), kCFStringEncodingUTF8);
	PPx::FSObject fspec;
	bool replacing;
	if (PP_StandardDialogs::AskSaveFile(cfstr, kPrefFileType, fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))
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
		mCurrentPrefsFile->DoSaveAs(fspec, replacing);
		mCurrentPrefsFile->SetSpecified(true);

		// Reset window title
		SetWindowTitle();

		// Fix for window manager bug after a replace operation
		Activate();

		return true;
	}
	else
		return false;
}

// Do save default file
void CPreferencesDialog::PrefsLocalSaveDefault(void)
{
	// Create new System Folder prefs file
	CPreferencesFile* temp = new CPreferencesFile(true, false);
	try
	{
		temp->CreateDefault();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Ignore duplicate file => file already exists
		if (ex.GetErrorCode() != dupFNErr)
		{
			CLOG_LOGRETHROW;
			throw;
		}
	}

	// Save current prefs in System Folder
	temp->SetPrefs(&mCopyPrefs);
	temp->SavePrefs(false, true);

	// Finished with file
	delete temp;
}

// Do open file
bool CPreferencesDialog::PrefsRemoteOpenFile(void)
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


	// Do the dialog
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

			// Set new prefs values in current panel dialog
			((CPrefsPanel*) mCurrentPanel)->SetPrefs(&mCopyPrefs);

			// Reset title
			SetWindowTitle();

			result = true;
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			result = false;
		}
	}

	return result;
}

// Do save as on file
bool CPreferencesDialog::PrefsRemoteSaveAs(void)
{
	cdstring saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
		saveAsName = mCurrentPrefsFile->GetName();

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

	// Do the dialog
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

			result = true;
			break;
		}
		else
			break;
	}

	return result;
}

// Do save default file
void CPreferencesDialog::PrefsRemoteSaveDefault(void)
{
	// Check for option key
	bool option_key = ::GetCurrentKeyModifiers() & optionKey;

	// Do delete of default set first if requested
	if (option_key)
	{
		// Must be logged in to server
		if (CMulberryApp::sApp->BeginRemote(GetNewPrefs()))
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
