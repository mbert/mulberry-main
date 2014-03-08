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


// Source for CFilterManager class

#include "CFilterManager.h"

#include "CAdminLock.h"
#include "CErrorDialog.h"
#include "CFilterProtocol.h"
#include "CLocalCommon.h"
#include "CManageSIEVEAccount.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMessage.h"
#include "CPreferenceKeys.h"
#include "CPreferenceValue.h"
#include "CPreferences.h"
#include "CRFC822.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif
#include "char_stream.h"
#include "cdfstream.h"

#include <algorithm>
#include <memory>

std::strstream* CFilterManager::sStrLog = NULL;

CFilterManager::CFilterManager()
{
	mUseLog.Value() = false;
	mIsDirty = false;
}

CFilterManager::CFilterManager(const CFilterManager& copy)
{
	mUseLog = copy.mUseLog;
	mLocalFilters = copy.mLocalFilters;
	mSIEVEFilters = copy.mSIEVEFilters;

	// Must use convert function to ensure function pointers are not stale
	mLocalTargets = copy.mLocalTargets;
	for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
		(*iter)->ConvertFilters(this, false);

	// Must use convert function to ensure function pointers are not stale
	mSIEVEScripts = copy.mSIEVEScripts;
	for(CFilterScriptList::iterator iter = mSIEVEScripts.begin(); iter != mSIEVEScripts.end(); iter++)
		(*iter)->ConvertFilters(this, false);

	mIsDirty = copy.mIsDirty;
}

CFilterManager::~CFilterManager()
{
}

void CFilterManager::PrefsInit()
{
	// If targets are empty add some defaults
	if (!GetTargets(CFilterItem::eLocal).size())
	{
		{
			GetTargets(CFilterItem::eLocal).push_back(new CTargetItem);
			CTargetItem* target = GetTargets(CFilterItem::eLocal).back();
			target->SetEnabled(true);
			target->SetName("All incoming mail");
			target->SetSchedule(CTargetItem::eIncomingMailbox);
			CFilterTargetList* list = new CFilterTargetList;
			target->SetTargets(list);
			list->push_back(new CFilterTarget(CFilterTarget::eCabinet, cdstring("#0")));
		}
		{
			GetTargets(CFilterItem::eLocal).push_back(new CTargetItem);
			CTargetItem* target = GetTargets(CFilterItem::eLocal).back();
			target->SetEnabled(true);
			target->SetName("When opening a mailbox");
			target->SetSchedule(CTargetItem::eIncomingMailbox);
			CFilterTargetList* list = new CFilterTargetList;
			target->SetTargets(list);
			list->push_back(new CFilterTarget(CFilterTarget::eAny));
		}
	}
}

void CFilterManager::ExecuteList(const CFilterItems* filters, CMbox* mbox, const ulvector* selected, const CSearchItem* andit) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	ulvector exclude;
	for(CFilterItems::const_iterator iter = filters->begin(); iter != filters->end(); iter++)
		(*iter)->Execute(mbox, selected, andit, exclude);
}

void CFilterManager::ExecuteManual(CMbox* mbox, const ulvector* selected, long index) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	ulvector exclude;
	long apply_index = 0;
	for(CFilterItemList::const_iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
	{
		if ((*iter)->GetManual())
		{
			if ((index < 0) || (apply_index == index))
				(*iter)->Execute(mbox, selected, NULL, exclude);
			apply_index++;
		}
	}
}

void CFilterManager::ExecuteManual(CMboxList& mboxes) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	// Execute against each mailbox
	for(CMboxList::iterator iter1 = mboxes.begin(); iter1 != mboxes.end(); iter1++)
	{
		// Start file log session
		mLog.StartLog(CLog::eLogFilters);
		if (mLog.DoLog())
		{
			cdstring why("Filters applied to mailbox: ");
			why += static_cast<CMbox*>(*iter1)->GetAccountName();
			mLog.AddEntry(why);
		}

		ulvector exclude;
		for(CFilterItemList::const_iterator iter2 = mLocalFilters.begin(); iter2 != mLocalFilters.end(); iter2++)
		{
			if ((*iter2)->GetManual())
				(*iter2)->Execute(static_cast<CMbox*>(*iter1), NULL, NULL, exclude);
		}

		// Stop file log session
		mLog.StopLog();
	}
}

const CFilterItems* CFilterManager::MatchFilters(CTargetItem::ESchedule schedule, const CMbox* mbox) const
{
	// Find names of matching targets
	CFilterItems* list = new CFilterItems;
	for(CTargetItemList::const_iterator iter1 = mLocalTargets.begin(); iter1 != mLocalTargets.end(); iter1++)
	{
		if ((*iter1)->IsEnabled() && (*iter1)->Match(schedule, mbox))
		{
			// Find filters that include matched targets
			for(CFilterItems::const_iterator iter2 = (*iter1)->GetFilters().begin(); iter2 != (*iter1)->GetFilters().end(); iter2++)
			{
				CFilterItems::const_iterator found = std::find(list->begin(), list->end(), *iter2);
				if (found == list->end())
					list->push_back(*iter2);
			}
		}
	}
	
	return list;
}

void CFilterManager::OpenMailbox(CMbox* mbox) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Force new mail check filter if unseen count non-zero
	if (mbox->GetNumberUnseen())
		NewMailMailbox(mbox, 0);

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	// Now do open filter
	std::auto_ptr<const CFilterItems> list(MatchFilters(CTargetItem::eOpenMailbox, mbox));

	// Only bother if filters exist
	if (list->size())
	{
		// Start file log session
		mLog.StartLog(CLog::eLogFilters);
		if (mLog.DoLog())
		{
			cdstring why("Filtering open mailbox: ");
			why += mbox->GetAccountName();
			mLog.AddEntry(why);
		}

		ExecuteList(list.get(), mbox, NULL, NULL);

		// Stop file log session
		mLog.StopLog();
	}
}

void CFilterManager::NewMailMailbox(CMbox* mbox, unsigned long count) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// The mailbox must be fully open to process rules. This method can be called as a result of
	// a background mail check when the mailbox is not open, so we must ignore rules.
	if (!mbox->IsFullOpen())
		return;

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	// Get matching filters
	std::auto_ptr<const CFilterItems> list(MatchFilters(CTargetItem::eIncomingMailbox, mbox));

	// Only bother if filters exist
	if (list->size())
	{
		// When opening, filter only 'new' mail: unseen undeleted undraft
		CSearchItemList* items = new CSearchItemList;
		items->push_back(new CSearchItem(CSearchItem::eUnseen));
		items->push_back(new CSearchItem(CSearchItem::eUndeleted));
		items->push_back(new CSearchItem(CSearchItem::eUndraft));
		std::auto_ptr<CSearchItem> ands(new CSearchItem(CSearchItem::eAnd, items));


		// Make sure only the new ones are tested
		if (count)
		{
			// Determine which messages need to have a UID associated with them
			ulvector matched_uids;
			bool got_all_uids = true;
			for(unsigned long ctr = mbox->GetNumberFound() - count + 1; got_all_uids && (ctr <= mbox->GetNumberFound()); ctr++)
			{
				if (mbox->GetMessage(ctr)->GetUID() == 0)
					got_all_uids = false;
				else
					matched_uids.push_back(mbox->GetMessage(ctr)->GetUID());
			}

			// Do search for missing UIDs
			if (!got_all_uids)
			{
				// Determine range of seq to search for
				ulvector temp;
				temp.reserve(count);
				for(unsigned long i = 1; i <= count; i++)
					temp.push_back(mbox->GetNumberFound() - count + i);
				CSearchItem criteria(CSearchItem::eNumber, temp);

				// Do search using criteria
				mbox->Search(&criteria, &matched_uids, true, true);
			}

			// Add list of uids as a search criteria item
			items->push_back(new CSearchItem(CSearchItem::eUID, matched_uids));
		}

		// Start file log session
		mLog.StartLog(CLog::eLogFilters);
		if (mLog.DoLog())
		{
			cdstring why(count ? "Filtering check mailbox: " : "Filtering check mailbox on open: ");
			why += mbox->GetAccountName();
			mLog.AddEntry(why);
			
			if (count)
			{
				why = "New messages: ";
				why += cdstring(count);
			}

			mLog.AddEntry(why);
		}

		ExecuteList(list.get(), mbox, NULL, ands.get());

		// Stop file log session
		mLog.StopLog();
	}
}

void CFilterManager::CloseMailbox(CMbox* mbox) const
{
	// Only if rules allowed
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Lock to prevent changes whilst running
#ifdef __MULBERRY
	cdmutex::lock_cdmutex _lock(RunningLock());
#endif

	// Now do open filter
	std::auto_ptr<const CFilterItems> list(MatchFilters(CTargetItem::eCloseMailbox, mbox));

	// Only bother if filters exist
	if (list->size())
	{
		// Start file log session
		mLog.StartLog(CLog::eLogFilters);
		if (mLog.DoLog())
		{
			cdstring why("Filtering close mailbox: ");
			why += mbox->GetAccountName();
			mLog.AddEntry(why);
		}

		ExecuteList(list.get(), mbox, NULL, NULL);

		// Stop file log session
		mLog.StopLog();
	}
}

void CFilterManager::GetManualFilters(cdstrvect& list) const
{
	for(CFilterItemList::const_iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
	{
		if ((*iter)->GetManual())
			list.push_back((*iter)->GetName());
	}
}

unsigned long CFilterManager::GetManualIndex(unsigned long uid) const
{
	unsigned long index = 0;
	for(CFilterItemList::const_iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
	{
		if ((*iter)->GetManual())
		{
			if ((*iter)->GetUID() == uid)
				return index;
			index++;
		}
	}
	
	return -1;
}

unsigned long CFilterManager::GetManualUID(unsigned long index) const
{
	unsigned long match_index = 0;
	for(CFilterItemList::const_iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
	{
		if ((*iter)->GetManual())
		{
			if (match_index == index)
				return (*iter)->GetUID();
			match_index++;
		}
	}
	
	return -1;
}

const CFilterItem* CFilterManager::GetManualFilter(unsigned long uid) const
{
	for(CFilterItemList::const_iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
	{
		if ((*iter)->GetUID() == uid)
			return *iter;
	}
	
	return NULL;
}

void CFilterManager::ChangedFilters()
{
	// Inform listeners of possible global change to accounts
	Broadcast_Message(eBroadcast_RulesChanged, NULL);
	
	mIsDirty = true;
}

bool CFilterManager::DoLog() const
{
	return mUseLog.GetValue() || mLog.DoLog();
}

void CFilterManager::Log(const char* str)
{
	// Add to string log if required
	if (mUseLog.GetValue())
	{
		if (!sStrLog)
			sStrLog = new std::strstream;
		*sStrLog << str << os_endl;
	}

	// Add to file log
	mLog.AddEntry(str);
}

const CFilterItemList& CFilterManager::GetFilters(CFilterItem::EType type) const
{
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		return mLocalFilters;
	case CFilterItem::eSIEVE:
		return mSIEVEFilters;
	}
}

CFilterItemList& CFilterManager::GetFilters(CFilterItem::EType type)
{
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		return mLocalFilters;
	case CFilterItem::eSIEVE:
		return mSIEVEFilters;
	}
}

CFilterItem* CFilterManager::GetFilter(CFilterItem::EType type, const cdstring& name)
{
	// Look for named filter
	for(CFilterItemList::iterator iter = GetFilters(type).begin(); iter != GetFilters(type).end(); iter++)
	{
		if ((*iter)->GetName() == name)
			return *iter;
	}
	
	return NULL;
}

const CTargetItemList& CFilterManager::GetTargets(CFilterItem::EType type) const
{
	return mLocalTargets;
}

CTargetItemList& CFilterManager::GetTargets(CFilterItem::EType type)
{
	return mLocalTargets;
}

const CFilterScriptList& CFilterManager::GetScripts(CFilterItem::EType type) const
{
	return mSIEVEScripts;
}

CFilterScriptList& CFilterManager::GetScripts(CFilterItem::EType type)
{
	return mSIEVEScripts;
}

// Check whether rule is assigned to a target/script or manual
bool CFilterManager::RuleUsed(CFilterItem* rule)
{
	// Quick test for Apply Rules
	if (rule->GetManual())
		return true;
	
	// Check whether it appears in the relevant list
	if (rule->GetType() == CFilterItem::eLocal)
	{
		for(CTargetItemList::iterator iter = GetTargets(rule->GetType()).begin();
				iter != GetTargets(rule->GetType()).end(); iter++)
		{
			// Determine if this rule is in the trigger
			if ((*iter)->ContainsFilter(rule))
				return true;
		}
	}
	else
	{
		for(CFilterScriptList::iterator iter = GetScripts(rule->GetType()).begin();
				iter != GetScripts(rule->GetType()).end(); iter++)
		{
			// Determine if this rule is in the script
			if ((*iter)->ContainsFilter(rule))
				return true;
		}
	}

	// Not used
	return false;
}

// Add rule of the given type to the approriate list
void CFilterManager::AddRule(CFilterItem::EType type, CFilterItem* rule)
{
	// Add to appropriate list
	GetFilters(type).push_back(rule);

	// Indicate changed
	ChangedFilters();
}

// Changed a rule of the given type
void CFilterManager::ChangedRule(CFilterItem::EType type, CFilterItem* rule)
{
	// Indicate changed
	ChangedFilters();
}

// Remove rule of the given type from the appropriate list and any targets
void CFilterManager::RemoveRule(CFilterItem::EType type, CFilterItem* rule)
{
	// Find the rule
	CFilterItemList& list = GetFilters(type);
	CFilterItemList::iterator found = std::find(list.begin(), list.end(), rule);

	// Erase it if found
	if (found != list.end())
	{
		// Remove from all targets

		// Check whether it appears in the relevant list
		if (type == CFilterItem::eLocal)
		{
			for(CTargetItemList::iterator iter = GetTargets(type).begin();
					iter != GetTargets(type).end(); iter++)
			{
				(*iter)->RemoveFilter(rule);
			}
		}
		else
		{
			for(CFilterScriptList::iterator iter = GetScripts(type).begin();
					iter != GetScripts(type).end(); iter++)
			{
				(*iter)->RemoveFilter(rule);
			}
		}

		// Erase from list (will be automatically deleted)
		GetFilters(type).erase(found);
	}

	// Indicate changed
	ChangedFilters();
}

// Rules move by indices
void CFilterManager::MoveRules(const ulvector& movers, unsigned long dest, CFilterItem::EType type)
{
	// Copy ones being moved
	CFilterItemList list1;
	ulvector::const_iterator iter = movers.begin();
	for(unsigned long i = 0; (i < GetFilters(type).size()) && (iter != movers.end()); i++)
	{
		if (i == *iter)
		{
			list1.push_back(GetFilters(type).at(i));
			iter++;
		}
	}

	// Copy ones not being moved and also insert moved at right place
	CFilterItemList list2;
	iter = movers.begin();
	unsigned long i = 0;
	for(; i < GetFilters(type).size(); i++)
	{
		if (i == dest)
		{
			list2.insert(list2.end(), list1.begin(), list1.end());
			for(unsigned long j = 0; j < list1.size(); j++)
				list1.at(j) = NULL;
		}

		if ((iter == movers.end()) || (i != *iter))
			list2.push_back(GetFilters(type).at(i));
		else if (iter != movers.end())
			iter++;
	}
	if (i == dest)
	{
		list2.insert(list2.end(), list1.begin(), list1.end());
		for(unsigned long j = 0; j < list1.size(); j++)
			list1.at(j) = NULL;
	}
	
	// Copy back
	for(unsigned long i = 0; i < GetFilters(type).size(); i++)
	{
		GetFilters(type).at(i) = list2.at(i);
		list2.at(i) = NULL;
	}
	
	// Broadcast change
	ChangedFilters();
}

// Rules move by indices
void CFilterManager::MoveTargets(const ulvector& movers, unsigned long dest, CFilterItem::EType type)
{
	// Copy ones being moved
	CTargetItemList list1;
	ulvector::const_iterator iter = movers.begin();
	for(unsigned long i = 0; (i < GetTargets(type).size()) && (iter != movers.end()); i++)
	{
		if (i == *iter)
		{
			list1.push_back(GetTargets(type).at(i));
			iter++;
		}
	}

	// Copy ones not being moved and also insert moved at right place
	CTargetItemList list2;
	iter = movers.begin();
	unsigned long i = 0;
	for(; i < GetTargets(type).size(); i++)
	{
		if (i == dest)
		{
			list2.insert(list2.end(), list1.begin(), list1.end());
			for(unsigned long j = 0; j < list1.size(); j++)
				list1.at(j) = NULL;
		}

		if ((iter == movers.end()) || (i != *iter))
			list2.push_back(GetTargets(type).at(i));
		else if (iter != movers.end())
			iter++;
	}
	if (i == dest)
	{
		list2.insert(list2.end(), list1.begin(), list1.end());
		for(unsigned long j = 0; j < list1.size(); j++)
			list1.at(j) = NULL;
	}
	
	// Copy back
	for(unsigned long i = 0; i < GetTargets(type).size(); i++)
	{
		GetTargets(type).at(i) = list2.at(i);
		list2.at(i) = NULL;
	}
}

// Check validity of mailboxes in actions
bool CFilterManager::CheckActions(CFilterItem::EType type, cdstring& errs) const
{
	// Get list to check
	const CFilterItemList* check = NULL;
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		check = &mLocalFilters;
		break;
	case CFilterItem::eSIEVE:
		// Nothing to check in sieve
		return true;
	}
	
	// Check each filter
	for(CFilterItemList::const_iterator iter = check->begin(); iter != check->end(); iter++)
	{
		// Add name if it fails
		if (!(*iter)->CheckActions())
		{
			if (!errs.empty())
				errs += os_endl;
			errs += (*iter)->GetName();
		}
	}

	return errs.empty();	
}

// Rename account
void CFilterManager::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	// Change references to this account
	for(CFilterItemList::iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
		(*iter)->RenameAccount(old_acct, new_acct);

	// Change references to this account
	for(CFilterItemList::iterator iter = mSIEVEFilters.begin(); iter != mSIEVEFilters.end(); iter++)
		(*iter)->RenameAccount(old_acct, new_acct);

	// Change references to this account
	for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
		(*iter)->RenameAccount(old_acct, new_acct);
}

// Delete account
void CFilterManager::DeleteAccount(const cdstring& old_acct)
{
	// Change references to this account
	for(CFilterItemList::iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
		(*iter)->DeleteAccount(old_acct);

	// Change references to this account
	for(CFilterItemList::iterator iter = mSIEVEFilters.begin(); iter != mSIEVEFilters.end(); iter++)
		(*iter)->DeleteAccount(old_acct);

	// Change references to this account
	for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
		(*iter)->DeleteAccount(old_acct);
}

// Identity change
void CFilterManager::RenameIdentity(const cdstring& old_id, const cdstring& new_id)
{
	// Change references to this identity
	for(CFilterItemList::iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
		(*iter)->RenameIdentity(old_id, new_id);

	// Change references to this identity
	for(CFilterItemList::iterator iter = mSIEVEFilters.begin(); iter != mSIEVEFilters.end(); iter++)
		(*iter)->RenameIdentity(old_id, new_id);
}

// Identity deleted
void CFilterManager::DeleteIdentity(const cdstring& old_id)
{
	// Change references to this identity
	for(CFilterItemList::iterator iter = mLocalFilters.begin(); iter != mLocalFilters.end(); iter++)
		(*iter)->DeleteIdentity(old_id);

	// Change references to this identity
	for(CFilterItemList::iterator iter = mSIEVEFilters.begin(); iter != mSIEVEFilters.end(); iter++)
		(*iter)->DeleteIdentity(old_id);
}

void CFilterManager::RenameFavourite(unsigned long index, const cdstring& new_name)
{
	// Change references to this cabinet
	for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
		(*iter)->RenameFavourite(index, new_name);
}

void CFilterManager::RemoveFavourite(unsigned long index)
{
	// Change references to this cabinet
	for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
		(*iter)->RemoveFavourite(index);
}

void CFilterManager::WriteSIEVEScript(const CFilterScript* script) const
{
	// Generate entire script
	cdstring scripttxt;
	script->GetSIEVEScript(scripttxt, eEndl_Auto);

	// Write to file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	StCreatorType creatortype('R*ch', 'TEXT');
#endif

	cdstring fname(CConnectionManager::sConnectionManager.GetSIEVEDirectory());
	::addtopath(fname, script->GetName());

	cdofstream fout(fname);
	fout << scripttxt.c_str();
}

void CFilterManager::UploadSIEVEScript(const CFilterScript* script, unsigned long acct_index, bool upload, bool activate) const
{
	// Must have at least one SIEVE account
	if ((CPreferences::sPrefs->mSIEVEAccounts.GetValue().size() == 0) ||
		(acct_index >= CPreferences::sPrefs->mSIEVEAccounts.GetValue().size()))
		return;

	// Get the script if uploading
	cdstring scripttxt;
	if (upload)
		script->GetSIEVEScript(scripttxt, eEndl_CRLF);
	
	// Create protocol (using first account in prefs for now)
	std::auto_ptr<CFilterProtocol> proto(new CFilterProtocol(CPreferences::sPrefs->mSIEVEAccounts.GetValue().at(acct_index)));
	try
	{
		// Logon
		proto->Open();
		proto->Logon();
		
		// Get script name
		cdstring sname(script->GetName());

		// Check upload/delete
		if (upload)
		{
			// Put script on server
			proto->PutScript(sname, scripttxt);
			
			// Make it the active one if required
			if (activate)
				proto->SetActive(sname);
		}
		else
			// Delete script on server
			proto->DeleteScript(sname);

		// Clean up
		proto->Logoff();
		proto->Close();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	// Clean-up done by auto-ptrs
}

// Write data to a stream
void CFilterManager::WriteToMap(COptionsMap* theMap, bool dirty_only, bool local, bool sieve)
{
	// Only write changes if that is required
	if (!dirty_only || mIsDirty)
	{
		// Write each list to the map using multiple entries
		if (local)
		{
			mLocalFilters.WriteToMapMulti(theMap, cRulesLocalFiltersSection, dirty_only);
			mLocalTargets.WriteToMapMulti(theMap, cRulesLocalTargetsSection, dirty_only);
		}
		if (sieve)
		{
			mSIEVEFilters.WriteToMapMulti(theMap, cRulesSIEVEFiltersSection, dirty_only);
			mSIEVEScripts.WriteToMapMulti(theMap, cRulesSIEVEScriptsSection, dirty_only);
		}
	}
	
	// Reset dirty flag
	mIsDirty = false;
}

// Read data from a stream
bool CFilterManager::ReadFromMap(COptionsMap* theMap, NumVersion& vers_prefs, bool local, bool sieve)
{
	// Read each list from the map using multiple entries
	if (local)
	{
		mLocalFilters.ReadFromMapMulti(theMap, cRulesLocalFiltersSection, vers_prefs);
		mLocalTargets.ReadFromMapMulti(theMap, cRulesLocalTargetsSection, vers_prefs);
		for(CTargetItemList::iterator iter = mLocalTargets.begin(); iter != mLocalTargets.end(); iter++)
			(*iter)->ConvertFilters(this);

	}

	if (sieve)
	{
		mSIEVEFilters.ReadFromMapMulti(theMap, cRulesSIEVEFiltersSection, vers_prefs);
		mSIEVEScripts.ReadFromMapMulti(theMap, cRulesSIEVEScriptsSection, vers_prefs);
		for(CFilterScriptList::iterator iter = mSIEVEScripts.begin(); iter != mSIEVEScripts.end(); iter++)
			(*iter)->ConvertFilters(this);
	}
	
	// Broadcast change
	ChangedFilters();
	
	// Reset dirty flag
	mIsDirty = false;

	return true;
}

	void 	WriteToMap(COptionsMap* theMap,
						bool dirty_only,
						const CFilterItemList& items);						// Write data to a stream

// Write data to a stream
void CFilterManager::WriteToMap(COptionsMap* theMap, CFilterItem::EType type, const CFilterItemList& items) const
{
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		items.WriteToMapMulti(theMap, cRulesLocalFiltersSection, false);
		break;
	case CFilterItem::eSIEVE:
		items.WriteToMapMulti(theMap, cRulesSIEVEFiltersSection, false);
		break;
	}
}

// Read data from a stream
bool CFilterManager::ReadFromMap(COptionsMap* theMap, NumVersion& vers_prefs, CFilterItem::EType type, CFilterItemList& items) const
{
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		items.ReadFromMapMulti(theMap, cRulesLocalFiltersSection, vers_prefs);
		break;
	case CFilterItem::eSIEVE:
		items.ReadFromMapMulti(theMap, cRulesSIEVEFiltersSection, vers_prefs);
		break;
	}
	
	return true;
}

// Merge rule sets
void CFilterManager::MergeRules(CFilterItem::EType type, const CFilterItemList& items)
{
	// Get list to merge into
	CFilterItemList* merge_into = NULL;
	switch(type)
	{
	case CFilterItem::eLocal:
	default:
		merge_into = &mLocalFilters;
		break;
	case CFilterItem::eSIEVE:
		merge_into = &mSIEVEFilters;
		break;
	}
	
	// Look for conflicting names
	bool name_conflict = false;
	for(CFilterItemList::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		CFilterItemList::const_iterator found = std::find_if(merge_into->begin(), merge_into->end(), CFilterItem::same_name(*iter));
		if (found != merge_into->end())
		{
			name_conflict = true;
			break;
		}
	}
	
	// Do warning for merge
	if (name_conflict)
	{
		// Ask user whether to merge new rules into existing ones, or whether to add
		// unique versions of the new rules
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::RulesMergeKeep",
																		"ErrorDialog::Btn::Cancel",
																		"ErrorDialog::Btn::RulesMergeReplace",
																		NULL,
																		"ErrorDialog::Text::RulesMerge");
		
		if (result == CErrorDialog::eBtn1)
		{
			// Give conflicting items a new name
			for(CFilterItemList::const_iterator iter = items.begin(); iter != items.end(); iter++)
			{
				unsigned long ctr = 1;
				cdstring oldname = (*iter)->GetName();

				CFilterItemList::const_iterator found = std::find_if(merge_into->begin(), merge_into->end(), CFilterItem::same_name(*iter));
				while((found != merge_into->end()) && (ctr < 100))
				{
					// Append .### to name
					cdstring newname = oldname;
					newname += ".";
					newname += cdstring(ctr++);
					const_cast<CFilterItem*>(*iter)->SetName(newname);
					
					// Check to see whether new name is unique, if not will loop around
					found = std::find_if(merge_into->begin(), merge_into->end(), CFilterItem::same_name(*iter));
				}
			}
		}
		else if (result == CErrorDialog::eBtn2)
			return;
	}
	
	// Do merge replacing ones with the same name
	for(CFilterItemList::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		// Look for conflicts
		CFilterItemList::const_iterator found = std::find_if(merge_into->begin(), merge_into->end(), CFilterItem::same_name(*iter));
		if (found != merge_into->end())
			// Replace original with new one
			(*found)->ReplaceWith(*iter);
		else
			// Append new one
			merge_into->push_back(new CFilterItem(**iter));
	}

	// Indicate changed
	ChangedFilters();
}
