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


// Source for CSearchEngine class

#include "CSearchEngine.h"

#include "CActionManager.h"
#include "CLog.h"
#include "CMailboxInfoView.h"
#include "CMailboxInfoWindow.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMboxProtocol.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryApp.h"
#endif
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSearchItem.h"
#include "CSearchWindow.h"
#include "CTaskClasses.h"
#include "C3PaneWindow.h"

// Static members
CSearchEngine CSearchEngine::sSearchEngine;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CSearchEngine::CSearchEngine()
	: mSearchItem(CSearchItem::eFrom, cdstring::null_str), mTargets(true)
{
}

CSearchEngine::~CSearchEngine()
{
}

#pragma mark ____________________________Manage Target list

bool CSearchEngine::TargetsAddMbox(const CMbox* mbox)
{
	bool result = (mTargets.AddMbox(mbox) != NULL);
	if (result)
		mTargetHits.push_back(eNoSearch);

	return result;
}

const CMboxRef* CSearchEngine::GetCurrentTargetMbox() const
{
	return (mCurrentTarget >= 0 ? static_cast<const CMboxRef*>(mTargets.at(mCurrentTarget)) : NULL);
}

long CSearchEngine::NextTarget()
{
	while((++mCurrentTarget < mTargetHits.size()) && (mTargetHits.at(mCurrentTarget) != eSearchHit)) {}
	if (mCurrentTarget >= mTargetHits.size())
		mCurrentTarget = -1;

	return mCurrentTarget;
}

void CSearchEngine::OpenTarget(long target, CMailboxView* recycle)
{
	// Create mbox ref
	const CMboxRef* ref = static_cast<CMboxRef*>(mTargets.at(target));

	// Resolve to closest match
	CMbox* mbox = ref->ResolveMbox(true);
	if (!mbox)
		return;

	// Resolve to open mailbox
	CMbox* open_mbox = (mbox->IsOpen() ? mbox : mbox->GetProtocol()->FindOpenMbox(mbox->GetName()));
	if (open_mbox)
		mbox = open_mbox;

	// Check current search state of mailbox before opening
	// Must check that status exists first
	bool has_search = mbox->HasStatus() ? mbox->GetSearchResults().size() : false;

	// Does window or view already exist?
	CMailboxInfoWindow* theWindow = (CMailboxInfoWindow*) CMailboxWindow::FindWindow(mbox);
	CMailboxInfoView* theView = (CMailboxInfoView*) CMailboxView::FindView(mbox);

	if (theView)
	{
		// Change mode in window
		if (has_search)
			theView->ResetSearch();

		// Bring existing window to front
		if (theWindow)
			FRAMEWORK_WINDOW_TO_TOP(theWindow)
		else
			FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
	}
	else if (recycle)
		recycle->Recycle(mbox, has_search);
	else
		CActionManager::OpenMailbox(mbox, true, has_search);

	// Set as new target
	mCurrentTarget = target;
}

#pragma mark ____________________________Searching

void CSearchEngine::StartSearch()
{
	// Reset counters
	mNextSearchIndex = 0;
	mDone = 0;
	mFound = 0;
	mMessages = 0;
	mAbort = false;
	mAbortCompleted = false;
	mCurrentTarget = -1;

	// Now start its thread!
	StartExecution();
}

// Forcibly abort all busy threads
void CSearchEngine::Abort()
{
	mAbort = true;
	mAbortCompleted = false;

	// Force abort of all connections - threads will cleared next time through PollThreads
	for(cdthreadvect::iterator iter = mSearchThreads.begin(); iter != mSearchThreads.end(); iter++)
	{
		CMbox* mbox = const_cast<CMbox*>(static_cast<CSearchThread*>(*iter)->GetMbox());
		if (mbox)
			mbox->Abort(true);
	}
}

// This is executed by the search engine thread
void CSearchEngine::Execute()
{
	long num_search = mTargets.size();
	long num_threads = std::min(CPreferences::sPrefs->mMultipleSearch.GetValue() ?
								CPreferences::sPrefs->mNumberMultipleSearch.GetValue() : 1, num_search);

	// Reset thread done event
	mThreadDone.clear();

	// Clear icons in list and set to top
	{
		CSearchWindowStartSearchTask* task = new CSearchWindowStartSearchTask;
		task->Go();
	}

	// Create threads and start them running
	for(long i = 0; i < num_threads; i++)
	{
		CSearchThread* thread = new CSearchThread;
		mSearchThreads.push_back(thread);
		thread->Start();
	}

	bool still_running = true;
	while(still_running)
	{
		// Now wait for condition (1 sec timeout)
		mThreadDone.wait(1000);

		// Test each thread to see if they are running
		still_running = false;
		for(long i = 0; i < num_threads; i++)
			still_running |= !static_cast<CSearchThread*>(mSearchThreads.at(i))->GetDone();
	}

	// Delete all threads
	for(long i = 0; i < num_threads; i++)
		delete mSearchThreads.at(i);
	mSearchThreads.clear();

	// Determine first target
	NextTarget();

	// Set to first found
	{
		CSearchWindowEndSearchTask* task = new CSearchWindowEndSearchTask;
		task->Go();
	}

	// Do open first if required and one exists
	if (!mAbort && CPreferences::sPrefs->mOpenFirstSearchResult.GetValue() && (GetCurrentTarget() >= 0))
	{
		CSearchOpenTargetTask* task = new CSearchOpenTargetTask(this, GetCurrentTarget());
		task->Go();
	}

	// Indicate abort complete
	if (mAbort)
		mAbortCompleted = true;
}

// WARNING: this is executed from within each search thread
// It MUST be thread safe wrt to shared resources
bool CSearchEngine::SearchNextMbox(CSearchThread* thread)
{
	// Check abort state
	if (mAbort)
		return false;

	CMbox* mbox = NULL;
	unsigned long search_index = 0;

	{
		// Lock the mailbox list mutex
		cdmutex::lock_cdmutex _lock(_search_lock);

		// Loop to get the next valid mailbox
		while(true)
		{
			search_index = mNextSearchIndex;

			// Check index - if list exhausted exit
			if (mNextSearchIndex >= mTargets.size())
				return false;

			// Only search mailboxes which resolve and are logged in and locally cached
			mbox = static_cast<CMboxRef*>(mTargets.at(mNextSearchIndex++))->ResolveMbox(true);
			if (!mbox || !mbox->GetProtocol()->IsLoggedOn() ||
				(mbox->IsLocalMbox() && !mbox->IsCachedMbox()))
				continue;
			else
				break;
		}
	}

	// Exit if nothing left
	if (!mbox)
		return false;

	// Update visual state to indicate mailbox being searched
	{
		CSearchWindowNextSearchTask* task = new CSearchWindowNextSearchTask(search_index);
		task->Go();
	}

	// Do the search
	thread->SetMbox(mbox);
	unsigned long found_count = 0;
	bool failed = !SearchMbox(mbox, found_count);
	thread->SetMbox(NULL);

	bool hit = false;
	unsigned long done = 0;
	unsigned long found = 0;
	unsigned long messages = 0;

	// Must lock mutex while changing data in the search engine
	{
		cdmutex::lock_cdmutex _lock(_search_lock);
		
		if (failed)
		{
			// Clear any icon
			mTargetHits.at(search_index) = eNoSearch;
		}
		else
		{
			hit = (found_count > 0);
			mTargetHits.at(search_index) = hit ? eSearchHit : eSearchMiss;
			done = ++mDone;
			if (hit)
			{
				found = ++mFound;
				mMessages += found_count;
				messages = mMessages;
			}
		}
	}

	// Update visual state of mailbox after search
	{
		CSearchWindowUpdateTask* task = new CSearchWindowUpdateTask(failed, search_index, hit, done, found, messages);
		task->Go();
	}

	// Done with search
	return true;
}

// WARNING: this is executed from within each search thread
// It MUST be thread safe wrt to shared resources
bool CSearchEngine::SearchMbox(CMbox* mbox, unsigned long& found)
{
	bool result = false;
	CMbox* open_mbox = NULL;

	// Try to find window first
	try
	{
		// Resolve to open mailbox if one exists
		open_mbox = (mbox->IsOpen() ? mbox : mbox->GetProtocol()->FindOpenMbox(mbox->GetName()));

		// Grab an open lock on the mailbox, if not we'll have to open it from scratch
		if (open_mbox && !open_mbox->OpenIfOpen())
			open_mbox = NULL;

		// Now we get a global lock on the mailbox and prevent any other use
		// while searching
		// Grab the global lock on the mailbox
		CMbox::mbox_lock _lock(CMbox::_smutex, open_mbox ? open_mbox : mbox);

		// Now open the mailbox if no other open
		if (!open_mbox)
		{
			// Try to locate a suitable free protocol to reuse for this mailbox
			// Open the mailbox at the same time
			PopReusableProtocol(mbox);
			open_mbox = mbox;
		}

		// Search in existing open mailbox
		open_mbox->Search(&mSearchItem, NULL, false, true);
		found = open_mbox->GetSearchResults().size();

		// Do silent close
		PushReusableProtocol(open_mbox);

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		result = false;

		// Always do silent close to bump down ref count
		if (open_mbox)
			PushReusableProtocol(open_mbox);
	}

	return result;
}

// WARNING: this is executed from within each search thread
// It MUST be thread safe wrt to shared resources
// Get a reusable protocol for this mbox
void CSearchEngine::PopReusableProtocol(CMbox* mbox)
{
	// Open using EXAMINE to keep it read-only - i.e. no flag changes
	// Also open but do not use 'full open' state so that message lists are not created
	mbox->Open(NULL, false, true, false);
}

// WARNING: this is executed from within each search thread
// It MUST be thread safe wrt to shared resources
// Push the reusable protocol for this mbox
void CSearchEngine::PushReusableProtocol(CMbox* mbox)
{
	// Close the mailbox to actually release the protocol
	mbox->Close();
}
