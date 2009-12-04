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


// Source for CFilterSchedule class

#include "CFilterSchedule.h"
#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

CFilterSchedule::CFilterSchedule()
{
	mType = eNone;
}

void CFilterSchedule::_copy(const CFilterSchedule& copy)
{
	mType = copy.mType;
}

void CFilterSchedule::_tidy()
{
}

const char* cDescriptors[] =
	{"None",
	 "Open Mailbox",
	 "Close Mailbox",
	 "Open Message",
	 "Close Message",
	 "Delete Message",
	 "Reply Message",
	 "Forward Message",
	 "Bounce Message"};

// Get text expansion for prefs
cdstring CFilterSchedule::GetInfo(void) const
{
	cdstring info;
	cdstring temp = cDescriptors[mType];
	temp.quote();
	info += temp;
	
	return info;
}

// Convert text to items
void CFilterSchedule::SetInfo(char_stream& txt)
{
	char* p = txt.get();
	mType = eNone;
	if (p)
	{
		for(int i = eNone; i < eScheduleLast; i++)
		{
			if (!::strcmp(p, cDescriptors[i]))
			{
				mType = static_cast<ESchedule>(i);
				break;
			}
		}
	}
}