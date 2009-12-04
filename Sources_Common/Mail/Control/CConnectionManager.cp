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


// CConnectionManager.h

#include "CConnectionManager.h"

#include "CAddressBookManager.h"
#include "CCalendarStoreManager.h"
#include "CLocalCommon.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionsProtocol.h"
#include "CPreferences.h"
#include "CServerWindow.h"
#include "CSMTPAccountManager.h"
#include "CTCPSocket.h"
#include "CUnicodeStdLib.h"
#include "char_stream.h"

#include __stat_header
#include <unistd.h>

#if __dest_os == __win32_os
#include <sysenv_api.h>
#include <ShFolder.h>
#elif __dest_os == __linux_os
#include <jDirUtil.h>
#endif

#pragma mark ____________________________consts

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* cMboxSyncDescriptors[] =
	{"All",
	 "New",
	 "None"};

const char* cMsgSyncDescriptors[] =
	{"Entire",
	 "Below",
	 "First"};

#if __dest_os == __linux_os
const char* cTempFiles = "Temporary_Files";
const char* cSafetySaveDrafts = "Recover_Drafts";
const char* cExternalEditDrafts = "External_Edit";
const char* cViewAttachments = "View_Attachments";
#else
const char* cTempFiles = "Temporary Files";
const char* cSafetySaveDrafts = "Recover Drafts";
const char* cExternalEditDrafts = "External Edit";
const char* cViewAttachments = "View Attachments";
#endif
const char* cSecurityTemp = "Security";
const char* cSIEVEScripts = "SIEVE Scripts";

const char* cCertificates = "Certificates";
const char* cAuthoritiesCertificates = "Authorities";
const char* cServerCertificates = "Servers";
const char* cUserCertificates = "Users";
const char* cPersonalCertificates = "Personal";

const char* cTimezones = "Timezones";

#pragma mark ____________________________CConnectOptions

CConnectionManager::CConnectOptions::CConnectOptions()
{
	mStayConnected = true;
	mMboxPlayback = true;
	mSMTPSend = true;
	mAdbkPlayback = true;
	mUpdatePOP3 = true;
}		

// Compare with same type
int CConnectionManager::CConnectOptions::operator==(const CConnectOptions& test) const
{
	return (mStayConnected == test.mStayConnected) &&
			(mMboxPlayback == test.mMboxPlayback) &&
			(mSMTPSend == test.mSMTPSend) &&
			(mAdbkPlayback == test.mAdbkPlayback) &&
			(mUpdatePOP3 == test.mUpdatePOP3);
}

// Copy same type
void CConnectionManager::CConnectOptions::_copy(const CConnectOptions& copy)
{
	mStayConnected = copy.mStayConnected;
	mMboxPlayback = copy.mMboxPlayback;
	mSMTPSend = copy.mSMTPSend;
	mAdbkPlayback = copy.mAdbkPlayback;
	mUpdatePOP3 = copy.mUpdatePOP3;
	mFuture = copy.mFuture;
}

cdstring CConnectionManager::CConnectOptions::GetInfo() const
{
	cdstring info;
	info += '(';

	info += (mStayConnected ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mMboxPlayback ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mSMTPSend ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mAdbkPlayback ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mUpdatePOP3 ? cValueBoolTrue : cValueBoolFalse);

	info += mFuture.GetInfo();

	info += ')';
	
	return info;
}

bool CConnectionManager::CConnectOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.start_sexpression();

	txt.get(mStayConnected);

	txt.get(mMboxPlayback);

	txt.get(mSMTPSend);

	txt.get(mAdbkPlayback);

	txt.get(mUpdatePOP3);

	mFuture.SetInfo(txt, vers_prefs);

	txt.end_sexpression();
	
	return true;
}

#pragma mark ____________________________CDisconnectOptions

CConnectionManager::CDisconnectOptions::CDisconnectOptions()
{
	mMboxSync = eNewMessages;
	mMsgSync = eMessageBelow;
	mMsgSyncSize = 10240;
	mListSync = true;
	mSMTPWait = true;
	mAdbkSync = false;
}	

// Compare with same type
int CConnectionManager::CDisconnectOptions::operator==(const CDisconnectOptions& test) const
{
	return (mMboxSync == test.mMboxSync) &&
			(mMsgSync == test.mMsgSync) &&
			(mMsgSyncSize == test.mMsgSyncSize) &&
			(mListSync == test.mListSync) &&
			(mSMTPWait == test.mSMTPWait) &&
			(mAdbkSync == test.mAdbkSync);
}

// Copy same type
void CConnectionManager::CDisconnectOptions::_copy(const CDisconnectOptions& copy)
{
	mMboxSync = copy.mMboxSync;
	mMsgSync = copy.mMsgSync;
	mMsgSyncSize = copy.mMsgSyncSize;
	mListSync = copy.mListSync;
	mSMTPWait = copy.mSMTPWait;
	mAdbkSync = copy.mAdbkSync;
	mFuture = copy.mFuture;
}

cdstring CConnectionManager::CDisconnectOptions::GetInfo() const
{
	cdstring info;
	info += '(';

	info += cMboxSyncDescriptors[mMboxSync];
	info += cSpace;

	info += cMsgSyncDescriptors[mMsgSync];
	info += cSpace;

	cdstring temp = mMsgSyncSize;
	info += temp;
	info += cSpace;

	info += (mListSync ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mSMTPWait ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mAdbkSync ? cValueBoolTrue : cValueBoolFalse);

	info += mFuture.GetInfo();

	info += ')';
	
	return info;
}

bool CConnectionManager::CDisconnectOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.start_sexpression();

	char* p = txt.get();
	if (::strcmp(p, cMboxSyncDescriptors[eAllMessages]) == 0)
		mMboxSync = eAllMessages;
	else if (::strcmp(p, cMboxSyncDescriptors[eNewMessages]) == 0)
		mMboxSync = eNewMessages;
	else if (::strcmp(p, cMboxSyncDescriptors[eNoMessages]) == 0)
		mMboxSync = eNoMessages;

	p = txt.get();
	if (::strcmp(p, cMsgSyncDescriptors[eEntireMessage]) == 0)
		mMsgSync = eEntireMessage;
	else if (::strcmp(p, cMsgSyncDescriptors[eMessageBelow]) == 0)
		mMsgSync = eMessageBelow;
	else if (::strcmp(p, cMsgSyncDescriptors[eFirstDisplayable]) == 0)
		mMsgSync = eFirstDisplayable;

	txt.get(mMsgSyncSize);

	txt.get(mListSync);

	txt.get(mSMTPWait);

	txt.get(mAdbkSync);

	mFuture.SetInfo(txt, vers_prefs);

	txt.end_sexpression();
	
	return true;
}

#pragma mark ____________________________CConnectionManager

CConnectionManager CConnectionManager::sConnectionManager;

CConnectionManager::CConnectionManager()
{
	mConnected = false;

	// Get current CWD at app start
	mApplicationCWD = cdstring::null_str;
	mApplicationCWD.reserve(FILENAME_MAX);

#if __dest_os == __mac_os_x
	{
		// Get our application's main bundle from Core Foundation
		CFBundleRef myAppsBundle = ::CFBundleGetMainBundle();
		if (myAppsBundle != NULL)
		{
		    // Retrieve the URL to our bundle
			CFURLRef myBundleURL = ::CFBundleCopyBundleURL(myAppsBundle);
			if (myBundleURL != NULL)
			{
				// convert the URL to a FSRef
				FSRef myBundleRef;
				if (::CFURLGetFSRef(myBundleURL, &myBundleRef))
				{
					mApplicationCWD = GetFullPath(&myBundleRef);
				}

				::CFRelease(myBundleURL);
			}
		}
	}
#else
	::getcwd(mApplicationCWD, FILENAME_MAX);
#endif

	// Must end with dir-delim
	if (mApplicationCWD.length() && (mApplicationCWD[mApplicationCWD.length() - 1] != os_dir_delim))
		mApplicationCWD += os_dir_delim;

	// Get user CWD
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	
	// Don't create the folder now - only when actually needed
	FSRef fref;
	OSErr err = ::FSFindFolder(kUserDomain, kDocumentsFolderType, kCreateFolder, &fref);
	if (err != noErr)
		err = ::FSFindFolder(kOnSystemDisk, kDocumentsFolderType, kCreateFolder, &fref);
	if (err == noErr)
	{
		// Convert to path
		PPx::FSObject temp(fref);
		mUserCWD = cdstring(temp.GetPath());
		::addtopath(mUserCWD, "Mulberry");
	}
	else if (err != fnfErr)
		mUserCWD = mApplicationCWD;
#elif __dest_os == __win32_os
	// Try Shell folder first
	if (!::GetSpecialFolderPath(CSIDL_APPDATA, mUserCWD))
	{
		// Use user profile directory
#ifdef _UNICODE
		mUserCWD = ::_wgetenv(L"USERPROFILE");
#endif
		if (mUserCWD.length() && (mUserCWD[mUserCWD.length() - 1] != os_dir_delim))
		{
			mUserCWD += os_dir_delim;
			mUserCWD += "Application Data";
			try
			{
				::chkdir(mUserCWD);
			}
			catch(CGeneralException& ex)
			{
				CLOG_LOGCATCH(CGeneralException& ex);
			}
			mUserCWD += os_dir_delim;
		}
	}

	// Must have some sort of user profile
	if (mUserCWD.length())
	{
		try
		{
			mUserCWD += "Cyrusoft";
			::chkdir(mUserCWD);

			mUserCWD += "\\Mulberry";
			::chkdir(mUserCWD);
		}
		catch(CGeneralException& ex)
		{
			CLOG_LOGCATCH(CGeneralException& ex);
			
			::report_file_error(mUserCWD, ex.GetErrorCode());
		}
	}
	else
	{
		// No user profile (probably Win95) use application directory instead
		mUserCWD = mApplicationCWD;
		
		// For compatability with the broken behaviour on Win95 put our data
		// inside "Application Data" - which is where it ended up with v2.0
		mUserCWD += "Application Data";
		try
		{
			::chkdir(mUserCWD);
		}
		catch(CGeneralException& ex)
		{
			CLOG_LOGCATCH(CGeneralException& ex);
			
			::report_file_error(mUserCWD, ex.GetErrorCode());
		}
		mUserCWD += os_dir_delim;
	}

#elif __dest_os == __linux_os
	// Use user's home directory
	JString homeDir;
	JGetHomeDirectory(&homeDir);
	mUserCWD = JCombinePathAndName(homeDir, ".mulberry");
	//Create it if it doesn't exist
	JCreateDirectory(mUserCWD, 0700);
	mUserCWD += os_dir_delim;

	// Force application CWD to user CWD since we don't
	// want to use the wd where Mulberry was launched.
	// The user can use the -D switch to change the application
	// cwd directly.
	mApplicationCWD = mUserCWD;
#endif

	// Must end with dir-delim
	if (mUserCWD.length() && (mUserCWD[mUserCWD.length() - 1] != os_dir_delim))
		mUserCWD += os_dir_delim;
	
	// Clean out existing temp directories
	CleanTempDirectories();
}

CConnectionManager::~CConnectionManager()
{
}

const cdstring& CConnectionManager::GetCWD() const
{
	bool use_os_default = CPreferences::sPrefs->mOSDefaultLocation.GetValue();
	
	// If using app directory, check whether it is writeable
	if (!use_os_default)
	{
		// Get file permissions
		// If no write access or error with stat - force OS default location
		if (::access_utf8(mApplicationCWD, R_OK | W_OK) != 0)
			use_os_default = true;
	}

	if (use_os_default)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Special for Mac OS - may need to create Documents folder
		if (mUserCWD.empty())
		{
			FSRef fref;
			OSErr err = ::FSFindFolder(kUserDomain, kDocumentsFolderType, kCreateFolder, &fref);
			if (err != noErr)
				err = ::FSFindFolder(kOnSystemDisk, kDocumentsFolderType, kCreateFolder, &fref);
			if (err == noErr)
			{
				// Convert to path
				PPx::FSObject temp(fref);
				cdstring path(temp.GetPath());
				::addtopath(path, "Mulberry");
				const_cast<CConnectionManager*>(this)->mUserCWD = path;

				// Must end with dir-delim
				if (mUserCWD.length() && (mUserCWD[mUserCWD.length() - 1] != os_dir_delim))
					const_cast<CConnectionManager*>(this)->mUserCWD += os_dir_delim;
			}
			else
				const_cast<CConnectionManager*>(this)->mUserCWD = mApplicationCWD;
		}
#endif
		return mUserCWD;
	}
	else
		return mApplicationCWD;
}

// Special directories
void CConnectionManager::CleanTempDirectories() const
{
}

cdstring CConnectionManager::GetTempDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cTempFiles);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetSafetySaveDraftDirectory() const
{
	cdstring cwd = GetTempDirectory();
	::addtopath(cwd, cSafetySaveDrafts);
	try
	{
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetExternalEditDraftDirectory() const
{
	cdstring cwd = GetTempDirectory();
	::addtopath(cwd, cExternalEditDrafts);
	try
	{
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetViewAttachmentDirectory() const
{
	cdstring cwd = GetTempDirectory();
	::addtopath(cwd, cViewAttachments);
	try
	{
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetSecurityTempDirectory() const
{
	cdstring cwd = GetTempDirectory();
	::addtopath(cwd, cSecurityTemp);
	try
	{
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetSIEVEDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cSIEVEScripts);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetAuthoritiesCertsDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cCertificates);
		::chkdir(cwd);
		::addtopath(cwd, cAuthoritiesCertificates);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetServerCertsDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cCertificates);
		::chkdir(cwd);
		::addtopath(cwd, cServerCertificates);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetUserCertsDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cCertificates);
		::chkdir(cwd);
		::addtopath(cwd, cUserCertificates);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetPersonalCertsDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cCertificates);
		::chkdir(cwd);
		::addtopath(cwd, cPersonalCertificates);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

cdstring CConnectionManager::GetTimezonesDirectory() const
{
	cdstring cwd = GetCWD();
	try
	{
		::chkdir(cwd);
		::addtopath(cwd, cTimezones);
		::chkdir(cwd);
	}
	catch(CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(cwd, ex.GetErrorCode());
	}

	return cwd;
}

bool CConnectionManager::CanDisconnect() const
{
	return CMailAccountManager::sMailAccountManager->HasDisconnectedProtocols();
}

bool CConnectionManager::DoConnection(bool connect)
{
	// Try auto dial/hangup etc if required
	if (connect)
	{
		// See if any modem will dial and auto dial required - fail if auto dial fails
		if (CTCPSocket::WillDial() &&
			CPreferences::sPrefs->mAutoDial.GetValue() &&
			!CTCPSocket::AutoDial())
			return false;
	}
	else
	{
		// See if modem present and auto hangup required
		if (CTCPSocket::HasInet() &&
			CTCPSocket::HasModem() &&
			CPreferences::sPrefs->mAutoDial.GetValue())
			CTCPSocket::AutoHangup();
	}
	
	return true;
}

void CConnectionManager::Connect(CProgress* progress1, CProgress* progress2)
{
	// See if modem present and auto dial required - fail if auto dial fails
	if (!DoConnection(true))
		return;

	// Get connection options from prefs
	const CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.GetValue();

	mConnected = true;

	try
	{
		if (connect.mSMTPSend)
			CSMTPAccountManager::sSMTPAccountManager->SetConnected(true);
		CMailAccountManager::sMailAccountManager->GoOnline(connect.mMboxPlayback,
															!connect.mStayConnected,
															connect.mUpdatePOP3,
															progress1, progress2);
		CAddressBookManager::sAddressBookManager->GoOnline(connect.mAdbkPlayback);
		if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
			calstore::CCalendarStoreManager::sCalendarStoreManager->GoOnline(true);
		
		if (CMulberryApp::sApp->sOptionsProtocol)
			CMulberryApp::sApp->sOptionsProtocol->GoOnline();

		// Check for update not permanent connection
		if (!connect.mStayConnected)
			Disconnect(true, false, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Force disconnect of all
		Disconnect(true, true, NULL);
	}
	
	// Always refresh server window titles
	{
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::iterator iter = CServerView::sServerViews->begin(); iter != CServerView::sServerViews->end(); iter++)
			(*iter)->GetTable()->SetTitle();
	}
}

void CConnectionManager::Disconnect(bool fast, bool force, CProgress* progress)
{
	// Get disconnection options from prefs
	const CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	mConnected = false;

	try
	{
		CMailAccountManager::sMailAccountManager->GoOffline(force, (disconnect.mMboxSync != eNoMessages),
															(disconnect.mMboxSync == eNewMessages),
															(disconnect.mMsgSync == eFirstDisplayable),
															(disconnect.mMsgSync == eMessageBelow) ? disconnect.mMsgSyncSize : 0,
															!fast && disconnect.mListSync, progress);
		CAddressBookManager::sAddressBookManager->GoOffline(force, !fast && disconnect.mAdbkSync, false);
		if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
			calstore::CCalendarStoreManager::sCalendarStoreManager->GoOffline(force, !fast && disconnect.mAdbkSync, false);
		
		if (CMulberryApp::sApp->sOptionsProtocol)
			CMulberryApp::sApp->sOptionsProtocol->GoOffline();

		// Do SMTP account last to allow any messages still in the queue to be processed right up until the end
		// This also ensures that mail/adbk/options sync is not blocked and runs in parallel with message send
		CSMTPAccountManager::sSMTPAccountManager->SetConnected(false, !force && disconnect.mSMTPWait);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		mConnected = false;
	}

	// See if modem present and auto hangup required
	if (!mConnected)
		DoConnection(false);
	
	// Always refresh server window titles
	{
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::iterator iter = CServerView::sServerViews->begin(); iter != CServerView::sServerViews->end(); iter++)
			(*iter)->GetTable()->SetTitle();
	}
}

// Computer about to sleep
void CConnectionManager::Suspend()
{
	try
	{
		// May be called when some objects are not assigned
		if (CMailAccountManager::sMailAccountManager)
			CMailAccountManager::sMailAccountManager->Suspend();
		if (CAddressBookManager::sAddressBookManager)
			CAddressBookManager::sAddressBookManager->Suspend();
		if (calstore::CCalendarStoreManager::sCalendarStoreManager)
			calstore::CCalendarStoreManager::sCalendarStoreManager->Suspend();
		if (CSMTPAccountManager::sSMTPAccountManager)
			CSMTPAccountManager::sSMTPAccountManager->Suspend();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Computer woken from sleep
void CConnectionManager::Resume()
{
	try
	{
		// May be called when some objects are not assigned
		if (CMailAccountManager::sMailAccountManager)
			CMailAccountManager::sMailAccountManager->Resume();
		if (CAddressBookManager::sAddressBookManager)
			CAddressBookManager::sAddressBookManager->Resume();
		if (calstore::CCalendarStoreManager::sCalendarStoreManager)
			calstore::CCalendarStoreManager::sCalendarStoreManager->Resume();
		if (CSMTPAccountManager::sSMTPAccountManager)
			CSMTPAccountManager::sSMTPAccountManager->Resume();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}
