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


// Header for CFilterItem class

#ifndef __CFILTERITEM__MULBERRY__
#define __CFILTERITEM__MULBERRY__

#include "cdstring.h"
#include "prefsvector.h"

#include "CActionItem.h"
#include "CFilterProtocol.h"
#include "CSearchItem.h"

// Classes
class CFilterItem;
typedef prefsptrvector<CFilterItem> CFilterItemList;
typedef std::vector<CFilterItem*> CFilterItems;

class CMbox;
class char_stream;

class CFilterItem
{
public:
	// find_if functors
	class same_name
	{
	public:
		same_name(const CFilterItem* data) :
			mData(data) {}

		bool operator() (const CFilterItem* item) const
			{ return item && mData ? (item->GetName() == mData->GetName()) : false; }
	private:
		const CFilterItem* mData;
	};

	enum EType
	{
		eLocal = 0,
		eSIEVE
	};

	CFilterItem();
	CFilterItem(EType type);
	CFilterItem(const CFilterItem& copy)
		{ _copy(copy); }
	~CFilterItem()
		{ _tidy(); }

	CFilterItem& operator=(const CFilterItem& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }
	
	int operator==(const CFilterItem& comp) const			// Compare with same type
		{ return (this == &comp); }

	void Execute(CMbox* mbox, const ulvector* selected, const CSearchItem* andit, ulvector& exclude);

	unsigned long GetUID() const
	{
		return mUID;
	}

	EType GetType() const
		{ return mType; }
	void SetType(EType type)
		{ mType = type; }

	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }

	bool GetManual() const
		{ return mManual && (mType == eLocal); }
	void SetManual(bool manual)
		{ mManual = manual && (mType == eLocal); }

	bool GetUseScript() const
		{ return mUseScript && (mType == eSIEVE); }
	void SetUseScript(bool script);

	const CSearchItem* GetCriteria() const
		{ return mCriteria; }
	void SetCriteria(CSearchItem* criteria)
		{ delete mCriteria; mCriteria = criteria; }

	const CActionItemList* GetActions() const
		{ return mActions; }
	void SetActions(CActionItemList* actions)
		{ delete mActions; mActions = actions; }

	const cdstring& GetScript() const
		{ return mScript; }
	void SetScript(const cdstring& script)
		{ mScript = script; }

	bool Stop() const
		{ return mStop; }
	void SetStop(bool stop)
		{ mStop = stop; }

	bool	CheckActions() const;

	// Account management
	void	RenameAccount(const cdstring& old_acct, const cdstring& new_acct);	// Rename account
	void	DeleteAccount(const cdstring& old_acct);							// Delete account

	// Identity management
	void	RenameIdentity(const cdstring& old_id, const cdstring& new_id);				// Identity change
	void	DeleteIdentity(const cdstring& old_id);										// Identity deleted

	// SIEVE items
	void	GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const;
	void	GenerateSIEVEScript(std::ostream& out, EEndl line_end) const;

	// Reade/write prefs
	cdstring GetInfo(void) const;											// Get text expansion for prefs
	bool SetInfo(char_stream& info, NumVersion vers_prefs);					// Convert text to items

	void	ReplaceWith(const CFilterItem* replace);						// Replace with details from another filter

protected:
	unsigned long		mUID;
	EType				mType;
	cdstring			mName;
	bool				mManual;
	bool				mUseScript;
	CSearchItem*		mCriteria;
	CActionItemList*	mActions;
	cdstring			mScript;
	bool				mStop;

private:
	void _copy(const CFilterItem& copy);
	void _tidy();
};

#endif
