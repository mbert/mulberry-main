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


// Header for CFilterManager class

#ifndef __CFILTERMANAGER__MULBERRY__
#define __CFILTERMANAGER__MULBERRY__

#include "CBroadcaster.h"

#include "CFilterItem.h"
#include "CTargetItem.h"
#include "CFilterScript.h"

#include "CPreferenceValue.h"
#include "CLog.h"
#ifdef __MULBERRY
#include "cdmutex.h"
#endif
#include <strstream>

// Classes
class CMbox;
class CMboxList;
class COptionsMap;
class CSearchItem;

class CFilterManager : public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_RulesChanged = 'rulc',
		eBroadcast_RulesSelectionChanged = 'ruls'
	};

	CFilterManager();
	CFilterManager(const CFilterManager& copy);
	~CFilterManager();

	void PrefsInit();

#ifdef __MULBERRY
	cdmutex& RunningLock() const
		{ return mRunning; }
#endif

	void ExecuteList(const CFilterItems* filters, CMbox* mbox, const ulvector* selected, const CSearchItem* andit) const;
	void ExecuteManual(CMbox* mbox, const ulvector* selected, long index = -1) const;
	void ExecuteManual(CMboxList& mboxes) const;

	const CFilterItems* MatchFilters(CTargetItem::ESchedule schedule, const CMbox* mbox) const;

	void OpenMailbox(CMbox* mbox) const;
	void NewMailMailbox(CMbox* mbox, unsigned long count) const;
	void CloseMailbox(CMbox* mbox) const;

	void GetManualFilters(cdstrvect& list) const;
	unsigned long GetManualIndex(unsigned long uid) const;
	unsigned long GetManualUID(unsigned long index) const;
	const CFilterItem* GetManualFilter(unsigned long uid) const;

	void ChangedFilters();

	const CPreferenceValueMap<bool>& UseLog() const
		{ return mUseLog; }
	CPreferenceValueMap<bool>& UseLog()
		{ return mUseLog; }

	bool DoLog() const;
	void Log(const char* str);

	const CFilterItemList& GetFilters(CFilterItem::EType type) const;
	CFilterItemList& GetFilters(CFilterItem::EType type);
	CFilterItem* GetFilter(CFilterItem::EType type, const cdstring& name);

	const CTargetItemList& GetTargets(CFilterItem::EType type) const;
	CTargetItemList& GetTargets(CFilterItem::EType type);

	const CFilterScriptList& GetScripts(CFilterItem::EType type) const;
	CFilterScriptList& GetScripts(CFilterItem::EType type);

	bool	RuleUsed(CFilterItem* rule);

	void	AddRule(CFilterItem::EType type, CFilterItem* rule);
	void	ChangedRule(CFilterItem::EType type, CFilterItem* rule);
	void	RemoveRule(CFilterItem::EType type, CFilterItem* rule);

	void	MoveRules(const ulvector& movers, unsigned long dest, CFilterItem::EType type);				// Rules move by indices
	void	MoveTargets(const ulvector& movers, unsigned long dest, CFilterItem::EType type);				// Rules targets by indices

	bool	CheckActions(CFilterItem::EType type, cdstring& errs) const;

	// Account management
	void	RenameAccount(const cdstring& old_acct, const cdstring& new_acct);						// Rename account
	void	DeleteAccount(const cdstring& old_acct);												// Delete account

	// Identity management
	void	RenameIdentity(const cdstring& old_id, const cdstring& new_id);							// Identity change
	void	DeleteIdentity(const cdstring& old_id);													// Identity deleted

	// Favourite Management
	void	RenameFavourite(unsigned long index, const cdstring& new_name);							// Favourite renamed
	void	RemoveFavourite(unsigned long index);													// Favourite removed

	void	WriteSIEVEScript(const CFilterScript* script) const;
	void	UploadSIEVEScript(const CFilterScript* script,
								unsigned long acct_index, bool upload, bool activate) const;

	cdstring GetInfo(void) const;											// Get text expansion for prefs
	void SetInfo(char_stream& info);										// Convert text to items

	void 	WriteToMap(COptionsMap* theMap,
						bool dirty_only,
						bool local = true, bool sieve = true);				// Write data to a stream

	bool	ReadFromMap(COptionsMap* theMap,
						NumVersion& vers_prefs,
						bool local = true, bool sieve = true);				// Read data from a stream

	void 	WriteToMap(COptionsMap* theMap,
						CFilterItem::EType type,
						const CFilterItemList& items) const;				// Write data to a stream

	bool	ReadFromMap(COptionsMap* theMap,
						NumVersion& vers_prefs,
						CFilterItem::EType type,
						CFilterItemList& items) const;						// Read data from a stream

	void	MergeRules(CFilterItem::EType type, const CFilterItemList& items); // Merge rule sets

private:
	CPreferenceValueMap<bool>	mUseLog;
	CFilterItemList				mLocalFilters;
	CFilterItemList				mSIEVEFilters;
	CTargetItemList				mLocalTargets;
	CFilterScriptList			mSIEVEScripts;
	bool						mIsDirty;
#ifdef __MULBERRY
	mutable cdmutex				mRunning;
#endif
	mutable CLog				mLog;
	static std::strstream*		sStrLog;
};

#endif
