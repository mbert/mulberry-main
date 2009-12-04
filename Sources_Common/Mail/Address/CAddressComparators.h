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


// Header for CAddressComparators class

#ifndef __CADDRESSCOMPARATORS__MULBERRY__
#define __CADDRESSCOMPARATORS__MULBERRY__

// Classes

class CAddress;

typedef bool (*AddressCompare)(const CAddress*, const CAddress*);
typedef long (*AddressEqual)(const CAddress*, const CAddress*);

class CAddressComparator
{
public:
	virtual ~CAddressComparator();

	virtual AddressCompare	GetComparator() = 0;
	virtual AddressEqual	GetEquality() = 0;

	static long	FirstNameFN(const CAddress* addr1, const CAddress* addr2);
	static long	LastNameFN(const CAddress* addr1, const CAddress* addr2);
	static long	EmailFN(const CAddress* addr1, const CAddress* addr2);
	static long	EmailDomainFN(const CAddress* addr1, const CAddress* addr2);
	static long	NickNameFN(const CAddress* addr1, const CAddress* addr2);
	static long	CompanyFN(const CAddress* addr1, const CAddress* addr2);

	static bool	CompareFirstName(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareLastName(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareEmail(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareEmailOnly(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareEmailDomain(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareNickName(const CAddress* addr1, const CAddress* addr2);
	static bool	CompareCompany(const CAddress* addr1, const CAddress* addr2);

	//private:						// Never construct one of these, always a sub-class
 protected: //But the sub class has to be able to call it's baseclass's constructor, which it can't do if it's private
	CAddressComparator();
	
	static const char* ExtractLastName(const char* name);
};

class CFirstNameComparator : public CAddressComparator
{
public:
	CFirstNameComparator() {}
	virtual ~CFirstNameComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareFirstName; }
	virtual AddressEqual	GetEquality()
		{ return FirstNameFN; }
};

class CLastNameComparator : public CAddressComparator
{
public:
	CLastNameComparator() {}
	virtual ~CLastNameComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareLastName; }
	virtual AddressEqual	GetEquality()
		{ return LastNameFN; }
};

class CEmailComparator : public CAddressComparator
{
public:
	CEmailComparator() {}
	virtual ~CEmailComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareEmail; }
	virtual AddressEqual	GetEquality()
		{ return EmailFN; }
};

class CEmailOnlyComparator : public CAddressComparator
{
public:
	CEmailOnlyComparator() {}
	virtual ~CEmailOnlyComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareEmailOnly; }
	virtual AddressEqual	GetEquality()
		{ return EmailFN; }
};

class CEmailDomainComparator : public CAddressComparator
{
public:
	CEmailDomainComparator() {}
	virtual ~CEmailDomainComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareEmailDomain; }
	virtual AddressEqual	GetEquality()
		{ return EmailDomainFN; }
};

class CNickNameComparator : public CAddressComparator
{
public:
	CNickNameComparator() {}
	virtual ~CNickNameComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareNickName; }
	virtual AddressEqual	GetEquality()
		{ return NickNameFN; }
};

class CCompanyComparator : public CAddressComparator
{
public:
	CCompanyComparator() {}
	virtual ~CCompanyComparator() {}

	virtual AddressCompare	GetComparator()
		{ return CompareCompany; }
	virtual AddressEqual	GetEquality()
		{ return CompanyFN; }
};


#endif
