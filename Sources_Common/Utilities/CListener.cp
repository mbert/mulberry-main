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


// Class to provide notification of changes

#include "CListener.h"

#include "CBroadcaster.h"
#include <algorithm>

CListener::~CListener()
{
	// Cannot use iterator as list will change during removal
	while(mBroadcasters.size())
		mBroadcasters.front()->Remove_Listener(this);
}

void CListener::Add_Broadcaster(CBroadcaster* broadcaster)
{
	CBroadcastList::iterator found = std::find(mBroadcasters.begin(), mBroadcasters.end(), broadcaster);
	if (found == mBroadcasters.end())
		mBroadcasters.push_back(broadcaster);
}

void CListener::Remove_Broadcaster(CBroadcaster* broadcaster)
{
	CBroadcastList::iterator found = std::find(mBroadcasters.begin(), mBroadcasters.end(), broadcaster);
	if (found != mBroadcasters.end())
		mBroadcasters.erase(found);
}
