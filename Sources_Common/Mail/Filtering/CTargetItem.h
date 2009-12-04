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


// Header for CTargetItem class

#ifndef __CTARGETITEM__MULBERRY__
#define __CTARGETITEM__MULBERRY__

#include "CFilterScript.h"

#include "cdstring.h"
#include "prefsvector.h"

#include "CFilterTarget.h"

// Classes
class CTargetItem;
typedef prefsptrvector<CTargetItem> CTargetItemList;

class CMbox;
class char_stream;

class CTargetItem : public CFilterScript
{
public:
	enum ESchedule
	{
		eNever = 0,

		// Mailbox schedules
		eIncomingMailbox,
		eOpenMailbox,
		eCloseMailbox,
		
		eAll,

		eScheduleLast
	};

	CTargetItem();
	CTargetItem(const CTargetItem& copy)
		: CFilterScript(copy)
		{ _copy1(copy); }
	virtual ~CTargetItem()
		{ _tidy1(); }

	CTargetItem& operator=(const CTargetItem& copy)							// Assignment with same type
		{ if (this != &copy) { CFilterScript::operator=(copy); _tidy1(); _copy1(copy); } return *this; }
	
	int operator==(const CTargetItem& comp) const			// Compare with same type
		{ return (this == &comp); }

	bool Match(ESchedule schedule, const CMbox* mbox) const;

	ESchedule	GetSchedule() const
		{ return mSchedule; }
	void SetSchedule(ESchedule schedule)
		{ mSchedule = schedule; }

	const CFilterTargetList* GetTargets() const
		{ return mTargets; }
	void SetTargets(CFilterTargetList* targets)
		{ delete mTargets; mTargets = targets; }

	// Account management
	void	RenameAccount(const cdstring& old_acct, const cdstring& new_acct);	// Rename account
	void	DeleteAccount(const cdstring& old_acct);							// Delete account

	// Favourite Management
	void	RenameFavourite(unsigned long index, const cdstring& new_name);	// Favourite renamed
	void	RemoveFavourite(unsigned long index);							// Favourite removed

	cdstring GetInfo(void) const;											// Get text expansion for prefs
	bool SetInfo(char_stream& info, NumVersion vers_prefs);					// Convert text to items

protected:
	ESchedule			mSchedule;
	CFilterTargetList*	mTargets;

private:
	void _copy1(const CTargetItem& copy);
	void _tidy1();
};

#endif
