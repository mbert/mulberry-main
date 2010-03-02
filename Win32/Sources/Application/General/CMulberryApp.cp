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

//#define DEBUG_STARTUP

#include "CMulberryApp.h"

#include "CActionManager.h"
#include "CAboutBox.h"
#include "CAboutPluginsDialog.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkProtocol.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookDoc.h"
#include "CAddressBookDocTemplate.h"
#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CAttachmentManager.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CChangePswdAcctDialog.h"
#include "CConfigPlugin.h"
#include "CCopyToMenu.h"
#include "CConnectionManager.h"
#include "CDisconnectDialog.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFileOptionsMap.h"
#include "CFindReplaceWindow.h"
#include "CFontCache.h"
#include "CGetPassphraseDialog.h"
#include "CGetStringDialog.h"
#include "CIMAPClient.h"
#include "CINETAccount.h"
#include "CLetterDoc.h"
#include "CLetterWindow.h"
#include "CMacroEditDialog.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMboxProtocol.h"
#include "CMultiUserDialog.h"
#include "CMulberryWindow.h"
#include "COptionsMap.h"
#include "COptionsProtocol.h"
#include "CPasswordManagerKeyring.h"
#include "CPeriodicCheck.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferenceKeys.h"
#include "CPreferencesDialog.h"
#include "CPreferencesFile.h"
#include "CRegistration.h"
#include "CRemotePrefsSets.h"
#include "CRulesWindow.h"
//#include "CSafetySave.h"
#include "CSDIFrame.h"
#include "CSearchEngine.h"
#include "CSearchWindow.h"
#include "CServerWindow.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CStatusWindow.h"
#include "CTaskQueue.h"
#include "CTCPSocket.h"
#include "CToolbarView.h"
#include "CClickElement.h"
#include "CUnicodeUtils.h"
#include "CUserPswdDialog.h"
#include "CWindowOptionsDialog.h"
#include "CWinRegistry.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "CICalendarManager.h"

#include "CCalendarStoreManager.h"

#pragma mark ________________________Message Map

BEGIN_MESSAGE_MAP(CMulberryApp, CWinApp)

	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(IDM_PLUGINS_ABOUT, OnAppAboutPlugins)
	ON_UPDATE_COMMAND_UI(IDM_HELP_DETAILED_TOOLTIPS, OnUpdateHelpDetailedTooltips)
	ON_COMMAND(IDM_HELP_DETAILED_TOOLTIPS, OnHelpDetailedTooltips)
	ON_COMMAND(IDM_HELP_MULBERRY_ONLINE, OnHelpMulberryOnline)
	ON_COMMAND(IDM_HELP_ONLINE_FAQ, OnHelpOnlineFAQ)
	ON_COMMAND(IDM_HELP_CHECK_UPDATES, OnHelpCheckUpdates)
	ON_COMMAND(IDM_HELP_MULBERRY_SUPPORT, OnHelpMulberrySupport)
	ON_COMMAND(IDM_HELP_BUY_MULBERRY, OnHelpBuyMulberry)

	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnAppNewDraft)
	ON_UPDATE_COMMAND_UI(IDM_FILE_OPEN_MAILBOX, OnUpdateServerManager)
	ON_COMMAND(IDM_FILE_OPEN_MAILBOX, OnAppOpenMailbox)
	ON_COMMAND(IDM_FILE_OPEN_DRAFT, OnAppOpenDraft)
	ON_UPDATE_COMMAND_UI(IDM_FILE_OPEN_DRAFT, OnUpdateOpenDraft)
	ON_COMMAND(IDM_FILE_PREFERENCES, OnAppPreferences)
	ON_UPDATE_COMMAND_UI(IDM_FILE_DISCONNECTED, OnUpdateDisconnected)
	ON_COMMAND(IDM_FILE_DISCONNECTED, OnAppDisconnected)
	ON_UPDATE_COMMAND_UI(IDM_FILE_OPENSMTPQUEUES, OnUpdateOpenSMTPQueue)
	ON_COMMAND(IDM_FILE_OPENSMTPQUEUES, OnAppOpenSMTPQueue)
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)

	ON_COMMAND(IDM_EDIT_TEXTMACROS, OnAppTextMacros)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_CHANGEPASSWORD, OnUpdateChangePassword)
	ON_COMMAND(IDM_EDIT_CHANGEPASSWORD, OnAppChangePassword)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CREATE, OnUpdateServerManager)
	ON_COMMAND(IDM_MAILBOX_CREATE, OnAppCreateMailbox)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_AUTOCHECK, OnUpdateLoggedIn)
	ON_COMMAND(IDM_MAILBOX_AUTOCHECK, OnAppMailCheck)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SEARCH, OnUpdateLoggedIn)
	ON_COMMAND(IDM_MAILBOX_SEARCH, OnAppSearch)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_RULES, OnUpdateAppRules)
	ON_COMMAND(IDM_MAILBOX_RULES, OnAppRules)

	ON_COMMAND(IDM_ADDR_MANAGER, OnAppAddressBookManager)
	ON_COMMAND(IDM_ADDR_NEW, OnAppNewAddressBook)
	ON_COMMAND(IDM_ADDR_OPEN, OnAppOpenAddressBook)
	ON_COMMAND_RANGE(IDM_AddressImportStart, IDM_AddressImportEnd, OnAppAddrImportExport)
	ON_COMMAND_RANGE(IDM_AddressExportStart, IDM_AddressExportEnd, OnAppAddrImportExport)
	ON_COMMAND(IDM_ADDR_SEARCH, OnAppSearchAddress)
	
	ON_COMMAND(IDM_CALENDAR_MANAGER, OnAppCalendarManager)
	ON_COMMAND(IDM_CALENDAR_SUBSCRIBED, OnAppCalendarSubscribed)

	ON_COMMAND(IDM_WINDOWS_OPTIONS, OnWindowOptions)

	ON_UPDATE_COMMAND_UI(IDM_LAYOUT_3PANE, OnUpdateAppUse3Pane)
	ON_COMMAND(IDM_LAYOUT_3PANE, OnAppUse3Pane)
	ON_UPDATE_COMMAND_UI(IDM_LAYOUT_SEPARATE, OnUpdateAppUseSeparatePanes)
	ON_COMMAND(IDM_LAYOUT_SEPARATE, OnAppUseSeparatePanes)

	ON_UPDATE_COMMAND_UI(IDM_TOOLBARS_BUTTONS, OnUpdateAppToolbarsShow)
	ON_COMMAND(IDM_TOOLBARS_BUTTONS, OnAppToolbarsShow)

	ON_UPDATE_COMMAND_UI(IDM_TOOLBARS_SMALL, OnUpdateAppToolbarsSmall)
	ON_COMMAND(IDM_TOOLBARS_SMALL, OnAppToolbarsSmall)

	ON_UPDATE_COMMAND_UI(IDM_TOOLBARS_ICONS, OnUpdateAppToolbarsIcon)
	ON_COMMAND(IDM_TOOLBARS_ICONS, OnAppToolbarsIcon)
	ON_UPDATE_COMMAND_UI(IDM_TOOLBARS_CAPTIONS, OnUpdateAppToolbarsCaption)
	ON_COMMAND(IDM_TOOLBARS_CAPTIONS, OnAppToolbarsCaption)
	ON_UPDATE_COMMAND_UI(IDM_TOOLBARS_BOTH, OnUpdateAppToolbarsBoth)
	ON_COMMAND(IDM_TOOLBARS_BOTH, OnAppToolbarsBoth)

	ON_COMMAND(AFX_IDM_FIRST_MDICHILD, OnAppSelect3PaneWindow)
	ON_COMMAND_RANGE(IDM_WINDOWS_First, IDM_WINDOWS_Last, OnAppSelectWindow)

	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnAppNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnAppNewDraft)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARCREATEMAILBOXBTN, OnUpdateServerManager)
	ON_COMMAND(IDC_TOOLBARCREATEMAILBOXBTN, OnAppCreateMailbox)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSEARCHMAILBOXBTN, OnUpdateLoggedIn)
	ON_COMMAND(IDC_TOOLBARSEARCHMAILBOXBTN, OnAppSearch)

	ON_COMMAND(IDC_TOOLBARADBKMGRNEWBTN, OnAppNewAddressBook)
	ON_COMMAND(IDC_TOOLBARADBKMGROPENBTN, OnAppOpenAddressBook)
	ON_COMMAND(IDC_TOOLBARADBKMGRSEARCHBTN, OnAppSearchAddress)

END_MESSAGE_MAP()

//CMulberryApp NEAR theApp;
CMulberryApp NEAR theApp;

#pragma mark ________________________Statics

CMulberryApp*		CMulberryApp::sApp = NULL;				// This application
CFont*				CMulberryApp::sAppFont = NULL;			// The application font
CFont*				CMulberryApp::sAppFontBold = NULL;		// The application font in bold
CFont*				CMulberryApp::sAppSmallFont = NULL;		// The button font
BOOL				CMulberryApp::sLargeFont = false;		// Large font flag
COptionsProtocol*	CMulberryApp::sOptionsProtocol = NULL;	// The Options client
CRemotePrefsSet*	CMulberryApp::sRemotePrefs = NULL;		// Set of remote prefs
CPreferencesFile*	CMulberryApp::sCurrentPrefsFile = NULL;	// The current preferences file
CPeriodicCheck*		CMulberryApp::sMailCheck = NULL;			// The periodic mail check task
bool				CMulberryApp::sMultiTaskbar = true;		// Use multiple taskbar items
bool				CMulberryApp::sCommonControlsEx = false;	// Can use CommonControlsEx
cdstring			CMulberryApp::sPostponeDDE;				// Postponed DDE command appearing at startup

CLIPFORMAT			CMulberryApp::sFlavorServerList = 0;
CLIPFORMAT			CMulberryApp::sFlavorWDList = 0;
CLIPFORMAT			CMulberryApp::sFlavorMboxList = 0;
CLIPFORMAT			CMulberryApp::sFlavorMboxRefList = 0;

CLIPFORMAT			CMulberryApp::sFlavorMsgList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorAddrList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorAdbkAddrList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorGrpList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorAtchList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorMsgAtchList = 0;
CLIPFORMAT 			CMulberryApp::sFlavorRuleType = 0;

CLIPFORMAT 			CMulberryApp::sFlavorCalServer = 0;
CLIPFORMAT 			CMulberryApp::sFlavorCalendar = 0;
CLIPFORMAT 			CMulberryApp::sFlavorCalendarItem = 0;

#pragma mark ________________________Methods

// Default consructor
CMulberryApp::CMulberryApp()
{
	// Init static of this app
	sApp = this;
	mPaused = false;
	mSleeping = false;
	mQuitting = false;
	mStartInit = false;

	// Set prefs load flag
	mPrefsLoaded = false;
	mMUPrefsUsed = false;

	mRunDemo = false;
	mDemoTimeout = 0;
	
	mNotificationOn = false;
	
	mPageSetupDone = false;
	mPageMargins = CRect(750, 500, 750, 500);
	
	// We now use HTML help
	EnableHtmlHelp();

}

// Open special file
void CMulberryApp::RegisterClipboard()
{
	// Servers
	sFlavorServerList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Server List"));

	// WDs
	sFlavorWDList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Search Hierarchy List"));

	// Mboxes
	sFlavorMboxList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Mailbox List"));

	// Mbox Refs
	sFlavorMboxRefList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Mailbox Reference List"));

	// Messages
	sFlavorMsgList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Message List"));

	// Addresses
	sFlavorAddrList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Address List"));
	sFlavorAdbkAddrList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Adbk Address List"));

	// Groups
	sFlavorGrpList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Group List"));

	// Outgoing attachments
	sFlavorAtchList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Outgoing Attachments"));

	// Incoming attachments
	sFlavorMsgAtchList = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Incoming Attachments"));

	// Incoming attachments
	sFlavorRuleType = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Rules"));

	// Calendar servers
	sFlavorCalServer = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Calendar Servers"));
	
	// Calendars
	sFlavorCalendar = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Calendars"));
	
	// Calendar items (e.g. events)
	sFlavorCalendarItem = (CLIPFORMAT)::RegisterClipboardFormat(_T("Mulberry Calendar Items"));
}

// Open special file
CDocument* CMulberryApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	bool failure = false;

#ifdef DEBUG_STARTUP
	::AfxMessageBox("CMulberryApp::OpenDocumentFile");
	::AfxMessageBox(lpszFileName);
#endif

	// Check for first time ( = no current prefs file)
	cdstring fname(lpszFileName);
	if (!mPrefsLoaded)
	{
		// Check for preferences file
		if (fname.compare_end(cPrefFileExtension, true))
		{
			// Set start flag
			mStartInit = true;

			// Try multi-user first
			COptionsMap* muser_prefs = NULL;
			try
			{
				// Look for multi-user prefs in profile
				muser_prefs = GetMultiuser();

				// Check for multi-user use
				if (muser_prefs)
				{
					// First read in MU prefs and cache 'special' values
					// Must do this before getting admin, as admin may lock out changes to prefs
					ReadMultiuser(muser_prefs);

					// Get admin details from MU prefs
					CAdminLock::sAdminLock.ReadFromMap(muser_prefs);

					delete muser_prefs;
					muser_prefs = NULL;
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Clean up and throw up
				delete muser_prefs;
				::PostQuitMessage(0);

				CLOG_LOGRETHROW;
				throw;
			}

			// Must reject if local prefs denied
			if (CAdminLock::sAdminLock.mNoLocalPrefs)
				return NULL;

			// Create the prefs file
			sCurrentPrefsFile = new CPreferencesFile(fname.length() == 4, false);

			// Set the FSSpec of the prefs file
			sCurrentPrefsFile->SetSpecifier(fname.length() == 4 ? cdstring::null_str : fname);
			sCurrentPrefsFile->SetSpecified(fname.length() != 4);

			// Read in new prefs
			bool read_ok = sCurrentPrefsFile->VerifyRead(true);

			// If not OK must remove prefs file
			if (!read_ok)
			{
				delete sCurrentPrefsFile->GetPrefs();
				delete sCurrentPrefsFile;
				sCurrentPrefsFile = NULL;
			}
			else
				// Make it global prefs
				sCurrentPrefsFile->GetPrefs()->SetAsDefault();

			// Verify the prefs read in
			if (!read_ok || !sCurrentPrefsFile->GetPrefs()->Valid(false))

				// Force setting of prefs - throw if fail
				if (!ForcePrefsSetting())
				{
					::PostQuitMessage(0);
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

			// Do remaining init
			DoPostPrefsStartup();

			return NULL;
		}
		else
		{
			try
			{
				// Try default startup (will throw if failed)
				StartUp();
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				failure = true;
			}
			
			// Do not allow startup without prefs
			if (failure)
				::PostQuitMessage(0);
		}
	}

	// Fall through to here if DDE command startup with non-prefs file
	if (!failure)
	{
		// Check for preferences file
		if (fname.compare_end(cPrefFileExtension, true))
		{
			// Must reject if local prefs denied
			if (CAdminLock::sAdminLock.mNoLocalPrefs)
				return NULL;

			// Check to see if the same, in which case ignore it
			if (sCurrentPrefsFile && (sCurrentPrefsFile->GetSpecifier() == fname))
				return NULL;

			// Ask whether replace is really wanted
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::ReallyReplacePrefs") == CErrorHandler::Cancel)
				return NULL;

			// Create new prefs file
			CPreferencesFile* newPrefsFile = new CPreferencesFile(false, false);

			// Set the FSSpec of the prefs file
			newPrefsFile->SetSpecifier(fname);
			newPrefsFile->SetSpecified(true);

			// Read in new prefs
			bool read_ok = newPrefsFile->VerifyRead(true);

			// Verify the prefs read in
			if (!read_ok || !newPrefsFile->GetPrefs()->Valid(false))
			{
				// Inform of error and exit without file and new prefs
				if (read_ok)
					CErrorHandler::PutStopAlertRsrc("Alerts::General::InvalidPrefsInFile");
				delete newPrefsFile->GetPrefs();
				delete newPrefsFile;
				return NULL;
			}

			// Make new prefs the default
			newPrefsFile->GetPrefs()->SetAsDefault();

			// Delete existing file object
			delete sCurrentPrefsFile;

			// Replace existing file
			sCurrentPrefsFile = newPrefsFile;

			// Restart with new prefs
			DoPostPrefsStartup();

			return NULL;
		}

#if 0
		// Check for address book file
		else if (fname.compare_end(".mba", true))
		{
			if (CAdminLock::sAdminLock.mNoLocalAdbks)
			{
				::MessageBeep(-1);
				return NULL;
			}

			// Check for existing file
			CAddressBook* adbk = (CAddressBook*) CAddressBookManager::sAddressBookManager->CheckLocalOpen(fname);
			
			// Does window already exist?
			if (adbk)
			{
				CAddressBookWindow* theWindow = CAddressBookWindow::FindWindow(adbk);
		
				if (theWindow)
				{
					// Found existing window so select
					FRAMEWORK_WINDOW_TO_TOP(theWindow)
				}
				adbk = NULL;
			}
			else
			{
				try
				{
					// Create address book
					adbk = new CLocalAddressBook(fname);
					
					// Open it
					adbk->Open();
					
					// If not opened (ie failed) delete it
					if (!adbk->IsOpen())
					{
						delete adbk;
						adbk = NULL;
					}
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Remove from manager
					if (adbk)
						adbk->Close();
					delete adbk;
					adbk = NULL;
				}
			}
			
			return (adbk ? CAddressBookWindow::FindWindow(adbk)->GetDocument() : NULL);
		}
#endif

		// Check for draft file
		else if (fname.compare_end(".mbd", true))
		{
			if (CAdminLock::sAdminLock.mNoLocalDrafts)
			{
				::MessageBeep(-1);
				return NULL;
			}

			return CWinApp::OpenDocumentFile(fname.win_str());
		}
		else
		{
			::MessageBeep(-1);
			::ModalMessageBox(_T("Could not open file."), MB_OK | MB_ICONSTOP);
			return NULL;
		}
	}
	
	return NULL;
}

#pragma mark ________________________

// App launched without documents
void CMulberryApp::StartUp()
{
	// Flag init start
	mStartInit = true;

	// Set it to NULL to force prefs to ask save as
	sCurrentPrefsFile = NULL;

	COptionsMap* muser_prefs = NULL;

	try
	{
		// Look for multi-user prefs in profile
		muser_prefs = GetMultiuser();

		// Check for multi-user use
		if (muser_prefs)
		{
			// First read in MU prefs and cache 'special' values
			// Must do this before getting admin, as admin may lock out changes to prefs
			ReadMultiuser(muser_prefs);

			// Get admin details from MU prefs
			CAdminLock::sAdminLock.ReadFromMap(muser_prefs);


			// Check for calendar and remove calendar menu items
			if (CAdminLock::sAdminLock.mPreventCalendars)
			{
				CMenu* menu = CMulberryApp::sApp->m_pMainWnd->GetMenu();
				menu->DeleteMenu(5, MF_BYPOSITION);
			}

			// Look in system folder for prefs file if requested
			if (!CAdminLock::sAdminLock.mAllowDefault || !DoDefaultPrefsFile())
			{
				// If no sys folder prefs try multi-user
				if (CAdminLock::sAdminLock.mUseMultiUser && DoMultiuser(muser_prefs) ||
					!CAdminLock::sAdminLock.mUseMultiUser && ForcePrefsSetting())
					DoPostPrefsStartup();
				else
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}

			delete muser_prefs;
			muser_prefs = NULL;
		}
		else
		{

			// If sys folder prefs does not exist force prefs to be set
			if (!DoDefaultPrefsFile())
			{
				if (ForcePrefsSetting())

					// Do remaining init
					DoPostPrefsStartup();

				else
				{
					// Failed to set valid prefs
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		delete muser_prefs;

		CLOG_LOGRETHROW;
		throw;
	}

} // CMulberryApp::StartUp

BOOL CMulberryApp::OnDDECommand(LPTSTR lpszCommand)
{
#ifdef DEBUG_STARTUP
	::AfxMessageBox("OnDDECommand");
	::AfxMessageBox(lpszCommand);
#endif
	if (CWinApp::OnDDECommand(lpszCommand))
		return TRUE;
	
	// Handle any DDE commands recognized by your application
	// and return TRUE.  See implementation of CWinApp::OnDDEComand
	// for example of parsing the DDE command string.

	// url format is "[url("%s")]" - no whitespace allowed, one per line
	CString strCommand = lpszCommand;
	if (strCommand.Left(6) == _T("[url(\""))
	{
		strCommand = strCommand.Right(strCommand.GetLength() - 6);

		int i = strCommand.Find('"');
		if (i == -1)
			return FALSE; // illegally terminated
	
		strCommand = strCommand.Left(i);
		
		// Bring main window to front
		m_pMainWnd->BringWindowToTop();
		if (m_pMainWnd->IsIconic())
			m_pMainWnd->ShowWindow(SW_RESTORE);
		if (m_pMainWnd->GetLastActivePopup() != m_pMainWnd)
			m_pMainWnd->GetLastActivePopup()->BringWindowToTop();

		if (mPrefsLoaded)
			ProcessURL(cdstring(strCommand));
		else
			sPostponeDDE = strCommand;

		return TRUE;
	}
	else if (strCommand.Left(6) == _T("[ping]"))
	{
		return TRUE;
	}

	// Return FALSE for any DDE commands you do not handle.
	return FALSE;
 }

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
	
#ifdef DEBUG_STARTUP
	::AfxMessageBox("ProcessMailto");
	::AfxMessageBox(mailto);
#endif
	CURLClickElement::ParseMailto(mailto, to, cc, bcc, subject, body, files);
	CAddressList toField(to.c_str(), to.length());
	CAddressList ccField(cc.c_str(), cc.length());
	CAddressList bccField(bcc.c_str(), bcc.length());

#ifdef DEBUG_STARTUP
	::AfxMessageBox("CActionManager::NewDraft for mailto:");
#endif
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

// Try to load default prefs file
bool CMulberryApp::DoDefaultPrefsFile()
{
	// Look for [PREFERENCES] VERSION in profile
	CString version = GetProfileString(cdstring(cPrefsSection).win_str(), cdstring(cVersionKey_v2_0).win_str());
	if (!version.GetLength())
		version = GetProfileString(cdstring(cPrefsSection).win_str(), cdstring(cVersionKey_v1_4).win_str());

	// If found open as if double-clicked
	if (version.GetLength() != 0)
	{
		// Open default prefs file and set as current
		OpenDocumentFile(cdstring(cPrefFileExtension).win_str());
		return true;
	}

	return false;
}

// Do multiuser dialog
bool CMulberryApp::DoMultiuser(COptionsMap* muser_prefs)
{
	// Set multi-user prefs as apps prefs
	CPreferences* newPrefs = new CPreferences(*CPreferences::sPrefs);
	NumVersion vers_app = GetVersionNumber();
	NumVersion vers_prefs;
	newPrefs->ReadFromMap(muser_prefs, vers_app, vers_prefs);
	newPrefs->CheckIC();
	newPrefs->SetAsDefault();

	// Determine whether remote login to be used
	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : 0);

	// Do multi-user prefs dialog
	bool okayed = false;
	if (!CPluginManager::sPluginManager.HasConfig() ||
		CPluginManager::sPluginManager.GetConfig()->PromptStartup())
	{
		// Create the dialog
		CMultiUserDialog dlog(CAdminLock::sAdminLock.mAskRealName && !remote, CSDIFrame::GetAppTopWindow());
		
		dlog.mServerTxt = remote ?
							CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetServerIP() :
							CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetServerIP();

		dlog.mVersion = CPreferences::sPrefs->GetVersionText();
		dlog.mSerial = CRegistration::sRegistration.GetSerialNumber();
		dlog.mLicensedTo = CRegistration::sRegistration.GetLicensee();
		if (CRegistration::sRegistration.GetOrganisation().length() > 0)
		{
			dlog.mLicensedTo += "\r\n";
			dlog.mLicensedTo += CRegistration::sRegistration.GetOrganisation();
		}

		// Let DialogHandler process events
		int result = dlog.DoModal();

		if (result == IDOK)
		{
			// Update prefs from dialog
			dlog.UpdatePrefs();
			okayed = true;
		}
	}
	else
	{
		// Do plugin configuration without any input
		CPluginManager::sPluginManager.GetConfig()->DoConfiguration(NULL, NULL, NULL, NULL);
		okayed = true;
	}

	if (okayed)
	{
		// Flag use of MU prefs before remote login
		mMUPrefsUsed = true;

		// Do remote logon
		bool failure = false;
		if (remote)
		{
			// Determine if UIDs in use
			bool use_uid = CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().RequiresUserPswd();
			cdstring cache_uid;
			if (use_uid)
				cache_uid = CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorUserPswd()->GetUID();

			// Create the prefs file
			sCurrentPrefsFile = new CPreferencesFile(true, true);

			// Set the FSSpec of the prefs file
			sCurrentPrefsFile->SetSpecified(false);

			// Read in new prefs
			sCurrentPrefsFile->SetPrefs(CPreferences::sPrefs);
			CAdminLock::sAdminLock.PrepareRemote();
			try
			{
				// Read it in but do not do write-back
				bool pending_update = false;
				if (!sCurrentPrefsFile->VerifyRead(false, false, &pending_update))
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}

				// Make it global prefs
				sCurrentPrefsFile->GetPrefs()->SetAsDefault();

				// Copy uid from suitable authenticator
				if (!use_uid)
					cache_uid = CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetAuthenticator()->GetActualUID();

				// Make sure empty remote prefs gets proper uids
				if (CPreferences::sPrefs->TransferUIDs(cache_uid) || pending_update)
					sCurrentPrefsFile->SavePrefs(false, pending_update);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Mark failure if local not supported
				failure = CAdminLock::sAdminLock.mNoLocalPrefs;
				delete sCurrentPrefsFile;
				sCurrentPrefsFile = NULL;
			}
				
		}

		// Make sure prefs are valid before allowing success
		okayed = !failure && CPreferences::sPrefs->Valid(true);
	}

	// May need to check for real name
	if (okayed && CAdminLock::sAdminLock.mAskRealName && remote)
	{
		// Check whether real name exists in first identity
		CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
		
		if (addr.GetName().empty())
		{
			// Ask for real name
			cdstring real_name;
			if (CGetStringDialog::PoseDialog("Alerts::General::GetRealName_Title", "Alerts::General::GetRealName", real_name))
			{
				// Insert into first identity
				addr.SetName(real_name);
				CIdentity& first = CPreferences::sPrefs->mIdentities.Value().front();
				first.SetFrom(addr.GetFullAddress(), first.UseFrom());
				CPreferences::sPrefs->mIdentities.SetDirty();
			}
		}
	}

	return okayed;

}

const TCHAR cMUResourceName[] = _T("TEXT");

// Get multiuser prefs with update
COptionsMap* CMulberryApp::GetMultiuser()
{
	// Look for multiuser resource
	HRSRC hrsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDT_MULTIUSER), cMUResourceName);
	if (!hrsrc)
		return NULL;

	// Try to load resource
	HGLOBAL hResource = ::LoadResource(AfxGetResourceHandle(), hrsrc);
	if (!hResource)
		return NULL;

	char* p = (char*) ::LockResource(hResource);
	DWORD rsrc_size = ::SizeofResource(AfxGetResourceHandle(), hrsrc);

	// Look for proper key at start
	const char* MUKey = "MULTIUSERPREFERENCESPACE";
	int len = ::strlen(MUKey) + 2;
	if ((::memcmp(p, MUKey, len - 2) != 0) || (p[len] == '*'))
	{
		//::FreeResource(hResource);	<- Crash on NT
		return NULL;
	}
	
	// Bump past key
	p += len;
	rsrc_size -= len;

	// Create mem file for resource data and archive
	CMemFile file((unsigned char*) p, rsrc_size);
	CArchive ar(&file, CArchive::load);

	// Create file options map and give archive to it
	CFileOptionsMap* map = new CFileOptionsMap;
	map->SetArchive(&ar);

	// Read the map from archive
	map->ReadMap();
	
	// Clean up
	map->SetArchive(NULL);
	ar.Close();
	file.Close();

	//::FreeResource(hResource);	<- Crash on NT

	return map;
}

// Read certain multi-user prefs
void CMulberryApp::ReadMultiuser(COptionsMap* muser_prefs)
{
	CPreferences* newPrefs = NULL;

	try
	{
		// Check for multi-user use
		if (muser_prefs)
		{
			// Read in prefs (if old version write it back out to sync)
			newPrefs = new CPreferences(*CPreferences::sPrefs);
			NumVersion vers_app = GetVersionNumber();
			NumVersion vers_prefs;
			if (!newPrefs->ReadFromMap(muser_prefs, vers_app, vers_prefs))
				newPrefs->WriteToMap(muser_prefs, false);

			// Extract admin specific values from prefs
			CAdminLock::sAdminLock.ProcessPrefs(newPrefs);

			delete newPrefs;
			newPrefs = NULL;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		delete muser_prefs;
		delete newPrefs;

		CLOG_LOGRETHROW;
		throw;
	}
}

// Force setting of prefs, quit if cancel
bool CMulberryApp::ForcePrefsSetting()
{
	// Must have account manager before doing prefs dialog
	// This will sync with empty prefs
	new CMailAccountManager;

	// Also must force connection active to allow creation and use of IMSP
	CConnectionManager::sConnectionManager.SetConnected(true);

	// Force setting of prefs
	if (!DoPreferences())
	{
		// Stick up error
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoPrefsRun");
		return false;
	}

	return true;

} // CMulberryApp:ForcePrefsSetting

// Terminate
BOOL CMulberryApp::OnClose()
{
	// Check for held messages in SMTP queue when disconnecting
	if (CConnectionManager::sConnectionManager.IsConnected() &&
		CSMTPAccountManager::sSMTPAccountManager &&
		CSMTPAccountManager::sSMTPAccountManager->PendingItems())
	{
		// Ask user
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::ReallyDisconnectSMTPHeldItems") == CErrorHandler::Cancel)
			return false;
	}

	// Must do this first to prevent dangling drafts
	if (!SaveAllModified())
		return false;

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
			return false;
	}

	// Look for locked quit
	if (CAdminLock::sAdminLock.mNoQuit)
	{
		// See if allowed to quit by existing docs
		if ((!(::GetKeyState(VK_MENU) < 0) || !(::GetKeyState(VK_SHIFT) < 0) || !(::GetKeyState(VK_CONTROL) < 0)) && AttemptClose())
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
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Failure => do quit again to recycle
				::PostQuitMessage(0);
			}
			//CAdminLock::sAdminLock.mNoQuit = true;
			
			// Done
			return false;
		}
	}
	else if (!AttemptClose())
		return false;

	CloseDown();

	return true;
}

// Terminate
BOOL CMulberryApp::AttemptClose()
{
	mQuitting = true;

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
				mQuitting = false;
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
			mQuitting = false;
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
		CCopyToMenu::sMailboxMainMenu->ResetMenuList();	// Must do here because required in prefs dlog
	}
	if (!CApplyRulesMenu::sApplyRules)
		CApplyRulesMenu::sApplyRules = new CApplyRulesMenu;
		
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
	mQuitting = true;

	CloseAllDocuments(false);

	// Stop network mail ops
	StopNetworkMail(true);
	StopAddressBooks(true);
	StopCalendars(true);

	// Must close any open Find & Replace dialog
	if (CFindReplaceWindow::sFindReplaceWindow)
	{
		CFindReplaceWindow::sFindReplaceWindow->DestroyWindow();
		CFindReplaceWindow::sFindReplaceWindow = NULL;
	}

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

	delete CPreferences::sPrefs;
	CPreferences::sPrefs = NULL;
	
	// Stop remote options protocol
	delete sOptionsProtocol;
	sOptionsProtocol = NULL;
	delete sRemotePrefs;
	sRemotePrefs = NULL;
}

int CMulberryApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
	ASSERT_VALID(this);

	// disable windows for modal dialog
	EnableModeless(FALSE);
	HWND hWndTop = CSDIFrame::GetAppTopWindow()->GetSafeHwnd();
	CWnd* pWnd = CSDIFrame::GetAppTopWindow();

	// set help context if possible
	DWORD* pdwContext = &m_dwPromptContext;
	if (pWnd != NULL)
	{
		// use app-level context or frame level context
		ASSERT_VALID(pWnd);
		CWnd* pMainWnd = pWnd->GetTopLevelParent();
		ASSERT_VALID(pMainWnd);
		if (pMainWnd->IsFrameWnd())
			pdwContext = &((CFrameWnd*)pMainWnd)->m_dwPromptContext;
	}

	ASSERT(pdwContext != NULL);
	DWORD dwOldPromptContext = *pdwContext;
	if (nIDPrompt != 0)
		*pdwContext = HID_BASE_PROMPT+nIDPrompt;

	// determine icon based on type specified
	if ((nType & MB_ICONMASK) == 0)
	{
		switch (nType & MB_TYPEMASK)
		{
		case MB_OK:
		case MB_OKCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_YESNO:
		case MB_YESNOCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_ABORTRETRYIGNORE:
		case MB_RETRYCANCEL:
			// No default icon for these types, since they are rarely used.
			// The caller should specify the icon.
			break;

#ifdef _MAC
		case MB_SAVEDONTSAVECANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;
#endif
		}
	}

#ifdef _DEBUG
	if ((nType & MB_ICONMASK) == 0)
		TRACE0("Warning: no icon specified for message box.\n");
#endif

	// Must pause tasks while alert is on screen
	CTaskQueue::sTaskQueue.Pause(true);
	int nResult = 0;
	try
	{
		nResult = ::MessageBox(pWnd->GetSafeHwnd(), lpszPrompt, m_pszAppName, nType);
	}
	catch(...)
	{
		// Don't allow throws out of here
	}
	CTaskQueue::sTaskQueue.Pause(false);
	*pdwContext = dwOldPromptContext;

	// re-enable windows
	if (hWndTop != NULL)
		::EnableWindow(hWndTop, TRUE);
	EnableModeless(TRUE);

	return nResult;
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
	CString txt;
	txt.LoadString(CConnectionManager::sConnectionManager.IsConnected() ? IDS_DISCONNECT_CMD_TEXT : IDS_CONNECT_CMD_TEXT);
	OnUpdateMenuTitle(pCmdUI, txt);
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
	CString s;
	s.LoadString(CPreferences::sPrefs->mToolbarShow.GetValue() ? IDS_WINDOWSHIDETOOLBAR : IDS_WINDOWSSHOWTOOLBAR);
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
	HWND old_focus = CWnd::GetFocus()->GetSafeHwnd();

	{
		CAboutBox dlog(CSDIFrame::GetAppTopWindow());
		dlog.mVersion = CPreferences::sPrefs->GetVersionText();
		
		// Process dialog
		dlog.DoModal();
	}
	
	if (old_focus && ::IsWindow(old_focus))
		::SetFocus(old_focus);
}

// Do about box
void CMulberryApp::OnAppAboutPlugins()
{
	HWND old_focus = CWnd::GetFocus()->GetSafeHwnd();

	{
		CAboutPluginsDialog dlog(CSDIFrame::GetAppTopWindow());

		// Process dialog
		dlog.DoModal();
	}
	
	if (old_focus && ::IsWindow(old_focus))
		::SetFocus(old_focus);
}

void CMulberryApp::OnAppNewDraft()
{
	CActionManager::NewDraft();
}

void CMulberryApp::OnAppOpenMailbox()
{
	// Pass on to server manager window (must exist due to command status)
	CMailAccountManager::sMailAccountManager->GetMainView()->GetTable()->SendMessage(WM_COMMAND, IDM_FILE_OPEN_MAILBOX);
}

void CMulberryApp::OnAppOpenDraft()
{
	// prompt the user (with all document templates)
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, CLetterWindow::sLetterDocTemplate))
		return; // open cancelled

	OpenDocumentFile(newName);
		// if returns NULL, the user has already been alerted
}

void CMulberryApp::OnAppPreferences()
{
	// Look for alt key and logging allowed
	if ((::GetKeyState(VK_MENU) < 0) && CAdminLock::sAdminLock.mAllowLogging)
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

void CMulberryApp::OnFilePageSetup()
{
	CPageSetupDialog ps(PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES);
	ps.m_psd.rtMargin = mPageMargins;
	if (ps.DoModal() == IDOK)
	{
		SelectPrinter(ps.m_psd.hDevNames, ps.m_psd.hDevMode);
		ps.GetMargins(mPageMargins, NULL);
		mPageSetupDone = true;
	}
}

void CMulberryApp::OnAppTextMacros()
{
	CMacroEditDialog::PoseDialog();
}

void CMulberryApp::OnAppChangePassword()
{
	CChangePswdAcctDialog::AcctPasswordChange();
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

void CMulberryApp::OnAppAddrImportExport(UINT nID)
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
	{
		FRAMEWORK_WINDOW_TO_TOP(CAdbkSearchWindow::sAdbkSearch)
	}
	else
	{
		CAdbkSearchWindow* adbksrch = CAdbkSearchWindow::ManualCreate();
		adbksrch->ResetState();
	}
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
	{
		if (AfxGetMainWnd()->IsIconic())
			AfxGetMainWnd()->ShowWindow(SW_RESTORE);
		AfxGetMainWnd()->BringWindowToTop();
	}
}

void CMulberryApp::OnAppSelectWindow(UINT nID)
{
	CSDIFrame::SelectWindowItem(nID);
}

#pragma mark ____________________________Help Menu Items

const char* cUpdatesMajor = "?vmajor=";
const char* cUpdatesMinor = "&vminor=";
const char* cUpdatesBugRel = "&vbugrel=";
const char* cUpdatesStage = "&vstage=";
const char* cUpdatesNonRel = "&vnonrel=";
const char* cMulberrySupportURLInfo = "?subject=Mulberry%20Support%20Request&body=";
const char* cBuyMulberryURL = "http://www.mulberrymail.com";


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
		bool ctrlKey = (::GetKeyState(VK_CONTROL) < 0);
		
		// Possible disconnect prompt
		if (!prefs.mDisconnected.GetValue() && prefs.mPromptDisconnected.GetValue() && CTCPSocket::WillDial() ||
			ctrlKey)
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

//extern char sCrashLogDir[1024];		// Crash dump log path defined in CExceptionFix.cp
char sCrashLogDir[1024] = {0};		// Crash dump log path defined in CExceptionFix.cp

// Init after prefs set
void CMulberryApp::DoPostPrefsStartup()
{
	try
	{
		// Set flag to indicate one prefs read in
		mPrefsLoaded = true;

		// Check for default mail/webcal client
		CheckDefaultMailClient();
		CheckDefaultWebcalClient();

		// Init MDI/SDI state
		InitWindowState();

		// Reset main window state
		((CMulberryWindow*) m_pMainWnd)->RestoreState();

		// Possible disconnect prompt
		InitConnection(*CPreferences::sPrefs);

		// Start address books
		StartAddressBooks();

		// Start network mail ops
		StartNetworkMail();

		// Start calendars
		StartCalendars();

		// Must do initial address book open at start for 3-pane after starting mail
		CActionManager::Open3PaneAddressBook();

		if (!sPostponeDDE.empty())
			ProcessURL(sPostponeDDE);
		
		// Recover saved drafts
		CLetterDoc::ReadTemporary();

		// Setup crash dump log path after preferences have been set
		::strcpy(sCrashLogDir, CConnectionManager::sConnectionManager.GetCWD().c_str());
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Failure during start up - throw up
		CLOG_LOGRETHROW;
		throw;
	}

} // CMulberryApp::DoPostPrefsStartup

// Start the network mail controllers
void CMulberryApp::StartNetworkMail()
{
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
			{
				FRAMEWORK_WINDOW_TO_TOP(CMailAccountManager::sMailAccountManager->GetMainView()->GetServerWindow())
			}
		}
	}

} // CMulberryApp::StartNetworkMail

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
	delete CSMTPAccountManager::sSMTPAccountManager;
	delete CMailAccountManager::sMailAccountManager;
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
	// Just delete the manager & window
	delete CAddressBookManager::sAddressBookManager;
	CAddressBookManager::sAddressBookManager = NULL;
	delete CAdbkSearchWindow::sAdbkSearch;
	CAdbkSearchWindow::sAdbkSearch = NULL;
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

// Begin IMAP - do password if required
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
					CUnicodeUtils::SetWindowTextUTF8(wnd, proto->GetDescriptor());
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
	CPreferencesDialog dlog(CSDIFrame::GetAppTopWindow());

	bool result = false;
	if (dlog.DoModal() == IDOK)
	{
		// Tear down active accounts here BEFORE changing prefs
		// Look for complete change in prefs
		bool stopped_mail = false;
		bool stopped_adbk = false;
		bool stopped_cal = false;
		if (dlog.GetLoadedNewPrefs())
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
		dlog.UpdatePrefs();

		// Init MDI/SDI state - may get here during startup but before MDI/SDI doc templates are defined
		InitWindowState();

		// Look for complete change in prefs
		if (dlog.GetLoadedNewPrefs())
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
			if (dlog.GetAccountRename())
			{
				CMailAccountManager::sMailAccountManager->RenameFavourites(dlog.GetRenames());
				CMailAccountManager::sMailAccountManager->RenameMRUs(dlog.GetRenames());
				
				// Not very efficient because this might get done in SyncAccounts
				CCopyToMenu::ResetMenuList();
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
	// See whether check has been turned off
	if (!CPreferences::sPrefs->mCheckDefaultMailClient.GetValue())
		return;
	
	// Check to see whether Mulberry is set as the default client in internet config
	if (!CWinRegistry::CheckHelperMailto())
	{
		bool dont_check = false;
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::DefaultMailto", "Alerts::General::DoOnStartup", &dont_check) == CErrorHandler::Ok)
			CWinRegistry::SetHelperMailto();
		CPreferences::sPrefs->mCheckDefaultMailClient.SetValue(!dont_check);
	}
}

// Check whether its the default client
void CMulberryApp::CheckDefaultWebcalClient()
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// See whether check has been turned off
	if (!CPreferences::sPrefs->mCheckDefaultWebcalClient.GetValue())
		return;
	
	// Check to see whether Mulberry is set as the default client in internet config
	if (!CWinRegistry::CheckHelperWebcal())
	{
		bool dont_check = false;
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::DefaultWebcal", "Alerts::General::DoOnStartup", &dont_check) == CErrorHandler::Ok)
			CWinRegistry::SetHelperWebcal();
		CPreferences::sPrefs->mCheckDefaultWebcalClient.SetValue(!dont_check);
	}
}

bool CMulberryApp::IsMAPIInstalled() const
{
	return false;
}

void CMulberryApp::InstallMAPI(bool install) const
{
}

bool CMulberryApp::LaunchURL(const cdstring& url)
{
	TCHAR dir[MAX_PATH];
	if (::GetCurrentDirectory(MAX_PATH, dir))
	{
		HINSTANCE hinst = ::ShellExecute(*::AfxGetMainWnd(), _T("open"), url.win_str(), NULL, dir, SW_SHOWNORMAL);
		
		return (((int) hinst) > 32);
	}
	else
		return false;
}

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
		::lstrcpyn(nid.szTip, s, sizeof(nid.szTip) - 1);
		
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
#pragma mark ____________________________Sleep

LRESULT CMulberryApp::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	{
		char buffer[256];
		::snprintf(buffer, 256, "OnPowerBroadcast wParam = %d, lParam = %d, sleeping = %s", wParam, lParam, mSleeping ? "true" : "false");
		CLOG_LOGERROR(buffer);
	}

	switch(wParam)
	{
	case PBT_APMQUERYSUSPEND:
	case PBT_APMQUERYSTANDBY:
	case PBT_APMSUSPEND:
	case PBT_APMSTANDBY:
		return CMulberryApp::sApp->GoToSleep((lParam & 1) != 0) ? TRUE : BROADCAST_QUERY_DENY;

	case PBT_APMQUERYSUSPENDFAILED:
	case PBT_APMQUERYSTANDBYFAILED:
		// Force out of sleep state as this message means we did not get a suspend
		// before the actual sleep took place, but we must now try to recover.
		// Since there was no real machine sleep, do the wake silently.
		mSleeping = true;
		CMulberryApp::sApp->WakeFromSleep(true);
		return TRUE;

	case PBT_APMRESUMECRITICAL:
	case PBT_APMRESUMESUSPEND:
	case PBT_APMRESUMESTANDBY:
		CMulberryApp::sApp->WakeFromSleep();
		return TRUE;
	}
	
	return TRUE;
}

// Computer is being put to sleep
bool CMulberryApp::GoToSleep(bool force)
{
	CLOG_LOGERROR(force ? "GoToSleep - force" : "GoToSleep - !force");

	// Not if already in sleep state
	if (mSleeping)
	{
		CLOG_LOGERROR("GoToSleep - Already sleeping");
		return true;
	}

	// Tell it to suspend checks/sending etc
	CConnectionManager::sConnectionManager.Suspend();
	
	// Turn off periodic processing
	if (sMailCheck)
		sMailCheck->Suspend();

	mSleeping = true;
	CLOG_LOGERROR("GoToSleep - done");
	return true;
}

// Computer woken from sleep
void CMulberryApp::WakeFromSleep(bool silent)
{
	CLOG_LOGERROR(silent ? "WakeFromSleep - silent" : "WakeFromSleep - !silent");

	// Not if already out of sleep state
	if (!mSleeping)
	{
		CLOG_LOGERROR("WakeFromSleep - Not sleeping");
		return;
	}

	// Turn onperiodic processing - this will check the connection state etc
	if (sMailCheck)
		sMailCheck->Resume(silent);
	
	// Tell it to resume checks/sending etc
	CConnectionManager::sConnectionManager.Resume();
	
	mSleeping = false;

	CLOG_LOGERROR("WakeFromSleep - done");
}
