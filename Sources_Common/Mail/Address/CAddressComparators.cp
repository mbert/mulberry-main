/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CAddressComparators class

#include "CAddressComparators.h"

#include "CAddress.h"
#include "CAdbkAddress.h"
#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressComparator::CAddressComparator()
{
} // CAddressComparator::CAddressComparator

// Default destructor
CAddressComparator::~CAddressComparator()
{
} // CAddressComparator::~CAddressComparator

long CAddressComparator::FirstNameFN(const CAddress* addr1, const CAddress* addr2)
{
	// Get names
	const cdstring& name1 = addr1->GetName();
	const cdstring& name2 = addr2->GetName();
	bool length1 = !name1.empty();
	bool length2 = !name2.empty();

	// Handle empty strings
	if (!length1 && !length2)
		return 0;
	else if (!length1)
		return 1;
	else if (!length2)
		return -1;
	else
		// Do compare
		return ::strcmpnocase(name1, name2);
}

const char* CAddressComparator::ExtractLastName(const char* name)
{
	// Any comma => first name sort
	if (::strchr(name, ',') != NULL)
		return name;

	// Get pointer to last name for compare
	const char* last = name + ::strlen(name);
	
	// Keep going until we have a valid end word or we run out of characters
	while(true)
	{
		// Go backwards to start of last word
		while((--last > name) && (isspace((unsigned char)*last) || ispunct((unsigned char)*last))) {}
		if (last == name)
			return last;

		// Check for period at the end of the word
		bool got_period_end = (*(last+1) == '.');

		// Now look for start of word
		while((--last > name) && !isspace((unsigned char)*last) && (*last != '.')) {}
		if (last == name)
			return last;

		switch(*last)
		{
		case '.':
			// Return what is after
			return last + 1;
		case ' ':
		case '\t':
			// If word ends in period, keep going backwards to previous word
			if (got_period_end)
				break;
			// If word starts with a bracket, keep going backwards to previous word
			else if ((*(last+1) == '(') || (*(last+1) == '[') || (*(last+1) == '{'))
				break;
			else
				// Return what is after
				return last + 1;
		}
	}
}

long CAddressComparator::LastNameFN(const CAddress* addr1, const CAddress* addr2)
{
	// Get names
	cdstring name1 = addr1->GetName();
	name1.trimspace();
	cdstring name2 = addr2->GetName();
	name2.trimspace();
	bool length1 = !name1.empty();
	bool length2 = !name2.empty();

	// Handle zero length case
	if (!length1 && !length2)
		return 0;
	else if (!length1)
		return 1;
	else if (!length2)
		return -1;

	// Check for email addresses
	bool is_email1 = (addr1->GetName().compare(addr1->GetMailAddress(), true) == 0);
	bool is_email2 = (addr2->GetName().compare(addr2->GetMailAddress(), true) == 0);
	
	// Handle email items
	if (is_email1 ^ is_email2)
		return is_email1 ? 1 : -1;
	else if (is_email1 && is_email2)
		return ::strcmpnocase(name1, name2);

	// Get pointer to last name for compare
	const char* last1 = ExtractLastName(name1.c_str());
	const char* last2 = ExtractLastName(name2.c_str());

	// Do compare
	int result = ::strcmpnocase(last1, last2);

	// Return if not equal or no more to compare
	if ((result != 0) || ((last1 == (const char*) name1) && (last2 == (const char*) name2)))
		return result;

	// Make sure first name exists
	if ((last1 == (const char*) name1) && (last2 != (const char*) name2))
		return 1;
	if ((last1 != (const char*) name1) && (last2 == (const char*) name2))
		return -1;

	// Tie off
	*((char*) --last1) = '\0';
	*((char*) --last2) = '\0';

	// Do remaining compare
	result = ::strcmpnocase(name1, name2);

	// Untie
	*(char*) last1 = ' ';
	*(char*) last2 = ' ';

	// Return result
	return result;
}

long CAddressComparator::EmailFN(const CAddress* addr1, const CAddress* addr2)
{
	// Get email addresses
	cdstring eaddr1 = addr1->GetMailAddress();
	cdstring eaddr2 = addr2->GetMailAddress();
	bool length1 = !eaddr1.empty();
	bool length2 = !eaddr2.empty();

	// Handle zero length case
	if (!length1 && !length2)
		return 0;
	else if (!length1)
		return 1;
	else if (!length2)
		return -1;

	// Do compare and return
	return ::strcmpnocase(eaddr1, eaddr2);
}

long CAddressComparator::EmailDomainFN(const CAddress* addr1, const CAddress* addr2)
{
	// Get email addresses
	cdstring eaddr1 = addr1->GetMailAddress();
	cdstring eaddr2 = addr2->GetMailAddress();

	// Get domains
	bool got_at1 = false;
	bool got_at2 = false;
	char* domain1 = ::strrchr(eaddr1.c_str_mod(), '@');
	char* domain2 = ::strrchr(eaddr2.c_str_mod(), '@');
	if (!domain1)
		domain1 = eaddr1;
	else
	{
		// Tie off
		*domain1 = '\0';
		domain1++;
		got_at1 = true;
	}
	if (!domain2)
		domain2 = eaddr2;
	else
	{
		// Tie off
		*domain2 = '\0';
		domain2++;
		got_at2 = true;
	}

	// Loop over domains
	while(true)
	{

		unsigned long length1 = ::strlen(domain1);
		unsigned long length2 = ::strlen(domain2);

		// Handle zero length case
		if (!length1 && !length2)
			return 0;
		else if (!length1)
			return 1;
		else if (!length2)
			return -1;

		// Get last remaining domain
		char* last_domain1 = ::strrchr(domain1, '.');
		char* last_domain2 = ::strrchr(domain2, '.');
		bool end_domain1 = false;
		bool end_domain2 = false;

		// Was domain found
		if (!last_domain1) {
			last_domain1 = domain1;
			end_domain1 = true;
		}
		else
			last_domain1++;

		// Was domain found
		if (!last_domain2) {
			last_domain2 = domain2;
			end_domain2 = true;
		}
		else
			last_domain2++;

		// Check end status - return comparison if one ended
		if (end_domain1 ^ end_domain2) {
			int result = ::strcmpnocase(last_domain1, last_domain2);

			if (result) return result;

			if (end_domain1)
				return -1;
			else
				return 1;
		}

		// Compare domains
		int result = ::strcmpnocase(last_domain1, last_domain2);

		// If not matched return
		if (result) return result;

		// Tie off
		if (last_domain1 > domain1)
			--last_domain1;
		*last_domain1 = '\0';
		if (last_domain2 > domain2)
			--last_domain2;
		*last_domain2 = '\0';

		// End loop if both ended
		if (end_domain1 && end_domain2) break;
	}

	// Domains are equal if we get here - compare uids

	// Check that they have uids
	if (got_at1 && !got_at2)
		return 1;
	if (!got_at1 && got_at2)
		return -1;
	if (!got_at1 && !got_at2)
		return 0;

	// Do compare and return
	return ::strcmpnocase(eaddr1, eaddr2);
}

long CAddressComparator::NickNameFN(const CAddress* addr1, const CAddress* addr2)
{
	// Get names
	const cdstring& nick_name1 = addr1->GetADL();
	const cdstring& nick_name2 = addr2->GetADL();
	bool length1 = !nick_name1.empty();
	bool length2 = !nick_name2.empty();

	// Handle empty strings
	if (!length1 && !length2)
		return 0;
	else if (!length1)
		return 1;
	else if (!length2)
		return -1;
	else
		// Do compare
		return ::strcmpnocase(nick_name1, nick_name2);
}

long CAddressComparator::CompanyFN(const CAddress* addr1, const CAddress* addr2)
{
	// Check that these are adbk address objects
	const CAdbkAddress* adbkaddr1 = dynamic_cast<const CAdbkAddress*>(addr1);
	const CAdbkAddress* adbkaddr2 = dynamic_cast<const CAdbkAddress*>(addr2);

	if (!adbkaddr1 || !adbkaddr2)
	{
		if (adbkaddr1)
			return -1;
		else if (adbkaddr2)
			return 1;
		else
			return 0;
	}
	// Get names
	const cdstring& company1 = adbkaddr1->GetCompany();
	const cdstring& company2 = adbkaddr2->GetCompany();
	bool length1 = !company1.empty();
	bool length2 = !company2.empty();

	// Handle empty strings
	if (!length1 && !length2)
		return 0;
	else if (!length1)
		return 1;
	else if (!length2)
		return -1;
	else
		// Do compare
		return ::strcmpnocase(company1, company2);
}

bool CAddressComparator::CompareFirstName(const CAddress* addr1, const CAddress* addr2)
{

	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are the same - check email
	result = EmailFN(addr1, addr2);

	if (result)
		// Emails are different
		return (result < 0);

	// Emails are also equal - check nickname finally
	return (NickNameFN(addr1, addr2) < 0);
}

bool CAddressComparator::CompareLastName(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = LastNameFN(addr1, addr2);

	if (result)
		// Last names are different
		return (result < 0);

	// Last names are the same - check full name
	result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are the same - check email
	result = EmailFN(addr1, addr2);

	if (result)
		// Emails are different
		return (result < 0);

	// Emails are also equal - check nickname finally
	return (NickNameFN(addr1, addr2) < 0);
}

bool CAddressComparator::CompareEmail(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = EmailFN(addr1, addr2);

	if (result)
		// Emails are different
		return (result < 0);

	// Emails are the same - check name
	result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are also equal - check nickname finally
	return (NickNameFN(addr1, addr2) < 0);
}

bool CAddressComparator::CompareEmailOnly(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return true;

	// Check email
	return !EmailFN(addr1, addr2);
}

bool CAddressComparator::CompareEmailDomain(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = EmailDomainFN(addr1, addr2);

	if (result)
		// Email domains are different
		return (result < 0);

	// Email domains are the same - check full email
	result = EmailFN(addr1, addr2);

	if (result)
		// Emails are different
		return (result < 0);

	// Emails are the same - check name
	result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are also equal - check nickname finally
	return (NickNameFN(addr1, addr2) < 0);
}

bool CAddressComparator::CompareNickName(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = NickNameFN(addr1, addr2);

	if (result)
		// Nick names are different
		return (result < 0);

	// Nick names are the same - check name
	result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are the same - check email finally
	return (EmailFN(addr1, addr2) < 0);
}

bool CAddressComparator::CompareCompany(const CAddress* addr1, const CAddress* addr2)
{
	// Compare ptrs first
	if (addr1 == addr2)
		return false;

	long result = CompanyFN(addr1, addr2);

	if (result)
		// Companys are different
		return (result < 0);

	// Companys are the same - check name
	result = FirstNameFN(addr1, addr2);

	if (result)
		// Names are different
		return (result < 0);

	// Names are the same - check email
	result = EmailFN(addr1, addr2);

	if (result)
		// Emails are different
		return (result < 0);

	// Emails are also equal - check nickname finally
	return (NickNameFN(addr1, addr2) < 0);
}
