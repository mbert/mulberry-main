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


// Source for CPreferencesFile class

#include "CPreferencesFile.h"

#include "COptionsMap.h"
#if __dest_os == __win32_os
#include "CDefaultOptionsMap.h"
#include "CSDIFrame.h"
#endif
#include "CFileOptionsMap.h"
#include "CGeneralException.h"
#include "CRemoteOptionsMap.h"

#include "CErrorHandler.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPreferences.h"
#include "CPreferenceKeys.h"
#include "CPreferenceVersions.h"
#include "CRemotePrefsSets.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CPreferencesDialog.h"
#include "MyCFString.h"
#endif
#include "CTextListChoice.h"
#include "CUtils.h"
#include "CXStringResources.h"

#if __dest_os == __linux_os
#include "CTextTable.h"

#include <JError.h>
#include <jDirUtil.h>
#include <jFileUtil.h>
#include <jXGlobals.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

cdstring CPreferencesFile::mDefaultFileName;
#endif

#include <string.h>

#if __dest_os == __mac_os || __dest_os == __mac_os_x

extern void Read_Boolean(LStream* stream, bool& logic);
extern void Read_Str(LStream* stream, cdstring& str);
#endif

// __________________________________________________________________________________________________
// C L A S S __ C P R E F E R E N C E S F I L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPreferencesFile::CPreferencesFile(bool default_file, bool remote_file)
{
	// Init prefs
	mItsPrefs = NULL;
	mIsSpecified = false;

	// Init map based on default
	mDefaultFile = default_file;
	mRemoteFile = remote_file;
	mOriginalLocal = false;
	mOriginalDefault = false;

	mMap = NULL;
	InitMap();
}

// Default destructor
CPreferencesFile::~CPreferencesFile()
{
	mItsPrefs = NULL;
	delete mMap;
	mMap = NULL;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Initialise map based on status
void CPreferencesFile::InitMap()
{
	// Remote existing
	delete mMap;
	mMap = NULL;

	if (mRemoteFile)
	{
		mMap = new CRemoteOptionsMap;
	}
	else
	{
#if __dest_os == __win32_os
		if (mDefaultFile)
			mMap = new CDefaultOptionsMap;
		else
#endif
			mMap = new CFileOptionsMap;
	}
}

// Change remote/local status of file
void CPreferencesFile::SetRemote(bool remote)
{
	if (remote != mRemoteFile)
	{
		// Change flag and reinit map to get right type
		mRemoteFile = remote;
		InitMap();
	}
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Get the file name
void CPreferencesFile::SetName(const char* name)
{
	if (name && *name)
	{
		mFileName = name;

		if (mMacFileRef.IsValid())
		{
			FSRef parent;
			mMacFileRef.GetParent(parent);

			MyCFString cfstr(name, kCFStringEncodingUTF8);
			mMacFileRef = PPx::FSObject(parent, cfstr);
		}

		// Change default state
		mDefaultFile = (::strcmp(name, cDefaultPrefsSetKey_2_0) == 0);
	}
	else
	{
		mDefaultFile = true;
	}
}

// Get the file name
cdstring& CPreferencesFile::GetName()
{
	if (mMacFileRef.IsValid())
		mFileName = cdstring(mMacFileRef.GetName());
	return mFileName;
}

#elif __dest_os == __win32_os
void CPreferencesFile::SetSpecifier(const char* name)
{
	if (name && *name)
	{
		mFileName = name;

		// Change default state
		mDefaultFile = (::strcmp(name, cDefaultPrefsSetKey_2_0) == 0);
	}
	else
	{
		mFileName = cdstring::null_str;

		// Change default state
		mDefaultFile = true;
	}
}
#elif __dest_os == __linux_os
void CPreferencesFile::SetName(const char* name)
{
	if (name && *name)
	{
		mFileName = name;

		mDefaultFile = (::strcmp(name, cDefaultPrefsSetKey_2_0) == 0);
	}
	else
	{
		mFileName = cdstring::null_str;
		mDefaultFile = true;
	}
}

// Get the file name
cdstring& CPreferencesFile::GetName()
{
	return mFileName;
}

void CPreferencesFile::GetDefaultLocalFileName(cdstring& fileName) {
	
	fileName = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fileName += "preferences.mbp";
	
}
const cdstring& CPreferencesFile::GetDefaultLocalFileName() {
	mDefaultFileName = CConnectionManager::sConnectionManager.GetApplicationCWD();
	mDefaultFileName += "preferences.mbp";
	return mDefaultFileName;
}
#endif


// Create default file in System Folder
void CPreferencesFile::CreateDefault()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (!mRemoteFile)
	{
		// Find folder
		FSRef prefsFolder;
		OSErr err = ::FSFindFolder(kUserDomain, kPreferencesFolderType, kCreateFolder, &prefsFolder);
		if (err != noErr)
			err = ::FSFindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &prefsFolder);
		if (err == noErr)
		{
			cdstring name;
			name.FromResource(STR_DefaultPrefsFileName << 16);
			MyCFString temp(name);
			PPx::FSObject file_spec(prefsFolder, temp);
			SetSpecifier(file_spec);
			CreateNewFile(kPrefFileCreator, kPrefFileType, 0);

			SetSpecified(true);
		}
	}
#elif __dest_os == __linux_os
	if (!mRemoteFile)
	{
		cdstring defaultFile;
		GetDefaultLocalFileName(defaultFile);

		if (!JFileExists(defaultFile))
		{
			Open(defaultFile);
			Close();
		}

		if (!JFileWritable(defaultFile))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		SetName(defaultFile);

		SetSpecified(true);
	}	
#endif
}

// Save the preferences
void CPreferencesFile::SavePrefs(bool verify, bool force_dirty)
{
#if __dest_os == __win32_os
	CArchive* ar = NULL;
#endif

	// Try remote login here to throw up
	bool was_logged_in = false;
	bool can_partial_save = false;
	if (mRemoteFile)
	{
		was_logged_in = CMulberryApp::sOptionsProtocol && CMulberryApp::sOptionsProtocol->IsLoggedOn();
		
		// Cache the 'actual' uid (Kerberos id)
		cdstring actual_uid;
		if (CMulberryApp::sOptionsProtocol)
			actual_uid = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetAuthenticator()->GetActualUID();

		// Initiate login
		if (!CMulberryApp::sApp->BeginRemote(mItsPrefs))
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Now check for change in actual UID
		if (CMulberryApp::sOptionsProtocol)
		{
			cdstring new_actual_uid = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetAuthenticator()->GetActualUID();
			if (!actual_uid.empty() && (actual_uid != new_actual_uid))
			{
				// Alert the user
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::UIDReplacePrefs");
				return;
			}
		}
		can_partial_save = CMulberryApp::sOptionsProtocol && CMulberryApp::sOptionsProtocol->DoesPartialPrefs();
	}

	// Must start with fresh map
	InitMap();

	try
	{
		// Must have prefs first
		if (!mItsPrefs)
			mItsPrefs = new CPreferences(*CPreferences::sPrefs);

		if (!mRemoteFile)
		{
			// Open file first
#ifdef PREFS_USE_LSTREAM
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			OpenDataFork(fsRdWrPerm);
#else
			Open(mFileName.c_str());
#endif
			((CFileOptionsMap*) mMap)->SetStream(this);
#else
			if (!mDefaultFile)
			{
				if (!Open(mFileName.win_str(), CFile::modeCreate | CFile::modeWrite))
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}
				ar = new CArchive(this, CArchive::store);
				((CFileOptionsMap*) mMap)->SetArchive(ar);
			}
#endif
		}
		else
		{
			// Set section based on name
			if (mDefaultFile)
				mMap->SetTitle(cDefaultPrefsSetKey_2_0);
			else
#ifdef PREFS_USE_LSTREAM
				mMap->SetTitle(GetName());
#else
				mMap->SetTitle(GetSpecifier());
#endif

			// Always reset map's protocol
			((CRemoteOptionsMap*) mMap)->SetOptionsProtocol(CMulberryApp::sOptionsProtocol);
		}

		// Write to map (if remote do dirty only, otherwise write out all)
		mItsPrefs->WriteToMap(mMap, can_partial_save && !force_dirty);

		// Flush map to store
		bool did_write = mMap->WriteMap(verify);

		if (!mRemoteFile)
		{
			// Done with file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			CloseDataFork();
#elif __dest_os == __win32_os
			if (!mDefaultFile)
			{
				ar->Close();
				delete ar;
				ar = NULL;
				Close();
			}
#elif __dest_os == __linux_os
			Close();	
#endif
		}
#if 0
		else if (did_write && verify)
		{
			// This causes problrems with account sync'ing
			// Technically the preferences that are read-only should have been
			// locked in the UI anyway, so this check need not be done.
			// Ultimately reading the ACL bit from the remote prefs is the best
			// way to deal with this.
			// Fix for read-only options on remote
			NumVersion vers_app = CMulberryApp::GetVersionNumber();
			NumVersion vers_prefs;

			// Read from map
			mItsPrefs->ReadFromMap(mMap, vers_app, vers_prefs);
		}
#endif
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();

	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Do error alert
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Preferences::CannotSavePrefs", ex.GetErrorCode());

		// Close file if open
		if (!mRemoteFile)
			CloseDataFork();
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close file if open
		if (!mRemoteFile)
			CloseDataFork();
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#elif __dest_os == __win32_os
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Do error alert
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Preferences::CannotSavePrefs", *ex);

		// Close file if open
		if (!mRemoteFile)
		{
			delete ar;
			Close();
		}
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do error alert
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotSavePrefs");

		// Close file if open
		if (!mRemoteFile)
		{
			delete ar;
			Close();
		}
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#elif __dest_os == __linux_os
	catch (CFileException& ex)
	{
		CLOG_LOGCATCH(CFileException&);

		//Do error alert
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Preferences::CannotSavePrefs", ex);
		
		//Close file if open
		if (!mRemoteFile) {
			Close();
		} else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		//Do error alert
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotSavePrefs");
		
		//Close file if open
		if (!mRemoteFile) {
			Close();
		} else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#else	
#error __dest_os
#endif
}

// Save the connected state back to the local prefs file that started a remote prefs file
void CPreferencesFile::SaveOriginalLocal()
{
	// Only if it had original local
	if (!IsOriginalLocal())
		return;
	
	// Create temp prefs file for local
	CPreferencesFile tempfile(IsOriginalDefault(), false);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	tempfile.SetSpecifier(mOriginalLocalFileSpec);
#elif __dest_os == __linux_os
	tempfile.SetName(mOriginalLocalFileName);
#else
	tempfile.SetSpecifier(mOriginalLocalFileName);
#endif

	// Read in temporary prefs
	//CPreferences tempprefs;
	tempfile.SetPrefs(mItsPrefs);
	//tempfile.VerifyRead(false, false);
	
	// Change connected state
	//tempprefs.mDisconnected.SetValue(CPreferences::sPrefs->mDisconnected.GetValue());
	
	// Write back only those that have changed
	tempfile.SavePrefs(false, false);
}

// Read the preferences
bool CPreferencesFile::ReadPrefs(NumVersion vers_app, NumVersion& vers_prefs)
{
	bool old_version = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	bool mac_rsrc = false;
#elif __dest_os == __win32_os
	CArchive* ar = NULL;
#elif __dest_os == __linux_os
	//nothing needed
#else
#error __dest_os
#endif

	// Try remote login here to throw up
	cdstring cached_pswd;
	cdstring cached_actual_uid;
	bool was_logged_in = false;
	if (mRemoteFile)
	{
		was_logged_in = CMulberryApp::sOptionsProtocol && CMulberryApp::sOptionsProtocol->IsLoggedOn();
		if (!CMulberryApp::sApp->BeginRemote(mItsPrefs))
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// NB Reading in a new set of preferences will overwrite any existing login information
		// Must preserve the password or the recovered uid (K4) and put these back in the new preferences
		CAuthenticator& auth = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator();
		if (auth.RequiresUserPswd() &&
			mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorUserPswd()->GetSavePswd())
			cached_pswd = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorUserPswd()->GetPswd();
		else if (auth.RequiresKerberos())
			cached_actual_uid = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorKerberos()->GetRecoveredUID();
	}

	try
	{
		// Must have prefs first (start with blank set)
		if (!mItsPrefs)
			mItsPrefs = new CPreferences;

		if (!mRemoteFile)
		{
#ifdef PREFS_USE_LSTREAM
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Open file first
			OpenDataFork(fsRdWrPerm);

			// Look for data
			if (GetLength() == 0)
			{
				// Use resources instead since old version
				mac_rsrc = true;
				CloseDataFork();
				OpenResourceFork(fsRdWrPerm);
			}
#else
			Open(mFileName.c_str());
#endif
			((CFileOptionsMap*) mMap)->SetStream(this);
#else
			// Open file first
			if (!mDefaultFile)
			{
				if (!Open(mFileName.win_str(), CFile::modeRead))
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}
				ar = new CArchive(this, CArchive::load);
				((CFileOptionsMap*) mMap)->SetArchive(ar);
			}

#endif
		}
		else
		{
			// Set section based on name
			if (mDefaultFile)
				mMap->SetTitle(cDefaultPrefsSetKey_2_0);
			else
#ifdef PREFS_USE_LSTREAM
				mMap->SetTitle(GetName());
#else
				mMap->SetTitle(GetSpecifier());
#endif

			// Always reset map's protocol
			((CRemoteOptionsMap*) mMap)->SetOptionsProtocol(CMulberryApp::sOptionsProtocol);

		}

		// Get map data from store
		mMap->ReadMap();

		// Special for remote prefs upgrade v1.x -> v2.0
		bool set_convert = false;
		if (mRemoteFile && mDefaultFile)
		{
			// Try to read version key for 2.x
			cdstring txt;
			StMapSection section(mMap, cPrefsSection);
			if (mMap->ReadValue(cVersionKey_v2_0, txt, vers_app))
			{
				// Got v2 versions key => has legitimate v2 prefs
			}
			else
			{
				// Switch to 1.x default set
				mMap->SetTitle(cDefaultPrefsSetKey_1_3);
				
				// Re-read the map
				mMap->ReadMap();
				
				// Now check for previous versions
				// Try to read v1.4 version key
				if (mMap->ReadValue(cVersionKey_v1_4, txt, vers_app) ||
					mMap->ReadValue(cVersionKey, txt, vers_app))
				{
					// Have old v1.x default prefs set => do conversion
					set_convert = true;
				}
				else
				{
					// Switch back to 2.x default set
					mMap->SetTitle(cDefaultPrefsSetKey_2_0);
					
					// Re-read the map
					mMap->ReadMap();
				}
			}
		}

		// Must close the file/logout of server BEFORE reading map data into
		// preferences as that will replace account objects in current prefs
		if (!mRemoteFile)
		{
			// Done with file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (!mac_rsrc)
				CloseDataFork();
			else
				CloseResourceFork();
#elif __dest_os == __win32_os
			if (!mDefaultFile)
			{
				ar->Close();
				delete ar;
				ar = NULL;
				
				// Only close real file
				Close();
			}
#elif __dest_os == __linux_os
			Close();
#endif
		}
		else
		{
			// Do logoff to kill connection
			if (!was_logged_in)
				CMulberryApp::sOptionsProtocol->Logoff();

			CAuthenticator& auth = mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator();
			if (auth.RequiresUserPswd() &&
				mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorUserPswd()->GetSavePswd())
				mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorUserPswd()->SetPswd(cached_pswd);
			else if (auth.RequiresKerberos())
				mItsPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticatorKerberos()->SetRecoveredUID(cached_actual_uid);
		}

		// Read from map
		old_version = mItsPrefs->ReadFromMap(mMap, vers_app, vers_prefs);

		// Special for remote prefs upgrade v1.x -> v2.0
		if (set_convert)
		{
			// Switch back to 2.x default set
			mMap->SetTitle(cDefaultPrefsSetKey_2_0);
			
			// Force version to appear as v1.4.4
			NumVersionVariant vers1_4;
			vers1_4.whole = VERS_1_4_0;
			vers_prefs = vers1_4.parts;
		}
	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Do error alert
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Preferences::CannotReadPrefs", ex.GetErrorCode());

		// Close file if open
		if (!mRemoteFile)
			CloseDataFork();
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do error alert
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotReadPrefs");

		// Close file if open
		if (!mRemoteFile)
			CloseDataFork();
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#elif __dest_os == __win32_os
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Do error alert
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Preferences::CannotReadPrefs", *ex);

		// Close file if open
		if (!mRemoteFile)
		{
			delete ar;
			Close();
		}
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do error alert
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotReadPrefs");

		// Close file if open
		if (!mRemoteFile)
		{
			delete ar;
			Close();
		}
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#elif __dest_os == __linux_os
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do error alert
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotReadPrefs");

		// Close file if open
		if (!mRemoteFile)
		{
			Close();
		}
		else if (!was_logged_in)
			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
	}
#else
#error __dest_os
#endif

	return old_version;
}

// Verify and read preferences
bool CPreferencesFile::VerifyRead(bool is_local, bool write_back, bool* pending_update)
{
	bool read_ok = true;

	NumVersion vers_app = CMulberryApp::GetVersionNumber();
	NumVersion vers_prefs;
	*(long*) &vers_prefs = 0;

	// Do choice of multiple sets if reached during startup phase and remote set
	if (mRemoteFile && !CMulberryApp::sApp->LoadedPrefs())
	{
		// Must first check connection state
		CMulberryApp::sApp->InitConnection(*mItsPrefs);

		AskForRemoteSet();
	}

	// Read file and check version
	if (ReadPrefs(vers_app, vers_prefs))
	{
		// Update prefs file only if required
		if (write_back)
		{
			bool force_dirty = false;
			
			// Need special test for default remote set upgrade
			if (mRemoteFile && mDefaultFile && (VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0))
				force_dirty = true;

			SavePrefs(false, force_dirty);
		}
		else if (pending_update)
			*pending_update = true;
	}

	// Now check for remote operation after loading local file
	if (is_local &&
		mItsPrefs->mRemoteAccounts.GetValue().size() &&
		mItsPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart())
	{
		bool old_default = mDefaultFile;
#ifdef PREFS_USE_LSTREAM
#if  __dest_os == __mac_os || __dest_os == __mac_os_x
		PPx::FSObject old_spec;
		GetSpecifier(old_spec);
#endif
		cdstring old_name = GetName();
#else
		cdstring old_name = GetSpecifier();
#endif

		// Reset file for remote operation (must also be default)
		mRemoteFile = true;
		mDefaultFile = true;

		InitMap();

		// Must set the default OS location to the value read in in case disconnected
		// and we need to get the disconnected prefs from the proper place
		bool use_os_default = CPreferences::sPrefs->mOSDefaultLocation.GetValue();
		CPreferences::sPrefs->mOSDefaultLocation.SetValue(GetPrefs()->mOSDefaultLocation.GetValue(), false);

		try
		{
			// Read again, but do not do remote again!
			read_ok = VerifyRead(false);
			if (!read_ok)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			
			// Now copy over the original items
			mOriginalLocal = true;
			mOriginalDefault = old_default;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			mOriginalLocalFileSpec = old_spec;
#else
			mOriginalLocalFileName = old_name;
#endif
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// If conversion to remote fails, revert to local
			mRemoteFile = false;
			mDefaultFile = old_default;
#ifdef PREFS_USE_LSTREAM
			SetName(old_name);
#else
			SetSpecifier(old_name);
#endif

			InitMap();

			// Force prefs to local
			//mItsPrefs->mRemoteAccount.GetValue().SetLoginAtStart(false);
		}
		
		// Restore the default OS location
		CPreferences::sPrefs->mOSDefaultLocation.SetValue(use_os_default, false);
	}

	return read_ok;
}

void CPreferencesFile::AskForRemoteSet()
{
	// Make sure set is read in
	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(mItsPrefs))
	{
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();

		// Check to see if multiple sets exist
		if (CMulberryApp::sRemotePrefs->GetRemoteSets().empty())
			// Just return with no error - will use single default prefs
			return;
	}
	else
		// Must throw if failure
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	bool result = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Add default name first
	cdstrvect items;
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

		// Set the name of this file to the new set name (only bother if not default)
		if (remote != cDefaultPrefsSetKey_2_0)
			SetName(remote);

		result = true;
	}
#elif __dest_os == __win32_os
	// Create the dialog
	CTextListChoice dlog(CSDIFrame::GetAppTopWindow());
	dlog.mSingleSelection = true;
	dlog.mSelectFirst = true;
	dlog.mButtonTitle = rsrc::GetString("Alerts::Preferences::OpenRemoteButton");
	dlog.mTextListDescription = rsrc::GetString("Alerts::Preferences::OpenRemoteDesc");

	// Add default name first
	dlog.mItems.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		dlog.mItems.push_back(*iter);

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		// Get selection from list
		cdstring remote = cDefaultPrefsSetKey_2_0;
		if (dlog.mSelection.front())
			remote = CMulberryApp::sRemotePrefs->GetRemoteSets().at(dlog.mSelection.front() - 1);

			// Set the name of this file to the new set name (only bother if not default)
			if (remote != cDefaultPrefsSetKey_2_0)
				SetSpecifier(remote);

			result = true;
	}
#elif __dest_os == __linux_os
	// Create the dialog
	CTextListChoice* dlog = new CTextListChoice(JXGetApplication());
	dlog->mSingleSelection = true;
	dlog->mSelectFirst = true;
	dlog->mButtonTitle = rsrc::GetString("Alerts::Preferences::OpenRemoteButton");
	dlog->mTextListDescription = rsrc::GetString("Alerts::Preferences::OpenRemoteDesc");

	// Add default name first
	dlog->mItems.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		dlog->mItems.push_back(*iter);

	// Test for OK
	if (dlog->DoModal() == CDialogDirector::kDialogClosed_OK)
	{
		// Get selection from list
		cdstring remote = cDefaultPrefsSetKey_2_0;
		ulvector selection;
		dlog->mList->GetSelection(selection);
		if (selection.front() > 0)
			remote = CMulberryApp::sRemotePrefs->GetRemoteSets().at(selection.front() - 1);

		// Set the name of this file to the new set name (only bother if not default)
		if (remote != cDefaultPrefsSetKey_2_0)
			SetName(remote);

		result = true;
		dlog->Close();
	}
#else
#error __dest_os
#endif

	// Must throw out of here if cancelled
	if (!result)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Save the file as something else
void CPreferencesFile::DoSaveAs(PPx::FSObject& sfFile, bool replacing)
{
	bool saving = false;

	try
	{
		// If replacing delete old file first
		if (replacing && sfFile.Exists())
		{
			sfFile.Delete();
		}

		// Make this file point at new spec and create
		SetSpecifier(sfFile);
		CreateNewDataFile(kPrefFileCreator, kPrefFileType, 0);
		CreateNewFile(kPrefFileCreator, kPrefFileType, 0);
		sfFile.Update();

		// Do save
		saving = true;
		SavePrefs(false, true);
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user if not already done in SavePrefs
		if (!saving)
			CErrorHandler::PutOSErrAlertRsrc("Alerts::Preferences::CannotSavePrefs", ex.GetErrorCode());

		// Close file if open
		CloseDataFork();

		// Must clean up
		try
		{
			if (sfFile.Exists())
				sfFile.Delete();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close file if open
		CloseDataFork();

		// Must clean up
		try
		{
			if (sfFile.Exists())
				sfFile.Delete();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}
#elif __dest_os == __win32_os
// Save the file as something else
void CPreferencesFile::DoSaveAs(const CString& fileName)
{
	bool saving = false;

	try
	{
		// Make this file point at new spec and create (by opening/closing)
		SetSpecifier(cdstring(fileName));
		Open(fileName, CFile::modeCreate);
		Close();

		// Do save
		saving = true;
		SavePrefs(false, true);
	}
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Inform user if not already done in SavePrefs
		if (!saving)
			CErrorHandler::PutFileErrAlertRsrc("Alerts::Preferences::CannotSavePrefs", *ex);

		// Close file if open
		Close();

		// Must clean up
		Remove(fileName);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform user if not already done in SavePrefs
		if (!saving)
			CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotSavePrefs");

		// Close file if open
		Close();

		// Must clean up
		Remove(fileName);
	}
}
#elif __dest_os == __linux_os
// Save the file as something else
void CPreferencesFile::DoSaveAs(const cdstring& fileName)
{
	bool saving = false;

	try
	{
		// Make this file point at new spec and create (by opening/closing)
		SetName(fileName);
		Open(fileName);
		Close();

		// Do save
		saving = true;
		SavePrefs(false, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform user if not already done in SavePrefs
		if (!saving)
			CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::CannotSavePrefs");

		// Close file if open
		Close();

		// Must clean up
		unlink(fileName);
	}
}
#else
#error __dest_os
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x

void CPreferencesFile::WriteResource(Handle rsrc_data, OSType rsrc, short rsrc_id)
{
	// Delete any existing resource
	Handle old_rsrc = ::Get1Resource(rsrc, rsrc_id);
	if (old_rsrc)
	{
		::RemoveResource(old_rsrc);
		ThrowIfResError_();
		::UpdateResFile(GetResourceForkRefNum());
	}

	// Create new handle
	Handle new_rsrc = rsrc_data;
	::HandToHand(&new_rsrc);
	ThrowIfMemError_();

	// Create new resource and write it out
	::AddResource(new_rsrc, rsrc, rsrc_id, "\p");
	ThrowIfResError_();
	::WriteResource(new_rsrc);
	ThrowIfResError_();
	::ReleaseResource(new_rsrc);
	ThrowIfResError_();
}

#endif
