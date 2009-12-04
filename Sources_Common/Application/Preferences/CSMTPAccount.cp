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


// CSMTPAccount.cp

// Header file for class/structs that define mail accounts

#include "CSMTPAccount.h"

#include "char_stream.h"

// Classes

#pragma mark ____________________________CSMTPAccount

CSMTPAccount::CSMTPAccount()
{
	// Start with anonymous authenticator
	GetAuthenticator().ResetAuthenticatorType(CAuthenticator::eNone);
}

void CSMTPAccount::_copy(const CSMTPAccount& copy)
{
	mFuture = copy.mFuture;
}

// Compare with same type
int CSMTPAccount::operator==(const CSMTPAccount& comp) const
{
	return CINETAccount::operator==(comp);
}

cdstring CSMTPAccount::GetInfo(void) const
{
	// Create string list of items
	cdstring info;
	info += '(';
	info += CINETAccount::GetInfo();
	info += ')';

	info += '(';
	// Empty for now - ESMTP may require some items here

	info += mFuture.GetInfo();
	info += ')';

	// Got it all
	return info;
}

bool CSMTPAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.start_sexpression();
		result = CINETAccount::SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	// Ignore - it will be empty
	if (txt.start_sexpression())
	{
		mFuture.SetInfo(txt, vers_prefs);
		txt.end_sexpression();
	}
	else
		mFuture.clear();

	return result;
}
