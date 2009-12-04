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


// Header for CCommanderProtect class

// This class is used to prevent OnCmdMsg re-entrancy by tracking
// which commands are currently being handled. It should be declared as a member of
// a class that needs OnCmdMsg protection.

#ifndef __CCOMMANDERPROTECT__MULBERRY__
#define __CCOMMANDERPROTECT__MULBERRY__

#include "CBroadcaster.h"
#include "CListener.h"

const unsigned long cMaxCmds = 10;

class CCommanderProtect : public CBroadcaster
{
	enum
	{
		eBroadcast_Deleted = 'dele'
	};

public:
	class StCommandProtect : public CListener
	{
	public:
		StCommandProtect(CCommanderProtect& protector, UINT nID, int nCode) :
			mProtector(protector)
			{ protector.Add_Listener(this); mAlreadyProcessing = mProtector.AddCmdMsg(nID, nCode); mID = nID; mCode = nCode; mDeleted = false; }
		virtual ~StCommandProtect()
			{ if (!mDeleted && !mAlreadyProcessing) mProtector.RemoveCmdMsg(mID, mCode); }
		
		bool AlreadyProcessing() const
			{ return mAlreadyProcessing; }

		virtual void ListenTo_Message(long msg, void* param)
			{ mDeleted = true; };			// There is only one broadcast we can receive and it means the object we hold has been deleted

	private:
		CCommanderProtect&	mProtector;
		bool				mAlreadyProcessing;
		UINT				mID;
		int					mCode;
		bool				mDeleted;
	};

	CCommanderProtect();
	virtual ~CCommanderProtect()
		{ Broadcast_Message(eBroadcast_Deleted); }

	bool AddCmdMsg(UINT nID, int nCode);
	void RemoveCmdMsg(UINT nID, int nCode);

protected:
	struct SCmdMsg
	{
		UINT	mID;
		int		mCode;
	};
	SCmdMsg			mLocked[cMaxCmds];
	unsigned long	mNextIndex;
	
	unsigned long GetIndex(UINT nID, int nCode) const;
};

#endif