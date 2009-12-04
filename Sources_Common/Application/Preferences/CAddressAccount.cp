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


// CAddressAccount.cp

// Header file for class/structs that define address book accounts

#include "CAddressAccount.h"

#include "char_stream.h"
#include "CPreferenceVersions.h"
#include "CUtils.h"

#include <stdlib.h>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

int SAddressAccountLDAP::operator==(const SAddressAccountLDAP& comp) const		// Compare with same type
{
	bool result = true;
	for(int i = 0; i < CAdbkAddress::eNumberFields; i++)
		result = result && (mMatch[i] == comp.mMatch[i]);
	return (mRoot == comp.mRoot) &&
			result &&
			(mNotes == comp.mNotes) &&
			(mUseNickNames == comp.mUseNickNames) &&
			(mUseExpansion == comp.mUseExpansion);
}

CAddressAccount::CAddressAccount()
{
	mServerType = eIMSP;
	ldap = NULL;
	mDisconnected = false;
	mExpanded = false;
}

void CAddressAccount::_copy(const CAddressAccount& copy)
{
	switch(mServerType)
	{
	case eLDAP:
		ldap = new SAddressAccountLDAP;
		ldap->mRoot = copy.ldap->mRoot;
		for(int i = 0; i < CAdbkAddress::eNumberFields; i++)
			ldap->mMatch[i] = copy.ldap->mMatch[i];
		ldap->mNotes = copy.ldap->mNotes;
		ldap->mSearch = copy.ldap->mSearch;
		ldap->mUseNickNames = copy.ldap->mUseNickNames;
		ldap->mUseExpansion = copy.ldap->mUseExpansion;
		ldap->mVersion = copy.ldap->mVersion;
		break;
	default:
		ldap = NULL;
		break;
	}
	mDisconnected = copy.mDisconnected;
	mExpanded = copy.mExpanded;
	mBaseRURL = copy.mBaseRURL;
	mFuture = copy.mFuture;
}

CAddressAccount::~CAddressAccount()
{
	switch(mServerType)
	{
	case eLDAP:
		delete ldap;
		ldap = NULL;
		break;
	default:
		break;
	}
}

// Compare with same type
int CAddressAccount::operator==(const CAddressAccount& comp) const
{
	int result = 0;
	if (CINETAccount::operator==(comp))
	{
		switch(mServerType)
		{
		case eLDAP:
			result = (ldap == comp.ldap);
			break;
		default:
			result = 1;
			break;
		}
	}

	return result && (mDisconnected == comp.mDisconnected) &&
			(mBaseRURL == comp.mBaseRURL);
}

void CAddressAccount::SetServerType(EINETServerType type)
{
	// Delete existing
	switch(mServerType)
	{
	case eLDAP:
		delete ldap;
		ldap = NULL;
		break;
	default:
		break;
	}

	// Add new
	mServerType = type;
	switch(mServerType)
	{
	case eIMSP:
	case eACAP:
	case eCardDAVAdbk:
	default:
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::ePlainText);
		ldap = NULL;
		break;
	case eLDAP:
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
		ldap = new SAddressAccountLDAP;
		ldap->mMatch[CAdbkAddress::eName] = "cn";
		ldap->mMatch[CAdbkAddress::eEmail] = "mail";
		ldap->mNotes = false;
		ldap->mSearch = true;
		ldap->mUseNickNames = false;
		ldap->mUseExpansion = false;
		ldap->mVersion = 3;
		mDisconnected = false;
		mExpanded = false;
		break;
	case eLocalAdbk:
	case eOSAdbk:
		// Always login at startup, no auth
		mLogonAtStart = true;
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
		ldap = NULL;
		mDisconnected = false;
		mExpanded = false;
		break;
	}
}

cdstring CAddressAccount::GetInfo(void) const
{
	// Create string list of items
	cdstring info;
	info += '(';
	info += CINETAccount::GetInfo();
	info += ')';

	info += '(';

	info += mDisconnected ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	switch(mServerType)
	{
	case eLDAP:
		{
			cdstring temp = ldap->mRoot;
			temp.quote();
			temp.ConvertFromOS();
			info += temp;

			for(int i = 0; i < CAdbkAddress::eNumberFields; i++)
			{
				info += cSpace;
				temp = ldap->mMatch[i];
				temp.quote();
				temp.ConvertFromOS();
				info += temp;
			}
			info += cSpace;
			info += (ldap->mNotes ? cValueBoolTrue : cValueBoolFalse);
			// Justin 5/29/98
			info += cSpace;
			info += (ldap->mSearch ? cValueBoolTrue : cValueBoolFalse);

			// v2.2 new option
			info += cSpace;
			info += (ldap->mUseNickNames ? cValueBoolTrue : cValueBoolFalse);

			// v2.2 new option
			info += cSpace;
			info += (ldap->mUseExpansion ? cValueBoolTrue : cValueBoolFalse);

			info += cSpace;
		}
		break;
	default:
		break;
	}

	info += mExpanded ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	cdstring temp = mBaseRURL;
	temp.quote();
	info += temp;

	info += mFuture.GetInfo();
	info += ')';

	// Got it all
	return info;
}

bool CAddressAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// If >= v1.4
	if (::VersionTest(vers_prefs, VERS_1_4_0) >= 0)
	{
		txt.start_sexpression();
			result = CINETAccount::SetInfo(txt, vers_prefs);
		txt.end_sexpression();

		txt.start_sexpression();

		// >= v2.0a2
		if ((VersionTest(vers_prefs, VERS_2_0_0_A_2) >= 0))
			txt.get(mDisconnected);

		switch(mServerType)
		{
		case eLDAP:
			if (!ldap)
				ldap = new SAddressAccountLDAP;
			txt.get(ldap->mRoot, true);
			for(int i = 0; i < CAdbkAddress::eNumberFields; i++)
				txt.get(ldap->mMatch[i], true);
			txt.get(ldap->mNotes);
			txt.get(ldap->mSearch);
			txt.get(ldap->mUseNickNames);
			txt.get(ldap->mUseExpansion);
			ldap->mVersion = 3;
			break;
		default:
			ldap = NULL;
			break;
		}

		// Look for expansion item
		if (!txt.test_end_sexpression())
			txt.get(mExpanded);

		// Expansion items:
		char* p = txt.get();
		if (p)
			mBaseRURL = p;

		mFuture.SetInfo(txt, vers_prefs);
		txt.end_sexpression();
	}
	else
	{
		txt.get(mName);

		// Bump past comma & space
		txt += 2;

		txt.get(mServerIP);

		// Bump past comma & space
		txt += 2;

		char* item = txt.get();
		if (item && *item)
			SetServerType((EINETServerType) (::atoi(item) + 5));
		switch(mServerType)
		{
		case eIMSP:
		case eACAP:
		default:
			{
				GetAuthenticatorUserPswd()->SetUID(txt.get());
				txt += 2;
				cdstring aPswd = txt.get();
				txt += 2;
				// >= v1.3.2
				if (::VersionTest(vers_prefs, VERS_1_3_2) >= 0)
					aPswd.Decrypt(cdstring::eEncryptSimplemUTF7);
				else
				{
					// Use old encoding and force dirty for update
					aPswd.Decrypt(cdstring::eEncryptSimple);
					result = false;
				}
				GetAuthenticatorUserPswd()->SetPswd(aPswd);

				GetAuthenticatorUserPswd()->SetSaveUID(::atoi(txt.get()));
				GetAuthenticatorUserPswd()->SetSavePswd(::atoi(txt.get()));
				mLogonAtStart = ::atoi(txt.get());
				if (!GetAuthenticatorUserPswd()->GetSavePswd())
					GetAuthenticatorUserPswd()->SetPswd(cdstring::null_str);

				// Now set account and authenticator

			}
			break;
		case eLDAP:
			txt.get(ldap->mRoot);
			txt += 2;
			for(int i = 0; i < CAdbkAddress::eNumberFields; i++)
			{
				txt.get(ldap->mMatch[i]);
				txt += 2;
			}
			ldap->mNotes = ::atoi(txt.get());
			break;
		}

		result = false;
	}

	return result;
}
