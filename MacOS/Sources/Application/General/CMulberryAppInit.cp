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


// Source for CMulberryApp class

#define REG_PROTECTION		0
#define DATE_PROTECTION		0

#define	COPYP_MAX_YEAR	2005
#define COPYP_MAX_MONTH	5

#include "CMulberryApp.h"

#include "CAdbkManagerWindow.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CBalloonDialog.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CConfigPlugin.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CFileOptionsMap.h"
#include "CFindReplaceWindow.h"
#include "CSearchWindow.h"
#include "CGetStringDialog.h"
#include "CHelpMenuAttachment.h"
#include "CICSupport.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMulberryCommon.h"
#include "CMultiUserDialog.h"
#include "COptionClick.h"
#include "COptionsProtocol.h"
#include "CPasswordManagerKeychain.h"
#include "CPeriodicCheck.h"
#include "CPluginManager.h"
#include "CPPRegister.h"
#include "CPreferences.h"
#include "CPreferencesFile.h"
#include "CRegistration.h"
#include "CRegistrationDialog.h"
#include "CRemotePrefsSets.h"
#include "CResources.h"
#include "CRulesWindow.h"
#include "CScrollWheelAttachment.h"
#include "CServerWindow.h"
#include "CSpeechSynthesis.h"
#include "CSplashScreen.h"
#include "CStatusWindow.h"
#include "CUtils.h"
#include "CWindowsMenu.h"
#include "C3PaneWindow.h"

#include "CCarbonMBAR.h"

#include "CICalendarManager.h"

#include <AppleHelp.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdstring.h"

// Startup
void CMulberryApp::Initialize()
{
	short thisResFile = ::CurResFile();

	// Required for floating desktop
	UScreenPort::Initialize();

	// Register resource based classes
	RegisterClasses();

	// Add speech menus if available
	CSpeechSynthesis::InstallSpeechSynthesis();

	::UseResFile(thisResFile);

	// Get process information for this app
	ThrowIfOSErr_(::GetCurrentProcess(&sPSN));
	sPInfo.processInfoLength = sizeof(ProcessInfoRec);
	sPInfo.processName = nil;
	sPInfo.processAppSpec = &sPSpec;
	ThrowIfOSErr_(::GetProcessInformation(&sPSN, &sPInfo));

	AddAttachment(new COptionMenu);

	// Start context menus
	AddAttachment(new CContextMenuProcessAttachment(this));

	AddAttachment(new CScrollWheelAttachment);

	AddAttachment(mHelpMenu = new CHelpMenuAttachment(MENU_HelpExtras));

	// Only register help on OS X as Carbon on classic seems to recognise the Guide file
	if (UEnvironment::IsRunningOSX())
	{
		CFBundleRef myAppsBundle;
		CFURLRef myBundleURL;
		FSRef myBundleRef;
		OSStatus err;

		    /* set up a known state */
		myAppsBundle = NULL;
		myBundleURL = NULL;

		    /* Get our application's main bundle
		    from Core Foundation */
		myAppsBundle = CFBundleGetMainBundle();
		if (myAppsBundle == NULL) { err = fnfErr; goto bail;}

		    /* retrieve the URL to our bundle */
		myBundleURL = CFBundleCopyBundleURL(myAppsBundle);
		if (myBundleURL == nil) { err = fnfErr; goto bail;}

		    /* convert the URL to a FSRef */
		if ( ! CFURLGetFSRef(myBundleURL, &myBundleRef) ) {
		    err = fnfErr;
		    goto bail;
		}

		    /* register our application's help book */
		err = AHRegisterHelpBook(&myBundleRef);
		if (err != noErr) goto bail;

		    /* done */
bail:
		if (myBundleURL != NULL) CFRelease(myBundleURL);
	}
	else
	{
	}

	// Init static of this app
	sApp = this;

	// Create Calendar manager
	new iCal::CICalendarManager;
	iCal::CICalendarManager::sICalendarManager->InitManager();
	
	// Create Keychain manager
	CPasswordManagerKeychain::MakePasswordManagerKeychain();

	// Now do standard app init
	OpenUp(true);
}

void CMulberryApp::MakeMenuBar()
{
	ResIDT	MBARid = MBAR_Standard;
	
	if (UEnvironment::HasFeature(env_HasAquaTheme)) {
		MBARid = MBAR_Aqua;
	}
	
	new CCarbonMBAR(MBARid);
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

	// Read in default resources
	ReadDefaults();

	mFailedProtect = false;

	// Create dynamic menus
	if (first_time)
		new CWindowsMenu;
	CCopyToMenu::sMailboxMainMenu = new CCopyToMenu;
	CCopyToMenu::sMailboxMainMenu->ResetMenuList();	// Must do here because required in prefs dlog
	CApplyRulesMenu::sApplyRules = new CApplyRulesMenu;

	sMailCheckReset = new CPeriodicReset;
	AddAttachment(sMailCheckReset);
	sMailCheck = new CPeriodicCheck;
	sMailCheck->StartIdling();

	// Do splash screen if correctly registered and first time through

	// Display splash if first time through (could be recyling app)
	// and valid registration
	if (first_time)
	{

		CSplashScreen* splash = (CSplashScreen*) CSplashScreen::CreateWindow(paneid_SplashScreen, this);
		splash->Activate();
		splash->Draw(nil);
		long ticker = ::TickCount();

		QDFlushPortBuffer(splash->GetMacPort(), NULL);

		// Wait for end of splash
		while(::TickCount() - ticker < 180) ;

		delete splash;
	}

	// Create status window
	{
		// Determine OS
		long gestalt_result;
		::Gestalt(gestaltSystemVersion, &gestalt_result);
		bool OS8 = (gestalt_result > 0x07FF);
		CStatusWindow::CreateWindow(OS8 ? paneid_StatusWindow8 : paneid_StatusWindow, this);
	}
	
	// Warn about logging
	if (CLog::AnyActiveLogs())
	{
		if (::CautionAlert(207, NULL) != kOK_Btn)
			CLog::DisableActiveLogs();
	}
}

// Close down app without quit
void CMulberryApp::CloseDown()
{
	// Destroy some windows here
	CFindReplaceWindow::DestroyFindReplaceWindow();
	CSearchWindow::DestroySearchWindow();
	CRulesWindow::DestroyRulesWindow();
	CAdbkManagerWindow::DestroyAdbkManagerWindow();
	CCalendarStoreWindow::DestroyCalendarStoreWindow();
	CCalendarWindow::CloseAllWindows();

	// Stop network ops
	StopNetworkMail(true);
	StopAddressBooks(true);
	StopCalendars(true);

	// Delete all server windows now to get correct window state in saved prefs
	{
		cdmutexprotect<CServerWindow::CServerWindowList>::lock _lock(CServerWindow::sServerWindows);
		for(CServerWindow::CServerWindowList::reverse_iterator riter = CServerWindow::sServerWindows->rbegin();
				riter != CServerWindow::sServerWindows->rend(); riter++)
			(*riter)->GetServerView()->DoClose();
	}

	C3PaneWindow::Destroy3PaneWindow();

	// Must get status window state before saving prefs
	if (CStatusWindow::sStatusWindow)
		CStatusWindow::sStatusWindow->SaveState();

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
	delete sRemotePrefs;
	sRemotePrefs = NULL;

	// Remove password cache after last protocol is shut down
	CINETProtocol::ClearUserPswdCache();

	// Also do the status window
	delete CStatusWindow::sStatusWindow;

	delete CPreferences::sPrefs;
	CPreferences::sPrefs = NULL;

	CLog::StopLogging();
	delete sMailCheck;
	sMailCheck = NULL;
	delete sMailCheckReset;
	sMailCheckReset = NULL;

	// Reset switches
	mStartInit = false;
	mPrefsLoaded = false;
	mMUPrefsUsed = false;
	mPaused = false;
}

// Read in default resources
void CMulberryApp::ReadDefaults(void)
{
	// Get version handles of this app
	sVersion1 = (VersRecHndl) ::GetResource('vers', 1);
	if (sVersion1)
		::DetachResource((Handle) sVersion1);
	sVersion2 = (VersRecHndl) ::GetResource('vers', 2);
	if (sVersion2)
		::DetachResource((Handle) sVersion2);

	// Must force current prefs to new version
	CPreferences::sPrefs->vers = GetVersionNumber(1);

	// Get app registration info
	CRegistration::sRegistration.LoadAppInfo(0);

	// Now reset statically created default prefs file
	CPreferences::sPrefs->ResetAllStates(false);
}

// Do copy protection
bool CMulberryApp::Protect(void)
{
	// Only if not done before
	if (mDoneProtect)
		return true;

#if DATE_PROTECTION
	time_t systime = ::time(nil);
	struct tm* currtime = ::localtime(&systime);

	// Look for expirary
	if ((currtime->tm_year + 1900 > COPYP_MAX_YEAR) ||
		((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 > COPYP_MAX_MONTH)))
	{
		// Force quit if over time limit
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoMoreTime");
		return false;
	}
	
	// Look for expirary within a week
	else if ((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 == COPYP_MAX_MONTH))
	{
		// Check day
		bool alert_needed = false;
		unsigned long last_day = 0;
		switch(currtime->tm_mon + 1)
		{
		case 1:		// January
		case 3: 	// March
		case 5: 	// May
		case 7: 	// July
		case 8: 	// August
		case 10:	// October
		case 12:	// December
			last_day = 31;
			break;
		case 4:		// April
		case 6:		// June
		case 9:		// September
		case 11:	// December
			last_day = 30;
			break;
		case 2:		// February
			last_day = (currtime->tm_year % 4) ? 28 : 29;
			break;
		}
		
		if (currtime->tm_mday > last_day - 7)
		{
			// Put up alert warning of time left
			cdstring txt;
			txt.FromResource("Alerts::General::BetaTimeleft");
			txt.Substitute(last_day - currtime->tm_mday);
			CErrorHandler::PutNoteAlert(txt);
		}
	}
#endif

#if REG_PROTECTION
	if (DoRegistration(true))
	{
#endif
		// Initialise plugins now
		PluginInit();
	
		mDoneProtect = true;
		
		// Strip out help menu Buy item if this is registered
#if REG_PROTECTION
		if (!CMulberryApp::sApp->IsDemo())
#endif
			AdjustHelpMenu();

		return true;
#if REG_PROTECTION
	}
	else
		return false;
#endif
}

// Do registration dialog
bool CMulberryApp::DoRegistration(bool initial)
{
	// Check against actual CRC
	if (!initial ||
		!CRegistration::sRegistration.CheckAppRegistration(true) && !IsDemo() ||
		!CRegistration::sRegistration.ValidSerial(CRegistration::sRegistration.GetSerialNumber()))
	{
		// Try to re-register
		if (CRegistrationDialog::DoRegistration(initial))
		{
			if (IsDemo())
				return true;

			// Reload registration details
			CRegistration::sRegistration.LoadAppInfo(0);

			// Recalc. and match CRC
			if (CRegistration::sRegistration.CheckAppRegistration(true))
				return true;
		}
		mFailedProtect = true;
		return false;
	}
	
	return true;
}

// Do copy protection
void CMulberryApp::PluginInit(void)
{
	// Do plug-in search/registration here
	CPluginManager::sPluginManager.LookforPlugins();

	// Check for spelling and remove spell menu items
	if (!CPluginManager::sPluginManager.HasSpelling())
	{
		LMenu* edit = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_Edit);
		SInt16 index = edit->IndexFromCommand(cmd_SpellCheck);
		edit->RemoveItem(index + 1);
		edit->RemoveItem(index);
		edit->RemoveItem(index - 1);
	}

	// Check for security and remove security menu items
	if (!CPluginManager::sPluginManager.HasSecurity())
	{
		LMenu* msgs = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_Messages);
		SInt16 index = msgs->IndexFromCommand(cmd_VerifyDecrypt);
		msgs->RemoveItem(index);

		LMenu* drafts = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_Drafts);
		index = drafts->IndexFromCommand(cmd_SignDraft);
		for(short i = -1; i < 2; i++)
			drafts->RemoveItem(index - 1);
	}
}

// App launched without documents
void CMulberryApp::StartUp(void)
{
	// Flag init start
	mStartInit = true;

	// Do protection first
	if (!Protect())
	{
		DoQuit();
		return;
	}

	// Must attempt to load Internet Config first
	if ((CICSupport::ICStart(kApplID) == noErr) && CICSupport::ICFindConfigFile(sPSpec))
		CICSupport::ICStop();

	// Set it to nil to force prefs to ask save as
	sCurrentPrefsFile = nil;

	COptionsMap* muser_prefs = nil;

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
				LMenu* calendar = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_Calendar);
				LMenuBar::GetCurrentMenuBar()->RemoveMenu(calendar);
			}
			// Look in system folder for prefs file if requested
			if (!CAdminLock::sAdminLock.mAllowDefault || !DoDefaultPrefsFile())
			{
				// If no sys folder prefs try multi-user
				if (CAdminLock::sAdminLock.mUseMultiUser && DoMultiuser(muser_prefs) ||
					!CAdminLock::sAdminLock.mUseMultiUser && ForcePrefsSetting())
					DoPostPrefsStartup();
				else
					DoQuit();
			}

			delete muser_prefs;
			muser_prefs = nil;
		}
		else
		{

			// If sys folder prefs does not exist force prefs to be set
			if (!DoDefaultPrefsFile() && ForcePrefsSetting())

				// Do remaining init
				DoPostPrefsStartup();
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

// Try to load default prefs file
bool CMulberryApp::DoDefaultPrefsFile(void)
{
	// Locate suitable file in prefs folder in System Folder
	FSRef prefs;
	OSErr err = ::FSFindFolder(kUserDomain, kPreferencesFolderType, kCreateFolder, &prefs);
	if (err != noErr)
		err = ::FSFindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &prefs);
	if (err == noErr)
	{
		FSIterator iterator;
		err = ::FSOpenIterator(&prefs, kFSIterateFlat, &iterator);
		if (err == noErr)
		{
			ItemCount itemCount;
			FSCatalogInfo cinfo;
			FSRef ref;
			while(::FSGetCatalogInfoBulk(iterator, 1, &itemCount, NULL, kFSCatInfoFinderInfo, &cinfo, &ref, NULL, NULL) == noErr)
			{
				FinderInfo* finfo = (FinderInfo*)&cinfo.finderInfo;
				if ((finfo->file.fileType == kPrefFileType) &&
					(finfo->file.fileCreator == kPrefFileCreator))
				{
					::FSCloseIterator(iterator);

					// Open default prefs file and set as current
					PPx::FSObject file(ref);
					OpenDocument(&file);
					return true;
				}
			}
			::FSCloseIterator(iterator);
		}
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

	// Check for availability of Internet Config
	if (CPreferences::sPrefs->mUse_IC.GetValue() && !CICSupport::ICInstalled())
	{
		// Force quit if not available
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoForceInternetConfig");
		return false;
	}

	// Determine whether remote login to be used
	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : 0);

	// Do multi-user prefs dialog
	bool okayed = false;
	if (!CPluginManager::sPluginManager.HasConfig() ||
		CPluginManager::sPluginManager.GetConfig()->PromptStartup())
	{
		// Create the dialog
		CBalloonDialog	theHandler(CAdminLock::sAdminLock.mAskRealName && !remote ? paneid_MultiUserRealNameDialog : paneid_MultiUserDialog, this);
		theHandler.StartDialog();
		bool caps_lock = false;

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Update prefs from dialog
				((CMultiUserDialog*) theHandler.GetDialog())->UpdatePrefs();
				okayed = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
			else if (hitMessage == msg_Nothing)
			{
				// Check for caps lock
				bool current_caps_lock = ::GetCurrentKeyModifiers() & alphaLock;
				if (current_caps_lock != caps_lock)
				{
					CMultiUserDialog* dlog = (CMultiUserDialog*) theHandler.GetDialog();
					dlog->CapsLockChange(current_caps_lock);
					caps_lock = current_caps_lock;
				}
			}
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
				// Read in prefs but do not do immediate write-back
				bool pending_update = false;
				if (!sCurrentPrefsFile->VerifyRead(false, false, &pending_update))
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}

				// Do virtual domain config if required
				if (CAdminLock::sAdminLock.mLockVirtualDomain)
					CAdminLock::sAdminLock.VirtualDomainPrefs(sCurrentPrefsFile->GetPrefs(), cache_uid);

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
				sCurrentPrefsFile = nil;
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
			if (CGetStringDialog::PoseDialog("Alerts::General::GetRealName", real_name))
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

// Get multiuser prefs with update
COptionsMap* CMulberryApp::GetMultiuser(void)
{
	// Look for multi-user prefs in app
	Handle muser_prefs = ::GetResource(krsrcType, krsrcID);
	CFileOptionsMap* map = nil;

	// Check for multi-user use
	if (muser_prefs)
	{
		// Create handle stream of MU prefs data
		::DetachResource(muser_prefs);
		LHandleStream aStream(muser_prefs);

		// Create file options map and give archive to it
		map = new CFileOptionsMap;
		map->SetStream(&aStream);

		// Read the map from stream
		map->ReadMap();

		// Clean up
		map->SetStream(nil);
	}

	return map;
}

// Read certain multi-user prefs
void CMulberryApp::ReadMultiuser(COptionsMap* muser_prefs)
{
	CPreferences* newPrefs = nil;

	try
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
		newPrefs = nil;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		delete newPrefs;

		CLOG_LOGRETHROW;
		throw;
	}
}

// Force setting of prefs, quit if cancel
bool CMulberryApp::ForcePrefsSetting(void)
{
	// Must have account manager before doing prefs dialog
	// This will sync with empty prefs
	new CMailAccountManager;

	// Also must force connection active to allow creation and use of IMSP
	CConnectionManager::sConnectionManager.SetConnected(true);

	// Force setting of prefs
	if (DoMyPreferences()) {

		// Force quit if cancelled
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoPrefsRun");
		DoQuit();
		return false;
	}

	return true;

} // CMulberryApp:ForcePrefsSetting

// Check whether its the default client
void CMulberryApp::CheckDefaultMailClient()
{
	// See whether check has been turned off
	if (!CPreferences::sPrefs->mCheckDefaultMailClient.GetValue())
		return;
	
	// Check to see whether Mulberry is set as the default client in internet config
	if (!CICSupport::ICCheckHelperMailto())
	{
		bool dont_check = false;
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::DefaultMailto", "Alerts::General::DoOnStartup", &dont_check) == CErrorHandler::Ok)
			CICSupport::ICSetHelperMailto();
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
	if (!CICSupport::ICCheckHelperWebcal())
	{
		bool dont_check = false;
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::DefaultWebcal", "Alerts::General::DoOnStartup", &dont_check) == CErrorHandler::Ok)
			CICSupport::ICSetHelperWebcal();
		CPreferences::sPrefs->mCheckDefaultWebcalClient.SetValue(!dont_check);
	}
}
