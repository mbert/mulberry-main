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

#ifndef __CBROADCASTER__MULBERRY__
#define __CBROADCASTER__MULBERRY__

#include <cstddef>
#include <vector>
#include <stddef.h>

class CListener;
typedef std::vector<CListener*> CListenerList;

class CBroadcaster
{
public:
	enum
	{
		eBroadcasterDied = 'died'
	};

	CBroadcaster() { mIsBroadcasting = true; }
	virtual ~CBroadcaster();

	void Add_Listener(CListener* listener);
	void Remove_Listener(CListener* listener);

	void Start_Broadcasting()
		{ mIsBroadcasting = true; }
	void Stop_Broadcasting()
		{ mIsBroadcasting = false; }
	bool Is_Broadcasting() const
		{ return mIsBroadcasting; }

	void Broadcast_Message(long msg, void* param = NULL) const;

protected:
	CListenerList mListeners;
	bool mIsBroadcasting;
};

#endif
