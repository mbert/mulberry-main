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


// COptionsAccount.cp

// Header file for class/structs that define preference accounts

#include "COptionsAccount.h"

#include "char_stream.h"
#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

COptionsAccount::COptionsAccount()
{
	mServerType = eIMSP;
}

void COptionsAccount::_copy(const COptionsAccount& copy)
{
	mBaseRURL = copy.mBaseRURL;
}

COptionsAccount::~COptionsAccount()
{
}

// Compare with same type
int COptionsAccount::operator==(const COptionsAccount& comp) const
{
	return CINETAccount::operator==(comp) && (mBaseRURL == comp.mBaseRURL);
}

void COptionsAccount::SetServerType(EINETServerType type)
{
	// Add new
	mServerType = type;
	switch(mServerType)
	{
	case eIMSP:
	case eACAP:
	case eWebDAVPrefs:
	default:
		GetAuthenticator().ResetAuthenticatorType(CAuthenticator::ePlainText);
		break;
	}
}

cdstring COptionsAccount::GetInfo(void) const
{
	cdstring result = CINETAccount::GetInfo();
	result += cSpace;
	
	cdstring temp = mBaseRURL;
	temp.quote();
	result += temp;
	return result;
}

bool COptionsAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = CINETAccount::SetInfo(txt, vers_prefs);
	if (mFuture.size() > 0)
	{
		mBaseRURL = mFuture.front();
		mFuture.erase(mFuture.begin(), mFuture.begin() + 1);
	}

	return result;
}
