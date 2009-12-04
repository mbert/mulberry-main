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


// CSearchThread : class to do threaded message searching

#include "CSearchThread.h"

#include "CLog.h"
#include "CSearchEngine.h"

CSearchThread::CSearchThread()
{
	mDone = false;
	mMbox = NULL;
}

CSearchThread::~CSearchThread()
{
	mMbox = NULL;
}

// NB This will be called from within the thread itself
void CSearchThread::Execute()
{
	try
	{
		// Loop doing searches until no more
		while(CSearchEngine::sSearchEngine.SearchNextMbox(this)) {}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform engine we are done
		mDone = true;
		CSearchEngine::sSearchEngine.ThreadDone();

		CLOG_LOGRETHROW;
		throw;
	}

	// Inform engine we are done
	mDone = true;
	CSearchEngine::sSearchEngine.ThreadDone();
}
