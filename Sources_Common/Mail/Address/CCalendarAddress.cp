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


// Source for Calendar Address class

#include "CCalendarAddress.h"

#include "CStringUtils.h"

#include <string.h>

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S
// __________________________________________________________________________________________________

void CCalendarAddress::FromIdentityText(const cdstring& txt, CCalendarAddressList& list)
{
	cdstrvect addrs;
	txt.split("\r\n", addrs);
	for (cdstrvect::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
	{
		list.push_back(new CCalendarAddress(*iter));
	}
}

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCalendarAddress::CCalendarAddress()
{
} // CCalendarAddress::CCalendarAddress

// Copy constructor
CCalendarAddress::CCalendarAddress(const CCalendarAddress &copy)
{
	// Copy those that are not NULL
	mName = copy.mName;
	mCalendar = copy.mCalendar;

} // CCalendarAddress::CCalendarAddress

// Construct from text
CCalendarAddress::CCalendarAddress(const char* txt)
{
	// Parse text to extract address
	ParseAddress(txt);

} // CCalendarAddress::CCalendarAddress

// Construct from actual address, name & adl
CCalendarAddress::CCalendarAddress(const char* uaddr,
								   const char* uname)
{
	// Copy those that are not NULL
	mName = uname;
	mCalendar = uaddr;

} // CCalendarAddress::CCalendarAddress

// Assignment with same type
CCalendarAddress& CCalendarAddress::operator=(const CCalendarAddress& copy)
{
	// Do not assign this to this
	if (this != &copy)
	{
		// Copy those that are not NULL
		mName = copy.mName;
		mCalendar = copy.mCalendar;
	}

	return *this;
}

// Default destructor
CCalendarAddress::~CCalendarAddress()
{
} // CCalendarAddress::~CCalendarAddress

// Compare with address
int CCalendarAddress::operator==(const char* addr) const
{
	cdstring compare = GetCalendarAddress();
	return (::strcmpnocase(compare, addr) == 0);
}

// Compare with another
int CCalendarAddress::operator==(const CCalendarAddress& addr) const
{
	cdstring compare1;
	cdstring compare2;

	// Policy:
	//
	// 1 If both calendar addresses are not empty just compare calendar addresses
	// 2 If one calendar address is empty => not equal
	// 3 If both names are not empty just compare names

	// Compare email addresses first
	compare1 = GetCalendarAddress();
	compare2 = addr.GetCalendarAddress();
	if (!compare1.empty() && !compare2.empty())
		return (::strcmpnocase(compare1, compare2) == 0);
	else if (!compare1.empty() || !compare2.empty())
		return 0;

	// Compare names
	if (!mName.empty() && !addr.mName.empty())
		return (::strcmp(mName, addr.mName) == 0);
	else if (!mName.empty() || !addr.mName.empty())
		return 0;

	// Completely empty so match
	return 1;

}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Parse address from text
void CCalendarAddress::ParseAddress(const char* txt)
{
	// Format we parse is:
	//
	// *word "<" uri ">"
	//
	// or
	//
	// uri
	//

	// Only if something present
	if (!txt || !*txt)
		return;
	
	cdstring temp(txt);
	temp.trimspace();
	if (temp.compare_end(">"))
	{
		cdstring::size_type pos = temp.rfind('<');
		if (pos != cdstring::npos)
		{
			if (pos > 0)
			{
				mName.assign(temp, 0, pos - 1);
				mName.trimspace();
			}
			else
				mName = cdstring::null_str;
			mCalendar.assign(temp, pos + 1, temp.length() - pos - 2);
			return;
		}
	}
	
	mName = temp;
	mCalendar = cdstring::null_str;
	
	return;
}

// Get sensible name from address
cdstring CCalendarAddress::GetNamedAddress() const
{
	// Use name if there
	if (mName.empty())
		// Form address from url
		return GetCalendarAddress();
	else
		return mName;
}

// Get sensible calendar address
cdstring CCalendarAddress::GetCalendarAddress() const
{
	// Form address from url
	cdstring copy = mCalendar;
	return copy;
}

// Get sensible full address
cdstring CCalendarAddress::GetFullAddress(bool encode) const
{
	// Get email address
	cdstring result;
	cdstring calendar = mCalendar;
	
	// Add name if there and not same as email
	if (calendar != mName)
	{
		result = mName;
		if (!result.empty())
			result += ' ';

		// Form address from mailbox & host
		bool bracket = !result.empty();
		if (bracket)
			result += "<";
		result += calendar;
		if (bracket)
			result += ">";
	}
	else
		result.steal(calendar.grab_c_str());

	return result;
}

// Set calendar
void CCalendarAddress::SetAddress(const char* calendar)
{
	// Copy as is
	mCalendar = calendar;
	mCalendar.trimspace();
}

// Check whether it has some info
bool CCalendarAddress::IsEmpty() const
{
	return mName.empty() && mCalendar.empty();
}

// Check whether it is valid
bool CCalendarAddress::IsValid() const
{
	return !mCalendar.empty();
}
