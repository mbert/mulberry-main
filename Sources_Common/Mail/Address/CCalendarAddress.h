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


// Header for Calendar Address class

#ifndef __CCALENDARADDRESS__MULBERRY__
#define __CCALENDARADDRESS__MULBERRY__

#include "cdstring.h"
#include "ptrvector.h"

class CCalendarAddress;
typedef ptrvector<CCalendarAddress> CCalendarAddressList;

class CCalendarAddress
{
public:
	
	static void FromIdentityText(const cdstring& txt, CCalendarAddressList& list);

	CCalendarAddress();
	CCalendarAddress(const CCalendarAddress &copy);				// Copy constructor

	explicit CCalendarAddress(const char* txt);					// Construct from text
	explicit CCalendarAddress(const char* addr,
				const char* uname);						// Construct from actual parameters

	CCalendarAddress& operator=(const CCalendarAddress& copy);	// Assignment with same type
	virtual ~CCalendarAddress();

	int operator==(const char* addr) const;							// Compare with named addr
	int operator==(const CCalendarAddress& addr) const;				// Compare with another

	void	ParseAddress(const char* txt);							// Parse address from text

	void SetName(const char* name)								// Set name
		{ mName = name; }
	cdstring& GetName()											// Get name
		{ return mName; }
	const cdstring&	GetName() const								// Get name
		{ return mName; }

	void SetAddress(const char* addr);							// Set address
	const cdstring&	GetAddress() const							// Get address
		{ return mCalendar; }

	cdstring GetNamedAddress() const;							// Get sensible name from address
	cdstring GetCalendarAddress() const;						// Get sensible calendar address
	cdstring GetFullAddress(bool encode = false) const;			// Get sensible full address

	virtual bool IsEmpty() const;								// Check whether it has some info

			bool IsValid() const;								// Validate

private:
	cdstring			mName;						// Personal name
	cdstring			mCalendar;					// Calendar name

};

#endif
