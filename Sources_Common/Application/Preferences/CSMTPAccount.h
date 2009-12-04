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


// CSMTPAccount.h

// Header file for class/structs that define SMTP accounts

#ifndef __CSMTPACCOUNT__MULBERRY__
#define __CSMTPACCOUNT__MULBERRY__

#include "CINETAccount.h"

#include "CFutureItems.h"

// Classes

class CSMTPAccount : public CINETAccount
{
public:
			CSMTPAccount();
			CSMTPAccount(const CSMTPAccount& copy) : CINETAccount(copy)
				{ _copy(copy); }
	virtual	~CSMTPAccount() {}

	CSMTPAccount& operator=(const CSMTPAccount& copy)				// Assignment with same type
		{ if (this != &copy) { CINETAccount::operator=(copy); _copy(copy); } return *this; }
	int operator==(const CSMTPAccount& comp) const;							// Compare with same type

	// Getters/setters
	bool	GetUseQueue() const
		{ return !mLogonAtStart; }
	void	SetUseQueue(bool use_queue)
		{ mLogonAtStart = !use_queue; }

	bool	GetHoldMessages() const
		{ return mCWD.IsHierarchic(); }
	void	SetHoldMessages(bool hold_messages)
		{ mCWD.SetHierarchic(hold_messages); }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	CFutureItems		mFuture;

private:
	void _copy(const CSMTPAccount& copy);
};

typedef ptrvector<CSMTPAccount> CSMTPAccountList;

#endif
