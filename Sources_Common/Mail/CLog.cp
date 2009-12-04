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


// CLog: allows logging of network related comms

#include "CLog.h"

#include "CAdminLock.h"
#include "cdthread.h"
#include "CConnectionManager.h"
#include "CLocalCommon.h"
#include "CLoggingOptionsDialog.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#endif

#include "cdstring.h"
#include "cdfstream.h"

#include <time.h>

// __________________________________________________________________________________________________
// C L A S S __ C L O G
// __________________________________________________________________________________________________

CLog CLog::sExceptionLog;

CLog::SLogInfo CLog::sLogInfo[eLogTypeLast];

bool CLog::sLoggingActive = true;
bool CLog::sAllowAuthenticationLog = false;
bool CLog::sAllowPlaybackLog = false;
bool CLog::sOverwrite = false;
bool CLog::sUserCWD = false;

const char* cLogDir = "Logs";

const char* cLogGreeting[] = {	NULL,
								" Starting IMAP Session on ",
								" Starting POP3 Session on ",
								" Starting IMSP Session on ",
								" Starting ACAP Session on ",
								" Starting SMTP Session on ",
								" Starting HTTP Session on ",
								" Starting Plugin log on ",
								" Starting Filter Session on ",
								" Starting Exception Session on "};

const char* cLogTrailer[] = {	NULL,
								" Stopping IMAP Session on ",
								" Stopping POP3 Session on ",
								" Stopping IMSP Session on ",
								" Stopping ACAP Session on ",
								" Stopping SMTP Session on ",
								" Stopping HTTP Session on ",
								" Stopping Plugin log on ",
								" Stopping Filter Session on ",
								" Stopping Exception Session on "};

const char* cLogName[] = {	NULL,
								"IMAPlog.txt",
								"POP3log.txt",
								"IMSPlog.txt",
								"ACAPlog.txt",
								"SMTPlog.txt",
								"HTTPlog.txt",
								"Pluginlog.txt",
								"Filterlog.txt",
								"Errorlog.txt"};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Carbon
const char cLoggingSection[] = "Logging:";
const char* cLogSection[] = {	NULL,
								"IMAP",
								"POP3",
								"IMSP",
								"ACAP",
								"SMTP",
								"HTTP",
								"Plugins",
								"Filters",
								"Exceptions"};
const char cAuthenticationSection[] = "Authentication";
const char cPlaybackSection[] = "Playback";
const char cOverwriteSection[] = "Overwrite";
const char cUserCWDSection[] = "UserCWD";

const char cLogOn[] = "On";
const char cLogOff[] = "Off";
#else
const short cLogBits[] = {	0x0000,
							0x0001,
							0x0010,
							0x0004,
							0x0008,
							0x0002,
							0x0080,
							0x4000,
							0x0020,
							0x0040};
const short cAuthenticationBit = 0x8000;
const short cPlaybackBit = 0x2000;
const short cOverwriteBit = 0x0800;
const short cUserCWDBit = 0x1000;
#endif
#elif __dest_os == __win32_os
const TCHAR cLoggingSection[] = _T("Logging");
const TCHAR* cLogSection[] = {	NULL,
								_T("IMAP"),
								_T("POP3"),
								_T("IMSP"),
								_T("ACAP"),
								_T("SMTP"),
								_T("HTTP"),
								_T("Plugins"),
								_T("Filters"),
								_T("Exceptions")};
const TCHAR cAuthenticationSection[] = _T("Authentication");
const TCHAR cPlaybackSection[] = _T("Playback");
const TCHAR cOverwriteSection[] = _T("Overwrite");
const TCHAR cUserCWDSection[] = _T("UserCWD");

const TCHAR cLogOn[] = _T("On");
const TCHAR cLogOff[] = _T("Off");
#elif __dest_os == __linux_os
const char cLoggingSection[] = "Logging";
const char* cLogSection[] = {	NULL,
								"IMAP",
								"POP3",
								"IMSP",
								"ACAP",
								"SMTP",
								"HTTP",
								"Plugins",
								"Filters",
								"Exceptions"};
const char cAuthenticationSection[] = "Authentication";
const char cPlaybackSection[] = "Playback";
const char cOverwriteSection[] = "Overwrite";
const char cUserCWDSection[] = "UserCWD";

const char cLogOn[] = "On";
const char cLogOff[] = "Off";
#else
#error __dest_os
#endif

#pragma mark ____________________________Statics

// Create all logfiles if required
void CLog::StartLogging()
{
	// Determine logging options
	ReadOptions();

	// Open required log files
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (sLogInfo[i].mDoLog)
			BeginLog(sLogInfo[i].mLog, cLogName[i]);
	}
	
	// Start the exception log - this won't do logging
	// unless the log is actually on
	sExceptionLog.StartLog(eLogExceptions, "Application startup");
}

// Change logging options
void CLog::ChangeLogging(const SLogOptions& options)
{
	// Do enable disable change first to get old logging state correct
	if (options.mActivate ^ sLoggingActive)
	{
		sLoggingActive = options.mActivate;
		if (sLoggingActive)
			ActivateLogging();
		else
			DeactivateLogging();
	}
	
	// Start/stop each log
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (options.mEnable[i] != sLogInfo[i].mDoLog)
		{
			sLogInfo[i].mDoLog = options.mEnable[i];
			
			// Only if active
			if (sLoggingActive)
			{
				if (sLogInfo[i].mDoLog)
					BeginLog(sLogInfo[i].mLog, cLogName[i], true);
				else
					EndLog(sLogInfo[i].mLog, true);
			}
		}
	}

	sAllowAuthenticationLog = options.mAuthentication;
	sAllowPlaybackLog = options.mPlayback;
	sOverwrite = options.mOverwrite;
	sUserCWD = options.mUserCWD;

	// Now write out any changes
	WriteOptions();
}

// Flush all logfiles
void CLog::FlushLogs()
{
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (sLogInfo[i].mLog)
			*sLogInfo[i].mLog << std::flush;
	}
}

// Flush all logfiles
void CLog::ClearLogs()
{
	if (sLoggingActive)
		DeactivateLogging();
	bool old_overwrite = sOverwrite;
	sOverwrite = true;
	ActivateLogging();
	DeactivateLogging();
	sOverwrite = old_overwrite;
	if (sLoggingActive)
		ActivateLogging();
}

// Close all logfiles
void CLog::StopLogging()
{
	// Stop the exception log - this won't do logging
	// unless the log is actually on
	sExceptionLog.StopLog();

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (sLogInfo[i].mLog)
		{
			EndLog(sLogInfo[i].mLog);
			sLogInfo[i].mDoLog = false;
		}
	}
}

// Create all logfiles if required
void CLog::ActivateLogging()
{
	// Open required log files
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (sLogInfo[i].mDoLog)
			BeginLog(sLogInfo[i].mLog, cLogName[i]);
	}
	
	// Start the exception log - this won't do logging
	// unless the log is actually on
	sExceptionLog.StartLog(eLogExceptions, "Application startup");
}

// Suspend logging
void CLog::DeactivateLogging()
{
	// Stop the exception log - this won't do logging
	// unless the log is actually on
	sExceptionLog.StopLog();

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		if (sLogInfo[i].mLog)
		{
			EndLog(sLogInfo[i].mLog);
		}
	}
}

void CLog::ReadOptions()
{
	// Determine logging options
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Classic
	Handle test = ::GetResource('logo', 128);
	if (test)
	{
		for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
			sLogInfo[i].mDoLog = (*((short*) *test) & cLogBits[i]);

		sAllowAuthenticationLog = (*((short*) *test) & cAuthenticationBit);
		sAllowPlaybackLog = (*((short*) *test) & cPlaybackBit);
		sOverwrite = (*((short*) *test) & cOverwriteBit);
		sUserCWD = (*((short*) *test) & cUserCWDBit);
		::ReleaseResource(test);
	}
#else
	cdstring use_log[eLogTypeLast];
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		cdstring key = cLoggingSection;
		key += cLogSection[i];
		GetCFPreference(key, use_log[i]);
	}

	cdstring key = cLoggingSection;
	key += cAuthenticationSection;
	cdstring use_auth_log;
	GetCFPreference(key, use_auth_log);

	key = cLoggingSection;
	key += cPlaybackSection;
	cdstring use_playback_log;
	GetCFPreference(key, use_playback_log);

	key = cLoggingSection;
	key += cOverwriteSection;
	cdstring overwrite_log;
	GetCFPreference(key, overwrite_log);

	key = cLoggingSection;
	key += cUserCWDSection;
	cdstring user_cwd;
	GetCFPreference(key, user_cwd);

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		sLogInfo[i].mDoLog = (use_log[i] == cLogOn);

	sAllowAuthenticationLog = (use_auth_log == cLogOn);
	sAllowPlaybackLog = (use_playback_log == cLogOn);
	sOverwrite = (overwrite_log == cLogOn);
	sUserCWD = (user_cwd == cLogOn);
#endif
#elif __dest_os == __win32_os
	CString use_log[eLogTypeLast];

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		use_log[i] = AfxGetApp()->GetProfileString(cLoggingSection, cLogSection[i], _T(""));
	CString use_auth_log = AfxGetApp()->GetProfileString(cLoggingSection, cAuthenticationSection, _T(""));
	CString use_playback_log = AfxGetApp()->GetProfileString(cLoggingSection, cPlaybackSection, _T(""));
	CString overwrite_log = AfxGetApp()->GetProfileString(cLoggingSection, cOverwriteSection, _T(""));
	CString user_cwd = AfxGetApp()->GetProfileString(cLoggingSection, cUserCWDSection, _T(""));

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		sLogInfo[i].mDoLog = (use_log[i] == cLogOn);

	sAllowAuthenticationLog = (use_auth_log == cLogOn);
	sAllowPlaybackLog = (use_playback_log == cLogOn);
	sOverwrite = (overwrite_log == cLogOn);
	sUserCWD = (user_cwd == cLogOn);
#elif __dest_os == __linux_os

	// Get filename
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += ".logging";
	cdifstream fin(fname);

	// Read ~/.mulberry/.logging key-values
	cdstrmap kvmap;

	while(fin.good())
	{
		cdstring key;
		cdstring value;
		fin >> key >> value;
		if (key.empty() || value.empty())
			break;
		kvmap.insert(cdstrmap::value_type(key, value));
	}

	// Now find each one
	cdstring use_log[eLogTypeLast];
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		cdstrmap::const_iterator result = kvmap.find(cLogSection[i]);
		if (result != kvmap.end())
			use_log[i] = result->second;
	}
		
	cdstring use_auth_log;
	cdstrmap::const_iterator result = kvmap.find(cAuthenticationSection);
	if (result != kvmap.end())
		use_auth_log = result->second;
		
	cdstring use_playback_log;
	result = kvmap.find(cPlaybackSection);
	if (result != kvmap.end())
		use_playback_log = result->second;
		
	cdstring overwrite_log;
	result = kvmap.find(cOverwriteSection);
	if (result != kvmap.end())
		overwrite_log = result->second;
		
	cdstring user_cwd;
	result = kvmap.find(cUserCWDSection);
	if (result != kvmap.end())
		user_cwd = result->second;
		
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		sLogInfo[i].mDoLog = (use_log[i] == cLogOn);

	sAllowAuthenticationLog = (use_auth_log == cLogOn);
	sAllowPlaybackLog = (use_playback_log == cLogOn);
	sOverwrite = (overwrite_log == cLogOn);
	sUserCWD = (user_cwd == cLogOn);
#else
#error __dest_os
#endif
}

// Set logging options
void CLog::WriteOptions()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Classic
	// Write resource
	Handle test = ::GetResource('logo', 128);
	if (test)
	{
		// Set appropriate bits
		short options = 0;
		for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		{
			if (sLogInfo[i].mDoLog)
				options = options | cLogBits[i];
		}

		if (sAllowAuthenticationLog)
			options = options | cAuthenticationBit;
		if (sAllowPlaybackLog)
			options = options | cPlaybackBit;
		if (sOverwrite)
			options = options | cOverwriteBit;
		if (sUserCWD)
			options = options | cUserCWDBit;

		// Now change resource
		*(short*) *test = options;
		::ChangedResource(test);
		::WriteResource(test);
		::ReleaseResource(test);
	}
#else
	// Write to CoreFoundation prefs
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
	{
		cdstring key = cLoggingSection;
		key += cLogSection[i];
		SetCFPreference(key, sLogInfo[i].mDoLog ? cLogOn : cLogOff);
	}

	cdstring key = cLoggingSection;
	key += cAuthenticationSection;
	SetCFPreference(key, sAllowAuthenticationLog ? cLogOn : cLogOff);

	key = cLoggingSection;
	key += cPlaybackSection;
	SetCFPreference(key, sAllowPlaybackLog ? cLogOn : cLogOff);

	key = cLoggingSection;
	key += cOverwriteSection;
	SetCFPreference(key, sOverwrite ? cLogOn : cLogOff);

	key = cLoggingSection;
	key += cUserCWDSection;
	SetCFPreference(key, sUserCWD ? cLogOn : cLogOff);
#endif
#elif __dest_os == __win32_os
	// Write to registry
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		AfxGetApp()->WriteProfileString(cLoggingSection, cLogSection[i], sLogInfo[i].mDoLog ? cLogOn : cLogOff);

	AfxGetApp()->WriteProfileString(cLoggingSection, cAuthenticationSection, sAllowAuthenticationLog ? cLogOn : cLogOff);
	AfxGetApp()->WriteProfileString(cLoggingSection, cPlaybackSection, sAllowPlaybackLog ? cLogOn : cLogOff);
	AfxGetApp()->WriteProfileString(cLoggingSection, cOverwriteSection, sOverwrite ? cLogOn : cLogOff);
	AfxGetApp()->WriteProfileString(cLoggingSection, cUserCWDSection, sUserCWD ? cLogOn : cLogOff);
#elif __dest_os == __linux_os
	// Get filename
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += ".logging";

	// Write ~/.mulberry/.logging key-values
	cdofstream fout(fname);

	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		fout << cLogSection[i] << " " << (sLogInfo[i].mDoLog ? cLogOn : cLogOff) << std::endl;

	fout << cAuthenticationSection << " " << (sAllowAuthenticationLog ? cLogOn : cLogOff) << std::endl;
	fout << cPlaybackSection << " " << (sAllowPlaybackLog ? cLogOn : cLogOff) << std::endl;
	fout << cOverwriteSection << " " << (sOverwrite ? cLogOn : cLogOff) << std::endl;
	fout << cUserCWDSection << " " << (sUserCWD ? cLogOn : cLogOff) << std::endl;
#else
#error __dest_os
#endif
}

void CLog::BeginLog(cdofstream*& log, const char* name, bool user_start)
{
	// Get current CWD and make sure it exists
	cdstring fullpath = (sUserCWD ? CConnectionManager::sConnectionManager.GetUserCWD() :
							 CConnectionManager::sConnectionManager.GetApplicationCWD());
							
	bool ok = false;
	try
	{
		// Check that path exists
		::chkdir(fullpath);
		::addtopath(fullpath, cLogDir);
		::chkdir(fullpath);
		ok = true;
	}
	catch(...)
	{
	}
	if (!ok && !sUserCWD)
	{
		// Switch to user path if app path fails
		sUserCWD = true;
		fullpath = CConnectionManager::sConnectionManager.GetUserCWD();
		try
		{
			// Check that path exists
			::chkdir(fullpath);
			::addtopath(fullpath, cLogDir);
			::chkdir(fullpath);
			ok = true;
		}
		catch(...)
		{
		}
	}

	// No valid path => exit
	if (!ok)
		return;

	// Now add the actual log file name
	::addtopath(fullpath, name);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	StCreatorType creatortype('R*ch', 'TEXT');
#endif

	log = new cdofstream(fullpath, sOverwrite ? (std::ios::out | std::ios::binary) : (std::ios::out | std::ios::app | std::ios::binary));

	*log << os_endl;
	if (user_start)
		*log << "--> #Logging started by user on ";
	else
		*log << "--> #Logging started by Mulberry on ";
	time_t systime = ::time(NULL);
	cdstring atime = ::ctime(&systime);
	atime[atime.length() - 1] = 0;
	*log << atime << os_endl << std::flush;
}

void CLog::EndLog(cdofstream*& log, bool user_stop)
{
	*log << os_endl;
	if (user_stop)
		*log << "--> #Logging stopped by user on ";
	else
		*log << "--> #Logging stopped by Mulberry on ";
	time_t systime = ::time(NULL);
	cdstring atime = ::ctime(&systime);
	atime[atime.length() - 1] = 0;
	*log << atime << os_endl << std::flush;
	log->close();
	delete log;
	log = NULL;
}

bool CLog::AnyActiveLogs()
{
	bool result = false;
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		result = result || sLogInfo[i].mDoLog;
	result = result || sAllowPlaybackLog;
	result = result || sAllowAuthenticationLog;

	return result;
}

void CLog::DisableActiveLogs()
{
	sLoggingActive = false;
	DeactivateLogging();
}

void CLog::DoLoggingOptions()
{
	SLogOptions options;
	for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
		options.mEnable[i] = sLogInfo[i].mDoLog;
	options.mAuthentication = sAllowAuthenticationLog;
	options.mPlayback = sAllowPlaybackLog;
	options.mOverwrite = sOverwrite;
	options.mUserCWD = sUserCWD;
	options.mActivate = sLoggingActive;

	bool result = false;
	try
	{
		// Create the dialog
		result = CLoggingOptionsDialog::PoseDialog(options);
	}
	catch(...)
	{
	}

	// Check for changes
	if (result)
	{
		bool changed = 	false;
		for(unsigned long i = eLogTypeFirst; i < eLogTypeLast; i++)
			changed = changed || (options.mEnable[i] != sLogInfo[i].mDoLog);
		changed = changed || (options.mAuthentication != sAllowAuthenticationLog);
		changed = changed || (options.mPlayback != sAllowPlaybackLog);
		changed = changed || (options.mOverwrite != sOverwrite);
		changed = changed || (options.mUserCWD != sUserCWD);
		changed = changed || (options.mActivate != sLoggingActive);
		if (changed)
			ChangeLogging(options);
	}
}

#pragma mark ____________________________Exception logging

void CLog::LogThrow(const char* type, int err, const char* func, const char* file, int line)
{
	if (sExceptionLog.DoLog())
	{
		time_t systime  = ::time(NULL);
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*sExceptionLog.GetLog() << os_endl << "Throw:     " << atime << os_endl
				  << "\tTID: " << cdthread::current_tid() << " 0x" << std::hex << cdthread::current_tid() << std::dec << os_endl
				  << "\tType: " << type << os_endl
				  << "\tErrno: " << err << " 0x" << std::hex << err << std::																																																																																																																																																																																																																																																																																																									dec << os_endl
				  << "\tFunction: " << func << os_endl
				  << "\tFile: " << file
				  << "," << line << os_endl << std::flush;
	}
}

void CLog::LogCatch(const char* type, const char* func, const char* file, int line)
{
	if (sExceptionLog.DoLog())
	{
		time_t systime  = ::time(NULL);
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*sExceptionLog.GetLog() << "-Catch:    " << atime << os_endl
				  << "\tTID: " << cdthread::current_tid() << " 0x" << std::hex << cdthread::current_tid() << std::dec << os_endl
				  << "\tType: " << type << os_endl
				  << "\tFunction: " << func << os_endl
				  << "\tFile: " << file
				  << "," << line << os_endl << std::flush;
	}
}

void CLog::LogRethrow(const char* func, const char* file, int line)
{
	if (sExceptionLog.DoLog())
	{
		time_t systime  = ::time(NULL);
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*sExceptionLog.GetLog() << "--Rethrow: " << atime << os_endl
				  << "\tTID: " << cdthread::current_tid() << " 0x" << std::hex << cdthread::current_tid() << std::dec << os_endl
				  << "\tFunction: " << func << os_endl
				  << "\tFile: " << file
				  << "," << line << os_endl << std::flush;
	}
}

void CLog::LogError(const char* func, const char* file, int line, const char* err)
{
	if (sExceptionLog.DoLog())
	{
		time_t systime  = ::time(NULL);
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*sExceptionLog.GetLog() << "--Error: " << atime << os_endl
				  << "\tWhat: " << err << os_endl
				  << "\tTID: " << cdthread::current_tid() << " 0x" << std::hex << cdthread::current_tid() << std::dec << os_endl
				  << "\tFunction: " << func << os_endl
				  << "\tFile: " << file
				  << "," << line << os_endl << std::flush;
	}
}

#pragma mark ____________________________Individual logs

// Set type and bump session ID
void CLog::StartLog(ELogType type, const char* desc)
{
	const char* logdesc = NULL;

	mType = type;
	mSessionID = sLogInfo[type].mSessionID++;
	logdesc = cLogGreeting[type];

	if (DoLog())
	{
		time_t systime = ::time(NULL);
		cdstring session_id(GetSessionID());
		*GetLog() << os_endl << "--> #" << session_id << logdesc;
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*GetLog() << atime;
		if (desc)
			*GetLog() << ": " << desc;
		*GetLog() << os_endl << std::flush;
	}
}

// Stop logging
void CLog::StopLog()
{
	const char* desc = NULL;

	desc = cLogTrailer[mType];

	// Create log entry
	if (DoLog())
	{
		time_t systime  = ::time(NULL);
		cdstring session_id(GetSessionID());
		*GetLog() << os_endl << "--> #" << session_id << desc;
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*GetLog() << atime << os_endl << std::flush;
	}
}

// Can log this type?
bool CLog::DoLog()
{
	// Never if admin turned off
#ifdef __MULBERRY
	if (!CAdminLock::sAdminLock.mAllowLogging)
		return false;
#endif

	return sLoggingActive && sLogInfo[mType].mDoLog;
}

// Get log for this type
cdofstream* CLog::GetLog()
{
	return sLogInfo[mType].mLog;
}

// Write an entry header to the log
void CLog::LogEntry(const char* log)
{
	if (DoLog())
	{
		PuntPartial();

		time_t systime  = ::time(NULL);
		cdstring session_id(GetSessionID());
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*GetLog() << os_endl << "--> #" << session_id << "." << cdthread::current_tid() << " " << atime << os_endl;
		*GetLog() << log << os_endl << std::flush;
	}
}

// Write an entry header to the log
void CLog::AddEntry(const char* log)
{
	if (DoLog())
	{
		PuntPartial();

		*GetLog() << log << os_endl << std::flush;
	}
}

// Start a partial log entry
void CLog::StartPartialEntry()
{
	if (DoLog())
	{
		PuntPartial();

		mPartial = true;

		time_t systime  = ::time(NULL);
		cdstring session_id(GetSessionID());
		cdstring atime = ::ctime(&systime);
		atime[atime.length() - 1] = 0;
		*GetLog() << os_endl << "--> #" << session_id << "." << cdthread::current_tid() << " " << atime << os_endl << std::flush;
	}
}

// Write a partial log entry
void CLog::LogPartialEntry(const char* log)
{
	if (DoLog())
	{
		// Ignore CRLFs
		if ((log[0] != '\r') || (log[1] != '\n') || (log[2] != 0))
		{
			if (!mPartial)
				StartPartialEntry();
			*GetLog() << log << std::flush;
		}
	}
}

// End a partial log entry
void CLog::StopPartialEntry()
{
	if (DoLog())
	{
		*GetLog() << os_endl << std::flush;
		mPartial = false;
	}
}

// End a partial log entry
void CLog::PuntPartial()
{
	if (DoLog())
	{
		if (mPartial)
			StopPartialEntry();

		mPartial = false;
	}
}

