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


// CAddressAccount.h

// Header file for class/structs that define address book accounts

#ifndef __CADDRESSACCOUNT__MULBERRY__
#define __CADDRESSACCOUNT__MULBERRY__

#include "CINETAccount.h"
#include "ptrvector.h"

#include "CAdbkAddress.h"
#include "CFutureItems.h"

// Classes
class CAddressAccount;
typedef ptrvector<CAddressAccount> CAddressAccountList;

struct SAddressAccountLDAP
{
	cdstring		mRoot;										// LDAP search root
	cdstring		mMatch[CAdbkAddress::eNumberFields];		// Attribute mapping
	bool			mNotes;										// Put all attributes in Notes field
	bool			mSearch;									// Use in Address Search window
	bool			mUseNickNames;								// Use for nick-name resolution
	bool			mUseExpansion;								// Use for address expansion
	unsigned long	mVersion;									// Protocol version - auto-detected

	int operator==(const SAddressAccountLDAP& comp) const;		// Compare with same type
};

class CAddressAccount : public CINETAccount
{
public:
			CAddressAccount();
			CAddressAccount(const CAddressAccount& copy) : CINETAccount(copy)
				{ _copy(copy); }
	virtual ~CAddressAccount();

			CAddressAccount& operator=(const CAddressAccount& copy)					// Assignment with same type
				{ if (this != &copy) {CINETAccount::operator=(copy); _copy(copy);} return *this; }
	int operator==(const CAddressAccount& comp) const;								// Compare with same type

	// Getters/setters
	virtual void SetServerType(EINETServerType type);

	bool GetDisconnected() const
		{ return mDisconnected; }
	void SetDisconnected(bool disconnected)
		{ mDisconnected = disconnected; }

	bool GetExpanded() const
		{ return mExpanded; }
	void SetExpanded(bool expanded)
		{ mExpanded = expanded; }
	
	const cdstring& GetBaseRURL() const
	{
		return mBaseRURL;
	}
	void SetBaseRURL(const cdstring& rurl)
	{
		mBaseRURL = rurl;
	}

	const cdstring& GetLDAPRoot(void) const
		{ return ldap->mRoot; }
	void SetLDAPRoot(const cdstring& root)
		{ ldap->mRoot = root; }

	const cdstring& GetLDAPMatch(CAdbkAddress::EAddressField field) const
		{ return ldap->mMatch[field]; }
	void SetLDAPMatch(CAdbkAddress::EAddressField field, const cdstring& match)
		{ ldap->mMatch[field] = match; }

	bool GetLDAPNotes(void) const
		{ return ldap->mNotes; }
	void SetLDAPNotes(bool notes)
		{ ldap->mNotes = notes; }

	bool GetLDAPSearch(void) const
		{ return ldap->mSearch; }
	void SetLDAPSearch(bool search)
		{ ldap->mSearch = search; }

	bool GetLDAPUseNickNames(void) const
		{ return ldap->mUseNickNames; }
	void SetLDAPUseNickNames(bool nickname)
		{ ldap->mUseNickNames = nickname; }

	bool GetLDAPUseExpansion(void) const
		{ return ldap->mUseExpansion; }
	void SetLDAPUseExpansion(bool expansion)
		{ ldap->mUseExpansion = expansion; }

	unsigned long GetLDAPVersion() const
		{ return ldap->mVersion; }
	void SetLDAPVersion(unsigned long version)
		{ ldap->mVersion = version; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	SAddressAccountLDAP*	ldap;
	bool					mDisconnected;
	bool					mExpanded;
	cdstring				mBaseRURL;
	CFutureItems			mFuture;
	
	void _copy(const CAddressAccount& copy);
};

#endif
