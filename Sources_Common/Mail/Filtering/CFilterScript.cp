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

#include "CFilterScript.h"

#include "CFilterItem.h"
#include "CFilterManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStringUtils.h"
#include "CTargetItem.h"

#include "char_stream.h"

#include <algorithm>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

CFilterScript::CFilterScript()
{
	mEnabled = true;
}

void CFilterScript::_copy(const CFilterScript& copy)
{
	mEnabled = copy.mEnabled;
	mName = copy.mName;
	mFilters = copy.mFilters;
}

void CFilterScript::_tidy()
{
}

bool CFilterScript::AddFilter(CFilterItem* filter)
{
	// See if duplicate
	CFilterItems::const_iterator found = std::find(mFilters.begin(), mFilters.end(), filter);
	if (found == mFilters.end())
	{
		mFilters.push_back(filter);
		return true;
	}
	else
		return false;
}

void CFilterScript::RemoveFilter(CFilterItem* filter)
{
	// See if it exists
	CFilterItems::iterator found = std::find(mFilters.begin(), mFilters.end(), filter);
	if (found != mFilters.end())
		mFilters.erase(found);
}

bool CFilterScript::ContainsFilter(CFilterItem* filter) const
{
	// See if it exists
	CFilterItems::const_iterator found = std::find(mFilters.begin(), mFilters.end(), filter);
	return found != mFilters.end();
}

void CFilterScript::GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const
{
	for(CFilterItems::const_iterator iter = mFilters.begin(); iter != mFilters.end(); iter++)
		(*iter)->GetSIEVEExtensions(ext);
}

void CFilterScript::GetSIEVEScript(cdstring& txt, EEndl line_end) const
{
	std::ostrstream out;

	out << "# SIEVE Script" << get_endl(line_end);
	out << "# Name: " << GetName() << get_endl(line_end);
	out << "# Date: " << CRFC822::GetRFC822Date() << get_endl(line_end);
	out << "# User-Agent : Mulberry " << CPreferences::sPrefs->GetVersionText() << get_endl(line_end);
	out << get_endl(line_end);

	// Determine which SIEVE extensions are used
	CFilterProtocol::EExtension ext = CFilterProtocol::eNone;
	GetSIEVEExtensions(ext);

	if (ext != CFilterProtocol::eNone)
	{
		// We require reject and fileinto
		out << "# We may need these..." << get_endl(line_end);
		out << "require [";
		bool done_one = false;
		if (ext & CFilterProtocol::eReject)
		{
			if (done_one)
				out << ", ";
			out << "\"reject\"";
			done_one = true;
		}
		if (ext & CFilterProtocol::eFileInto)
		{
			if (done_one)
				out << ", ";
			out << "\"fileinto\"";
			done_one = true;
		}
		if (ext & CFilterProtocol::eVacation)
		{
			if (done_one)
				out << ", ";
			out << "\"vacation\"";
			done_one = true;
		}
		if (ext & CFilterProtocol::eRelational)
		{
			if (done_one)
				out << ", ";
			out << "\"relational\"";
			done_one = true;
		}
		if (ext & CFilterProtocol::eIMAP4Flags)
		{
			if (done_one)
				out << ", ";
			out << "\"imap4flags\"";
			done_one = true;
		}
		out << "];" << get_endl(line_end) << get_endl(line_end);
	}

	// Write out each rule
	unsigned long ctr = 1;
	for(CFilterItems::const_iterator iter = mFilters.begin(); iter != mFilters.end(); iter++, ctr++)
	{
		out << "# Rule " << cdstring(ctr) << ": " << (*iter)->GetName() << get_endl(line_end);
		(*iter)->GenerateSIEVEScript(out, line_end);
		out << get_endl(line_end);
	}

	out << "# SIEVE Script ends here" << get_endl(line_end);
	out << get_endl(line_end);
	out << std::ends;
	
	txt.steal(out.str());
}	

void CFilterScript::ConvertFilters(CFilterManager* mgr, bool delete_existing)
{
	bool is_sieve = (dynamic_cast<CTargetItem*>(this) == NULL);

	// Convert to filters
	for(CFilterItems::iterator iter = mFilters.begin(); iter != mFilters.end(); )
	{
		// Create temp filter
		
		// Look for matching filter
		CFilterItem* filter = mgr->GetFilter(is_sieve ? CFilterItem::eSIEVE : CFilterItem::eLocal, (*iter)->GetName());
		if (delete_existing)
			delete *iter;
		if (filter)
			*iter = filter;
		else
		{
			iter = mFilters.erase(iter);
			continue;
		}
		
		iter++;
	}
}	

// Get text expansion for prefs
cdstring CFilterScript::GetInfo(void) const
{
	cdstring all;
	all += (mEnabled ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	// Get names to list
	cdstrvect names;
	for(CFilterItems::const_iterator iter = mFilters.begin(); iter != mFilters.end(); iter++)
		names.push_back((*iter)->GetName());
	temp.CreateSExpression(names);
	temp.ConvertFromOS();
	all += temp;

	return all;
}

// Convert text to items
bool CFilterScript::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mEnabled);
	txt.get(mName, true);

	// Get filter names from stream
	cdstrvect names;
	cdstring::ParseSExpression(txt, names, true);

	bool is_sieve = (dynamic_cast<CTargetItem*>(this) == NULL);

	// Convert to filters
	for(cdstrvect::const_iterator iter = names.begin(); iter != names.end(); iter++)
	{
		// Create temp filter
		CFilterItem* temp = new CFilterItem;
		temp->SetName(*iter);
		mFilters.push_back(temp);
	}

	return true;
}
