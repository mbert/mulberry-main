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

#ifndef __CLOG__MULBERRY__
#define __CLOG__MULBERRY__

#define CLOG_LOGTHROW(_type, _param) \
		CLog::LogThrow(#_type, _param, __FUNCTION__, __FILE__, __LINE__)
#define CLOG_LOGCATCH(_type) \
		CLog::LogCatch(#_type, __FUNCTION__, __FILE__, __LINE__)
#define CLOG_LOGRETHROW \
		CLog::LogRethrow(__FUNCTION__, __FILE__, __LINE__)
#define CLOG_LOGERROR(_err) \
		CLog::LogError(__FUNCTION__, __FILE__, __LINE__, _err)

#include "cdfstream.h"

class CLog
{
public:
	enum ELogType
	{
		eLogNone,
		eLogTypeFirst,
		eLogIMAP = eLogTypeFirst,
		eLogPOP3,
		eLogIMSP,
		eLogACAP,
		eLogSMTP,
		eLogHTTP,
		eLogPlugin,
		eLogFilters,
		eLogExceptions,
		eLogTypeLast
	};
	
	struct SLogOptions
	{
		bool	mEnable[eLogTypeLast];
		bool	mAuthentication;
		bool	mPlayback;
		bool	mOverwrite;
		bool	mUserCWD;
		bool	mActivate;
	};

	CLog()
		{ mType = eLogNone; mSessionID = 0; mPartial = false; }
	~CLog() {}
	
	static void StartLogging();								// Create all logfiles if required
	static void ChangeLogging(const SLogOptions& options);	// Change logging options
	static void FlushLogs();								// Flush logs now
	static void ClearLogs();								// Clear logs now
	static void StopLogging();								// Close all logfiles

	static void ActivateLogging();							// Suspend logging without changing prefs
	static void DeactivateLogging();						// Suspend logging without changing prefs
	
	static void	ReadOptions();
	static void WriteOptions();

	static void BeginLog(cdofstream*& log, const char* name, bool user_start = false);
	static void EndLog(cdofstream*& log, bool user_stop = false);
	
	static bool AnyActiveLogs();
	static void DisableActiveLogs();

	static void LogThrow(const char* type, int err, const char* func, const char* file, int line);
	static void LogCatch(const char* type, const char* func, const char* file, int line);
	static void LogRethrow(const char* func, const char* file, int line);
	static void LogError(const char* func, const char* file, int line, const char* err);

	void StartLog(ELogType type,		// Set type and bump session ID
					const char* desc = NULL);
	void StopLog();					// Stop logging

	bool DoLog();					// Can log this type?
	long GetSessionID()				// Get ID for this log session
		{ return mSessionID; }
	cdofstream* GetLog();				// Get log stream
	void LogEntry(const char* log);				// Write an entry to log
	void AddEntry(const char* log);				// Write an entry to log
	
	void StartPartialEntry();
	void LogPartialEntry(const char* log);		// Write an entry to log
	void StopPartialEntry();

	static bool AllowAuthenticationLog()
		{ return sAllowAuthenticationLog; }

	static bool AllowPlaybackLog()
		{ return sAllowPlaybackLog; }

	static void DoLoggingOptions();	// Do dialog to change logging options

private:
	static CLog sExceptionLog;

	ELogType	mType;
	long		mSessionID;
	bool		mPartial;

	struct SLogInfo
	{
		bool			mDoLog;
		cdofstream*		mLog;
		long			mSessionID;
		
		SLogInfo()
			{ mDoLog = false; mLog = NULL; mSessionID = 1; }
	};

	static SLogInfo		sLogInfo[eLogTypeLast];

	static bool			sLoggingActive;
	static bool			sAllowAuthenticationLog;
	static bool			sAllowPlaybackLog;
	static bool			sOverwrite;
	static bool			sUserCWD;
	
	void PuntPartial();
};

#endif
