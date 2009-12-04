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


// Header for CFilterTarget class

#ifndef __CFILTERTARGET_MULBERRY__
#define __CFILTERTARGET_MULBERRY__

#include "cdstring.h"
#include "CDataItem.h"
#include "prefsvector.h"

// Classes
class CFilterTarget;
typedef prefsptrvector<CFilterTarget> CFilterTargetList;

class char_stream;
class CMbox;

class CFilterTarget
{
public:
	enum ETarget
	{
		eNone,					// NULL
		eMailbox,				// cdstring
		eCabinet,				// cdstring
		eAccount,				// cdstring
		eAny,					// NULL
		eTargetLast
	};

	CFilterTarget()
		{ mTarget = eNone; mData = NULL; }
	CFilterTarget(ETarget target)
		{ mTarget = target; mData = NULL; }
	CFilterTarget(ETarget target, const cdstring& data)
		{ mTarget = target; mData = new CDataItem<cdstring>(data); }
	CFilterTarget(const CFilterTarget& copy)
		{ _copy(copy); }
	~CFilterTarget()
		{ _tidy(); }

	CFilterTarget& operator=(const CFilterTarget& copy)					// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	int operator==(const CFilterTarget& comp) const			// Compare with same type
		{ return (this == &comp); }

	bool Match(const CMbox* mbox) const;

	ETarget	GetTarget() const
		{ return mTarget; }
	void SetTarget(ETarget target)
		{ mTarget = target; }

	const CDataItemBase* GetData() const												// Get data
		{ return mData; }
	const CDataItem<cdstring>* GetStringData() const									// Get data
		{ return static_cast<CDataItem<cdstring>*>(mData); }
	CDataItem<cdstring>* GetStringData()												// Get data
		{ return static_cast<CDataItem<cdstring>*>(mData); }

	// Account management
	bool	RenameAccount(const cdstring& old_acct, const cdstring& new_acct);	// Rename account
	bool	DeleteAccount(const cdstring& old_acct);							// Delete account

	// Favourite Management
	bool	RenameFavourite(unsigned long index, const cdstring& new_name);	// Favourite renamed
	bool	RemoveFavourite(unsigned long index);							// Favourite removed

	cdstring GetInfo(void) const;											// Get text expansion for prefs
	bool SetInfo(char_stream& info, NumVersion vers_prefs);								// Convert text to items

protected:
	ETarget			mTarget;
	CDataItemBase*	mData;

private:
	void _copy(const CFilterTarget& copy);
	void _tidy();
};

#endif
