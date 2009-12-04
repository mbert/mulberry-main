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


// CCalendarAccount.cp

// Header file for class/structs that define address book accounts

#include "CCalendarAccount.h"

#include "char_stream.h"
#include "CIdentity.h"
#include "CPreferences.h"
#include "CPreferenceVersions.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

CCalendarAccount::CCalendarAccount()
{
	mDisconnected = false;
	mTieIdentity = false;
}

void CCalendarAccount::_copy(const CCalendarAccount& copy)
{
	mDisconnected = copy.mDisconnected;
	mBaseRURL = copy.mBaseRURL;
	mTieIdentity = copy.mTieIdentity;
	mTiedIdentity = copy.mTiedIdentity;
	mWDs = copy.mWDs;
	mFuture = copy.mFuture;
}

CCalendarAccount::~CCalendarAccount()
{
	switch(mServerType)
	{
	default:
		break;
	}
}

// Compare with same type
int CCalendarAccount::operator==(const CCalendarAccount& comp) const
{
	return CINETAccount::operator==(comp) &&
			(mDisconnected == comp.mDisconnected) &&
			(mBaseRURL == comp.mBaseRURL) &&
			(mTieIdentity == comp.mTieIdentity) &&
			(mTiedIdentity == comp.mTiedIdentity) &&
			(mWDs == comp.mWDs);
}

void CCalendarAccount::SetServerType(EINETServerType type)
{
	// Delete existing
	switch(mServerType)
	{
	default:
		break;
	}

	// Add new
	mServerType = type;
	switch(mServerType)
	{
	case eWebDAVCalendar:
	default:
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::ePlainText);
		break;
	case eLocalCalendar:
		// Always login at startup, no auth
		mLogonAtStart = true;
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
		mDisconnected = false;
		break;
	}
}

CIdentity& CCalendarAccount::GetAccountIdentity() const
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

cdstring CCalendarAccount::GetInfo(void) const
{
	// Create string list of items
	cdstring info;
	info += '(';
	info += CINETAccount::GetInfo();
	info += ')';

	info += '(';
	info += mDisconnected ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	cdstring temp = mBaseRURL;
	temp.quote();
	info += temp;
	info += cSpace;

	info += (mTieIdentity ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	temp = mTiedIdentity;
	temp.quote();
	temp.ConvertFromOS();
	info += temp;
	info += cSpace;

	info += GetWDs().GetInfo();

	info += mFuture.GetInfo();
	info += ')';

	// Got it all
	return info;
}

bool CCalendarAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// If >= v1.4
	txt.start_sexpression();
	result = CINETAccount::SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	txt.start_sexpression();
	txt.get(mDisconnected);
	txt.get(mBaseRURL);
	txt.get(mTieIdentity);
	txt.get(mTiedIdentity, true);
	GetWDs().SetInfo(txt, vers_prefs);

	mFuture.SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	return result;
}
