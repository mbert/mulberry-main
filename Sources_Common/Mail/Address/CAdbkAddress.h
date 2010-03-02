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


// Header for Addressbook Address class

#ifndef __CADBKADDRESS__MULBERRY__
#define __CADBKADDRESS__MULBERRY__

#include "CAddress.h"

// Classes

class CAdbkAddress : public CAddress
{
public:

	// find_if functors
	class same_entry
	{
	public:
		same_entry(const CAdbkAddress* data) :
			mData(data) {}

		bool operator() (const CAddress* item) const
			{ return (dynamic_cast<const CAdbkAddress*>(item) && mData) ? (static_cast<const CAdbkAddress*>(item)->GetEntry() == mData->GetEntry()) : false; }
	private:
		const CAdbkAddress* mData;
	};

	class same_entry_str
	{
	public:
		same_entry_str(const char* data) :
			mData(data) {}

		bool operator() (const CAddress* item) const
			{ return dynamic_cast<const CAdbkAddress*>(item) ? (static_cast<const CAdbkAddress*>(item)->GetEntry() == mData) : false; }
	private:
		cdstring mData;
	};

	enum EAddressMatch
	{
		eMatchExactly = 1,
		eMatchAtStart,
		eMatchAtEnd,
		eMatchAnywhere
	};

	enum EAddressField
	{
		eName = 0,
		eNickName,
		eEmail,
		eCalendar,
		eCompany,
		eAddress,
		ePhoneWork,
		ePhoneHome,
		eFax,
		eURL,
		eNotes,
		eNumberFields
	};

	enum EEmailType
	{
		eDefaultEmailType = 0,
		eHomeEmailType,
		eWorkEmailType,
		eOtherEmailType
	};

	enum EAddressType
	{
		eDefaultAddressType = 0,
		eHomeAddressType,
		eWorkAddressType
	};

	enum EPhoneType
	{
		eDefaultPhoneType = 0,
		eHomePhoneType,
		eWorkPhoneType,
		eMobilePhoneType,
		eFaxType,
		eHomeFaxType,
		eWorkFaxType,
		ePagerType,
		eOtherPhoneType
	};

	typedef std::vector<EAddressField> CAddressFields;
	typedef std::multimap<EEmailType, cdstring>	emailmap;
	typedef std::multimap<EAddressType, cdstring> addrmap;
	typedef std::multimap<EPhoneType, cdstring> phonemap;

	CAdbkAddress()
	{
		_init_CAdbkAddress();
	}
	CAdbkAddress(const CAddress &copy);					// Copy constructor
	CAdbkAddress(const CAdbkAddress &copy) :			// Copy constructor
		CAddress(copy)
	{
		_copy_CAdbkAddress(copy);
	}

	explicit CAdbkAddress(const char* entry,
				const char* addr,
				const char* uname,
				const char* uadl = NULL,
				const char* ucalendar = NULL,
				const char* ucompany = NULL,
				const char* uaddress = NULL,
				const char* uphonework = NULL,
				const char* uphonehome = NULL,
				const char* ufax = NULL,
				const char*	uurl = NULL,
				const char* unotes = NULL);				// Construct from actual parameters

	~CAdbkAddress();

	CAdbkAddress& operator=(const CAdbkAddress& copy)	// Assignment with same type
	{
		if (this != &copy)
		{
			CAddress::operator=(copy);
			_copy_CAdbkAddress(copy);
		}

		return *this;
	}

	int operator==(const CAdbkAddress& addr) const;					// Compare with another

	static void ExpandMatch(EAddressMatch match, cdstring& str);

	bool Search(const cdstring& text, const CAdbkAddress::CAddressFields& fields) const;	// Compare specific fields

	void SetEntry(const char* theEntry)								// Set entry
		{ mEntry = theEntry; }
	const cdstring&	GetEntry() const								// Get entry
		{ return mEntry; }

	void AddMailAddressToList(cdstrvect& list, bool full) const;		// Add email addresses to list
	void AddCalendarAddressToList(cdstrvect& list, bool full) const;	// Add calendar addresses to list

	emailmap& GetEmails()												// Get email map
		{ return mEmails; }
	const emailmap& GetEmails() const									// Get email map
		{ return mEmails; }

	void SetEmail(const char* thePhone, EEmailType type, bool append = false);			// Set email
	const cdstring&	GetEmail(EEmailType type) const;									// Get email

	void SetPreferredEmail(EEmailType type)								// Set preferred email type
	{
		mPreferredEmail = type;
	}
	EEmailType	GetPreferredEmail() const								// Get preferred email type
	{
		return mPreferredEmail;
	}

	void SetCalendar(const char* theCalendar)						// Set calendar
		{ mCalendar = theCalendar; }
	const cdstring&	GetCalendar() const								// Get calendar
		{ return mCalendar; }
	
	void SetCompany(const char* theCompany)							// Set company
		{ mCompany = theCompany; }
	const cdstring&	GetCompany() const								// Get company
		{ return mCompany; }

	addrmap& GetAddresses()											// Get address map
		{ return mAddresses; }
	const addrmap& GetAddresses() const								// Get address map
		{ return mAddresses; }

	void SetAddress(const char* theAddress, EAddressType type, bool append = false);		// Set address
	const cdstring&	GetAddress(EAddressType type) const;									// Get address

	void SetPreferredAddress(EAddressType type)								// Set preferred address type
	{
		mPreferredAddress = type;
	}
	EAddressType	GetPreferredAddress() const								// Get preferred address type
	{
		return mPreferredAddress;
	}

	phonemap& GetPhones()											// Get phone number map
		{ return mPhones; }
	const phonemap& GetPhones() const								// Get phone number map
		{ return mPhones; }

	void SetPhone(const char* thePhone, EPhoneType type, bool append = false);			// Set phone
	const cdstring&	GetPhone(EPhoneType type) const;									// Get phone

	void SetPreferredPhone(EPhoneType type)								// Set preferred phone type
	{
		mPreferredPhone = type;
	}
	EPhoneType	GetPreferredPhone() const								// Get preferred phone type
	{
		return mPreferredPhone;
	}

	void SetURL(const char* theURL)									// Set URL
		{ mURL = theURL; }
	const cdstring&	GetURL() const									// Get URL
		{ return mURL; }

	void SetNotes(const char* theNotes)								// Set notes
		{ mNotes = theNotes; }
	const cdstring&	GetNotes() const								// Get notes
		{ return mNotes; }

	virtual bool IsEmpty() const;								// Check whether it has some info

private:
	cdstring			mEntry;							// Entry name
	emailmap			mEmails;						// Multiple email types
	EEmailType			mPreferredEmail;				// Preferred email type
	cdstring			mCalendar;						// Calendar name
	cdstring			mCompany;						// Company name
	addrmap				mAddresses;						// Multiple address types
	EAddressType		mPreferredAddress;				// Preferred address type
	phonemap			mPhones;						// Multiple phone types
	EPhoneType			mPreferredPhone;				// Preferred phone type
	cdstring			mURL;							// URL
	cdstring			mNotes;							// Notes

	void _init_CAdbkAddress();
	void _copy_CAdbkAddress(const CAdbkAddress& copy);
};

#endif
