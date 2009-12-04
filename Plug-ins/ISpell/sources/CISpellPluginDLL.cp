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

// CISpellPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements ISpell authentication DLL based plug-in for use in Mulberry.
//
// History:
// 13-Dec-1997: Created initial header and implementation.
// 26-Dec-1997: Doing Win32 support.
// 30-Jan-1999: Rebuilt Win32 to remove library bug
//

#include "CISpellPluginDLL.h"
#include "CPluginInfo.h"
#include "CStringUtils.h"

#include <algorithm>
#include <memory>
//using namespace std;

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#define ISPELLERROR(xx_errno, xx_msg) do { \
	char err_buf[256]; \
		::strcpy(err_buf, "ISpell Plugin Error: "); \
		::strncat(err_buf, xx_msg, 200); \
		LogEntry(err_buf); \
	} while (0)

//#define DEBUG_OUTPUT

#pragma mark ____________________________consts

const char* cPluginName = "ISpell Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSpelling;
const char* cPluginDescription = "ISpell spelling plugin for Mulberry." OS_ENDL OS_ENDL COPYRIGHT;

const char* cISPELL = "ispell";

const char cISPELL_OK = '*';
const char cISPELL_ROOT = '+';
const char cISPELL_COMPOUND = '-';
const char cISPELL_MISS = '&';
const char cISPELL_GUESS = '?';
const char cISPELL_NONE = '#';

const char* cISPELL_ADD = "*";
const char* cISPELL_ADDLOWER = "&";
const char* cISPELL_SKIP = "@";
const char* cISPELL_SAVEDICT = "#";
const char* cISPELL_SETPARAM = "~";
const char* cISPELL_ENTERTEXMODE = "+";
const char* cISPELL_EXITTEXMODE = "-";
const char* cISPELL_ENTERTERSE = "!";
const char* cISPELL_EXITTERSE = "%";
const char* cISPELL_DOLINE = "^";

#pragma mark ____________________________CISpellPluginDLL

CISpellPluginDLL::CErrorDetails::CErrorDetails(const CISpellPluginDLL::CErrorDetails& copy)
{
	mError.ask_user = copy.mError.ask_user;
	mError.do_replace = copy.mError.do_replace;
	mError.word = copy.mError.word;
	mError.replacement = copy.mError.replacement;
	mError.sel_start = copy.mError.sel_start;
	mError.sel_end = copy.mError.sel_end;
}

// Constructor
CISpellPluginDLL::CISpellPluginDLL()
{
	mData = new SData;
	mPID = -1;
	mSelectionAdjust = 0;
}

// Destructor
CISpellPluginDLL::~CISpellPluginDLL()
{
	delete mData;
	mData = NULL;
}

// Does plug-in need to be registered
bool CISpellPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'ISPL');
	return false;
}

// Can plug-in run as demo
bool CISpellPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CISpellPluginDLL::CanRun(void)
{
	// Check for ispell as an executable
	const char* path = ::getenv("PATH");
	if (!path)
		return false;

	// Tokenise PATH
	size_t plen = ::strlen(path);
	cdstring cstrpath(path, plen);
	const char* p = ::strtok(cstrpath.c_str_mod(), ":");
	while(p)
	{
		// Make full path
		cdstring nstrpath(p);
		if (nstrpath.c_str()[nstrpath.length() - 1] != '/')
			nstrpath += "/";
		nstrpath += cISPELL;

		// Check for executable
		if (::access(nstrpath.c_str(), X_OK) == 0)
			return true;

		// Next token
		p = ::strtok(NULL, ":");
	}

	return false;
}

// Returns the name of the plug-in
const char* CISpellPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CISpellPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CISpellPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CISpellPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CISpellPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Returns manufacturer of plug-in
long CISpellPluginDLL::GetDataLength(void) const
{
	return 0;
}

// Returns description of plug-in
const void* CISpellPluginDLL::GetData(void) const
{
	return nil;
}

int CISpellPluginDLL::SpInitialise(void)
{
	// Initialise the pipe here
	long result = 1;

	// These are the command options
	cdstrvect out;
	out.push_back("ispell");
	out.push_back("-a");
	out.push_back("-S");

	// Create the pipes
	mInputfd[0] = -1;
	mInputfd[1] = -1;
	mOutputfd[0] = -1;
	mOutputfd[1] = -1;
	try
	{
		if (pipe(mInputfd) == -1)
		{
			ISPELLERROR(eSpell_UnknownError, "Could not create input pipe");
			throw -1L;
		}
		if (pipe(mOutputfd) == -1)
		{
			ISPELLERROR(eSpell_UnknownError, "Could not create output pipe");
			throw -1L;
		}
	}
	catch(...)
	{
		if (mInputfd[0] != -1)
		{
			::close(mInputfd[0]);
			mInputfd[0] = -1;
		}
		if (mInputfd[1] != -1)
		{
			::close(mInputfd[1]);
			mInputfd[1] = -1;
		}
		if (mOutputfd[0] != -1)
		{
			::close(mOutputfd[0]);
			mOutputfd[0] = -1;
		}
		if (mOutputfd[1] != -1)
		{
			::close(mOutputfd[1]);
			mOutputfd[1] = -1;
		}

		return 0;
	}

	// Now fork
	mPID = fork();
	if (mPID == -1)
	{
		ISPELLERROR(eSpell_UnknownError, "Failed to fork");
		if (mInputfd[0] != -1)
		{
			::close(mInputfd[0]);
			mInputfd[0] = -1;
		}
		if (mInputfd[1] != -1)
		{
			::close(mInputfd[1]);
			mInputfd[1] = -1;
		}
		if (mOutputfd[0] != -1)
		{
			::close(mOutputfd[0]);
			mOutputfd[0] = -1;
		}
		if (mOutputfd[1] != -1)
		{
			::close(mOutputfd[1]);
			mOutputfd[1] = -1;
		}
	}
	else if (mPID == 0)
	{
		// Child process

		// Close other ends of pipes
		if (mInputfd[1] != -1)
		{
			::close(mInputfd[1]);
			mInputfd[1] = -1;
		}
		if (mOutputfd[0] != -1)
		{
			::close(mOutputfd[0]);
			mOutputfd[0] = -1;
		}

		// Direct stdin, stdout, stderr to /dev/null
		int devnull = ::open("/dev/null", O_RDWR);
		if (devnull == -1)
		{
			ISPELLERROR(eSpell_UnknownError, "Failed to open /dev/null in child process");
			exit(1);
		}

		// Setup stdin, stdout & stderr
		// input fd -> stdin
		::dup2(mInputfd[0], 0);
		::close(mInputfd[0]);
		mInputfd[0] = -1;

		// output fd -> stdout
		::dup2(mOutputfd[1], 1);
		::close(mOutputfd[1]);
		mOutputfd[1] = -1;

		// stderr -> /dev/null
		::dup2(devnull, 2);

		// Close fds that we do not want the child to inherit
		long open_max = ::sysconf(_SC_OPEN_MAX);
		if (open_max > 0)
		{
			for (int i = 3; i < open_max; i++)
				::close(i);
		}

		// create args
		int argc = out.size();
		char** argv = (char**) malloc((argc + 1) * sizeof(char*));
		char** p = argv;
 		for(unsigned int i = 0; i < out.size(); i++)
 			*p++ = out[i].c_str_mod();
 		*p = NULL;
 
		// Execute it
		::execvp(argv[0], argv);

		// Only get here if failed
		ISPELLERROR(eSpell_UnknownError, "Failed to exec process");
		exit(1);
	}
	else
	{
		// ISpell will print status line
		cdstring temp;
		GetLine(temp);

		// Turn on terse mode
		SendLine(cISPELL_ENTERTERSE);
		SendLine("\n");
	}

	return result;
}

int CISpellPluginDLL::SpTerminate(void)
{
	// Kill the pipe here
	if (mPID != -1)
	{
		// Save dictionary first as kill won't
		SendLine(cISPELL_SAVEDICT);
		SendLine("\n");

		// Now do fake check to sync with dictionary save
		SendLine(cISPELL_DOLINE);
		SendLine("\n");
		cdstring temp;
		GetLine(temp);

		// Kill it
		::kill(mPID, SIGINT);

		::close(mOutputfd[0]);
		::close(mOutputfd[1]);
		::close(mInputfd[0]);
		::close(mInputfd[1]);

		mInputfd[0] = -1;
		mInputfd[1] = -1;
		mOutputfd[0] = -1;
		mOutputfd[1] = -1;

		mPID = -1;
	}

	return 1;
}

void CISpellPluginDLL::UpdatePreferences(void)
{
	// ISpell has no user options right now
}

int CISpellPluginDLL::NewDictionary(const char* name)
{
	return 0;
}

int CISpellPluginDLL::OpenDictionary(const char* name)
{
	return 1;
}

int CISpellPluginDLL::CloseDictionary(int dict)
{
	return 1;
}

int CISpellPluginDLL::AddWord(const char* word)
{
	// Add a word to personal dictionary
	SendLine(cISPELL_ADD);
	SendLine(word);
	SendLine("\n");

	// Also do implicit SkipAll on this word in case it's
	// already flagged as an error 
	mSkipAll.insert(word);

	return 1;
}

const char* CISpellPluginDLL::GetAddSuffix(int i) const
{
	return NULL;
}

int CISpellPluginDLL::RemoveWord(const char* word)
{
	// Not supported by ISpell
	return 1;
}

int CISpellPluginDLL::CountPages(void) const
{
	// Not supported by ISpell
	return 0;
}

int CISpellPluginDLL::CurrentPage(void) const
{
	// Not supported by ISpell
	return 0;
}

int CISpellPluginDLL::SetPage(int page)
{
	// Not supported by ISpell
	return 0;
}

int CISpellPluginDLL::GetPage(const char* word, const char** buf, size_t* buflen)
{
	// Not supported by ISpell

	// If word exists then try to cache its page and get closest pos
	int result = -1;

	// Return page details
	*buf = NULL;
	*buflen = 0;
	
	return result;
}

const char* CISpellPluginDLL::GetSuggestions(const char* word, long ctr)
{
	// Look up error word in map
	cdstring temp_word(word);
	::strlower(temp_word.c_str_mod());
	cdstrmap::const_iterator found = mSuggestions.find(temp_word);
	if (found != mSuggestions.end())
		return (*found).second.c_str();

	// Do explicit lookup of word
	mData->mBuffer.clear();

	// Check a single word
	SendLine(cISPELL_DOLINE);
	SendLine(word);
	SendLine("\n");

	// Loop over errors
	bool result = true;
	bool run_loop = true;
	int dummy1 = 0;
	int dummy2 = 0;
	cdstring dummy3;
	while(run_loop)
	{
		cdstring temp;
		GetLine(temp);

		// Return result
		switch(temp[0UL])
		{
		case cISPELL_MISS:
		case cISPELL_GUESS:
			ParseMissGuess(temp, dummy3, dummy1, dummy2, mData->mBuffer);
			break;
		case cISPELL_NONE:
			ParseNone(temp, dummy3, dummy1, dummy2, mData->mBuffer);
			break;
		case '\0':
			run_loop = false;
			break;
		default:;
		}
	}

	return mData->mBuffer.c_str();
}

// ContainsWord: check a word for correct spelling
// word:    the word to check
//
// returns: true if word is in dictionary
//          false if not in dictionary
//
bool CISpellPluginDLL::ContainsWord(const char* word)
{
	// Check a single word
	SendLine(cISPELL_DOLINE);
	SendLine(word);
	SendLine("\n");

	// Loop over errors
	bool result = true;
	while(true)
	{
		cdstring temp;
		GetLine(temp);

		// Return result
		switch(temp[0UL])
		{
		case cISPELL_MISS:
		case cISPELL_GUESS:
		case cISPELL_NONE:
			result = false;
			break;
		case '\0':
			return result;
		default:;
		}
	}

	return result;
}

int CISpellPluginDLL::ClosestWord(const char* word)
{
	// Not supported by ISpell
	return 0;
}

int CISpellPluginDLL::CheckText(const char* txt)
{
	// Cache pointer to text
    const char* p = txt;
    mErrorIndex = 0;
    mErrorList.clear();
	mSelectionAdjust = 0;

	// Check each line until one fails
	while(*p)
	{
		// Bump over lines
		while(*p && (*p == '\n'))
			p++;

		if (!*p)
			break;

		// Cache line start
		const char* line_start = p;
		unsigned long line_offset = p - txt;

		// Bump to end of line
		while(*p && (*p != '\n'))
			p++;

		// Cache line end
		const char* line_end = p;

		// Check this line if something there
		if (line_end != line_start)
		{
			// Send line to ispell for checking
			SendLine(cISPELL_DOLINE);
			SendLine(line_start, line_end - line_start);
			SendLine("\n");

			// Check each returned line
			bool another_line = true;
			while(another_line)
			{
				cdstring result;
				GetLine(result);

				cdstring suggestions;
				int start = 0;
				int end = 0;
				switch(result[0UL])
				{
				case cISPELL_MISS:
				case cISPELL_GUESS:
					ParseMissGuess(result, mErrorWord, start, end, suggestions);
					break;
				case cISPELL_NONE:
					ParseNone(result, mErrorWord, start, end, suggestions);
					break;
				default:
					another_line = false;
					continue;
				}

				// Initialise the error structure
				CErrorDetails error;

				error.mError.ask_user = true;
				error.mError.do_replace = false;
				error.mError.word = mErrorWord.c_str();
				error.mError.replacement = NULL;
				error.mError.sel_start = line_offset + start;
				error.mError.sel_end = line_offset + end;

				cdstring temp_word(error.mError.word);
				::strlower(temp_word.c_str_mod());
				if (!mSuggestions.count(temp_word))
					mSuggestions.insert(cdstrmap::value_type(temp_word, suggestions));

				// Add error item to list
				mErrorList.push_back(error);

#ifdef DEBUG_OUTPUT
				::printf("Error Index: %d, Start: %ld, End: %ld\n", mErrorList.size(), error.mError.sel_start, error.mError.sel_end);
#endif
			}
		}
	}

    return 1;
}

bool CISpellPluginDLL::HasErrors(void) const
{
	return !mErrorList.empty();
}

const CISpellPluginDLL::SpError* CISpellPluginDLL::CurrentError(void) const
{
	return &mErrorList[mErrorIndex - 1].mError;
}

CISpellPluginDLL::SpError* CISpellPluginDLL::CurrentError(void)
{
	return &mErrorList[mErrorIndex - 1].mError;
}

const CISpellPluginDLL::SpError* CISpellPluginDLL::NextError(const char* txt)
{
	mErrorIndex++;
	if (mErrorIndex <= mErrorList.size())
	{
		// Do adjustment to selection first
		CurrentError()->sel_start += mSelectionAdjust;
		CurrentError()->sel_end += mSelectionAdjust;

		// Now check to see if error is a skip all
		cdstrset::iterator found_skip = mSkipAll.find(CurrentError()->word);
		bool do_skip = (found_skip != mSkipAll.end());

		// Now check to see if error is a replace all
		cdstrmap::iterator found_replace = mReplaceAll.find(CurrentError()->word);
		bool do_replace = (found_replace != mReplaceAll.end());

		CurrentError()->ask_user = !do_skip;
		CurrentError()->do_replace = do_replace;
		CurrentError()->replacement = (do_replace ? found_replace->second.c_str() : NULL);

		return CurrentError();
	}
	else
		return NULL;
}

bool CISpellPluginDLL::ErrorIsPunct(void) const
{
	return false;
}

bool CISpellPluginDLL::ErrorIsDoubleWord(void) const
{
	return false;
}

// Skip current error
int CISpellPluginDLL::Skip(void)
{
	// Nothing to do for a one-word skip
	return 1;
}

// Skip all current error
int CISpellPluginDLL::SkipAll(void)
{
	// Add current error to skip all set
	mSkipAll.insert(CurrentError()->word);

	return 1;
}

// Replace current error
int CISpellPluginDLL::Replace(void)
{
	// Caller is doing the replacement in the original text
	// We must adjust future selection offsets to accomodate
	// a change in length of the word being replaced
	if (CurrentError()->replacement)
		mSelectionAdjust += ::strlen(CurrentError()->replacement) - ::strlen(CurrentError()->word);
	return 1;
}

// Replace all current error
int CISpellPluginDLL::ReplaceAll(void)
{
	// Add current error to replace all map
	if (CurrentError()->replacement)
		mReplaceAll.insert(cdstrmap::value_type(CurrentError()->word, CurrentError()->replacement));

	// Also do replace behaviour
	return Replace();
}

// Process capitalisation etc
void CISpellPluginDLL::ProcessReplaceString(char* replace) const
{
	// Convert first char to upper if in error word
	if (replace && isupper(CurrentError()->word[0]))
		replace[0] = toupper(replace[0]);
}

// Get line from ispell process
int CISpellPluginDLL::GetLine(cdstring& line)
{
	// Check for complete line already in buffer
	const char* p = ::strchr(mData->mReadBuffer.c_str(), '\n');
	if (p)
	{
		line = cdstring(mData->mReadBuffer.c_str(), p - mData->mReadBuffer.c_str());
		mData->mReadBuffer = cdstring(p + 1);
#ifdef DEBUG_OUTPUT
		printf("%s\n", line.c_str());
#endif
		return 1;
	}

	// Parent process must wait
	int result = 0;
	int status;
	pid_t retpid = ::waitpid(mPID, &status, WNOHANG);
	//pid_t retpid = ::waitpid(mPID, &status, 0);

	fd_set readfds;
	FD_ZERO (&readfds);
	FD_SET (mOutputfd[0], &readfds);
	struct timeval timeout5 = { 5, 0 };
	struct timeval timeout0 = { 0, 0 };
	int dataavail = select (mOutputfd[0] + 1, &readfds, NULL, NULL, (retpid == 0) ? &timeout5 : &timeout0);

	while((retpid == 0) || (dataavail != 0))
	{
		// Handle output
		if (FD_ISSET(mOutputfd[0], &readfds))
		{
			const int buflen = 1024;
			char buf[buflen];
			int readsize = ::read(mOutputfd[0], buf, buflen);
			if (readsize < 0)
				continue;
			buf[std::min(buflen - 1, readsize)] = 0;
#ifdef DEBUG_OUTPUT
			//printf("%s", buf);
#endif
			LogEntry(buf);
			mData->mReadBuffer += buf;

			// Check for complete line
			const char* p = ::strchr(mData->mReadBuffer.c_str(), '\n');
			if (p)
			{
				line = cdstring(mData->mReadBuffer.c_str(), p - mData->mReadBuffer.c_str());
#ifdef DEBUG_OUTPUT
				printf("%s\n", line.c_str());
#endif
				mData->mReadBuffer = cdstring(p + 1);
				return 1;
			}
		}

		// Try some more
		if (retpid == 0)
			retpid = ::waitpid(mPID, &status, WNOHANG);
		FD_ZERO (&readfds);
		FD_SET (mOutputfd[0], &readfds);
		dataavail = select (mOutputfd[0] + 1, &readfds, NULL, NULL, (retpid == 0) ? &timeout5 : &timeout0);
	}

	// Check process return value
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	{
		ISPELLERROR(eSpell_UnknownError, "Exit status non-zero");
		result = 0;
		{
			char buf[1024];
			::sprintf(buf, "Exit status non-zero: %d\n", WEXITSTATUS(status));
#ifdef DEBUG_OUTPUT
			printf("%s", buf);
#endif
			LogEntry(buf);
		}
	}
	else if (WIFSIGNALED(status) && WTERMSIG(status) != 0)
	{
		ISPELLERROR(eSpell_UnknownError, "Unhandled signal\n");
		result = 0;
	}
	else
		result = 1;

	return result;
}


// Send line to ispell process
int CISpellPluginDLL::SendLine(const char* line)
{
	int result = ::write(mInputfd[1], line, ::strlen(line));

	return result;
}

// Send line to ispell process
int CISpellPluginDLL::SendLine(const char* line, size_t length)
{
	int result = ::write(mInputfd[1], line, length);

	return result;
}

// ParseMissGuess: parse ISpell '&' and '?' responses
// line:  text to parse
// start: return index to start of misspelled word
// end:   return index to end of misspelled word
//
void CISpellPluginDLL::ParseMissGuess(cdstring& line, cdstring& error, int& start, int& end, cdstring& suggestions)
{
	char* p = line.c_str_mod();

	// Step over '& '/'? '
	if ((*p != '&') && (*p != '?')) return;
	p++;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Step over word and trailing space
	char* wstart = p;
	while(*p && (*p != ' ')) p++;
	char* wend = p;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Step over count and trailing space
	int count = ::atoi(p);
	while(*p && (*p != ' ')) p++;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Step over offset and trailing colon space
	int offset = ::atoi(p);
	while(*p && (*p != ':')) p++;
	if (*p++ != ':') return;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Now tokenize on comma and space
	char* q = ::strtok(p, ", ");
	while(q)
	{
		// Suggestions are newline separated strings
		if (!suggestions.empty())
			suggestions += "\n";
		suggestions += q;
		q = ::strtok(NULL, ", ");
	}

	// Setup return values
	error.assign(wstart, wend - wstart);
	start = offset - 1;
	end = start + wend - wstart;
}

// ParseMissGuess: parse ISpell '#' response
// line:  text to parse
// start: return index to start of misspelled word
// end:   return index to end of misspelled word
//
void CISpellPluginDLL::ParseNone(cdstring& line, cdstring& error, int& start, int& end, cdstring& suggestions)
{
	char* p = line.c_str_mod();

	// Step over '# '
	if (*p++ != '#') return;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Step over word and trailing space
	char* wstart = p;
	while(*p && (*p != ' ')) p++;
	char* wend = p;
	if (*p++ != ' ') return;
	while(*p && (*p == ' ')) p++;

	// Get offset
	int offset = ::atoi(p);

	// Setup return values
	error.assign(wstart, wend - wstart);
	start = offset - 1;
	end = start + wend - wstart;
}

bool CISpellPluginDLL::CannotDisplayDictionary(void) const
{
	// Spellex cannot display contents of (compressed) dictionary
	return true;
}

int CISpellPluginDLL::SetMainDictionaryPath(const char* path)
{
	return 1;
}

int CISpellPluginDLL::SetUserDictionaryPath(const char* path)
{
	return 1;
}

int CISpellPluginDLL::GetDictionaries(const char** dicts)
{
	*dicts = NULL;

	// Copy dictionary names into list buffer
	mData->mListBuffer.clear();
	mData->mListBuffer.push_back("*");

	// Allocate space for array and arrays
	mData->mBuffer.clear();
	mData->mBuffer.reserve(sizeof(char*) * (mData->mListBuffer.size() + 1));

	// Assign list buffer items to buffer
	char** p = reinterpret_cast<char**>(mData->mBuffer.c_str_mod());
	for(cdstrvect::iterator iter = mData->mListBuffer.begin(); iter != mData->mListBuffer.end(); iter++)
		*p++ = (*iter).c_str_mod();
	*p++ = NULL;

	*dicts = mData->mBuffer.c_str();

	return 1;
}


int CISpellPluginDLL::SetDictionary(const char* name)
{
	return 1;
}

int CISpellPluginDLL::SetDictionaryByLanguage(const char* isolangtag)
{
	return 0;
}

bool CISpellPluginDLL::MoreSuggestions(long ctr)
{
	return false;
}

const CISpellPluginDLL::SpError* CISpellPluginDLL::CheckWord(const char* word)
{
	mErrorWord = word;

	// Check a single word
	SendLine(cISPELL_DOLINE);
	SendLine(word);
	SendLine("\n");

	// Loop over errors
	bool result = true;
	bool run_loop = true;
	while(run_loop)
	{
		cdstring temp;
		GetLine(temp);

		// Return result
		switch(temp[0UL])
		{
		case cISPELL_MISS:
		case cISPELL_GUESS:
		case cISPELL_NONE:
			result = false;
			break;
		case '\0':
			run_loop = false;
			break;
		default:;
		}
	}

	if (result)
		return NULL;
	else
	{
		// Now check to see if error is a replace all
		cdstrmap::iterator found_replace = mReplaceAll.find(word);
		bool do_replace = (found_replace != mReplaceAll.end());
		const char* replacement = (do_replace ? found_replace->second.c_str() : NULL);

		mError.ask_user = false;
		mError.do_replace = do_replace;
		mError.word = mErrorWord.c_str();
		mError.replacement = replacement;
		mError.sel_start = 0;
		mError.sel_end = 0;

	  	return &mError;
	}
}
