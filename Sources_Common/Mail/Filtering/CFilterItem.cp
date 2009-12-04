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


// Source for CFilter class

#include "CFilterItem.h"

#include "CActionItem.h"
#include "CFilterManager.h"
#include "CMbox.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include "char_stream.h"

#include <algorithm>
#include <memory>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

static unsigned long sUID_Counter = 1;

CFilterItem::CFilterItem()
{
	mUID = sUID_Counter++;
	mType = eLocal;
	mManual = true;
	mUseScript = false;
	mCriteria = NULL;
	mActions = NULL;
	mStop = false;
}

CFilterItem::CFilterItem(EType type)
{
	mUID = sUID_Counter++;
	mType = type;
	mManual = false;
	mUseScript = false;
	mCriteria = NULL;
	mActions = NULL;
	mStop = false;
}

void CFilterItem::_copy(const CFilterItem& copy)
{
	mUID = copy.mUID;
	mType = copy.mType;
	mName = copy.mName;
	mManual = copy.mManual;
	mUseScript = copy.mUseScript;
	mCriteria = copy.mCriteria ? new CSearchItem(*copy.mCriteria) : NULL;
	mActions = copy.mActions ? new CActionItemList(*copy.mActions) : NULL;
	mScript = copy.mScript;
	mStop = copy.mStop;
}

void CFilterItem::_tidy()
{
	delete mCriteria;
	mCriteria = NULL;
	delete mActions;
	mActions = NULL;
}

void CFilterItem::SetUseScript(bool script)
{
	if (mType == eSIEVE)
	{
		// Convert SIEVE script
		if (mUseScript && !script)
		{
			mScript = cdstring::null_str;
			mUseScript = false;
		}
		else if (!mUseScript && script)
		{
			// Generate the current script
			std::ostrstream out;
			GenerateSIEVEScript(out, eEndl_Auto);
			out << std::ends;
			mScript = out.str();
			out.freeze(false);

			// Remove items
			SetCriteria(NULL);
			SetActions(NULL);

			mUseScript = true;
		}
	}
	else
		mUseScript = false;
}

// Make sure actions are valid
bool CFilterItem::CheckActions() const
{
	if (!mActions)
		return true;

	// Check each action
	for(CActionItemList::iterator iter = mActions->begin(); iter != mActions->end(); iter++)
	{
		// Return false if any one fails
		if (!(*iter)->CheckAction())
			return false;
	}
	
	return true;
}

// Rename account
void CFilterItem::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	if (!mActions)
		return;

	// Change references to this target
	for(CActionItemList::iterator iter = mActions->begin(); iter != mActions->end(); iter++)
		(*iter)->RenameAccount(old_acct, new_acct);
}

// Delete account
void CFilterItem::DeleteAccount(const cdstring& old_acct)
{
	if (!mActions)
		return;

	// Change references to this target
	for(CActionItemList::iterator iter = mActions->begin(); iter != mActions->end(); )
	{
		if ((*iter)->DeleteAccount(old_acct))
		{
			iter = mActions->erase(iter);
			continue;
		}
		
		iter++;
	}
}

// Identity change
void CFilterItem::RenameIdentity(const cdstring& old_id, const cdstring& new_id)
{
	if (!mActions)
		return;

	// Change references to this target
	for(CActionItemList::iterator iter = mActions->begin(); iter != mActions->end(); iter++)
		(*iter)->RenameIdentity(old_id, new_id);
}

// Identity deleted
void CFilterItem::DeleteIdentity(const cdstring& old_id)
{
	if (!mActions)
		return;

	// Change references to this target
	for(CActionItemList::iterator iter = mActions->begin(); iter != mActions->end(); )
	{
		if ((*iter)->DeleteIdentity(old_id))
		{
			iter = mActions->erase(iter);
			continue;
		}
		
		iter++;
	}
}

void CFilterItem::Execute(CMbox* mbox, const ulvector* selected, const CSearchItem* andit, ulvector& exclude)
{
	// Only if valid
	if (!mCriteria || !mActions)
		return;

	// Log
	if (CPreferences::sPrefs->GetFilterManager()->DoLog())
	{
		cdstring what("  Using filter: ");
		what += mName;
		CPreferences::sPrefs->GetFilterManager()->Log(what);
	}

	ulvector uids;

	// Look for selected only
	if (mCriteria->GetType() == CSearchItem::eSelected)
	{
		if ((selected == NULL) || selected->empty())
		{
			// Log
			if (CPreferences::sPrefs->GetFilterManager()->DoLog())
				CPreferences::sPrefs->GetFilterManager()->Log("  No matching messages");

			return;
		}

		// Use all selected ones
		uids = *selected;
	}
	else if (andit || (mCriteria->GetType() != CSearchItem::eAll) || !selected || !selected->size())
	{
		// Modified criteria
		std::auto_ptr<CSearchItem> criteria;

		// Intersect with selected messages
		if (selected && selected->size())
		{
			CSearchItemList* items = new CSearchItemList;
			items->push_back(new CSearchItem(CSearchItem::eUID, *selected));
			items->push_back(new CSearchItem(*mCriteria));
			criteria.reset(new CSearchItem(CSearchItem::eAnd, items));
		}
		
		// Intersect with additional criteria
		else if (andit)
		{
			CSearchItemList* items = new CSearchItemList;
			items->push_back(new CSearchItem(*andit));
			items->push_back(new CSearchItem(*mCriteria));
			criteria.reset(new CSearchItem(CSearchItem::eAnd, items));
		}
		
		// Do search using criteria
		mbox->Search(criteria.get() ? criteria.get() : mCriteria, &uids, true, true);
	}
	else
		// Use all selected ones
		uids = *selected;

	// Only bother if something found
	if (uids.empty())
	{
		// Log
		if (CPreferences::sPrefs->GetFilterManager()->DoLog())
			CPreferences::sPrefs->GetFilterManager()->Log("  No matching messages");

		return;
	}

	// Subtract excluded
	ulvector process;
	std::set_difference(uids.begin(), uids.end(), exclude.begin(), exclude.end(), std::back_inserter<ulvector>(process));
	
	// Only bother if something left
	if (process.empty())
	{
		// Log
		if (CPreferences::sPrefs->GetFilterManager()->DoLog())
			CPreferences::sPrefs->GetFilterManager()->Log("  No matching messages");

		return;
	}
	else if (CPreferences::sPrefs->GetFilterManager()->DoLog())
	{
		cdstring what("  Matched messages: ");
		what += cdstring((unsigned long) process.size());
		CPreferences::sPrefs->GetFilterManager()->Log(what);
		what = "  Matched message UIDs: ";
		for(ulvector::const_iterator iter = process.begin(); iter != process.end(); iter++)
		{
			if (iter != process.begin())
				what += ",";
			what += cdstring(*iter);
		}
		CPreferences::sPrefs->GetFilterManager()->Log(what);
		
	}

	// May need to force stop if error occurs
	bool stop_it = mStop;

	// Execute each action
	try
	{
		for(CActionItemList::const_iterator iter = mActions->begin(); iter != mActions->end(); iter++)
			(*iter)->Execute(mbox, process);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Catch all errors and stop further processing
		// Really this needs to know which messages suceeded and which failed but that's hard!
		
		stop_it = true;

		// Log
		if (CPreferences::sPrefs->GetFilterManager()->DoLog())
			CPreferences::sPrefs->GetFilterManager()->Log("  Error while processing matched messages");
	}
	
	// Stop further processing if matched
	if (stop_it)
	{
		ulvector temp;
		std::set_union(uids.begin(), uids.end(), exclude.begin(), exclude.end(), std::back_inserter<ulvector>(temp));
		exclude = temp;

		// Log
		if (CPreferences::sPrefs->GetFilterManager()->DoLog())
			CPreferences::sPrefs->GetFilterManager()->Log("  No more processing on matched messages");
	}
}

void CFilterItem::GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const
{
	if (mUseScript)
	{
		// Must assume that the user manually inserts the requires line
	}
	else
	{
		if (!mCriteria || !mActions)
			return;
		
		// Test each action
		for(CActionItemList::const_iterator iter = mActions->begin(); iter != mActions->end(); iter++)
			(*iter)->GetSIEVEExtensions(ext);
	}
}

void CFilterItem::GenerateSIEVEScript(std::ostream& out, EEndl line_end) const
{
	if (mUseScript)
	{
		out << "# Generated from text" << get_endl(line_end);
		out << mScript << get_endl(line_end);
	}
	else
	{
		if (!mCriteria || !mActions)
			return;

		out << "# Generated from GUI" << get_endl(line_end);

		// Write the criteria
		out << "if ";
		mCriteria->GenerateSIEVEScript(out);
		out << " {" << get_endl(line_end);
		for(CActionItemList::const_iterator iter = mActions->begin(); iter != mActions->end(); iter++)
		{
			out << "\t";
			(*iter)->GenerateSIEVEScript(out);
			out << get_endl(line_end);
		}
		if (mStop)
			out << "\tstop;" << get_endl(line_end);
		out << "}" << get_endl(line_end);
	}
}	

const char* cTypeDescriptors[] = {"Mulberry", "SIEVE", NULL};

// Get text expansion for prefs
cdstring CFilterItem::GetInfo(void) const
{
	cdstring all;
	cdstring temp = cTypeDescriptors[mType];
	temp.quote();
	all += temp;
	all += cSpace;

	temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mManual ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mUseScript ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	if (mCriteria)
		all += mCriteria->GetInfo();
	else
		all += "";
	all += cSpace;

	if (mActions)
		all += mActions->GetInfo();
	else
		all += "";
	all += cSpace;

	temp = mScript;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mStop ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Convert text to items
bool CFilterItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	char* p = txt.get();
	mType = static_cast<EType>(::strindexfind(p, cTypeDescriptors, eLocal));

	txt.get(mName, true);

	txt.get(mManual);
	
	txt.get(mUseScript);
	
	if (!mUseScript)
	{
		CSearchItem* criteria = new CSearchItem;
		criteria->SetInfo(txt);
		SetCriteria(criteria);
	}
	else
		SetCriteria(NULL);

	if (!mUseScript)
	{
		CActionItemList* actions = new CActionItemList;
		actions->SetInfo(txt, vers_prefs);
		SetActions(actions);
	}
	else
		SetActions(NULL);

	txt.get(mScript, true);
		
	txt.get(mStop);
	
	return true;
}

// Replace with details from another filter
void CFilterItem::ReplaceWith(const CFilterItem* replace)
{
	if (replace == NULL)
		return;

	// Copy important bits
	SetUseScript(replace->GetUseScript());
	SetCriteria(new CSearchItem(*replace->GetCriteria()));
	SetActions(new CActionItemList(*replace->GetActions()));
	SetScript(replace->GetScript());
	SetStop(replace->Stop());
}
