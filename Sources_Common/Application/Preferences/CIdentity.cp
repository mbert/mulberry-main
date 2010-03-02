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


// CIdentity.cp

// Class that encapsulates a 'mail identity' used when sending messages

#include "CIdentity.h"

#include "char_stream.h"
#include "CCalendarAddress.h"
#include "CCalendarStoreNode.h"
#include "CMbox.h"
#include "CPreferences.h"
#include "CPreferenceVersions.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CURL.h"
#include "CUtils.h"

#include "cdfstream.h"

#include <algorithm>
#include <strstream>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CIdentity

CIdentity::CIdentity()
{
	mTemp = false;
	mFrom.second = false;
	mReplyTo.second = false;
	mSender.second = false;
	mCalendar.second = false;
	mCopyTo.second = false;
	mCopyToNone = false;
	mCopyToChoose = false;
	mCopyReplied = false;
	mHeader.second = false;
	mSignature.second = false;

	mAddTo.second = false;
	mAddCC.second = false;
	mAddBcc.second = false;

	mUseDSN = false;

	mUseSecurity = false;
	mSign = false;
	mEncrypt = false;
	mSignWith = eSignWithDefault;
}

// Copy construct
void CIdentity::_copy(const CIdentity& copy)
{
	mTemp = copy.mTemp;
	mInherit = copy.mInherit;
	mIdentity = copy.mIdentity;
	mSMTPAccount = copy.mSMTPAccount;
	mFrom = copy.mFrom;
	mReplyTo = copy.mReplyTo;
	mSender = copy.mSender;
	mCalendar = copy.mCalendar;
	mCopyTo = copy.mCopyTo;
	mCopyToNone = copy.mCopyToNone;
	mCopyToChoose = copy.mCopyToChoose;
	mCopyReplied = copy.mCopyReplied;
	mHeader = copy.mHeader;
	mSignature = copy.mSignature;

	mAddTo = copy.mAddTo;
	mAddCC = copy.mAddCC;
	mAddBcc = copy.mAddBcc;

	mUseDSN = copy.mUseDSN;
	mDSN = copy.mDSN;

	mUseSecurity = copy.mUseSecurity;
	mSign = copy.mSign;
	mEncrypt = copy.mEncrypt;
	mSignWith = copy.mSignWith;
	mSignOther = copy.mSignOther;
	
	mFuture = copy.mFuture;

	mTiedMailboxes = copy.mTiedMailboxes;
	mTiedCalendars = copy.mTiedCalendars;
}

// Compare with same type
int CIdentity::operator==(const CIdentity& test) const
{
	return (mInherit == test.mInherit) &&
			(mIdentity == test.mIdentity) &&
			(mSMTPAccount == test.mSMTPAccount) &&
			(mFrom == test.mFrom) &&
			(mReplyTo == test.mReplyTo) &&
			(mSender == test.mSender) &&
			(mCalendar == test.mCalendar) &&
			(mCopyTo == test.mCopyTo) &&
			(mCopyToNone == test.mCopyToNone) &&
			(mCopyToChoose == test.mCopyToChoose) &&
			(mCopyReplied == test.mCopyReplied) &&
			(mHeader == test.mHeader) &&
			(mSignature == test.mSignature) &&
			(mAddTo == test.mAddTo) &&
			(mAddCC == test.mAddCC) &&
			(mAddBcc == test.mAddBcc) &&
			(mUseSecurity == test.mUseSecurity) &&
			(mDSN == test.mDSN) &&
			(mSign == test.mSign) &&
			(mEncrypt == test.mEncrypt) &&
			(mSignWith == test.mSignWith) &&
			(mSignOther == test.mSignOther) &&
			(mTiedMailboxes == test.mTiedMailboxes) &&
			(mTiedCalendars == test.mTiedCalendars);
}

bool CIdentity::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	bool changed = false;
	size_t old_length = old_acct.length();

	// Rename any copy to
	if (::strncmp(mCopyTo.first.c_str(), old_acct.c_str(), old_length) == 0)
	{
		cdstring copy = new_acct + &mCopyTo.first.c_str()[old_length];
		mCopyTo.first = copy;
		changed = true;
	}

	// Find matching tied mailboxes
	cdstrvect found;
	for(cdstrset::iterator iter = mTiedMailboxes.begin(); iter != mTiedMailboxes.end(); iter++)
	{
		if (::strncmp((*iter).c_str(), old_acct.c_str(), old_length) == 0)
			found.push_back(*iter);
	}

	// Now erase and insert renamed one
	for(cdstrvect::iterator iter = found.begin(); iter != found.end(); iter++)
	{
		mTiedMailboxes.erase(*iter);
		cdstring copy = new_acct + &(*iter).c_str()[old_length];
		mTiedMailboxes.insert(copy);
	}
	changed = changed || found.size();

	// Find matching tied calendars
	found.clear();
	for(cdstrset::iterator iter = mTiedCalendars.begin(); iter != mTiedCalendars.end(); iter++)
	{
		if (::strncmp((*iter).c_str(), old_acct.c_str(), old_length) == 0)
			found.push_back(*iter);
	}

	// Now erase and insert renamed one
	for(cdstrvect::iterator iter = found.begin(); iter != found.end(); iter++)
	{
		mTiedCalendars.erase(*iter);
		cdstring copy = new_acct + &(*iter).c_str()[old_length];
		mTiedCalendars.insert(copy);
	}
	changed = changed || found.size();

	return changed;
}

void CIdentity::AddTiedMailbox(const CMbox* mbox)
{
	// Just insert - duplicate will not be added
	mTiedMailboxes.insert(mbox->GetAccountName());
}

void CIdentity::RemoveTiedMailbox(const CMbox* mbox)
{
	// Just erase
	mTiedMailboxes.erase(mbox->GetAccountName());
}

bool CIdentity::HasTiedMailbox(const CMbox* mbox) const
{
	// Just erase
	cdstrset::const_iterator found = mTiedMailboxes.find(mbox->GetAccountName());

	return found != mTiedMailboxes.end();
}

bool CIdentity::HasTiedMailbox(const cdstring& mboxname) const
{
	// Just erase
	cdstrset::const_iterator found = mTiedMailboxes.find(mboxname);

	return found != mTiedMailboxes.end();
}

void CIdentity::AddTiedCalendar(const calstore::CCalendarStoreNode* node)
{
	// Just insert - duplicate will not be added
	mTiedCalendars.insert(node->GetAccountName());
}

void CIdentity::RemoveTiedCalendar(const calstore::CCalendarStoreNode* node)
{
	// Just erase
	mTiedCalendars.erase(node->GetAccountName());
}

bool CIdentity::HasTiedCalendar(const calstore::CCalendarStoreNode* node) const
{
	// Just erase
	cdstrset::const_iterator found = mTiedCalendars.find(node->GetAccountName());

	return found != mTiedCalendars.end();
}

bool CIdentity::HasTiedCalendar(const cdstring& calname) const
{
	// Just erase
	cdstrset::const_iterator found = mTiedCalendars.find(calname);

	return found != mTiedCalendars.end();
}

cdstrset& CIdentity::GetTied(ETiedInfo type)
{
	switch(type)
	{
	case eMbox:
	default:
		return mTiedMailboxes;
	case eCal:
		return mTiedCalendars;
	}
}

const cdstrset& CIdentity::GetTied(ETiedInfo type) const
{
	switch(type)
	{
	case eMbox:
	default:
		return mTiedMailboxes;
	case eCal:
		return mTiedCalendars;
	}
}

bool CIdentity::SetTiedInfo(ETiedInfo type, char_stream& info, NumVersion vers_prefs)
{
	// Clear existing
	GetTied(type).clear();

	// Must have leading (
	if (!info.start_sexpression())
		return true;

	// Read quoted strings until ')'
	while(!info.end_sexpression())
	{
		char* q = info.get();
		if (q)
			GetTied(type).insert(cdstring::ConvertToOS(q));
	}

	return true;
}

cdstring CIdentity::GetTiedInfo(ETiedInfo type) const
{
	cdstring all;
	all = '(';

	// Add each mailbox
	bool first = true;
	for(cdstrset::const_iterator iter = GetTied(type).begin(); iter != GetTied(type).end(); iter++)
	{
		if (!first)
			all += cSpace;

		cdstring copy = *iter;
		copy.quote();
		copy.ConvertFromOS();
		all += copy;

		first = false;
	}

	all += ')';

	return all;
}

// Parse S-Expression element
bool CIdentity::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mIdentity, true);
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	// If >= v1.4
	if (::VersionTest(vers_prefs, VERS_1_4_0) >= 0)
	{
		// Special hack: ':' prefix means inherit from default
		cdstring acct;
		txt.get(acct, true);
		if (acct[(cdstring::size_type)0] == ':')
		{
			mSMTPAccount.first = &acct[(cdstring::size_type)1];
			mSMTPAccount.second = false;
		}
		else
		{
			mSMTPAccount.first = acct;
			mSMTPAccount.second = true;
		}
	}
		

	// If >= v2.0a1
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
		txt.get(mInherit, true);

	MakePair(txt, mFrom);
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	MakePair(txt, mReplyTo);
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	MakePair(txt, mSender);
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	MakePair(txt, mCopyTo);
	mCopyToNone = mCopyTo.first.empty();
	mCopyToChoose = (mCopyTo.first == "\t");
	if (mCopyToChoose)
		mCopyTo.first = cdstring::null_str;
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
		txt.get(mCopyReplied);

	MakePair(txt, mHeader);
	if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		txt += 2;

	MakePair(txt, mSignature);

	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
	{
		MakePair(txt, mAddTo);
		MakePair(txt, mAddCC);
		MakePair(txt, mAddBcc);

		if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
		{
			txt.get(mUseDSN);
			txt.start_sexpression();
			mDSN.SetInfo(txt, vers_prefs);
			txt.end_sexpression();
		}

		txt.get(mUseSecurity);
		txt.get(mSign);
		txt.get(mEncrypt);
		unsigned long temp;
		txt.get(temp);
		mSignWith = static_cast<ESignWith>(temp);
		txt.get(mSignOther, true);
	}

	if (::VersionTest(vers_prefs, VERS_4_1_0_A_1) >= 0)
	{
		MakePair(txt, mCalendar);
	}	
	
	// Expansion items:
	
	mFuture.SetInfo(txt, vers_prefs);

	return true;
}

// Create S_Expression element
cdstring CIdentity::GetInfo(void) const
{
	cdstring all;
	cdstring item = mIdentity;
	item.quote();
	item.ConvertFromOS();
	all += item;
	all += cSpace;

	// Special hack: ':' prefix means inherit from default
	item = mSMTPAccount.second ? "" : ":";
	item += mSMTPAccount.first;
	item.quote();
	item.ConvertFromOS();
	all += item;
	all += cSpace;

	item = mInherit;
	item.quote();
	item.ConvertFromOS();
	all += item;
	all += cSpace;

	AddPair(all, mFrom);
	all += cSpace;

	AddPair(all, mReplyTo);
	all += cSpace;

	AddPair(all, mSender);
	all += cSpace;

	if (mCopyToNone)
	{
		cdstrbool item;
		item.second = mCopyTo.second;
		AddPair(all, item);
	}
	else if (mCopyToChoose)
	{
		cdstrbool item;
		item.first = "\t";
		item.second = mCopyTo.second;
		AddPair(all, item);
	}
	else
		AddPair(all, mCopyTo);
	all += cSpace;

	all += (mCopyReplied ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	AddPair(all, mHeader);
	all += cSpace;

	AddPair(all, mSignature);
	all += cSpace;

	AddPair(all, mAddTo);
	all += cSpace;

	AddPair(all, mAddCC);
	all += cSpace;

	AddPair(all, mAddBcc);
	all += cSpace;

	all += (mUseDSN ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += "(";
	all += mDSN.GetInfo();
	all += ")";
	all += cSpace;

	all += (mUseSecurity ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mSign ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mEncrypt ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += cdstring(static_cast<unsigned long>(mSignWith));
	all += cSpace;

	item = mSignOther;
	item.quote();
	item.ConvertFromOS();
	all += item;

	AddPair(all, mCalendar);
	all += cSpace;

	all += mFuture.GetInfo();

	return all;
}

#pragma mark ____________________________Getters

const CIdentity& CIdentity::GetInheritIdentity() const
{
	CIdentity* inheritid = NULL;

	// Look for named identity in prefs
	if (!mInherit.empty())
		inheritid = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mInherit);

	// Return the default one if named one not found
	return inheritid ? *inheritid : CPreferences::sPrefs->mIdentities.GetValue().front();
}

const CIdentity* CIdentity::GetInherited(TInheritTest inheritfn) const
{
	// Start with self
	const CIdentity* inherit = this;

	// Holds the list of identities alread inherited from
	std::vector<const CIdentity*> ids;

	// Check whether to inherit data
	while(!(inherit->*inheritfn)())
	{
		// Add current one to chain
		ids.push_back(inherit);

		// Get the next one up
		inherit = &inherit->GetInheritIdentity();
		
		// Make sure its not in the current chain (circular inheritance)
		std::vector<const CIdentity*>::const_iterator found = std::find(ids.begin(), ids.end(), inherit);
		if (found != ids.end())
			return NULL;
	}
	
	return inherit;
}
	
// Helper function to traverse inheritance tree to return appropriate data
const cdstring& CIdentity::GetInheritedValue(TInheritDataStr datafn, TInheritTest inheritfn) const
{
	// Get inherited id that has real data
	const CIdentity* inherit = GetInherited(inheritfn);

	// Have an inherited id with real data - return the data
	if (inherit)
		return (inherit->*datafn)(false);
	else
		return cdstring::null_str;
}

// Helper function to traverse inheritance tree to return appropriate data
cdstring CIdentity::GetInheritedValue(TInheritDataStrVal datafn, TInheritTest inheritfn) const
{
	// Get inherited id that has real data
	const CIdentity* inherit = GetInherited(inheritfn);

	// Have an inherited id with real data - return the data
	if (inherit)
		return (inherit->*datafn)(false);
	else
		return cdstring::null_str;
}

// Helper function to traverse inheritance tree to return appropriate data
bool CIdentity::GetInheritedValue(TInheritDataBool datafn, TInheritTest inheritfn) const
{
	// Get inherited id that has real data
	const CIdentity* inherit = GetInherited(inheritfn);

	// Have an inherited id with real data - return the data
	if (inherit)
		return (inherit->*datafn)(false);
	else
		return false;
}

const cdstring& CIdentity::GetSMTPAccount(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetSMTPAccount, &CIdentity::UseSMTPAccount);
	else
		return mSMTPAccount.first;
}

const cdstring& CIdentity::GetFrom(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetFrom, &CIdentity::UseFrom);
	else
		return mFrom.first;
}

const cdstring& CIdentity::GetReplyTo(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetReplyTo, &CIdentity::UseReplyTo);
	else
		return mReplyTo.first;
}

const cdstring& CIdentity::GetSender(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetSender, &CIdentity::UseSender);
	else
		return mSender.first;
}

const cdstring& CIdentity::GetCalendar(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetCalendar, &CIdentity::UseCalendar);
	else
		return mCalendar.first;
}

cdstring CIdentity::GetCalendarAddress() const
{
	cdstring result = GetCalendar(true);
	if (!result.empty())
		return result;
	result = GetFrom(true);
	if (!result.empty())
	{
		CAddress addr(result);
		result = addr.GetName();
		if (!result.empty())
			result += " ";
		result += "<";
		result += addr.GetMailAddress();
		result += ">";
	}
	return result;
}

const cdstring& CIdentity::GetCopyTo(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetCopyTo, &CIdentity::UseCopyTo);
	else
		return mCopyTo.first;
}

bool CIdentity::GetCopyToNone(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetCopyToNone, &CIdentity::UseCopyTo);
	else
		return !UseCopyTo() || mCopyToNone;
}

bool CIdentity::GetCopyToChoose(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetCopyToChoose, &CIdentity::UseCopyTo);
	else
		return mCopyToChoose;
}

bool CIdentity::GetCopyReplied(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetCopyReplied, &CIdentity::UseCopyTo);
	else
		return mCopyReplied;
}

const cdstring& CIdentity::GetHeader(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetHeader, &CIdentity::UseHeader);
	else
		return mHeader.first;
}

cdstring CIdentity::GetSignature(bool resolve) const
{
	cdstring result;
	if (resolve)
		result = GetInheritedValue(&CIdentity::GetSignature, &CIdentity::UseSignature);
	else
		result = mSignature.first;
	
	// Check for file URL
	if (::strncmpnocase(result.c_str(), cFileURLScheme, ::strlen(cFileURLScheme)) == 0)
	{
		//Decode URL
		cdstring fpath(result.c_str() + ::strlen(cFileURLScheme));
		cdstring temp(cURLHierarchy);
		::strreplace(fpath.c_str_mod(), temp, os_dir_delim);
		fpath.DecodeURL();

		// Open file and read content
		cdifstream fin(fpath, std::ios_base::in | std::ios_base::binary);
		std::ostrstream out;
		::StreamCopy(fin, out, 0, ::StreamLength(fin));
		out << std::ends;
		
		result.steal(out.str());
		result.ConvertToOS();
	}
#if 0
	else if (::strncmpnocase(result.c_str(), cURLLocalhost, ::strlen(cURLLocalhost)) == 0)
	{
		// Read from stdout of app
	}
#endif

	return result;
}

cdstring CIdentity::GetSignatureRaw(bool resolve) const
{
	cdstring result;
	if (resolve)
		result = GetInheritedValue(&CIdentity::GetSignatureRaw, &CIdentity::UseSignature);
	else
		result = mSignature.first;
	
	return result;
}

const cdstring& CIdentity::GetAddTo(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetAddTo, &CIdentity::UseAddTo);
	else
		return mAddTo.first;
}

const cdstring& CIdentity::GetAddCC(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetAddCC, &CIdentity::UseAddCC);
	else
		return mAddCC.first;
}

const cdstring& CIdentity::GetAddBcc(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetAddBcc, &CIdentity::UseAddBcc);
	else
		return mAddBcc.first;
}

const CDSN& CIdentity::GetDSN(bool resolve) const
{
	if (resolve)
	{
		// Get inherited id that has real data
		const CIdentity* inherit = GetInherited(&CIdentity::GetUseDSN);

		// Have an inherited id with real data - return the data
		if (inherit)
			return inherit->GetDSN(false);
		else
		{
			static CDSN null_dsn;
			return null_dsn;
		}
	}
	else
		return mDSN;
}

bool CIdentity::GetSign(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetSign, &CIdentity::GetUseSecurity);
	else
		return mSign;
}

bool CIdentity::GetEncrypt(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetEncrypt, &CIdentity::GetUseSecurity);
	else
		return mEncrypt;
}

CIdentity::ESignWith CIdentity::GetSignWith(bool resolve) const
{
	if (resolve)
	{
		// Get inherited id that has real data
		const CIdentity* inherit = GetInherited(&CIdentity::GetUseSecurity);

		// Have an inherited id with real data - return the data
		if (inherit)
			return inherit->GetSignWith(false);
		else
			return eSignWithDefault;
	}
	else
		return mSignWith;
}

cdstring CIdentity::GetSigningID(bool resolve) const
{
	if (resolve)
		return GetInheritedValue(&CIdentity::GetSigningID, &CIdentity::GetUseSecurity);

	cdstring addr;
	switch(mSignWith)
	{
	case eSignWithDefault:
	default:
		addr = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
		break;
	case eSignWithFrom:
		addr = GetFrom(resolve);
		break;
	case eSignWithReplyTo:
		addr = GetReplyTo(resolve);
		break;
	case eSignWithSender:
		addr = GetSender(resolve);
		break;
	case eSignWithOther:
		addr = GetSignOther();
		break;
	}
	
	// Need email part of address only
	CAddress temp(addr);
	return temp.GetMailAddress();
}

#pragma mark ____________________________Utils

// Add a pair as text
void CIdentity::AddPair(cdstring& all, const cdstrbool& item) const
{
	cdstring add = item.first;
	add.quote();
	add.ConvertFromOS();

	all += add;
	all += cSpace;

	all += (item.second ? cValueBoolTrue : cValueBoolFalse);
}

// Make pair from text
void CIdentity::MakePair(char_stream& txt, cdstrbool& item)
{
	txt.get(item.first, true);

	// Old version has a comma here
	if (*txt == ',')
	{
		txt += 2;

		char p = *txt;
		while(*txt && (*txt != ',') && (*txt != ')')) txt += 1;
		//if (*txt) txt += 1;
		item.second = (p == *cValueBoolTrue);
	}

	// New version
	else
	{
		txt.get(item.second);
	}
}

#pragma mark ____________________________CIdentityList

bool CIdentityList::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	bool changed = false;

	for(iterator iter = begin(); iter != end(); iter++)
		changed = (*iter).RenameAccount(old_acct, new_acct) || changed;

	return changed;
}

CIdentity* CIdentityList::GetIdentity(const cdstring& id) const
{
	// Look for named identity in list
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter).GetIdentity() == id)
			return const_cast<CIdentity*>(&(*iter));
	}
	
	return NULL;
}

CIdentity* CIdentityList::GetIdentity(const CAddress& addr) const
{
	// Match the From address only
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		CAddress test((*iter).GetFrom(true));
		if (test == addr)
			return const_cast<CIdentity*>(&(*iter));
	}
	
	return NULL;
}

CIdentity* CIdentityList::GetIdentity(const CCalendarAddress& caladdr) const
{
	// Match the Calendar address first
	const cdstring& comp = caladdr.GetAddress();
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		CCalendarAddressList addrs;
		CCalendarAddress::FromIdentityText((*iter).GetCalendar(true), addrs);
		for(CCalendarAddressList::const_iterator iter2 = addrs.begin(); iter2 != addrs.end(); iter2++)
		{
			if ((*iter2)->GetAddress().compare(comp, true) == 0)
				return const_cast<CIdentity*>(&(*iter));
		}
	}
	
	// If calendar address is mailto, try to match a From address
	if (comp.compare_start(cMailtoURLScheme))
	{
		cdstring temp(comp, ::strlen(cMailtoURLScheme));
		CAddress addr(temp, caladdr.GetName());
		return GetIdentity(addr);
	}
	return NULL;
}

unsigned long CIdentityList::GetIndex(const cdstring& id) const
{
	// Look for identity in list
	unsigned long result = 0;
	for(const_iterator iter = begin(); iter != end(); iter++, result++)
	{
		if ((*iter).GetIdentity() == id)
			break;
	}
	
	return result;
}

const CIdentity* CIdentityList::GetTiedMboxIdentity(const CMbox* mbox) const
{
	// Look in each identity
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter).HasTiedMailbox(mbox))
			return &(*iter);
	}

	// None found
	return NULL;
}

const CIdentity* CIdentityList::GetTiedMboxIdentity(const cdstring& mboxname) const
{
	// Look in each identity
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter).HasTiedMailbox(mboxname))
			return &(*iter);
	}

	// None found
	return NULL;
}

const CIdentity* CIdentityList::GetTiedCalIdentity(const calstore::CCalendarStoreNode* node) const
{
	// Look in each identity
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter).HasTiedCalendar(node))
			return &(*iter);
	}

	// None found
	return NULL;
}

const CIdentity* CIdentityList::GetTiedCalIdentity(const cdstring& calname) const
{
	// Look in each identity
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter).HasTiedCalendar(calname))
			return &(*iter);
	}

	// None found
	return NULL;
}

bool CIdentityList::SetTiedInfo(CIdentity::ETiedInfo type, char_stream& info, NumVersion vers_prefs)
{
	bool result = true;

	// Must have leading (
	if (!info.start_sexpression()) return false;

	// Read tied info until ')'
	for(iterator iter = begin(); iter != end(); iter++)
		result = result && (*iter).SetTiedInfo(type, info, vers_prefs);

	// Must end with )
	if (!info.end_sexpression()) return false;

	return result;
}

cdstring CIdentityList::GetTiedInfo(CIdentity::ETiedInfo type) const
{
	cdstring all;
	all += '(';

	for(const_iterator iter = begin(); iter != end(); iter++)
		all += (*iter).GetTiedInfo(type);

	all += ')';

	return all;
}
