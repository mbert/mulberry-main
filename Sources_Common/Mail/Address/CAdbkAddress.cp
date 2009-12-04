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


// Source for Addressbook Address class

#include "CAdbkAddress.h"

#include "CRFC822.h"
#include "CURL.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy constructor
CAdbkAddress::CAdbkAddress(const CAddress &copy) :
	CAddress(copy)
{
	_init_CAdbkAddress();

	// Copy extra bits if its really a CAdbkAddress
	const CAdbkAddress* addr = dynamic_cast<const CAdbkAddress*>(&copy);
	if (addr)
	{
		_copy_CAdbkAddress(*addr);
	}
}

// Construct from actual address, name & adl
CAdbkAddress::CAdbkAddress(const char* entry,
					const char* uaddr,
					const char* uname,
					const char* uadl,
					const char* ucalendar,
					const char* ucompany,
					const char* uaddress,
					const char* uphonework,
					const char* uphonehome,
					const char* ufax,
					const char* uurl,
					const char* unotes) :
	CAddress(uaddr, uname, uadl)
{
	_init_CAdbkAddress();

	mEntry = entry;
	mCalendar = ucalendar;
	mCompany = ucompany;
	if (uaddress != NULL)
		mAddresses.insert(addrmap::value_type(eDefaultAddressType, uaddress));
	if (uphonehome != NULL)
		mPhones.insert(phonemap::value_type(eHomePhoneType, uphonehome));
	if (uphonework != NULL)
		mPhones.insert(phonemap::value_type(eWorkPhoneType, uphonework));
	if (ufax != NULL)
		mPhones.insert(phonemap::value_type(eFaxType, ufax));
	mURL = uurl;
	mNotes = unotes;
}

// Default destructor
CAdbkAddress::~CAdbkAddress()
{
}

void CAdbkAddress::_init_CAdbkAddress()
{
	mPreferredEmail = eDefaultEmailType;
	mPreferredAddress = eDefaultAddressType;
	mPreferredPhone = eDefaultPhoneType;
}

void CAdbkAddress::_copy_CAdbkAddress(const CAdbkAddress& copy)
{
	mEntry = copy.mEntry;
	mEmails = copy.mEmails;
	mPreferredEmail = copy.mPreferredEmail;
	mCalendar = copy.mCalendar;
	mCompany = copy.mCompany;
	mAddresses = copy.mAddresses;
	mPreferredAddress = copy.mPreferredAddress;
	mPhones = copy.mPhones;
	mPreferredPhone = copy.mPreferredPhone;
	mURL = copy.mURL;
	mNotes = copy.mNotes;
}

// Compare with another
int CAdbkAddress::operator==(const CAdbkAddress& addr) const
{
	// Just do inherited version
	return CAddress::operator==(addr);
}

void CAdbkAddress::ExpandMatch(EAddressMatch match, cdstring& str)
{
	cdstring matchit;

	switch(match)
	{
	case eMatchExactly:
		matchit = str;
		break;
	case eMatchAtStart:
		matchit = str;
		matchit += '*';
		break;
	case eMatchAtEnd:
		matchit = '*';
		matchit += str;
		break;
	case eMatchAnywhere:
	default:
		matchit = '*';
		matchit += str;
		matchit += '*';
		break;
	}
	
	str = matchit;
}

// Compare specific field
bool CAdbkAddress::Search(const cdstring& text, const CAddressFields& fields) const
{
	bool result = false;

	for(CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
	{
		switch(*iter)
		{
		case CAdbkAddress::eName:
			result = GetName().PatternMatch(text);
			break;
		case CAdbkAddress::eNickName:
			result = GetADL().PatternMatch(text);
			break;
		case CAdbkAddress::eEmail:
			result = GetMailAddress().PatternMatch(text);
			if (!result)
			{
				for(emailmap::const_iterator iter = GetEmails().begin(); !result && (iter != GetEmails().end()); iter++)
					result = (*iter).second.PatternMatch(text);
			}
			break;
		case CAdbkAddress::eCalendar:
			result = GetCalendar().PatternMatch(text);
			break;
		case CAdbkAddress::eCompany:
			result = GetCompany().PatternMatch(text);
			break;
		case CAdbkAddress::eAddress:
			for(addrmap::const_iterator iter = GetAddresses().begin(); !result && (iter != GetAddresses().end()); iter++)
				result = (*iter).second.PatternMatch(text);
			break;
		case CAdbkAddress::ePhoneWork:
			result = GetPhone(eWorkPhoneType).PatternMatch(text);
			break;
		case CAdbkAddress::ePhoneHome:
			result = GetPhone(eHomePhoneType).PatternMatch(text);
			break;
		case CAdbkAddress::eFax:
			result = GetPhone(eFaxType).PatternMatch(text) ||
						GetPhone(eHomeFaxType).PatternMatch(text) ||
						GetPhone(eWorkFaxType).PatternMatch(text);
			break;
		case CAdbkAddress::eURL:
			result = GetURL().PatternMatch(text);
			break;
		case CAdbkAddress::eNotes:
			result = GetNotes().PatternMatch(text);
			break;
		default:;
		}
		
		if (result)
			break;
	}

	return result;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Check whether it has some info
bool CAdbkAddress::IsEmpty() const
{
	// Don't include mEntry?
	return CAddress::IsEmpty() && mCalendar.empty() && mCompany.empty() && mAddresses.empty() &&  mPhones.empty() &&
			 mURL.empty() && mNotes.empty();
}

// Add email addresses to list
void CAdbkAddress::AddMailAddressToList(cdstrvect& list, bool full) const
{
	if (GetEmails().size() != 0)
	{
		for(emailmap::const_iterator iter = GetEmails().begin(); iter != GetEmails().end(); iter++)
		{
			cdstring email = (*iter).second;
			if (full)
			{
				// Add name if there and not same as email
				cdstring result;
				if (email != GetName())
				{
					result = GetName();
					CRFC822::HeaderQuote(result, true, false);
					if (!result.empty())
						result += ' ';

					// Form address from mailbox & host
					bool bracket = !result.empty();
					if (bracket)
						result += "<";
					result += email;
					if (bracket)
						result += ">";
				}
				else
					result.steal(email.grab_c_str());
				list.push_back(result);
			}
			else
				list.push_back(email);
		}
	}
	else
	{
		cdstring result;
		if (full)
			result = GetFullAddress();
		else
			result = GetMailAddress();
		list.push_back(result);
	}
}

// Add calendar addresses to list
void CAdbkAddress::AddCalendarAddressToList(cdstrvect& list, bool full) const
{
	cdstring result;
	if (!mCalendar.empty())
	{
		if (full)
		{
			if (!GetName().empty())
			{
				result += GetName();
				result += " ";
			}
		}
		result += "<";
		result += mCalendar;
		result += ">";
	}
	else
	{
		if (full)
		{
			result = GetName();
			if (!result.empty())
				result += " ";
		}

		result += "<";
		result += cMailtoURLScheme;
		result += GetMailAddress();
		result += ">";
	}
	
	list.push_back(result);
}

// Set email
void CAdbkAddress::SetEmail(const char* theEmail, EEmailType type, bool append)
{
	// Append - can be same type as existing one
	if (append)
	{
		mEmails.insert(emailmap::value_type(type, theEmail));
	
		// Set the email in the CAddress parent class if empty
		if (GetHost().empty() && GetMailbox().empty())
			CopyMailAddress(theEmail);
	}
	else
	{
		// Always set the email in the CAddress parent class if using default type
		if (type == eDefaultEmailType)
			CopyMailAddress(theEmail);

		// Look for existing and replace that
		emailmap::iterator found = mEmails.find(type);
		if (found != mEmails.end())
		{
			(*found).second = theEmail;
			return;
		}

		// Special case - map default to either home or work if either of those is present
		// This takes care of handling addresses with explicit home/work items as well
		// as those without
		else if (type == eDefaultEmailType)
		{
			// Look for home then work and use those instead
			if (mEmails.count(eHomeEmailType) != 0)
			{
				SetEmail(theEmail, eHomeEmailType);
				return;
			}
			else if (mEmails.count(eWorkEmailType) != 0)
			{
				SetEmail(theEmail, eWorkEmailType);
				return;
			}
			else if (mEmails.count(eOtherEmailType) != 0)
			{
				SetEmail(theEmail, eOtherEmailType);
				return;
			}
		}

		mEmails.insert(emailmap::value_type(type, theEmail));
	}
}

// Get email
const cdstring&	CAdbkAddress::GetEmail(EEmailType type) const
{
	// If map is empty, force the parent class into the map
	if (mEmails.empty())
		const_cast<CAdbkAddress*>(this)->mEmails.insert(emailmap::value_type(eDefaultEmailType, GetMailAddress()));

	emailmap::const_iterator found = mEmails.find(type);
	if (found != mEmails.end())
		return (*found).second;

	// Default type maps to home or work if default not present
	else if (type == eDefaultEmailType)
	{
		if (mEmails.count(eHomeEmailType) != 0)
			return GetEmail(eHomeEmailType);
		else if (mEmails.count(eWorkEmailType) != 0)
			return GetEmail(eWorkEmailType);
		else if (mEmails.count(eOtherEmailType) != 0)
			return GetEmail(eOtherEmailType);
	}

	return cdstring::null_str;
}

// Set address
void CAdbkAddress::SetAddress(const char* theAddress, EAddressType type, bool append)
{
	if (append)
		mAddresses.insert(addrmap::value_type(type, theAddress));
	else
	{
		// Replace first existing one
		addrmap::iterator found = mAddresses.find(type);
		if (found != mAddresses.end())
		{
			(*found).second = theAddress;
			return;
		}
		
		// Special case - map default to either home or work if either of those is present
		// This takes care of handling addresses with explicit home/work items as well
		// as those without
		else if (type == eDefaultAddressType)
		{
			// Look for home then work and use those instead
			if (mAddresses.count(eHomeAddressType) != 0)
			{
				SetAddress(theAddress, eHomeAddressType);
				return;
			}
			else if (mAddresses.count(eWorkAddressType) != 0)
			{
				SetAddress(theAddress, eWorkAddressType);
				return;
			}
		}

		mAddresses.insert(addrmap::value_type(type, theAddress));
	}
}

// Get address
const cdstring&	CAdbkAddress::GetAddress(EAddressType type) const
{
	addrmap::const_iterator found = mAddresses.find(type);
	if (found != mAddresses.end())
		return (*found).second;
	
	// Default type maps to home or work if default not present
	else if (type == eDefaultAddressType)
	{
		if (mAddresses.count(eHomeAddressType) != 0)
			return GetAddress(eHomeAddressType);
		else if (mAddresses.count(eWorkAddressType) != 0)
			return GetAddress(eWorkAddressType);
	}

	return cdstring::null_str;
}

// Set phone
void CAdbkAddress::SetPhone(const char* thePhone, EPhoneType type, bool append)
{
	// Append - can be multivalued
	if (append)
		mPhones.insert(phonemap::value_type(type, thePhone));
	else
	{
		// Replace first existing one
		phonemap::iterator found = mPhones.find(type);
		if (found != mPhones.end())
			(*found).second = thePhone;
		else
			mPhones.insert(phonemap::value_type(type, thePhone));
	}
}

// Get phone
const cdstring&	CAdbkAddress::GetPhone(EPhoneType type) const
{
	phonemap::const_iterator found = mPhones.find(type);
	if (found != mPhones.end())
		return (*found).second;

	// Default fax type maps to home or work if default not present
	else if (type == eFaxType)
	{
		if (mPhones.count(eHomeFaxType) != 0)
			return GetPhone(eHomeFaxType);
		else if (mPhones.count(eWorkFaxType) != 0)
			return GetPhone(eWorkFaxType);
	}

	return cdstring::null_str;
}
