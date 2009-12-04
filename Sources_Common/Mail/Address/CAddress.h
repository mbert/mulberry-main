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


// Header for Address class

#ifndef __CADDRESS__MULBERRY__
#define __CADDRESS__MULBERRY__

#include "cdstring.h"

class CAddress
{
public:
	CAddress();
	CAddress(const CAddress &copy);				// Copy constructor

	explicit CAddress(const char* txt);					// Construct from text
	explicit CAddress(const char* addr,
				const char* uname,
				const char* uadl = NULL);		// Construct from actual parameters

	CAddress& operator=(const CAddress& copy);	// Assignment with same type
	virtual ~CAddress();

	int operator==(const char* addr) const;							// Compare with named addr
	int operator==(const CAddress& addr) const;						// Compare with another

	bool StrictCompareEmail(const CAddress& addr) const;				// Compare (case-sensitive local part) email addresses

	void	ParseAddress(const char* txt);							// Parse address from text
	void	QualifyAddress(const char* qualify_domain);				// Qualify if unqualified address

	void SetName(const char* name)								// Set name
		{ mName = name; }
	cdstring& GetName()											// Get name
		{ return mName; }
	const cdstring&	GetName() const								// Get name
		{ return mName; }

	void SetADL(const char* adl)								// Set adl
		{ mAdl = adl; }
	const cdstring&	GetADL() const								// Get adl
		{ return mAdl; }

	void SetMailbox(const char* mailbox);						// Set mailbox
	const cdstring&	GetMailbox() const							// Get mailbox
		{ return mMailbox; }

	void SetHost(const char* host);								// Set host
	const cdstring&	GetHost() const								// Get host
		{ return mHost; }
	
	void CopyName(const char* copy, bool prefix);				// Copy parsed name
	void CopyMailAddress(const char* theAddr);					// Copy parsed mail address

	cdstring GetNamedAddress() const;							// Get sensible name from address
	cdstring GetMailAddress() const;							// Get sensible mail address
	cdstring GetFullAddress(bool encode = false) const;			// Get sensible full address

	virtual bool IsEmpty() const;								// Check whether it has some info

			bool IsValid() const;								// Validate

private:
	cdstring			mName;						// Personal name
	cdstring			mAdl;						// Source route
	cdstring			mMailbox;					// Mailbox name
	cdstring			mHost;						// Host name

};

#endif
