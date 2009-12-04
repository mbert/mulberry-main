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


// Source for CMulberryApp class

#include "CMulberryApp.h"

#include "CActionManager.h"
#include "CAdminLock.h"
#include "CConfigPlugin.h"
#include "CConnectionManager.h"
#include "CGeneralException.h"
#include "CGetStringDialog.h"
#include "CErrorHandler.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailcapMap.h"
#include "CMIMETypesMap.h"
#include "CMultiUserDialog.h"
#include "CPeriodicCheck.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesFile.h"
#include "CSplashScreen.h"
#include "CStringOptionsMap.h"
#include "CStringUtils.h"
#include "CURLHelpersMap.h"
#include "CWindowsMenu.h"

#include "CICalendarManager.h"

#include <jFileUtil.h>

#include <locale.h>

extern cdstring openpreffile;
extern cdstrvect openurls;

// App starting up
void CMulberryApp::StartUp()
{
	if (!CPreferences::sPrefs)
		CPreferences::sPrefs = new CPreferences;

	// Read default values
	ReadDefaults();

	// Splash screen
	CSplashScreen::PoseDialog();

	// Create Calendar manager
	new iCal::CICalendarManager;
	iCal::CICalendarManager::sICalendarManager->InitManager();

	// Do protection first
	if (!Protect())
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}

	// Start logging
	CLog::StartLogging();

	// Install our periodic task as a permanent task so it runs while dialogs are up
	InstallPermanentTask(sMailCheck = new CPeriodicCheck());
	
	//Initalize clipboard/dran and drop stuff
	RegisterClipboard();

	//Start using the correct locale
	if (!setlocale(LC_ALL, ""))
	{
		cerr <<"Can't set locale, using POSIX" << std::endl;
		if (!setlocale(LC_ALL, "POSIX"))
		{
			cerr << "well, that failed too. Weird." << std::endl;
		}
	}
	
	//Initalize CWindowsMenu
	CWindowsMenu::Init();

	// Setup Mimetypes
	CMIMETypesMap::LoadDefaults(mExtrasPath);

	// Setup mailcap
	CMailcapMap::LoadDefaults(mExtrasPath);

	// Setup URL helpers
	CURLHelpersMap::LoadDefaults(mExtrasPath);

	// Do open up action
	OpenUp(true);

	// Flag init start
	mStartInit = true;

	// Set it to nil to force prefs to ask save as
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

			// Try any pref file on command line
			if (!CAdminLock::sAdminLock.mNoLocalPrefs &&
				!openpreffile.empty() && OpenPrefs(openpreffile))
			{
				// Nothing to do - file was opened
			}

			// Look in system folder for prefs file if requested
			else if (!CAdminLock::sAdminLock.mAllowDefault || !DoDefaultPrefsFile())
			{
				// If no sys folder prefs try multi-user
				if ((CAdminLock::sAdminLock.mUseMultiUser && DoMultiuser(muser_prefs)) ||
					(!CAdminLock::sAdminLock.mUseMultiUser && ForcePrefsSetting()))
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
			// Try any pref file on command line
			if (!openpreffile.empty() && OpenPrefs(openpreffile))
			{
				// Nothing to do - file was opened
			}

			// Check for default prefs
			else if (!DoDefaultPrefsFile())
			{
				// Force prefs to be set
				if (ForcePrefsSetting())

					// Do remaining init
					DoPostPrefsStartup();

				else
					// Failed to set valid prefs
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
		}

		// Now try launching any URLs
		for(cdstrvect::iterator iter = openurls.begin(); iter != openurls.end(); iter++)
			ProcessURL(*iter);

		// Clean cached command line items
		openpreffile = cdstring::null_str;
		openurls.clear();
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
bool CMulberryApp::DoDefaultPrefsFile()
{
	// Get default prefs file path
	cdstring defaultFile = CConnectionManager::sConnectionManager.GetApplicationCWD();
	defaultFile += "preferences.mbp";
	if (!JFileWritable(defaultFile))
		return false;
	OpenPrefs(defaultFile, true);
	return true;
}

// Open special file
bool CMulberryApp::OpenPrefs(const cdstring& filename, bool default_prefs)
{
	// Must reject if local prefs denied
	if (CAdminLock::sAdminLock.mNoLocalPrefs)
		return false;

	// Check to see if the same, in which case ignore it
	if (sCurrentPrefsFile && (filename == ((const char*) sCurrentPrefsFile->GetName())))
		return false;

	// Ask whether replace is really wanted (not first time through)
	if (mPrefsLoaded && (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::ReallyReplacePrefs") == CErrorHandler::Cancel))
		return false;

	// Create new prefs file
	CPreferencesFile* newPrefsFile = new CPreferencesFile(default_prefs, false);

	// Set the FSSpec of the prefs file
	newPrefsFile->SetName(filename);

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
		return false;
	}

	// Make new prefs the default
	newPrefsFile->GetPrefs()->SetAsDefault();

	// Delete existing file object
	delete sCurrentPrefsFile;

	// Replace existing file
	sCurrentPrefsFile = newPrefsFile;

	// Restart with new prefs
	DoPostPrefsStartup();

	return true;
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
		if (CMultiUserDialog::PoseDialog())
		{
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

// Get multiuser prefs with update
COptionsMap* CMulberryApp::GetMultiuser()
{
	// The multiuser resource is a 20K string included here
#include "MUResource.txt"

	// Look for multiuser resource
	const char* p = cMUResource;

	// Look for proper key at start
	const char* MUKey = "MULTIUSERPREFERENCESPACE";
	int len = ::strlen(MUKey) + 2;
	if ((::memcmp(p, MUKey, len - 2) != 0) || (p[len] == '*'))
		return NULL;
	
	// Bump past key
	p += len;
	cdstring temp(p);

	// Create file options map and give archive to it
	CStringOptionsMap* map = new CStringOptionsMap;
	map->SetString(&temp);

	// Read the map from archive
	map->ReadMap();

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

	// Force setting of prefs
	if (!DoPreferences())
	{
		// Stick up error
		CErrorHandler::PutStopAlertRsrc("Alerts::General::NoPrefsRun");
		return false;
	}
	return true;

} // CMulberryApp:ForcePrefsSetting

// Init after prefs set
void CMulberryApp::DoPostPrefsStartup()
{
	// Update the colour of selections
	UpdateSelectionColour();

	try
	{
		// Set flag to indicate one prefs read in
		mPrefsLoaded = true;

		// Check for default mail client
		CheckDefaultMailClient();
		CheckDefaultWebcalClient();

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
		
		// Recover safety save drafts
		CLetterWindow::ReadTemporary();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Failure during start up - throw up
		CLOG_LOGRETHROW;
		throw;
	}

} // CMulberryApp::DoPostPrefsStartup

