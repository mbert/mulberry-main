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


// Source for CCommanderProtect class

#include "CCommanderProtect.h"

CCommanderProtect::CCommanderProtect()
{
	// Null out array
	for(int i = 0; i < cMaxCmds; i++)
	{
		mLocked[i].mID = 0;
		mLocked[i].mCode = 0;
	}
	
	mNextIndex = 0;
}

// Try to add command to stack - return true if already in stack
bool CCommanderProtect::AddCmdMsg(UINT nID, int nCode)
{
	// Scan stack for matching item
	if (GetIndex(nID, nCode) < cMaxCmds)
		return true;
	
	// Add new item
	if (mNextIndex < cMaxCmds)
	{
		mLocked[mNextIndex].mID = nID;
		mLocked[mNextIndex].mCode = nCode;
		mNextIndex++;

		return false;
	}
	else
		// IMPORTANT - we can get here if we have nested (different) commands.
		// We limit the amount f nesting via the max. size of the array
		return true;
}

void CCommanderProtect::RemoveCmdMsg(UINT nID, int nCode)
{
	// Scan stack for matching item
	unsigned long index = GetIndex(nID, nCode);

	// Ignore if not found - this ought to be an error though
	if (index == cMaxCmds)
		return;
	
	// Pop it off
	if (index == mNextIndex - 1)
	{
		mNextIndex--;
		mLocked[mNextIndex].mID = 0;
		mLocked[mNextIndex].mCode = 0;
	}
}

// Scan stack for matching item
unsigned long CCommanderProtect::GetIndex(UINT nID, int nCode) const
{
	for(int i = 0; i < mNextIndex; i++)
	{
		if ((mLocked[i].mID) == nID && (mLocked[i].mCode == nCode))
			return i;
	}
	
	return cMaxCmds;
}