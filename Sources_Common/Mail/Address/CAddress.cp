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


// Source for Address class

#include "CAddress.h"

#include "CRFC822.h"
#include "CStringUtils.h"

#include <string.h>

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddress::CAddress()
{
} // CAddress::CAddress

// Copy constructor
CAddress::CAddress(const CAddress &copy)
{
	// Copy those that are not NULL
	mName = copy.mName;
	mAdl = copy.mAdl;
	mMailbox = copy.mMailbox;
	mHost = copy.mHost;

} // CAddress::CAddress

// Construct from text
CAddress::CAddress(const char* txt)
{
	// Parse text to extract address
	ParseAddress(txt);

} // CAddress::CAddress

// Construct from actual address, name & adl
CAddress::CAddress(const char* uaddr,
					const char* uname,
					const char* uadl)
{
	// Copy those that are not NULL
	mName = uname;
	mAdl = uadl;
	CopyMailAddress(uaddr);

} // CAddress::CAddress

// Assignment with same type
CAddress& CAddress::operator=(const CAddress& copy)
{
	// Do not assign this to this
	if (this != &copy)
	{
		// Copy those that are not NULL
		mName = copy.mName;
		mAdl = copy.mAdl;
		mMailbox = copy.mMailbox;
		mHost = copy.mHost;
	}

	return *this;
}

// Default destructor
CAddress::~CAddress()
{
} // CAddress::~CAddress

// Compare with address
int CAddress::operator==(const char* addr) const
{
	cdstring compare = GetMailAddress();
	return (::strcmpnocase(compare, addr) == 0);
}

// Compare with another
int CAddress::operator==(const CAddress& addr) const
{
	cdstring compare1;
	cdstring compare2;

	// Policy:
	//
	// 1 If both email addresses are not empty just compare email addresses
	// 2 If one email address is empty => not equal
	// 3 If both names are not empty just compare names

	// Compare email addresses first
	compare1 = GetMailAddress();
	compare2 = addr.GetMailAddress();
	if (!compare1.empty() && !compare2.empty())
		return (::strcmpnocase(compare1, compare2) == 0);
	else if (!compare1.empty() || !compare2.empty())
		return 0;

	// Compare names
	if (!mName.empty() && !addr.mName.empty())
		return (::strcmp(mName, addr.mName) == 0);
	else if (!mName.empty() || !addr.mName.empty())
		return 0;

	// Compare nicknames
	if (!mAdl.empty() && !addr.mAdl.empty())
		return (::strcmp(mAdl, addr.mAdl) == 0);
	else if (!mAdl.empty() || !addr.mAdl.empty())
		return 0;

	// Completely empty so match
	return 1;

}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Compare (case-sensitive local part) email addresses
bool CAddress::StrictCompareEmail(const CAddress& addr) const
{
	return (::strcmp(GetMailbox(), addr.GetMailbox()) == 0) &&
			(::strcmpnocase(GetHost(), addr.GetHost()) == 0);
}

// Parse address from text
void CAddress::ParseAddress(const char* txt)
{
	// Only if something present
	if (!txt || !*txt)
		return;

	// Make local copy of text for tokenising
	char* s = ::strdup(txt);

	char* p = s;
	while(*p == ' ') p++;
	char* q = p;

	while (true)
	{

		switch (*q)
		{

			case '\r':
			case '\n':
			case '\0':
				// Terminate and copy
				*q = 0;
				if (p < q) CopyMailAddress(p);
				delete s;
				s = NULL;
				return;

			case '\"':
			//case '\'':
			  {
				// Step until term (remember escape chars)
				p = q++;
				while (*q)
				{
					if (*q == '\\')
					{
						// Bump past escape pair
						q++;
						if (*q) q++;
					}
					else if (*q != *p) q++;
					else break;
				}
				if (!*q)
				{
					delete s;
					s = NULL;
					return;
				}

				// Need to check for following domain
				char* r = q+1;
				while(*r == ' ')
					r++;

				// If no domain its a quoted name so just copy name
				if (*r != '@')
				{
					// tie off
					*q++ = '\0';
					p++;

					// Unescape name
					cdstring tmp = p;
					tmp.FilterOutEscapeChars();
					// Copy name and carry on
					CopyName(tmp, true);

					while(*q == ' ')
						q++;
					p = q;
					break;
				}

				// Got quoted mailbox and domain so set up to fall through to '@' case
				q = r;
			  }
			case '@':
			  {	
				// Goto end of address and terminate
				q += ::strcspn(q, WHITE_SPACE);
				bool more = false;
				bool finished = false;
				if ((*q == '\r') || (*q == '\n'))
				{
					finished = true;
					more = true;
				}
				else if (*q == '\0')
				{
					finished = true;
					more = false;
				}
				*q = '\0';
				CopyMailAddress(p);

				if (!finished)
				{
					// Advance over null and strip spaces
					q++;
					p = ::strgetbrastr(&q);

					if (p) CopyName(p, false);

					more = (*q);
				}
				else
					CopyName(NULL, false);

				q++;
				delete s;
				s = NULL;
				return;
			  }
			case '<':
			  {
				// Was there anything before?
				if (p != q)
				{
					// Tie off and get name
					*q = '\0';
					CopyName(p, true);
				}

				// Step until close
				p = ++q;
				while (*q && (*q != '>')) q++;
				if (!*q)
				{
					delete s;
					s = NULL;
					return;
				}

				// tie off
				*q++ = '\0';
				CopyMailAddress(p);

				delete s;
				s = NULL;
				return;
			  }
			default:
				q++;
		}
	}

	delete s;
	s = NULL;
	return;
}

// Qualify if unqualified address
void CAddress::QualifyAddress(const char* qualify_domain)
{
	// Only if domain exists and unqualified
	if (!mHost.empty() || !*qualify_domain) return;

	// Look for '@' in domain
	const char* p = ::strchr(qualify_domain, '@');
	if (!p)
		// Add as domain
		mHost = qualify_domain;
	else
	{
		mHost = p + 1;
		if (p != qualify_domain)
		{
			mMailbox += cdstring(qualify_domain, p - qualify_domain);
			mMailbox.trimspace();
		}
	}
	mHost.trimspace();
}

// Copy parsed name
void CAddress::CopyName(const char* copy, bool prefix)
{

	mName = cdstring::null_str;

	if (copy && *copy)
	{
		const char* copy_start = copy;
		short copy_length = ::strlen(copy);

		// Strip leading bracket if its a trailing phrase
		if (!prefix && (*copy == '('))
		{
			copy_length -= 2;
			copy_start++;
		}

		// Strip trailing spaces
		while (copy_length)
		{
			if (copy_start[copy_length-1] != ' ') break;
			copy_length--;
		}

		// Copy text
		if (copy_length)
		{
			mName = cdstring(copy_start, copy_length);
		}
	}
}

// Copy host
void CAddress::CopyMailAddress(const char* copy)
{
	mMailbox = cdstring::null_str;
	mHost = cdstring::null_str;

	if (copy && *copy)
	{
		// Get pos of last '@' (NB '@' could occur earlier if quoted (e.g. "cyrus@daboo"@sitename))
		const char* pos = ::strrchr(copy, '@');

		if (pos)
		{
			SetMailbox(cdstring(copy, pos - copy));		// Use explicit SetMailbox to ensure proper quoting
			SetHost(pos + 1);
		}
		else
			SetMailbox(copy);

	}
}

// Get sensible name from address
cdstring CAddress::GetNamedAddress() const
{
	// Use name if there
	if (mName.empty())
		// Form address from mailbox & host
		return GetMailAddress();
	else
		return mName;
}

// Get sensible mail address
cdstring CAddress::GetMailAddress() const
{
	// Form address from mailbox & host
	cdstring copy = mMailbox;
	if (!mHost.empty())
	{
		copy += "@";
		copy += mHost;
	}

	return copy;
}

// Get sensible full address
cdstring CAddress::GetFullAddress(bool encode) const
{
	// Get email address
	cdstring result;
	cdstring email = mMailbox;
	if (!mHost.empty())
	{
		email += "@";
		email += mHost;
	}
	
	// Add name if there and not same as email
	if (email != mName)
	{
		result = mName;
		CRFC822::HeaderQuote(result, true, encode);
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

	return result;
}

// Set mailbox
void CAddress::SetMailbox(const char* mailbox)
{
	// Copy as is
	mMailbox = mailbox;
	mMailbox.trimspace();

	// Look for unquoted mailbox
	if (!mMailbox.isquoted())
	{
		// Look for mailbox specials and quote with escapes
		if (::strpbrk(mMailbox.c_str(), cRFC822_MAILBOX_SPECIALS))
			mMailbox.quote(true);
	}
}

// Set host
void CAddress::SetHost(const char* host)
{
	mHost = host;
	mHost.trimspace();
}

// Check whether it has some info
bool CAddress::IsEmpty() const
{
	return mName.empty() && mAdl.empty() &&  mMailbox.empty() && mHost.empty();
}

// Check whether it is valid
bool CAddress::IsValid() const
{
	return !mMailbox.empty() && !mHost.empty();
}
