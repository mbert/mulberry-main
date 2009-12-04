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

#ifndef __CSEARCHTHREAD__MULBERRY__
#define __CSEARCHTHREAD__MULBERRY__

#include "cdthread.h"

class CMbox;

class CSearchThread: public cdthread
{
public:
	// Always use statics
	CSearchThread();
	virtual ~CSearchThread();

	void Start()
		{ StartExecution(); }

	bool GetDone() const
		{ return mDone; }

	CMbox* GetMbox() const
		{ return mMbox; }
	void SetMbox(CMbox* mbox)
		{ mMbox = mbox; }
protected:
	bool	mDone;
	CMbox*	mMbox;

	virtual void Execute();			// Must override - this is pure virtual in base
};

#endif
