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


// CMailAccount.h

// Header file for class/structs that define mail accounts

#ifndef __CMAILACCOUNT__MULBERRY__
#define __CMAILACCOUNT__MULBERRY__

#include "CINETAccount.h"

#include "CDisplayItem.h"
#include "CFutureItems.h"

const char cMailAccountSeparator = ':';

// Classes

class CIdentity;

class CMailAccount : public CINETAccount
{

	class CAccountItems
	{
	public:
		CAccountItems() {}
		CAccountItems(const CAccountItems& copy) {}
		virtual ~CAccountItems() {}
		
		virtual int equals(const CAccountItems& comp) const = 0;
	};
	
	class CLocalAccount : public CAccountItems
	{
	public:
		CDisplayItemList	mWDs;
		EEndl				mEndl;

		CLocalAccount();
		CLocalAccount(const CLocalAccount& copy);
		virtual ~CLocalAccount() {}
		
		virtual int equals(const CAccountItems& comp) const;
	};
		
	class CIMAPAccount : public CLocalAccount
	{
	public:
		char				mDirDelim;
		bool				mAutoNamespace;
		bool				mInitNamespace;
		bool				mDisconnected;

		CIMAPAccount();
		CIMAPAccount(const CIMAPAccount& copy);
		virtual ~CIMAPAccount() {}
		
		virtual int equals(const CAccountItems& comp) const;
	};
	
	class CPOP3Account : public CLocalAccount
	{
	public:
		bool				mLeaveOnServer;
		bool				mDoTimedDelete;
		unsigned long		mDeleteAfter;
		bool				mDeleteExpunged;
		bool				mUseMaxSize;
		unsigned long		mMaxSize;
		bool				mUseAPOP;

		CPOP3Account();
		CPOP3Account(const CPOP3Account& copy);
		virtual ~CPOP3Account() {}
		
		virtual int equals(const CAccountItems& comp) const;
	};
		
public:
			CMailAccount();
			CMailAccount(const CMailAccount& copy);
	virtual	~CMailAccount()
		{ delete mItems; mItems = NULL; }

	CMailAccount& operator=(const CMailAccount& copy)				// Assignment with same type
		{ if (this != &copy) {CINETAccount::operator=(copy); _copy(copy);} return *this; }
	int operator==(const CMailAccount& comp) const;							// Compare with same type

	// Initialise
	virtual void NewAccount();									// New account being created

	// Getters/setters
	virtual void SetServerType(EINETServerType type);

	bool IsIMAP() const
		{ return mServerType == eIMAP; }
	bool IsPOP3() const
		{ return mServerType == ePOP3; }
	bool IsLocal() const
		{ return mServerType == eLocal; }
	bool IsLocalType() const
		{ return IsPOP3() || IsLocal(); }
	CIMAPAccount* GetIMAPAccount()
		{ return dynamic_cast<CIMAPAccount*>(mItems); }
	const CIMAPAccount* GetIMAPAccount() const
		{ return dynamic_cast<const CIMAPAccount*>(mItems); }
	CLocalAccount* GetLocalAccount()
		{ return dynamic_cast<CLocalAccount*>(mItems); }
	const CLocalAccount* GetLocalAccount() const
		{ return dynamic_cast<const CLocalAccount*>(mItems); }
	CPOP3Account* GetPOP3Account()
		{ return dynamic_cast<CPOP3Account*>(mItems); }
	const CPOP3Account* GetPOP3Account() const
		{ return dynamic_cast<const CPOP3Account*>(mItems); }

	// Account specific
	
	// case eIMAP:
	char	GetDirDelim() const;
	void	SetDirDelim(char dir_delim)
		{ if (IsIMAP()) GetIMAPAccount()->mDirDelim = dir_delim; }

	bool	GetAutoNamespace() const
		{ return IsIMAP() ? GetIMAPAccount()->mAutoNamespace : false; }
	void	SetAutoNamespace(bool auto_namespace)
		{ if (IsIMAP()) GetIMAPAccount()->mAutoNamespace = auto_namespace; }

	bool	GetInitNamespace() const
		{ return IsIMAP() ? GetIMAPAccount()->mInitNamespace : false; }
	void	SetInitNamespace(bool init_namespace)
		{ if (IsIMAP()) GetIMAPAccount()->mInitNamespace = init_namespace; }

	bool GetDisconnected() const
		{ return IsIMAP() ? GetIMAPAccount()->mDisconnected : false; }
	void SetDisconnected(bool disconnected)
		{ if (IsIMAP()) GetIMAPAccount()->mDisconnected = disconnected; }

	// case ePOP3:
	bool GetLeaveOnServer() const
		{ return IsPOP3() ? GetPOP3Account()->mLeaveOnServer : false; }
	void SetLeaveOnServer(bool leave)
		{ if (IsPOP3()) GetPOP3Account()->mLeaveOnServer = leave; }
	bool GetDoTimedDelete() const
		{ return IsPOP3() ? GetPOP3Account()->mDoTimedDelete : false; }
	void SetDoTimedDelete(bool timed)
		{ if (IsPOP3()) GetPOP3Account()->mDoTimedDelete = timed; }
	unsigned long GetDeleteAfter() const
		{ return IsPOP3() ? GetPOP3Account()->mDeleteAfter : 0; }
	void SetDeleteAfter(unsigned long after)
		{ if (IsPOP3()) GetPOP3Account()->mDeleteAfter = after; }
	bool GetDeleteExpunged() const
		{ return IsPOP3() ? GetPOP3Account()->mDeleteExpunged : false; }
	void SetDeleteExpunged(bool expunged)
		{ if (IsPOP3()) GetPOP3Account()->mDeleteExpunged = expunged; }
	bool GetUseMaxSize() const
		{ return IsPOP3() ? GetPOP3Account()->mUseMaxSize : false; }
	void SetUseMaxSize(bool use)
		{ if (IsPOP3()) GetPOP3Account()->mUseMaxSize = use; }
	unsigned long GetMaxSize() const
		{ return IsPOP3() ? GetPOP3Account()->mMaxSize : 0; }
	void SetMaxSize(unsigned long after)
		{ if (IsPOP3()) GetPOP3Account()->mMaxSize = after; }
	bool GetUseAPOP() const
		{ return IsPOP3() ? GetPOP3Account()->mUseAPOP : false; }
	void SetUseAPOP(bool use)
		{ if (IsPOP3()) GetPOP3Account()->mUseAPOP = use; }
	
	// case eLocal (includes ePOP3 & eIMAP):
	CDisplayItemList& GetWDs()
		{ return GetLocalAccount()->mWDs; }
	const CDisplayItemList& GetWDs() const
		{ return GetLocalAccount()->mWDs; }
	void SetWDs(const CDisplayItemList& wds)
		{ GetLocalAccount()->mWDs = wds; }
	
	EEndl GetEndl() const
		{ return GetLocalAccount()->mEndl; }
	void SetEndl(EEndl endl)
		{ GetLocalAccount()->mEndl = endl; }
	
	// Account general
	bool GetExpanded() const
		{ return mExpanded; }
	void SetExpanded(bool expanded)
		{ mExpanded = expanded; }
	
	bool GetSubsHierarchic() const
		{ return mSubsHierarchic; }
	void SetSubsHierarchic(bool subsHierarchic)
		{ mSubsHierarchic = subsHierarchic; }
	
	bool GetSubsExpanded() const
		{ return mSubsExpanded; }
	void SetSubsExpanded(bool subsExpanded)
		{ mSubsExpanded = subsExpanded; }
	
	bool GetTieIdentity() const
		{ return mTieIdentity; }
	void SetTieIdentity(bool first_id)
		{ mTieIdentity = first_id; }
	
	const cdstring& GetTiedIdentity() const
		{ return mTiedIdentity; }
	void SetTiedIdentity(const cdstring& id)
		{ mTiedIdentity = id; }
	
	CIdentity& GetAccountIdentity() const;

	virtual cdstring GetInfo() const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	CAccountItems*		mItems;
	bool				mExpanded;
	bool				mSubsHierarchic;
	bool				mSubsExpanded;
	bool				mTieIdentity;
	cdstring			mTiedIdentity;
	CFutureItems		mFuture;

private:
	void _copy(const CMailAccount& copy);
};

typedef ptrvector<CMailAccount> CMailAccountList;

#endif
