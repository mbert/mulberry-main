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


// Class to provide notification of hierarchic list changes

#ifndef __CLISTENER__MULBERRY__
#define __CLISTENER__MULBERRY__

#include <vector>

class CBroadcaster;
typedef std::vector<CBroadcaster*> CBroadcastList;

class CListener
{
	friend class CBroadcaster;

public:
	CListener() { mIsListening = true; }
	virtual ~CListener();

	void Start_Listening()
		{ mIsListening = true; }				
	void Stop_Listening()	
		{ mIsListening = false; }					
	bool Is_Listening() const
		{ return mIsListening; }

	virtual void ListenTo_Message(long msg, void* param) {};

private:
	CBroadcastList mBroadcasters;
	bool mIsListening;

	void Add_Broadcaster(CBroadcaster* list);
	void Remove_Broadcaster(CBroadcaster* list);
};

#endif
