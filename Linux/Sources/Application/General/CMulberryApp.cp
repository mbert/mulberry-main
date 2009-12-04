/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CMulberryApp


#include "CMulberryApp.h"

#include "CAboutDialog.h"
#include "CAboutPluginsDialog.h"
#include "CActionManager.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CAttachmentManager.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CClickElement.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CDisconnectDialog.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterItem.h"
#include "CFindReplaceWindow.h"
#include "CGetPassphraseDialog.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CMacroEditDialog.h"
#include "CMailAccountManager.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMboxProtocol.h"
#include "CMulberryHelp.h"
#include "CMulberryMDIServer.h"
#include "COptionsProtocol.h"
#include "CPasswordManagerKeyring.h"
#include "CPeriodicCheck.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPreferenceKeys.h"
#include "CPreferencesFile.h"
#include "CRemotePrefsSets.h"
#include "CRFC822.h"
#include "CRulesWindow.h"
#include "CSearchEngine.h"
#include "CSearchWindow.h"
#include "CServerWindow.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CSpellPlugin.h"
#include "CSplashScreen.h"
#include "CStatusWindow.h"
#include "CStringOptionsMap.h"
#include "CStringUtils.h"
#include "CTaskQueue.h"
#include "CTCPSocket.h"
#include "CToolbarView.h"
#include "CURLHelpersMap.h"
#include "CWindowsMenu.h"
#include "CWindowOptionsDialog.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "CICalendarManager.h"

#include "CCalendarStoreManager.h"

#include "StValueChanger.h"

#include "jxStringData.h"

#include <JFontManager.h>
#include <JThisProcess.h>
#include <JXChooseSaveFile.h>
#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXHelpManager.h>
#include <JXWindowDirector.h>
#include <jXGlobals.h>
#include <jDirUtil.h>

#include <typeinfo>

#include <locale.h>


CMulberryApp* CMulberryApp::sApp = NULL;
CMulberryMDIServer*	CMulberryApp::sMDIServer = NULL;
bool CMulberryApp::mQuitting = false;

extern cdstring openpreffile;
extern cdstrvect openurls;

cdstring openpreffile;
cdstrvect openurls;
static bool start_disconnected = false;

const JCharacter* cSendMailHelpCmd = "mulberry -u \"$a\"";

void pwr_signal(int sigint);
void pwr_signal(int sigint)
{
	// Only if main thread
	if (CTaskQueue::sTaskQueue.InMainThread() && CMulberryApp::sMailCheck)
		CMulberryApp::sMailCheck->PwrSignal();
}

CMulberryApp::CMulberryApp(int *argc, char* argv[]) 
	: JXApplication(argc, argv, "Mulberry", kJXCoreDefaultStringData), startedYet(false)
{
	// Init this first
	sApp = this;

	// Create the MDI server
	sMDIServer = new CMulberryMDIServer();

	// Handle command line options
	// Before anything else set the application CWD for
	// local storage of mailboxes, preferences etc
	sMDIServer->HandleCmdLineOptions(*argc, argv);

	//For now we'll fill in the fonts.  They should come from prefs -honus
	//sAppFont.name_  //defaults are ok
	sAppSmallFont.size_ = 10;

	// Font styles must all be reset here AFTER a display has been created
	// as the default color index is not set until a display is created
	JFontStyle fs; //will start off as all plane
	sAppFont.style_ = fs;
	sAppSmallFont.style_ = fs;
	fs.bold = kTrue;
	sAppFontBold.style_ = fs;

	// Set prefs load flag
	mDoneProtect = false;
	mFailedProtect = false;
	mStartInit = false;
	mPrefsLoaded = false;
	mMUPrefsUsed = false;

	mRunDemo = false;
	mPaused = false;
	mSleeping = false;

	rsrc::CXStringResources::sStringResources.FullReset();

	//have to make the preferences here because the app needs to have
	//already been constructed (because CPreferences needs info about
	//the display)
	CPreferences::sPrefs = new CPreferences;

	JXInitHelp(HELP_NAME(TOC), cHelpSectionCount, cHelpSectionName);
	//JXGetHelpManager()->SetSendMailCmd(cSendMailHelpCmd);
	
	// Setup sleep handling
	//JThisProcess::ShouldCatchSignal(SIGPWR, kTrue);
	
	::signal(SIGPWR, pwr_signal);
}

CMulberryApp::~CMulberryApp()
{
	// Terminate logging
	CLog::StopLogging();
}

void CMulberryApp::CommandLine(const JPtrArray<JString>& argList)
{
	const JSize argCount = argList.GetElementCount();
	for (JIndex i = 2; i <= argCount; i++)
	{
		// Current argument
		const JString& arg = *(argList.NthElement(i));
		if (arg.IsEmpty())
			continue;

		// -D : location for .mulberry
		if ((arg == "-D") && (i++ < argCount))
		{
			// Get argument
			cdstring wd = argList.NthElement(i)->GetCString();

			// Make sure its a valid directory
			if (JDirectoryExists(wd) && JDirectoryReadable(wd))
			{
				// Must end in os_dir_delim
				if (wd[wd.length() - 1] != os_dir_delim)
					wd += os_dir_delim;

				// Now add Mulberry directory and force create
				wd += ".mulberry";
				JCreateDirectory(wd, 0700);
				wd += os_dir_delim;

				// Now give it to connection manager if not already started
				if (!startedYet)
					CConnectionManager::sConnectionManager.SetApplicationCWD(wd);
			}
		}

		// -plugins : location for plugins
		else if ((arg == "-plugins") && (i++ < argCount))
		{
			// Get argument
			cdstring plugins = argList.NthElement(i)->GetCString();

			// Make sure its a valid directory
			if (JDirectoryExists(plugins) && JDirectoryReadable(plugins))
			{
				// Must end in os_dir_delim
				if (plugins[plugins.length() - 1] != os_dir_delim)
					plugins += os_dir_delim;

				// Now add Mulberry directory and force create
				cdstrvect temp;
				temp.push_back(plugins);
				CPluginManager::sPluginManager.SetPluginDirs(temp);
			}
		}

		// -extras : location for extra files
		else if ((arg == "-extras") && (i++ < argCount))
		{
			// Get argument
			cdstring extras = argList.NthElement(i)->GetCString();

			// Make sure its a valid directory
			if (JDirectoryExists(extras) && JDirectoryReadable(extras))
			{
				// Must end in os_dir_delim
				if (extras[extras.length() - 1] != os_dir_delim)
					extras += os_dir_delim;

				// Now add Mulberry directory 
				mExtrasPath = extras;
			}
		}

		// -p : preference file to open on startup
		else if ((arg == "-p") && (i++ < argCount))
		{
			// Process pref file argument differently dending on whether started or not
			if (!startedYet)
				// Get filename for later processing
				openpreffile = argList.NthElement(i)->GetCString();
			else
			{
				// Open the file now
				cdstring temp = argList.NthElement(i)->GetCString();
				OpenPrefs(temp);
			}
		}

		// -u : url to open in Mulberry (mailto)
		else if ((arg == "-u") && (i++ < argCount))
		{
			// Process URL argument differently dending on whether started or not
			if (!startedYet)
				// Add URL to list for future processing
				openurls.push_back(argList.NthElement(i)->GetCString());
			else
				// Process URL now
				ProcessURL(cdstring(*(argList.NthElement(i))));
		}

		// -d, --disconnect : start in disconnected mode
		else if ((arg == "-d") || (arg == "--disconnect"))
		{
			// Get argument
			// NB This won't do anything if already started
			start_disconnected = true;
		}
	}
}

HFont				CMulberryApp::sAppFont;			// The application font
HFont				CMulberryApp::sAppFontBold;		// The application font in bold
HFont				CMulberryApp::sAppSmallFont;		// The button font
bool				CMulberryApp::sLargeFont = false;		// Large font flag

COptionsProtocol*	CMulberryApp::sOptionsProtocol = NULL;	// The Options client
CRemotePrefsSet*	CMulberryApp::sRemotePrefs = NULL;		// Set of remote prefs
CPreferencesFile*	CMulberryApp::sCurrentPrefsFile = NULL;	// The current preferences file
CPeriodicCheck*		CMulberryApp::sMailCheck = NULL;       	// Mail check

Atom			CMulberryApp::sFlavorServerList = 0;
Atom			CMulberryApp::sFlavorWDList = 0;
Atom			CMulberryApp::sFlavorMboxList = 0;
Atom			CMulberryApp::sFlavorMboxRefList = 0;

Atom			CMulberryApp::sFlavorMsgList = 0;
Atom 			CMulberryApp::sFlavorAddrList = 0;
Atom 			CMulberryApp::sFlavorAdbkAddrList = 0;
Atom 			CMulberryApp::sFlavorGrpList = 0;
Atom 			CMulberryApp::sFlavorAtchList = 0;
Atom 			CMulberryApp::sFlavorMsgAtchList = 0;
Atom 			CMulberryApp::sFlavorRuleType = 0;
Atom 			CMulberryApp::sFlavorRulePosType = 0;

Atom			CMulberryApp::sFlavorCalServer = 0;
Atom			CMulberryApp::sFlavorCalendar = 0;
Atom			CMulberryApp::sFlavorCalendarItem = 0;

// Open special file
void CMulberryApp::RegisterClipboard()
{
	JXDisplay* disp = GetDisplay(1);
	// Servers
	sFlavorServerList = 
		disp->RegisterXAtom("application/x-Mulberry-Server-List");

	// WDs
	sFlavorWDList = 
		disp->RegisterXAtom("application/x-Mulberry-Search-Hierarchy-List");

	// Mboxes
	sFlavorMboxList = disp->RegisterXAtom("application/x-Mulberry-Mailbox-List");

	// Mbox Refs
	sFlavorMboxRefList = 
		disp->RegisterXAtom("application/x-Mulberry-Mailbox-Reference-List");

	// Messages
	sFlavorMsgList = 
		disp->RegisterXAtom("application/x-Mulberry-Message-List");

	// Addresses
	sFlavorAddrList = 
		disp->RegisterXAtom("application/x-Mulberry-Address-List");

	// Adbk Addresses
	sFlavorAdbkAddrList = 
		disp->RegisterXAtom("application/x-Mulberry-AdbkAddress-List");

	// Groups
	sFlavorGrpList = 
		disp->RegisterXAtom("application/x-Mulberry-Group-List");

	// Outgoing attachments
	sFlavorAtchList = 
		disp->RegisterXAtom("application/x-Mulberry-Outgoing-Attachments");

	// Incoming attachments
	sFlavorMsgAtchList = 
		disp->RegisterXAtom("application/x-Mulberry-Incoming-Attachments");

	// Rules
	sFlavorRuleType = 
		disp->RegisterXAtom("application/x-Mulberry-Rules");

	// Rules position
	sFlavorRulePosType = 
		disp->RegisterXAtom("application/x-Mulberry-Rules-Position");

	sFlavorCalServer =
		disp->RegisterXAtom("application/x-Mulberry-Calendar-Server");

	sFlavorCalendar =
		disp->RegisterXAtom("application/x-Mulberry-Calendar");

	sFlavorCalendarItem =
		disp->RegisterXAtom("application/x-Mulberry-Calendar-Item");

}

void CMulberryApp::ReceiveWithFeedback(JBroadcaster* sender, Message* message)
{
	if (sender == JThisProcess::Instance())
	{
		JThisProcess::Signal* sig = dynamic_cast<JThisProcess::Signal*>(message);
		if (sig && (sig->GetValue() == SIGPWR))
		{
			OnPowerSignal(SIGPWR);
			sig->SetCaught();
			return;
		}
	}
	
	// Do default
	JXApplication::ReceiveWithFeedback(sender, message);
}

void CMulberryApp::CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason)
{
	// Do safety save
	unsigned long recovered = CLetterWindow::SaveAllTemporary();
	if (recovered > 0)
		cerr << "[Mulberry]: Saved " << recovered << " open drafts for recovery." << std::endl;
}

#pragma mark ________________________

// Parse mailto URL
void CMulberryApp::ProcessURL(const cdstring& url)
{
	// Get scheme and address
	cdstring dup(url);
	char* scheme = ::strtok(dup, " :");
	if (!scheme)
		return;
	if (::strcmpnocase(scheme, "mailto") == 0)
	{
		char* addr = ::strtok(NULL, "");		// Can be NULL

		ProcessMailto(addr);
	}
	else if (::strcmpnocase(scheme, "webcal") == 0)
	{
		char* addr = ::strtok(NULL, "");		// Can be NULL

		ProcessWebcal(addr);
	}
}

// Parse mailto URL
void CMulberryApp::ProcessMailto(const cdstring& mailto)
{
	cdstring to;
	cdstring cc;
	cdstring bcc;
	cdstring body;
	cdstring subject;
	cdstrvect files;

	CURLClickElement::ParseMailto(mailto, to, cc, bcc, subject, body, files);

	CAddressList toField(to.c_str(), to.length());
	CAddressList ccField(cc.c_str(), cc.length());
	CAddressList bccField(bcc.c_str(), bcc.length());

	CActionManager::NewDraft(&toField, &ccField, &bccField, subject, body, files);
	
	// Do warning of files
	if ((files.size() != 0) && CPreferences::sPrefs->mWarnMailtoFiles.GetValue())
	{
		bool dont_show = false;
		CErrorDialog::NoteAlertRsrc("ErrorDialog::Text::WarnMailtoFiles", NULL, &dont_show);
		if (dont_show)
			CPreferences::sPrefs->mWarnMailtoFiles.SetValue(false);
	}
}

void CMulberryApp::ProcessWebcal(const cdstring& webcal)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		try
		{
			// Prepend http:
			cdstring temp("http:");
			temp += webcal;

			// Check for duplicate
			const calstore::CCalendarStoreNode* cal = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNodeByRemoteURL(temp);

			if (cal != NULL)
				// Error
				CErrorHandler::PutStopAlertRsrcStr("CCalendarStoreTable::DuplicateWebCalendar", temp);
			else
				calstore::CCalendarStoreManager::sCalendarStoreManager->NewWebCalendar(temp);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}

// Terminate
JBoolean CMulberryApp::Close()
{
	// Check for held messages in SMTP queue when disconnecting
	if (CConnectionManager::sConnectionManager.IsConnected() &&
		CSMTPAccountManager::sSMTPAccountManager &&
		CSMTPAccountManager::sSMTPAccountManager->PendingItems())
	{
		// Ask user
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::ReallyDisconnectSMTPHeldItems") == CErrorHandler::Cancel)
			return kFalse;
	}

	// Try to clear out temp attachments
	cdstrvect still_open;
	if (!CAttachmentManager::sAttachmentManager.CanQuit(still_open))
	{
		// Create one string with all attachment names
		cdstring names;
		for(cdstrvect::const_iterator iter = still_open.begin(); iter != still_open.end(); iter++)
		{
			if (names.length())
				names += ", ";
			names += *iter;
		}

		// Display alert with list of open files
		if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::General::ViewAttachmentsStillOpen", names) == CErrorHandler::Cancel)
			return kFalse;
	}

	// Look for locked quit
	if (CAdminLock::sAdminLock.mNoQuit)
	{
		// See if allowed to quit by existing docs
		if ((!GetCurrentDisplay()->GetLatestKeyModifiers().control() ||
			 !GetCurrentDisplay()->GetLatestKeyModifiers().meta() ||
			 !GetCurrentDisplay()->GetLatestKeyModifiers().shift()) && AttemptClose())
		{
			bool done = false;
			while(!done)
			{
				// First close it
				CloseDown();
				
				// Now reopen it
				OpenUp(false);
				
				// Now do initial startup, but allow quit to proceed
				//CAdminLock::sAdminLock.mNoQuit = false;

				try
				{
					StartUp();
					done = true;
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Failure => do quit again to recycle
				}
			}
			//CAdminLock::sAdminLock.mNoQuit = true;
			
			// Done
			return kFalse;
		}
	}
	else if (!AttemptClose())
		return kFalse;

	CloseDown();

	return JXApplication::Close();
}

// Terminate
bool CMulberryApp::AttemptClose()
{
	// Try to close drafts first, in case they reference mailbox/message items
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::reverse_iterator riter = CLetterWindow::sLetterWindows->rbegin();
				riter != CLetterWindow::sLetterWindows->rend(); riter++)
		{
			// Check that close is allowed
			if (!(*riter)->Close())
				return false;
		}
	}

	// Try to close mailboxes first - do in reverse
	{
		cdmutexprotect<CMailboxWindow::CMailboxWindowList>::lock _lock(CMailboxWindow::sMboxWindows);
		for(CMailboxWindow::CMailboxWindowList::reverse_iterator riter = CMailboxWindow::sMboxWindows->rbegin();
				riter != CMailboxWindow::sMboxWindows->rend(); riter++)
		{
			// Check that close is allowed
			if ((*riter)->GetMailboxView()->TestClose())
				// Get document and close
				FRAMEWORK_DELETE_WINDOW(*riter)
			else
			{
				return false;
			}
		}
	}

	// Try to close three pane if it exists
	if (C3PaneWindow::s3PaneWindow)
	{
		if (C3PaneWindow::s3PaneWindow->AttemptClose())
			C3PaneWindow::Destroy3PaneWindow();
		else
		{
			return false;
		}
	}

	return true;
}

// Starting app
void CMulberryApp::OpenUp(bool first_time)
{
	// Set flags
	mPaused = false;
	mStartInit = false;
	mPrefsLoaded = false;
	mMUPrefsUsed = false;

	// Must have default prefs
	if (!CPreferences::sPrefs)
		CPreferences::sPrefs = new CPreferences;

	// Create dynamic menus
	if (!CCopyToMenu::sMailboxMainMenu)
	{
		CCopyToMenu::sMailboxMainMenu = new CCopyToMenu;
		CCopyToMenu::sMailboxMainMenu->ResetMenuList(NULL, NULL);	// Must do here because required in prefs dlog
	}
	
	if (!CApplyRulesMenu::sApplyRules)
		CApplyRulesMenu::sApplyRules = new CApplyRulesMenu;

	// Create status window
	CStatusWindow::CreateWindow(this);
	
	// Warn about logging
	if (CLog::AnyActiveLogs())
	{
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::LoggingInsecure") == CErrorHandler::Cancel)
			CLog::DisableActiveLogs();
	}

	// Do keyring load
	CPasswordManagerKeyring::MakePasswordManagerKeyring();
	if (CPasswordManagerKeyring::GetKeyRingManager()->KeyringExists())
	{
		// Ask for real name
		cdstring passphrase;
		if (CGetPassphraseDialog::PoseDialog(passphrase, "Alerts::General::GetPassphrase_Title"))
		{
			CPasswordManagerKeyring::GetKeyRingManager()->SetPassphrase(passphrase);
		}
	}
}

// Terminate
void CMulberryApp::CloseDown()
{
	// Must get status window state before saving prefs
	if (CStatusWindow::sStatusWindow)
		CStatusWindow::sStatusWindow->SaveState();

	// Destroy all here
	CloseAllDocuments();

	// Stop network mail ops
	StopNetworkMail(true);
	StopAddressBooks(true);
	StopCalendars(true);

	// Save prefs in case of any change to window states
	if (sCurrentPrefsFile)
	{
		sCurrentPrefsFile->SetPrefs(CPreferences::sPrefs);
		
		// Don't allow remote failure to prevent quitting
		try
		{
			// Do standard prefs file save
			sCurrentPrefsFile->SavePrefs(false, false);
			
			// Now save any original local
			sCurrentPrefsFile->SaveOriginalLocal();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}

		delete sCurrentPrefsFile;
		sCurrentPrefsFile = NULL;
	}

	// Stop remote options protocol
	delete sOptionsProtocol;
	sOptionsProtocol = NULL;

	// Remove password cache after last protocol is shut down
	CINETProtocol::ClearUserPswdCache();

	// Also do the status window
	CStatusWindow::DestroyWindow();

	delete CPreferences::sPrefs;
	CPreferences::sPrefs = NULL;
	delete sRemotePrefs;
	sRemotePrefs = NULL;

	CLog::StopLogging();

	// Reset switches
	mStartInit = false;
	mPrefsLoaded = false;
	mMUPrefsUsed = false;
	mPaused = false;
}

bool CMulberryApp::CloseAllDocuments()
{
	while (HasSubdirectors())
	{
		const JPtrArray<JXDirector>* list;
		if (!GetSubdirectors(&list))
			return true;
		JPtrArray<JXDirector>* subs = const_cast<JPtrArray<JXDirector>*>(list);

		JXDirector* theDirector = subs->FirstElement();
		if (!theDirector->Close())
		{
			return false;
		}
		else if (HasSubdirectors() && (theDirector == subs->FirstElement()))
		{
			// Since one JXDocument can keep another one open,
			// if the owned document is in front, we have to shove it to
			// the end so we close the owner first.

			subs->MoveElementToIndex(1, subs->GetElementCount());
		}
	}
	
	return true;
}

#pragma mark ____________________________Command Updaters

// Update command
void CMulberryApp::OnUpdateLocalAddr(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CAddressBookManager::sAddressBookManager->GetProtocolList().empty());
}

// Update command
void CMulberryApp::OnUpdateLoggedIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMailAccountManager::sMailAccountManager &&
								CMailAccountManager::sMailAccountManager->HasOpenProtocols());
}

// Update command
void CMulberryApp::OnUpdateServerManager(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMailAccountManager::sMailAccountManager &&
								CMailAccountManager::sMailAccountManager->GetMainView());
}

// Update command
void CMulberryApp::OnUpdateOpenDraft(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalDrafts);
}

// Update command
void CMulberryApp::OnUpdateChangePassword(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CPluginManager::sPluginManager.CountPswdChange() > 0);
}

// Update command
void CMulberryApp::OnUpdateDisconnected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoDisconnect);

	// Change title based on state
	cdstring txt;
	txt.FromResource(CConnectionManager::sConnectionManager.IsConnected() ? IDS_DISCONNECT_CMD_TEXT : IDS_CONNECT_CMD_TEXT);
	pCmdUI->SetText(txt);
}

// Update command
void CMulberryApp::OnUpdateOpenSMTPQueue(CCmdUI* pCmdUI)
{
	// Check for queuable SMTP senders
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoSMTPQueues &&
					CSMTPAccountManager::sSMTPAccountManager &&
					CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected());
}

// Update command
void CMulberryApp::OnUpdateAppRules(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoRules);
}

// Update command
void CMulberryApp::OnUpdateAppUse3Pane(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllow3PaneChoice);
	pCmdUI->SetCheck(CPreferences::sPrefs->mUse3Pane.GetValue());
}

// Update command
void CMulberryApp::OnUpdateAppUseSeparatePanes(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(CAdminLock::sAdminLock.mAllow3PaneChoice);
	pCmdUI->SetCheck(!CPreferences::sPrefs->mUse3Pane.GetValue());
}

void CMulberryApp::OnUpdateAppToolbarsShow(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Set string based on state
	cdstring s;
	s.FromResource(CPreferences::sPrefs->mToolbarShow.GetValue() ? IDS_WINDOWSHIDETOOLBAR : IDS_WINDOWSSHOWTOOLBAR);
	pCmdUI->SetText(s);
}

// Update command
void CMulberryApp::OnUpdateAppToolbarsSmall(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(CPreferences::sPrefs->mToolbarSmallIcons.GetValue());
}

// Update command
void CMulberryApp::OnUpdateAppToolbarsIcon(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(CPreferences::sPrefs->mToolbarShowIcons.GetValue() &&
							!CPreferences::sPrefs->mToolbarShowCaptions.GetValue());
}

// Update command
void CMulberryApp::OnUpdateAppToolbarsCaption(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(CPreferences::sPrefs->mToolbarShowCaptions.GetValue() &&
							!CPreferences::sPrefs->mToolbarShowIcons.GetValue());
}

// Update command
void CMulberryApp::OnUpdateAppToolbarsBoth(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(CPreferences::sPrefs->mToolbarShowCaptions.GetValue() &&
							CPreferences::sPrefs->mToolbarShowIcons.GetValue());
}

// Update command
void CMulberryApp::OnUpdateHelpDetailedTooltips(CCmdUI* pCmdUI)
{
	// Only if not locked out
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(CPreferences::sPrefs->mDetailedTooltips.GetValue());
}

#pragma mark ____________________________Command handlers

// Do about box
void CMulberryApp::OnAppAbout()
{
	CAboutDialog::PoseDialog();
}

// Do about box
void CMulberryApp::OnAppAboutPlugins()
{
	CAboutPluginsDialog::PoseDialog();
}

void CMulberryApp::OnAppNewDraft()
{
	CActionManager::NewDraft();
}

void CMulberryApp::OnAppOpenMailbox()
{
	// Pass on to server manager window (must exist due to command status)
	CMailAccountManager::sMailAccountManager->GetMainView()->GetTable()->ObeyCommand(CCommand::eFileOpenMailbox, NULL);
}

void CMulberryApp::OnAppOpenDraft()
{
	CLetterWindow::OpenDraft();
}

void CMulberryApp::OnAppPreferences()
{
	// Look for alt key
	if (GetCurrentDisplay()->GetLatestKeyModifiers().control())
		CLog::DoLoggingOptions();
	else
		DoPreferences();
}

void CMulberryApp::OnAppDisconnected()
{
	// Check for held messages in SMTP queue when disconnecting
	if (CConnectionManager::sConnectionManager.IsConnected() &&
		CSMTPAccountManager::sSMTPAccountManager &&
		CSMTPAccountManager::sSMTPAccountManager->PendingItems())
	{
		// Ask user
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::ReallyDisconnectSMTPHeldItems") == CErrorHandler::Cancel)
			return;
	}

	CDisconnectDialog::PoseDialog();
}

void CMulberryApp::OnAppOpenSMTPQueue()
{
	CSMTPWindow::OpenSMTPWindow();
}

void CMulberryApp::OnAppTextMacros()
{
	CMacroEditDialog::PoseDialog();
}

void CMulberryApp::OnAppChangePassword()
{
#ifdef NOTYET
	CChangePswdAcctDialog::AcctPasswordChange();
#endif
}

void CMulberryApp::OnAppCreateMailbox()
{
	CMailAccountManager::sMailAccountManager->GetMainView()->GetTable()->DoCreateMailbox(false);
}

void CMulberryApp::OnAppMailCheck()
{
	DoMailCheck();
}

void CMulberryApp::OnAppSearch()
{
	// Just create the search window
	CSearchWindow::CreateSearchWindow();
}

void CMulberryApp::OnAppOpenTarget()
{
	// Just create the search window
	CSearchEngine::sSearchEngine.OpenTarget(CSearchEngine::sSearchEngine.GetCurrentTarget());
}

void CMulberryApp::OnAppRules()
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Just create the rules window
	CRulesWindow::CreateRulesWindow();
}

void CMulberryApp::OnAppAddressBookManager()
{
	// Just create the window
	CAdbkManagerWindow::CreateAdbkManagerWindow();
}

void CMulberryApp::OnAppNewAddressBook()
{
	// Do local open if only local address books
	if (CPreferences::sPrefs->mUse3Pane.GetValue())
	{
		// Show three pane window and force to contacts tab
		if (C3PaneWindow::s3PaneWindow)
		{
			FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
			C3PaneWindow::s3PaneWindow->SetViewType(N3Pane::eView_Contacts);
		}
	}
	else
	{
		// Show address book manager
		CAdbkManagerWindow::CreateAdbkManagerWindow();
	}
}

void CMulberryApp::OnAppOpenAddressBook()
{
	// Do local open if only local address books
	if (CPreferences::sPrefs->mUse3Pane.GetValue())
	{
		// Show three pane window and force to contacts tab
		if (C3PaneWindow::s3PaneWindow)
		{
			FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
			C3PaneWindow::s3PaneWindow->SetViewType(N3Pane::eView_Contacts);
		}
	}
	else
	{
		// Show address book manager
		CAdbkManagerWindow::CreateAdbkManagerWindow();
	}
}

void CMulberryApp::OnAppAddrImportExport()
{
	// Display warning
	CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::WarnImportExport");
}

void CMulberryApp::OnAppSearchAddress()
{
	// Check that search address books exist
	if (!CAddressBookManager::sAddressBookManager->CanSearch())
	{
		// Display warning
		CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::NoSearch");
		return;
	}

	// Create Adbk manager window or bring to front
	if (CAdbkSearchWindow::sAdbkSearch)
		FRAMEWORK_WINDOW_TO_TOP(CAdbkSearchWindow::sAdbkSearch)
	else
		CAdbkSearchWindow::ManualCreate();
}

void CMulberryApp::OnAppCalendarManager()
{
	// Just create the window
	CCalendarStoreWindow::CreateCalendarStoreWindow();
}

void CMulberryApp::OnAppCalendarSubscribed()
{
	// Just create the window
	CCalendarWindow::CreateSubscribedWindow();
}

void CMulberryApp::OnWindowOptions()
{
	CWindowOptionsDialog::PoseDialog();
}

void CMulberryApp::OnAppUse3Pane()
{
	DoSet3Pane(true);
}

void CMulberryApp::OnAppUseSeparatePanes()
{
	DoSet3Pane(false);
}

void CMulberryApp::DoSet3Pane(bool use3pane)
{
	// Check admin lock
	if (!CAdminLock::sAdminLock.mAllow3PaneChoice)
		return;

	// Check for change and ignore if no required
	if (CPreferences::sPrefs->mUse3Pane.GetValue() == use3pane)
		return;
	
	// Try to close - may fail if user cancels
	bool cancel = false;
	try
	{
		CMailAccountManager::sMailAccountManager->GetMainView()->DoClose();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Do not change the state
		cancel = true;
	}

	// Cancel change if close did not work
	if (cancel)
		return;

	// Do the change
	CPreferences::sPrefs->mUse3Pane.SetValue(use3pane);
	
	if (use3pane)
	{
		// Close down server manager
		CMailAccountManager::sMailAccountManager->SetMainView(NULL);

		// Open server manager
		CMailAccountManager::sMailAccountManager->CreateMainView();

		// Make sure 3-pane is told to re-instate previously open mailboxes/adbks
		if (C3PaneWindow::s3PaneWindow)
		{
			C3PaneWindow::s3PaneWindow->DoneInitMailAccounts();
			C3PaneWindow::s3PaneWindow->DoneInitAdbkAccounts();
			C3PaneWindow::s3PaneWindow->DoneInitCalendarAccounts();
		}
	}
	else
	{
		// Close 3-pane
		C3PaneWindow::Destroy3PaneWindow();
		CMailAccountManager::sMailAccountManager->SetMainView(NULL);

		// Open server manager
		CMailAccountManager::sMailAccountManager->CreateMainView();

		// Create status window
		CStatusWindow::CreateWindow(this);
	}
}

void CMulberryApp::OnAppToolbarsShow()
{
	CPreferences::sPrefs->mToolbarShow.SetValue(!CPreferences::sPrefs->mToolbarShow.GetValue());
	CToolbarView::UpdateAllToolbars();
}

void CMulberryApp::OnAppToolbarsSmall()
{
	CPreferences::sPrefs->mToolbarSmallIcons.SetValue(!CPreferences::sPrefs->mToolbarSmallIcons.GetValue());
	CToolbarView::UpdateAllToolbars();
}

void CMulberryApp::OnAppToolbarsIcon()
{
	// Only if different
	if (!CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(true);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(false);
		CToolbarView::UpdateAllToolbars();
	}
}

void CMulberryApp::OnAppToolbarsCaption()
{
	// Only if different
	if (CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			!CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(false);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(true);
		CToolbarView::UpdateAllToolbars();
	}
}

void CMulberryApp::OnAppToolbarsBoth()
{
	// Only if different
	if (!CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			!CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(true);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(true);
		CToolbarView::UpdateAllToolbars();
	}
}

void CMulberryApp::OnAppSelect3PaneWindow()
{
	if (CPreferences::sPrefs->mUse3Pane.GetValue())
		FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
}

void CMulberryApp::OnAppSelectWindow(JIndex choice)
{
	JXWindowDirector* win = CWindowsMenu::GetWindow(choice);
	if (win)
		win->GetWindow()->Raise();
}

#pragma mark ____________________________Help Menu Items

const char* cUpdatesMajor = "?vmajor=";
const char* cUpdatesMinor = "&vminor=";
const char* cUpdatesBugRel = "&vbugrel=";
const char* cUpdatesStage = "&vstage=";
const char* cUpdatesNonRel = "&vnonrel=";
const char* cMulberrySupportURLInfo = "?subject=Mulberry%20Support%20Request&body=";
const char* cBuyMulberryURL = "http://www.mulberrymail.com";

// Do help
void CMulberryApp::OnAppHelp()
{
	JXGetHelpManager()->ShowSection(HELP_NAME(TOC));
}

void CMulberryApp::OnHelpDetailedTooltips()
{
	// Toggle tooltip option
	CPreferences::sPrefs->mDetailedTooltips.SetValue(!CPreferences::sPrefs->mDetailedTooltips.GetValue());
}

// Go to Mulberry web page
void CMulberryApp::OnHelpMulberryOnline()
{
	// Launch the Mulberry webpage URL
	cdstring url = CAdminLock::sAdminLock.mMulberryURL;
	LaunchURL(url);
}

// Go to Mulberry faq web page
void CMulberryApp::OnHelpOnlineFAQ()
{
	// Launch the Mulberry FAQ webpage URL
	cdstring url = CAdminLock::sAdminLock.mMulberryFAQ;
	LaunchURL(url);
}

// Go to check updates page
void CMulberryApp::OnHelpCheckUpdates()
{
	// Add version info to base URL
	cdstring url = CAdminLock::sAdminLock.mUpdateURL;
	url += cUpdatesMajor;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().majorRev));
	url += cUpdatesMinor;
	url += cdstring(static_cast<unsigned long>((GetVersionNumber().minorAndBugRev >> 4) & 0x0F));
	url += cUpdatesBugRel;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().minorAndBugRev & 0x0F));
	url += cUpdatesStage;
	switch (GetVersionNumber().stage)
	{
	case developStage:
		url += "d";
		break;
	case alphaStage:
		url += "a";
		break;
	case betaStage:
		url += "b";
		break;
	case finalStage:
		url += "r";
		break;
	default:
		url += "u";
		break;
	}
	url += cUpdatesNonRel;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().nonRelRev));

	// Launch the URL
	LaunchURL(url);
}

// Create Mulberry support message
void CMulberryApp::OnHelpMulberrySupport()
{
	// Add product info to base URL
	cdstring url = CAdminLock::sAdminLock.mMulberrySupportAddress;
	url += cMulberrySupportURLInfo;
	
	// Get details about this app
	cdstring details = CPreferences::sPrefs->GetSupportDetails();
	details.EncodeURL();
	url += details;

	ProcessMailto(url);
}

// Go to order form page
void CMulberryApp::OnHelpBuyMulberry()
{
	// Launch the Mulberry order form webpage URL
	cdstring url = cBuyMulberryURL;
	LaunchURL(url);
}

#pragma mark ____________________________Actions

// Initialise connection
void CMulberryApp::InitConnection(CPreferences& prefs)
{
	// Force connection if locked
	if (CAdminLock::sAdminLock.mNoDisconnect)
	{
		// Now set disconnected state
		prefs.mDisconnected.SetValue(false);
		CConnectionManager::sConnectionManager.SetConnected(true);
	}
	else
	{
		// Check for disconnect request via keyboard

		// Look for control key
		JXKeyModifiers modifiers(GetDisplay(1));
		GetDisplay(1)->GetCurrentButtonKeyState(NULL, &modifiers);
		bool ctrlKey = modifiers.control();
		
		// Possible disconnect prompt
		if ((!prefs.mDisconnected.GetValue() && ((prefs.mPromptDisconnected.GetValue() && CTCPSocket::WillDial()) ||
			start_disconnected)) || ctrlKey)
		{
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
													"ErrorDialog::Btn::Disconnected",
													"ErrorDialog::Btn::Connected",
													NULL,
													NULL,
													"ErrorDialog::Text::PromptDisconnect");
			prefs.mDisconnected.SetValue(result == CErrorDialog::eBtn1);
		}

		// Force connection now
		if (!prefs.mDisconnected.GetValue())
			prefs.mDisconnected.SetValue(!CConnectionManager::sConnectionManager.DoConnection(true));

		// Now set disconnected state
		CConnectionManager::sConnectionManager.SetConnected(!prefs.mDisconnected.GetValue());
	}
}

// Start the network mail controllers
void CMulberryApp::StartNetworkMail()
{
	// Only if not already there
	if (CStatusWindow::sStatusWindow)
		CStatusWindow::sStatusWindow->ResetState();

	// Do mail accounts
	if (!CMailAccountManager::sMailAccountManager)
	{
		// Start IMAP accounts
		new CMailAccountManager;
	}
	else
	{
		// Always sync accounts
		CMailAccountManager::sMailAccountManager->UpdateFavourites();
	}

	// Do SMTP accounts
	bool smtp_init_required = false;
	if (!CSMTPAccountManager::sSMTPAccountManager)
	{
		// Start SMTP accounts
		new CSMTPAccountManager;
		CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
		smtp_init_required = true;
	}
	else
	{
		// Always sync accounts
		CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
	}

	// Always do these AFTER main objects have been created
	CMailAccountManager::sMailAccountManager->SyncAccounts(CPreferences::sPrefs->mMailAccounts.GetValue());
	if (smtp_init_required)
	{
		
		// Check to see whether there are messages available and force queue window on screen
		// Check default window state for matching
		if (!CAdminLock::sAdminLock.mNoSMTPQueues && CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected() &&
			(CSMTPAccountManager::sSMTPAccountManager->ItemsHeld() ||
			 (CPreferences::sPrefs->mSMTPWindowDefault.Value().GetViewMode() == NMbox::eViewMode_ShowMatch)))
		{
			CSMTPWindow::OpenSMTPWindow();
			if (CMailAccountManager::sMailAccountManager->GetMainView()->GetServerWindow())
				FRAMEWORK_WINDOW_TO_TOP(CMailAccountManager::sMailAccountManager->GetMainView()->GetServerWindow())
		}
	}
}

// Stop the network mail controllers
void CMulberryApp::StopNetworkMail(bool quitting)
{
	// Must kill menu first to prevent unwanted updates (only if really quitting)
	if (quitting)
	{
		delete CCopyToMenu::sMailboxMainMenu;
		CCopyToMenu::sMailboxMainMenu = NULL;
		delete CApplyRulesMenu::sApplyRules;
		CApplyRulesMenu::sApplyRules = NULL;
	}

	// Need to pretend we are quitting so that server window does
	// not doe Close/Quit alert
	StValueChanger<bool> _save(mQuitting, true);

	// Check whether SMTP queue is still running
	if (CSMTPAccountManager::sSMTPAccountManager)
	{
		if (!CSMTPAccountManager::sSMTPAccountManager->TestHaltQueues())
		{
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::Wait",
																			"ErrorDialog::Btn::Postpone",
																			NULL,
																			NULL,
																			"ErrorDialog::Text::PromptWaitPostpone");

			// Wait for queues to finish
			CSMTPAccountManager::sSMTPAccountManager->SuspendAll(result == CErrorDialog::eBtn1);
		}
	}

	// Just delete the manager
	delete CMailAccountManager::sMailAccountManager;
	delete CSMTPAccountManager::sSMTPAccountManager;
}

// Start address book management
void CMulberryApp::StartAddressBooks()
{
	// Only do once!
	if (!CAddressBookManager::sAddressBookManager)
	{
		// Create address book manager
		new CAddressBookManager;

		// Force manager to update all accounts
		CAddressBookManager::sAddressBookManager->SyncAccounts();
	}
	else
	{
		// Force manager to update all accounts
		CAddressBookManager::sAddressBookManager->SyncAccounts();
	}
}

// Stop address book managment
void CMulberryApp::StopAddressBooks(bool quitting)
{
	// Need to pretend we are quitting so that adbk manager window is
	// actually closed and not hidden
	StValueChanger<bool> _save(mQuitting, true);

	// Just delete the manager & window
	delete CAddressBookManager::sAddressBookManager;
	CAddressBookManager::sAddressBookManager = NULL;

	CAdbkSearchWindow::DestroyWindow();
}

// Start calendar management
void CMulberryApp::StartCalendars()
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Only do once!
	if (calstore::CCalendarStoreManager::sCalendarStoreManager == NULL)
	{
		// Now init store
		new calstore::CCalendarStoreManager;
		calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();
	}
	else
	{
		// Force manager to update all accounts
		calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();
	}
}

// Stop calendar managment
void CMulberryApp::StopCalendars(bool quitting)
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Just delete the manager & window
	delete calstore::CCalendarStoreManager::sCalendarStoreManager;
	calstore::CCalendarStoreManager::sCalendarStoreManager = NULL;
}

// Begin IMAP - dopassword if required
bool CMulberryApp::BeginINET(CINETProtocol* proto)
{
	bool rmail = (typeid(*proto) == typeid(CMboxProtocol));
	bool canceled = false;
	bool do_open = false;

	if (proto->IsNotOpen())
		do_open = true;

	// Check that servers are the same
	else if (proto->GetDescriptor() != proto->GetAccount()->GetServerIP())
	{
		// Close and mark to re-open
		proto->Close();
		do_open = true;
	}
	
	// Must open/re-open
	if (do_open)
	{
		bool failed = false;

		try
		{
			proto->SetDescriptor(proto->GetAccount()->GetServerIP());
			proto->Open();
			
			if (rmail)
			{
				// Server name lookup may have taken place
				// Use canonical name in server window
				CServerWindow* wnd = CMailAccountManager::sMailAccountManager->GetProtocolWindow((CMboxProtocol*) proto);
				if (wnd)
					wnd->GetWindow()->SetTitle(proto->GetDescriptor());
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must reload default mailboxes as server's mailbox list now deleted
			if (rmail)
				((CMboxProtocol*) proto)->InitProtocol();

			failed = true;
		}

		// Proto may fail to open if locked by Admin
		if (failed || proto->IsNotOpen())
			return false;
	}

	// Make sure logged on to IMAP server
	cdstring actual_uid;
	if (!proto->IsLoggedOn())
	{
		try
		{
			// Try to logon
			proto->Logon();
			
			// Recover actual uid
			actual_uid = proto->GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();
			
			if (rmail)
			  {
				// See whether current proto is default or server d already set
				if (CAdminLock::sAdminLock.GetServerID().empty() ||
					(proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front()))
				  {
					// Lock return address if required
					if (CAdminLock::sAdminLock.mLockReturnAddress)
					  {
						CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
						cdstring real_name = addr.GetName();
						CPreferences::sPrefs->LockReturnAddress(actual_uid, real_name);
					  }
					
					// Set Originator-Info
					CAdminLock::sAdminLock.SetLoginID(actual_uid);
					CAdminLock::sAdminLock.SetServerID(proto->GetAccount()->GetServerIP());
				  }
			  }
		  }
		catch (...)
		  {
		  	CLOG_LOGCATCH(...);

			if (rmail)
			  {
				// See whether current proto is default
				if (proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front())
				  {
					// Null Originator-Info
					CAdminLock::sAdminLock.SetLoginID(cdstring::null_str);
					CAdminLock::sAdminLock.SetServerID(cdstring::null_str);
				  }
			  }
			
			canceled = true;
		  }
	  }

	// Recover correct MU settings if MU in use and no manual prefs setting
	if (mMUPrefsUsed && !canceled)
	{
		bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
						CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);
		// See whether current proto is default
		if (remote ||
			(proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front()))
		{
			CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
			cdstring real_name = addr.GetName();
			CPreferences::sPrefs->MultiuserConfigure(actual_uid, real_name);
			
			mMUPrefsUsed = false;
		}
	}

	// Return indicating non-cancel state
	return !canceled;
}

// Begin remote - do password if required
bool CMulberryApp::BeginRemote(CPreferences* prefs)
{
	// Create client & prefs set
	if (!sOptionsProtocol)
		sOptionsProtocol = new COptionsProtocol(prefs->mRemoteAccounts.GetValue().front());

	// Make sure accounts match
	else if (sOptionsProtocol->GetAccountUniqueness() != prefs->mRemoteAccounts.GetValue().front()->GetUniqueness())
	{
		delete sOptionsProtocol;
		sOptionsProtocol = NULL;
		sOptionsProtocol = new COptionsProtocol(prefs->mRemoteAccounts.GetValue().front());
	}
	
	// Give existing protocol new account
	else
	{
		sOptionsProtocol->SetAccount(prefs->mRemoteAccounts.GetValue().front());
	}
	if (!sRemotePrefs)
		sRemotePrefs = new CRemotePrefsSet;

	return BeginINET(sOptionsProtocol);
}

// Check for new mail
void CMulberryApp::DoMailCheck()
{
	// Check server
	if (CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->ForceMailCheck();
}


// Do preferences dialog
bool CMulberryApp::DoPreferences()
{
	CPreferencesDialog* dlog = new CPreferencesDialog(this);

	bool result = false;
	if (dlog->DoModal() == CDialogDirector::kDialogClosed_OK)
	{
		// Must make sure dialog is closed if anything fails
		try
		{
			// Tear down active accounts here BEFORE changing prefs
			// Look for complete change in prefs
			bool stopped_mail = false;
			bool stopped_adbk = false;
			bool stopped_cal = false;
			if (dlog->GetLoadedNewPrefs())
			{
				// Restart mail accounts
				if (CMailAccountManager::sMailAccountManager)
				{
					StopNetworkMail(false);
					stopped_mail = true;
				}

				// Restart addressbook accounts
				if (CAddressBookManager::sAddressBookManager)
				{
					StopAddressBooks(false);
					stopped_adbk = true;
				}

				// Restart calendar accounts
				if (iCal::CICalendarManager::sICalendarManager)
				{
					StopCalendars(false);
					stopped_cal = true;
				}
			}
			
			// Now force update of prefs
			dlog->UpdatePrefs();

			// Force colour change if required
			UpdateSelectionColour();

			// Look for complete change in prefs
			if (dlog->GetLoadedNewPrefs())
			{
				// Restart mail accounts
				if (stopped_mail)
					StartNetworkMail();

				// Restart addressbook accounts
				if (stopped_adbk)
					StartAddressBooks();

				// Restart calendar accounts
				if (stopped_cal)
					StartCalendars();
			}

			// Sync mail accounts
			if (!stopped_mail && CMailAccountManager::sMailAccountManager)
			{
				// Look for renames
				if (dlog->GetAccountRename())
				{
					CMailAccountManager::sMailAccountManager->RenameFavourites(dlog->GetRenames());
					CMailAccountManager::sMailAccountManager->RenameMRUs(dlog->GetRenames());
					
					// Not very efficient because this might get done in SyncAccounts
					CCopyToMenu::ResetMenuList(NULL, NULL);
				}
				CMailAccountManager::sMailAccountManager->SyncAccounts(CPreferences::sPrefs->mMailAccounts.GetValue());
			}
			
			// Sync SMTPs
			if (!stopped_mail && CSMTPAccountManager::sSMTPAccountManager)
			{
				// Must close SMTP queue window
				CSMTPWindow::CloseSMTPWindow();

				CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
			}

			// Sync address books
			if (!stopped_adbk && CAddressBookManager::sAddressBookManager)
				CAddressBookManager::sAddressBookManager->SyncAccounts();

			// Sync calendars
			if (!stopped_cal && calstore::CCalendarStoreManager::sCalendarStoreManager)
				calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();

			// Stop restting of multi-user prefs when logging in
			mMUPrefsUsed = false;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			dlog->Close();
			CLOG_LOGRETHROW;
			throw;
		}

		dlog->Close();
		result = true;
	}

	// Make sure accounts match - might have been changed to do set management etc
	if (sOptionsProtocol)
	{
		// Check for removal of remote prefs account
		if (CPreferences::sPrefs->mRemoteAccounts.GetValue().empty())
		{
			delete sOptionsProtocol;
			sOptionsProtocol = NULL;
		}

		// Make sure accounts match
		else if (sOptionsProtocol->GetAccountUniqueness() != CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetUniqueness())
		{
			delete sOptionsProtocol;
			sOptionsProtocol = NULL;
			sOptionsProtocol = new COptionsProtocol(CPreferences::sPrefs->mRemoteAccounts.GetValue().front());
		}

		// Give existing protocol new account
		else
			sOptionsProtocol->SetAccount(CPreferences::sPrefs->mRemoteAccounts.GetValue().front());
	}

	return result;

} // CMulberryApp::DoPreferences

// Change the color used for text/table selections
void CMulberryApp::UpdateSelectionColour()
{
	// Only do if actually changing
	if (mSelectionColour != CPreferences::sPrefs->mSelectionColour.GetValue())
	{
		mSelectionColour = CPreferences::sPrefs->mSelectionColour.GetValue();

		// Set selection color in all displays
		for(JSize index = 1; index <= GetDisplayCount(); index++)
		{
			static_cast<JXColormap*>(GetDisplay(index)->GetColormap())->SetDefaultSelectionColor(mSelectionColour);
			GetDisplay(index)->RefreshAllWindows();
		}
	}
}

// Pause periodic actions while error alert pending
void CMulberryApp::ErrorPause(bool pause)
{
	//if (sMboxProtocol)
	//	sMboxProtocol->Pause(pause);

	mPaused = pause;
}

// Check whether its the default client
void CMulberryApp::CheckDefaultMailClient()
{
	// Ignore on linux for now
}

// Check whether its the default client
void CMulberryApp::CheckDefaultWebcalClient()
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Ignore on linux for now
}

bool CMulberryApp::LaunchURL(const cdstring& url)
{
	return CURLHelpersMap::LaunchURL(url);
}

#if NOTYET
#pragma mark ____________________________Notification

void CMulberryApp::ShowNotification()
{
	if (!mNotificationOn)
	{
		CString s;
		s.LoadString(IDS_NOTIFYICON_TIP);
		HICON icon = CMulberryApp::sApp->LoadIcon(IDR_MAINFRAME);
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = CMulberryApp::sApp->m_pMainWnd->GetSafeHwnd();
		nid.uID = MYWM_NOTIFYICON;
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		nid.uCallbackMessage = MYWM_NOTIFYICON;
		nid.hIcon = icon;
		::strncpy(nid.szTip, s, sizeof(nid.szTip) - 1);
		
		// Do notification
		Shell_NotifyIcon(NIM_ADD, &nid);
		
		DestroyIcon(icon);
		
		mNotificationOn = true;
	}
}

void CMulberryApp::HideNotification()
{
	if (mNotificationOn)
	{
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = m_pMainWnd->GetSafeHwnd();
		nid.uID = MYWM_NOTIFYICON;
		
		// Remove notification
		Shell_NotifyIcon(NIM_DELETE, &nid);
		
		mNotificationOn = false;
	}
}

#endif

#pragma mark ____________________________Sleep

void CMulberryApp::OnPowerSignal(int param)
{
	if (!mSleeping)
		CMulberryApp::sApp->GoToSleep(true);
	else
		CMulberryApp::sApp->WakeFromSleep();
}

// Computer is being put to sleep
bool CMulberryApp::GoToSleep(bool force)
{
	// Not if already in sleep state
	if (mSleeping)
		return true;

	CLOG_LOGCATCH(Sleep Request);

	// Tell it to suspend checks/sending etc
	CConnectionManager::sConnectionManager.Suspend();
	
	// Turn off periodic processing
	if (sMailCheck)
		sMailCheck->Suspend();

	mSleeping = true;
	return true;
}

// Computer woken from sleep
void CMulberryApp::WakeFromSleep(bool silent)
{
	// Not if already out of sleep state
	if (!mSleeping)
		return;

	CLOG_LOGCATCH(Wake Request);

	// Turn onperiodic processing - this will check the connection state etc
	if (sMailCheck)
		sMailCheck->Resume(silent);
	
	// Tell it to resume checks/sending etc
	CConnectionManager::sConnectionManager.Resume();
	
	mSleeping = false;
}

#pragma mark ____________________________Other Bits

JBoolean CMulberryApp::HandleCustomEvent() 
{
  if (!startedYet) {
	startedYet = true;
	try
	{
		StartUp();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Quit the app - couldn't start up!
		Quit();
	}
  }
  return kTrue;
}

void CMulberryApp::Quit()
{
	mQuitting = true;

	JXApplication::Quit();
}

void CMulberryApp::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnUpdateOpenDraft(cmdui);
		return;
	case CCommand::eFileOpenMailbox:
		OnUpdateServerManager(cmdui);
		return;
	case CCommand::eFileOpenDraft:
		OnUpdateServerManager(cmdui);
		return;
	case CCommand::eFilePreferences:
		OnUpdateAlways(cmdui);
		return;
	case CCommand::eFileDisconnect:
		OnUpdateDisconnected(cmdui);
		return;
	case CCommand::eFileShowOutgoingQueues:
		OnUpdateOpenSMTPQueue(cmdui);
		return;
	case CCommand::eFileExit:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eEditTextMacros:
		OnUpdateAlways(cmdui);
		return;
	case CCommand::eEditChangePassword:
		OnUpdateChangePassword(cmdui);
		return;

	case CCommand::eMailboxCreate:
	case CCommand::eToolbarCreateMailboxBtn:
		OnUpdateServerManager(cmdui);
		return;
	case CCommand::eMailboxCheckFavourites:
		OnUpdateLoggedIn(cmdui);
		return;
	case CCommand::eMailboxSearch:
		OnUpdateLoggedIn(cmdui);
		return;
	case CCommand::eMailboxRules:
		// Only if not locked out
		OnUpdateAppRules(cmdui);
		return;

	case CCommand::eAddressesAddBookMan:
	case CCommand::eAddressesNew:
	case CCommand::eAddressesOpen:
	case CCommand::eAddressSearch:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eCalendarManager:
	case CCommand::eCalendarSubscribe:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eWindowsLayout:
	case CCommand::eWindowsOptions:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eLayout3Pane:
		OnUpdateAppUse3Pane(cmdui);
		return;

	case CCommand::eLayoutSingle:
		OnUpdateAppUseSeparatePanes(cmdui);
		return;

	case CCommand::eWindowsToolbars:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eToolbarsSmall:
		OnUpdateAppToolbarsSmall(cmdui);
		return;

	case CCommand::eToolbarsIcons:
		OnUpdateAppToolbarsIcon(cmdui);
		return;

	case CCommand::eToolbarsCaptions:
		OnUpdateAppToolbarsCaption(cmdui);
		return;

	case CCommand::eToolbarsBoth:
		OnUpdateAppToolbarsBoth(cmdui);
		return;

	case CCommand::eToolbarsButtons:
		OnUpdateAppToolbarsShow(cmdui);
		return;

	case CCommand::eWindowsStatus:
		cmdui->Enable(CStatusWindow::sStatusWindow != NULL);
		cmdui->SetCheck(CStatusWindow::sStatusWindow && CStatusWindow::sStatusWindow->GetWindow()->IsVisible());
		return;

	case CCommand::eHelpHelpTopics:
	case CCommand::eHelpAboutMul:
	case CCommand::eHelpAboutPlugins:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eHelpDetailedTooltips:
		OnUpdateHelpDetailedTooltips(cmdui);
		return;

	case CCommand::eHelpMulberryOnline:
	case CCommand::eHelpMulberryFAQ:
	case CCommand::eHelpMulberryUpdates:
	case CCommand::eHelpMulberrySupport:
		OnUpdateAlways(cmdui);
		return;
	}

	CCommander::UpdateCommand(cmd, cmdui);
}

bool CMulberryApp::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnAppNewDraft();
		return true;
	case CCommand::eFileOpenMailbox:
		OnAppOpenMailbox();
		return true;
	case CCommand::eFileOpenDraft:
		OnAppOpenDraft();
		return true;
	case CCommand::eFilePreferences:
		OnAppPreferences();
		return true;
	case CCommand::eFileDisconnect:
		OnAppDisconnected();
		return true;
	case CCommand::eFileShowOutgoingQueues:
		OnAppOpenSMTPQueue();
		return true;
	case CCommand::eFileExit:
		Quit();
		return true;

	case CCommand::eEditTextMacros:
		OnAppTextMacros();
		return true;
	case CCommand::eEditChangePassword:
		OnAppChangePassword();
		return true;

	case CCommand::eMailboxCreate:
	case CCommand::eToolbarCreateMailboxBtn:
		OnAppCreateMailbox();
		return true;
	case CCommand::eMailboxCheckFavourites:
		OnAppMailCheck();
		return true;
	case CCommand::eMailboxSearch:
		OnAppSearch();
		return true;
	case CCommand::eMailboxRules:
		OnAppRules();
		return true;


	case CCommand::eAddressesAddBookMan:
		OnAppAddressBookManager();
		return true;
	case CCommand::eAddressesNew:
		OnAppNewAddressBook();
		return true;
	case CCommand::eAddressesOpen:
		OnAppOpenAddressBook();
		return true;
	case CCommand::eAddressSearch:
		OnAppSearchAddress();
		return true;

	case CCommand::eCalendarManager:
		OnAppCalendarManager();
		return true;
	case CCommand::eCalendarSubscribe:
		OnAppCalendarSubscribed();
		return true;

	case CCommand::eLayout3Pane:
		DoSet3Pane(true);
		return true;

	case CCommand::eLayoutSingle:
		DoSet3Pane(false);
		return true;

	case CCommand::eToolbarsButtons:
		OnAppToolbarsShow();
		return true;

	case CCommand::eToolbarsSmall:
		OnAppToolbarsSmall();
		return true;

	case CCommand::eToolbarsIcons:
		OnAppToolbarsIcon();
		return true;

	case CCommand::eToolbarsCaptions:
		OnAppToolbarsCaption();
		return true;

	case CCommand::eToolbarsBoth:
		OnAppToolbarsBoth();
		return true;

	case CCommand::eWindowsOptions:
		OnWindowOptions();
		return true;

	// Windows menu selection handled in CWindow etc
	case CCommand::eWindowsStatus:
		if (CStatusWindow::sStatusWindow->GetWindow()->IsVisible())
			CStatusWindow::sStatusWindow->GetWindow()->Hide();
		else
			CStatusWindow::sStatusWindow->GetWindow()->Show();
		return true;

	case CCommand::eHelpHelpTopics:
		OnAppHelp();
		return true;

	case CCommand::eHelpAboutMul:
		OnAppAbout();
		return true;
	case CCommand::eHelpAboutPlugins:
		OnAppAboutPlugins();
		return true;

	case CCommand::eHelpDetailedTooltips:
		OnHelpDetailedTooltips();
		return true;

	case CCommand::eHelpMulberryOnline:
		OnHelpMulberryOnline();
		return true;

	case CCommand::eHelpMulberryFAQ:
		OnHelpOnlineFAQ();
		return true;

	case CCommand::eHelpMulberryUpdates:
		OnHelpCheckUpdates();
		return true;

	case CCommand::eHelpMulberrySupport:
		OnHelpMulberrySupport();
		return true;
	}

	return CCommander::ObeyCommand(cmd, menu);
}
