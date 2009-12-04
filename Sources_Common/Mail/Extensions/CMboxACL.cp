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


// ACL support for mailboxes

#include "CMboxACL.h"


const char cMboxACLFlags[] = "lrswipcda";

// CMboxACL: contains specific ACL item for a mailbox

// Assignment with same type
CMboxACL& CMboxACL::operator=(const CMboxACL& copy)
{
	CACL::operator=(copy);

	return *this;
}

// Parse string to specified rigths location
void CMboxACL::ParseRights(const char* txt, SACLRight& rights)
{
	// Null existing rights
	rights.SetRight(SACLRight::eACL_AllRights, false);

	// Look for specific characters in string
	long pos = 0;
	const char* p = cMboxACLFlags;
	if (txt)
	{
		while(*p)
		{
			if (::strchr(txt, *p) != nil)
				rights.SetRight(1L << pos, true);

			p++;
			pos++;
		}
	}
}

// Get text form of rights
cdstring CMboxACL::GetTextRights() const
{
	cdstring rights;

	// Look for specific rights
	long pos = 0;
	const char* p = cMboxACLFlags;
	while(*p)
	{
		if (HasRight(1L << pos))
			rights += *p;

		p++;
		pos++;
	}

	// Must quote empty string
	rights.quote();

	return rights;
}
// Get full text form of rights
cdstring CMboxACL::GetFullTextRights() const
{
	cdstring txt = mUID + ' ';

	return txt + GetTextRights();
}
