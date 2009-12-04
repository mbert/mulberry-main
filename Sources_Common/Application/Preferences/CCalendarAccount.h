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


// CCalendarAccount.h

// Header file for class/structs that define calendar book accounts

#ifndef __CCALENDARACCOUNT__MULBERRY__
#define __CCALENDARACCOUNT__MULBERRY__

#include "CINETAccount.h"
#include "ptrvector.h"

#include "CDisplayItem.h"
#include "CFutureItems.h"

// Classes
class CCalendarAccount;
typedef ptrvector<CCalendarAccount> CCalendarAccountList;

class CIdentity;

class CCalendarAccount : public CINETAccount
{
public:
			CCalendarAccount();
			CCalendarAccount(const CCalendarAccount& copy) : CINETAccount(copy)
				{ _copy(copy); }
	virtual ~CCalendarAccount();

			CCalendarAccount& operator=(const CCalendarAccount& copy)					// Assignment with same type
				{ if (this != &copy) {CINETAccount::operator=(copy); _copy(copy);} return *this; }
	int operator==(const CCalendarAccount& comp) const;								// Compare with same type

	// Getters/setters
	virtual void SetServerType(EINETServerType type);

	bool GetDisconnected() const
		{ return mDisconnected; }
	void SetDisconnected(bool disconnected)
		{ mDisconnected = disconnected; }

	const cdstring& GetBaseRURL() const
	{
		return mBaseRURL;
	}
	void SetBaseRURL(const cdstring& rurl)
	{
		mBaseRURL = rurl;
	}

	bool GetTieIdentity() const
		{ return mTieIdentity; }
	void SetTieIdentity(bool first_id)
		{ mTieIdentity = first_id; }
	
	const cdstring& GetTiedIdentity() const
		{ return mTiedIdentity; }
	void SetTiedIdentity(const cdstring& id)
		{ mTiedIdentity = id; }
	
	CIdentity& GetAccountIdentity() const;

	CDisplayItemList& GetWDs()
		{ return mWDs; }
	const CDisplayItemList& GetWDs() const
		{ return mWDs; }
	void SetWDs(const CDisplayItemList& wds)
		{ mWDs = wds; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	bool					mDisconnected;
	cdstring				mBaseRURL;
	bool					mTieIdentity;
	cdstring				mTiedIdentity;
	CDisplayItemList		mWDs;
	CFutureItems			mFuture;
	
	void _copy(const CCalendarAccount& copy);
};

#endif
