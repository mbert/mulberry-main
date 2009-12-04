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


// Header for CSearchWindow class

#ifndef __CSEARCHENGINE__MULBERRY__
#define __CSEARCHENGINE__MULBERRY__

#include "cdthread.h"
#include "cdcond.h"

#include "CMboxRefList.h"
#include "CSearchItem.h"
#include "CSearchThread.h"
#include "CTreeNode.h"

#include "templs.h"

#include <map>

// Classes

class CMailboxView;
class CMbox;
class CMboxRef;
class CMboxProtocol;

typedef std::multimap<CMboxProtocol*, CMboxProtocol*> CMboxProtocolMap;

class CSearchEngine : public cdthread
{
	friend class CSearchThread;

public:
	enum EMboxSearchState
	{
		eNoSearch = 0,
		eSearchHit,
		eSearchMiss
	};

	static CSearchEngine	sSearchEngine;

	CSearchEngine();
	virtual ~CSearchEngine();

	void Abort();												// Cancel searching
	bool AbortCompleted() const									// Canceled searching
		{ return mAbortCompleted; }

	void SetSearchItem(const CSearchItem* spec)
		{ mSearchItem = *spec; }
	const CSearchItem* GetSearchItem() const
		{ return &mSearchItem; }
	
	const CMboxRefList& GetTargets() const
		{ return mTargets;}
	const ulvector& GetTargetHits() const
		{ return mTargetHits; }

	void ClearTargets()
		{ mTargets.DeleteAll(); mTargetHits.clear(); }
	bool TargetsAddMbox(const CMbox* mbox);
	long TargetSize() const
		{ return mTargetHits.size(); }
	const cdstring& GetTargetName(unsigned long i) const
		{ return mTargets.at(i)->GetName(); }
	long GetTargetState(unsigned long i) const
		{ return mTargetHits.at(i); }

	void SetCurrentTarget(long target)
		{ mCurrentTarget = target; }
	long GetCurrentTarget() const
		{ return mCurrentTarget; }
	const CMboxRef* GetCurrentTargetMbox() const;
	long NextTarget();

	void OpenTarget(long target, CMailboxView* recycle = NULL);

	void StartSearch();

protected:
	virtual void Execute();										// Must override - pure virtual in base class

private:
	cdmutex				_search_lock;							// Lock for access to search engine data from threads
	cdcond				mThreadDone;							// Event signalled by each thread when it is finished
	CSearchItem			mSearchItem;							// Current search criteria
	CMboxRefList		mTargets;								// List of mbox refs to search
	ulvector			mTargetHits;							// Search status of mbox refs
	unsigned long		mNextSearchIndex;						// Next mailbox to be searched
	long				mCurrentTarget;							// Current search target for display
	cdthreadvect		mSearchThreads;							// List of search threads
	long				mDone;									// Number of mailboxes searched
	long				mFound;									// Number of mailboxes found
	long				mMessages;								// Number of messages found
	bool				mAbort;									// Abort requested
	bool				mAbortCompleted;						// Abort completed

	bool SearchNextMbox(CSearchThread* thread);					// Search next mbox - called from search thread
	bool SearchMbox(CMbox* mbox, unsigned long& found);			// Search one mbox again - called from search thread
	void PopReusableProtocol(CMbox* mbox);						// Get a reusable protocol for this mbox and open it
	void PushReusableProtocol(CMbox* mbox);						// Push the reusable protocol for this mbox and close it

	void ThreadDone()											// Signal thread done event
		{ mThreadDone.signal(); }
};

#endif
