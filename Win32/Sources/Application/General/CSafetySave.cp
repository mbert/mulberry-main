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


// Source for CSafetySave class

#include "CSafetySave.h"

#include "CConnectionManager.h"
#include "CLetterWindow.h"
#include "CLocalCommon.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CUnicodeStdLib.h"

#include "diriterator.h"

#pragma mark ____________________________consts

const char* cDraftExtension = ".mbd";
const char* cRecoverDraftName = "RecoverDraft";

#pragma mark ____________________________statics

cdstring CSafetySave::sSafetySaveDirectory;

#pragma mark ____________________________methods

// SafetySaveDrafts
//   Save open drafts to disk when crash occurs.
//
// recovered <- number of drafts actually saved
// result    <- total number of open drafts
//
// Notes:
// This function is called from the Win32 exception handler. It
// must therefore protect aganst any kind of additional exception (Win32 or C++)
// to prevent total failure of the exception handler.
//
// History:
// 11-Sep-2002 cd: created initial implementation
//

unsigned long CSafetySave::SafetySaveDrafts(unsigned long& recovered)
{
	// Init counters
	unsigned long total = 0;
	recovered = 0;

	// Protect against all exceptions
	try
	{
		// Make sure proper path is setup - fail if it is not as we don't want to try and
		// create it here in case it generates an exception. If it is not setup then that must
		// mean the crash occurred before the restore function was called so no drafts exist anyway.
		if (sSafetySaveDirectory.empty())
			return 0;
		
		// Now look for any drafts
		unsigned long file_ctr = 1;
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
		{
			// Create file name
			cdstring fpath;
			while(file_ctr < 100)
			{
				// Buffer to use for file name
				fpath = sSafetySaveDirectory;
				::addtopath(fpath, cRecoverDraftName);
				fpath += cdstring(file_ctr++);
				fpath += cDraftExtension;
				
				// Check whether it exists and exit loop if not
				if (!fileexists(fpath))
					break;
			}

			// Adjust counters and attempt safety save
			total++;
			if ((*iter)->SafetySave(fpath))
				recovered++;
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return total;
}

// SafetyRestoreDrafts
//   Restore drafts from safety save directory when Mulberry restarts.
//   after a crash.
//
// result <- total number of recovered drafts
//
// Notes:
// This function is called after Mulberry has started up and preferences loaded.
// The directory to search in is dependent on the preferences so it must be done
// after prefs have loaded. It should only be called once.
//
// History:
// 11-Sep-2002 cd: created initial implementation
//

unsigned long CSafetySave::SafetyRestoreDrafts()
{
	// Init counters
	unsigned long total = 0;
	
	// Protect against all exceptions
	try
	{
		// Make sure proper path is setup
		if (sSafetySaveDirectory.empty())
			sSafetySaveDirectory = CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory();
		
		// Iterate over all draft files in this directory and open them
		cdstring dir = sSafetySaveDirectory;
		diriterator iter(dir, cDraftExtension);
		dir += os_dir_delim;
		const char* p = NULL;
		while(iter.next(&p))
		{
			// Convert to full path and attempt open
			cdstring fullpath = dir;
			fullpath += p;
			if (RecoverDraft(fullpath))
				// Bump recover count
				total++;
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return total;
}

// RecoverDrafts
//   Recover a draft file saved to disk, then delete it.
//
// fpath  -> file path of draft to open
// result <- indicates successful recovery
//
// Notes:
// This function is called from the Win32 exception handler. It
// must therefore protect aganst any kind of additional exception (Win32 or C++)
// to prevent total failure of the exception handler.
//
// History:
// 11-Sep-2002 cd: created initial implementation
//

bool CSafetySave::RecoverDraft(const cdstring& path)
{
	bool result = false;

	// Protect against all execptions so that failure to open this one
	// will not prevent others from opening
	try
	{
		// Use default MFC document open - this assumes the file extension is
		// properly set to .mbd
		result = (CMulberryApp::sApp->OpenDocumentFile(path.win_str()) != NULL);
		
		// Delete the file
		::remove_utf8(path);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		result = false;
	}
	
	return result;
}
