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


// Mix-in class to broadcast changes

#include "CBroadcaster.h"

#include "CListener.h"

#include <algorithm>

CBroadcaster::~CBroadcaster()
{
	Broadcast_Message(eBroadcasterDied, this);

	// Cannot use iterator as list will change during removal
	for(CListenerList::iterator iter = mListeners.begin(); iter != mListeners.end(); iter++)
		(*iter)->Remove_Broadcaster(this);
}

void CBroadcaster::Add_Listener(CListener* listener)
{
	CListenerList::iterator found = std::find(mListeners.begin(), mListeners.end(), listener);
	if (found == mListeners.end())
	{
		mListeners.push_back(listener);
		listener->Add_Broadcaster(this);
	}
}

void CBroadcaster::Remove_Listener(CListener* listener)
{
	CListenerList::iterator found = std::find(mListeners.begin(), mListeners.end(), listener);
	if (found != mListeners.end())
	{
		mListeners.erase(found);
		listener->Remove_Broadcaster(this);
	}
}

void CBroadcaster::Broadcast_Message(long msg, void* param) const
{
	if (!mIsBroadcasting)
		return;

	// Warning: listener may remove itself during broadcast - cannot use iterators
	size_t old_size = mListeners.size();
	for(size_t i = 0; i < old_size; i++)
	{
		if (mListeners.at(i)->Is_Listening())
			mListeners.at(i)->ListenTo_Message(msg, param);

		// Look for change in size caused by listener removal
		if (mListeners.size() < old_size)
		{
			// Adjust index to cope with removal of listeners and reset loop termination condition
			i -= old_size - mListeners.size();
			old_size = mListeners.size();
		}
	}
}
