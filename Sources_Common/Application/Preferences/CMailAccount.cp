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


// CMailAccount.cp

// Header file for class/structs that define mail accounts

#include "CMailAccount.h"

#include "char_stream.h"
#include "CIdentity.h"
#include "CINETCommon.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

#pragma mark ____________________________CLocalAccount

CMailAccount::CLocalAccount::CLocalAccount()
{
	mEndl = eEndl_Auto;
}

CMailAccount::CLocalAccount::CLocalAccount(const CLocalAccount& copy)
			: CAccountItems(copy)
{
	mWDs = copy.mWDs;
	mEndl = copy.mEndl;
}

int CMailAccount::CLocalAccount::equals(const CAccountItems& comp) const
{
	bool result = 0;
	try
	{
		const CLocalAccount& lcomp = dynamic_cast<const CLocalAccount&>(comp);

		result = (mWDs == lcomp.mWDs) && (mEndl == lcomp.mEndl);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return result;
}

#pragma mark ____________________________CIMAPAccount

CMailAccount::CIMAPAccount::CIMAPAccount()
{
	mDirDelim = 0;
	mAutoNamespace = false;		// Don't do auto namespace - use init namespace and require user to turn this on
	mInitNamespace = false;		// Only true when created via prefs dialog or first set of prefs
	mDisconnected = false;
}

CMailAccount::CIMAPAccount::CIMAPAccount(const CIMAPAccount& copy)
			: CLocalAccount(copy)
{
	mDirDelim = copy.mDirDelim;
	mAutoNamespace = copy.mAutoNamespace;
	mInitNamespace = copy.mInitNamespace;
	mDisconnected = copy.mDisconnected;
}

int CMailAccount::CIMAPAccount::equals(const CAccountItems& comp) const
{
	bool result = 0;
	try
	{
		const CIMAPAccount& icomp = dynamic_cast<const CIMAPAccount&>(comp);

		result = CLocalAccount::equals(comp) &&
					(mDirDelim == icomp.mDirDelim) &&
					(mAutoNamespace == icomp.mAutoNamespace) &&
					(mInitNamespace == icomp.mInitNamespace) &&
					(mDisconnected == icomp.mDisconnected);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return result;
}

#pragma mark ____________________________CPOP3Account

CMailAccount::CPOP3Account::CPOP3Account()
{
	mLeaveOnServer = false;
	mDoTimedDelete = false;
	mDeleteAfter = 0;
	mDeleteExpunged = false;
	mUseMaxSize = false;
	mMaxSize = 10;
	mUseAPOP = false;
}

CMailAccount::CPOP3Account::CPOP3Account(const CPOP3Account& copy)
			: CLocalAccount(copy)
{
	mLeaveOnServer = copy.mLeaveOnServer;
	mDoTimedDelete = copy.mDoTimedDelete;
	mDeleteAfter = copy.mDeleteAfter;
	mDeleteExpunged = copy.mDeleteExpunged;
	mUseMaxSize = copy.mUseMaxSize;
	mMaxSize = copy.mMaxSize;
	mUseAPOP = copy.mUseAPOP;
}

int CMailAccount::CPOP3Account::equals(const CAccountItems& comp) const
{
	bool result = 0;
	try
	{
		const CPOP3Account& pcomp = dynamic_cast<const CPOP3Account&>(comp);

		result = CLocalAccount::equals(comp) &&
					(mLeaveOnServer == pcomp.mLeaveOnServer) &&
					(mDoTimedDelete == pcomp.mDoTimedDelete) &&
					(mDeleteAfter == pcomp.mDeleteAfter) &&
					(mDeleteExpunged == pcomp.mDeleteExpunged) &&
					(mUseMaxSize == pcomp.mUseMaxSize) &&
					(mMaxSize == pcomp.mMaxSize) &&
					(mUseAPOP == pcomp.mUseAPOP);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return result;
}

#pragma mark ____________________________CMailAccount

CMailAccount::CMailAccount()
{
	mItems = NULL;
	mExpanded = false;
	mSubsHierarchic = false;
	mSubsExpanded = false;
	mTieIdentity = false;
}

CMailAccount::CMailAccount(const CMailAccount& copy) : CINETAccount(copy)
{
	mItems = NULL;
	_copy(copy);
}

void CMailAccount::_copy(const CMailAccount& copy)
{
	delete mItems;
	switch(copy.mServerType)
	{
	case eIMAP:
		mItems = new CIMAPAccount(*copy.GetIMAPAccount());
		break;
	case ePOP3:
		mItems = new CPOP3Account(*copy.GetPOP3Account());
		break;
	case eLocal:
		mItems = new CLocalAccount(*copy.GetLocalAccount());
		break;
	default:
		mItems = NULL;
		break;
	}

	mExpanded = copy.mExpanded;
	mSubsHierarchic = copy.mSubsHierarchic;
	mSubsExpanded = copy.mSubsExpanded;
	mTieIdentity = copy.mTieIdentity;
	mTiedIdentity = copy.mTiedIdentity;
	mFuture = copy.mFuture;
}

// Compare with same type (do not compare UIDs as these must be unique - only compare contents)
int CMailAccount::operator==(const CMailAccount& comp) const
{
	return CINETAccount::operator==(comp) &&
			mItems && mItems->equals(*comp.mItems) &&
			(mExpanded == comp.mExpanded) &&
			(mSubsHierarchic == comp.mSubsHierarchic) &&
			(mSubsExpanded == comp.mSubsExpanded) &&
			(mTieIdentity == comp.mTieIdentity) &&
			(mTiedIdentity == comp.mTiedIdentity);
}

// New account being created
void CMailAccount::NewAccount()
{
	// Add '*' search hierarchy if POP3 or Local
	if (IsPOP3() || IsLocal())
	{
		CDisplayItem wd(cWILDCARD);
		GetWDs().push_back(wd);
	}

	// Force IMAP to init NAMESPACE
	if (IsIMAP())
		SetInitNamespace(true);
}

void CMailAccount::SetServerType(EINETServerType type)
{
	CINETAccount::SetServerType(type);

	delete mItems;
	switch(mServerType)
	{
	case eIMAP:
		mItems = new CIMAPAccount;
		break;
	case ePOP3:
		mItems = new CPOP3Account;

		// Always login at startup
		mLogonAtStart = true;
		break;
	case eLocal:
		mItems = new CLocalAccount;

		// Always login at startup
		mLogonAtStart = true;
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
		break;
	default:
		mItems = NULL;
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
		break;
	}
}

char CMailAccount::GetDirDelim() const
{
	return IsIMAP() ? GetIMAPAccount()->mDirDelim : os_dir_delim;
}

CIdentity& CMailAccount::GetAccountIdentity() const
{
	if (mTieIdentity)
	{
		for(CIdentityList::const_iterator iter = CPreferences::sPrefs->mIdentities.GetValue().begin();
				iter != CPreferences::sPrefs->mIdentities.GetValue().end(); iter++)
		{
			if ((*iter).GetIdentity() == mTiedIdentity)
				return const_cast<CIdentity&>(*iter);
		}
	}

	return const_cast<CIdentity&>(CPreferences::sPrefs->mIdentities.GetValue().front());
}

const char* cEndlDescriptors[] = {"Auto", "CR", "LF", "CRLF", NULL};

cdstring CMailAccount::GetInfo() const
{
	// Create string list of items
	cdstring info;
	info += '(';
	info += CINETAccount::GetInfo();
	info += ')';

	info += '(';

	// Account specific part
	cdstring temp;
	switch(mServerType)
	{
	case eIMAP:
		temp = GetIMAPAccount()->mDirDelim;
		temp.quote();
		info += temp;
		info += cSpace;

		info += (GetIMAPAccount()->mAutoNamespace ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;

		// mInitNamespace is not written to prefs, its a runtime setting

		info += GetWDs().GetInfo();
		info += cSpace;

		info += (GetIMAPAccount()->mDisconnected ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;
		break;

	case ePOP3:
		info += GetWDs().GetInfo();
		info += cSpace;
		
		info += cEndlDescriptors[GetPOP3Account()->mEndl];
		info += cSpace;

		info += (GetPOP3Account()->mLeaveOnServer ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;

		info += (GetPOP3Account()->mDoTimedDelete ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;

		info += cdstring(GetPOP3Account()->mDeleteAfter);
		info += cSpace;

		info += (GetPOP3Account()->mDeleteExpunged ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;

		info += (GetPOP3Account()->mUseMaxSize ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;

		info += cdstring(GetPOP3Account()->mMaxSize);
		info += cSpace;

		info += (GetPOP3Account()->mUseAPOP ? cValueBoolTrue : cValueBoolFalse);
		info += cSpace;
		break;

	case eLocal:
		info += GetWDs().GetInfo();
		info += cSpace;
		
		info += cEndlDescriptors[GetLocalAccount()->mEndl];
		info += cSpace;
		break;
	default:;
	}

	// Account general part
	info += (mExpanded ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mSubsHierarchic ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mSubsExpanded ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mTieIdentity ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	temp = mTiedIdentity;
	temp.quote();
	temp.ConvertFromOS();
	info += temp;

	info += mFuture.GetInfo();

	info += ')';

	// Got it all
	return info;
}

bool CMailAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.start_sexpression();
		result = CINETAccount::SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	txt.start_sexpression();
		// Account specific part
		switch(mServerType)
		{
		case eIMAP:
			GetIMAPAccount()->mDirDelim = *txt.get();
			txt.get(GetIMAPAccount()->mAutoNamespace);
			// mInitNamespace is not read from prefs - its a runtime setting
			GetWDs().SetInfo(txt, vers_prefs);

			// >= v2.0a1
			if ((VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0))
			{
				txt.get(GetIMAPAccount()->mDisconnected);
				// < v2.0a2
				if (VersionTest(vers_prefs, VERS_2_0_0_A_2) < 0)
				{
					txt.start_sexpression();
						GetCWD().SetInfo(txt, vers_prefs);
					txt.end_sexpression();
				}
			}
			break;

		case ePOP3:
			// < v2.0a2
			if (VersionTest(vers_prefs, VERS_2_0_0_A_2) < 0)
			{
				txt.start_sexpression();
					GetCWD().SetInfo(txt, vers_prefs);
				txt.end_sexpression();
			}

			// >= v2.0a1
			if (VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
				GetWDs().SetInfo(txt, vers_prefs);

			// >= v2.0a6
			if (VersionTest(vers_prefs, VERS_2_0_0_A_6) >= 0)
			{
				char* p = txt.get();
				GetPOP3Account()->mEndl = static_cast<EEndl>(::strindexfind(p, cEndlDescriptors, eEndl_Auto));
			}

			txt.get(GetPOP3Account()->mLeaveOnServer);
			txt.get(GetPOP3Account()->mDoTimedDelete);
			txt.get(GetPOP3Account()->mDeleteAfter);
			txt.get(GetPOP3Account()->mDeleteExpunged);
			// >= v2.0a8
			if (VersionTest(vers_prefs, VERS_2_0_0_A_8) >= 0)
			{
				txt.get(GetPOP3Account()->mUseMaxSize);
				txt.get(GetPOP3Account()->mMaxSize);
			}
			// >= v2.0a2
			if (VersionTest(vers_prefs, VERS_2_0_0_A_2) >= 0)
				txt.get(GetPOP3Account()->mUseAPOP);
			break;

		case eLocal:
			// < v2.0a2
			if (VersionTest(vers_prefs, VERS_2_0_0_A_2) < 0)
			{
				txt.start_sexpression();
					GetCWD().SetInfo(txt, vers_prefs);
				txt.end_sexpression();
			}
			// >= v2.0a1
			if (VersionTest(vers_prefs, VERS_2_0_0_A_1) >= 0)
				GetWDs().SetInfo(txt, vers_prefs);

			// >= v2.0a6
			if (VersionTest(vers_prefs, VERS_2_0_0_A_6) >= 0)
			{
				char* p = txt.get();
				GetLocalAccount()->mEndl = static_cast<EEndl>(::strindexfind(p, cEndlDescriptors, eEndl_Auto));
			}

			break;
		default:;
		}

		// Account general part
		txt.get(mExpanded);
		txt.get(mSubsHierarchic);
		txt.get(mSubsExpanded);
		txt.get(mTieIdentity);
		txt.get(mTiedIdentity, true);

		mFuture.SetInfo(txt, vers_prefs);

	txt.end_sexpression();

	return result;
}
