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


// Header for CFilterSchedule class

#ifndef __CFILTERSCHEDULE_MULBERRY__
#define __CFILTERSCHEDULE_MULBERRY__

#include "cdstring.h"

// Classes
class char_stream;

class CFilterSchedule
{
public:
	enum ESchedule
	{
		eNone = 0,

		// Mailbox schedules
		eOpenMailbox,
		eCloseMailbox,
		
		// Message schedules
		eOpenMessage,
		eCloseMessage,
		eDeleteMessage,
		eReplyMessage,
		eForwardMessage,
		eBounceMessage,
		
		eScheduleLast
	};

	CFilterSchedule();
	CFilterSchedule(const CFilterSchedule& copy)
		{ _copy(copy); }
	~CFilterSchedule()
		{ _tidy(); }

	CFilterSchedule& operator=(const CFilterSchedule& copy)					// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	cdstring GetInfo(void) const;											// Get text expansion for prefs
	void SetInfo(char_stream& info);										// Convert text to items

protected:
	ESchedule	mType;

private:
	void _copy(const CFilterSchedule& copy);
	void _tidy();
};

#endif
