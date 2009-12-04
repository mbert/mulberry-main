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

#include "CTargetItem.h"

#include "CActionItem.h"
#include "CFilterManager.h"
#include "CMbox.h"
#include "CStringUtils.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

CTargetItem::CTargetItem()
{
	mSchedule = eIncomingMailbox;
	mTargets = NULL;
}

void CTargetItem::_copy1(const CTargetItem& copy)
{
	mSchedule = copy.mSchedule;
	mTargets = copy.mTargets ? new CFilterTargetList(*copy.mTargets) : NULL;
}

void CTargetItem::_tidy1()
{
	delete mTargets;
	mTargets = NULL;
}

bool CTargetItem::Match(ESchedule schedule, const CMbox* mbox) const
{
	// Schedules must match (or using All schedule) and must have targets
	if ((schedule != mSchedule) && (mSchedule != eAll) || !mTargets)
		return false;
	
	// Look at each target and see if it matches mailbox
	for(CFilterTargetList::const_iterator iter = mTargets->begin(); iter != mTargets->end(); iter++)
	{
		if ((*iter)->Match(mbox))
			return true;
	}
	
	return false;
}

// Rename account
void CTargetItem::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	if (!mTargets)
		return;

	// Change references to this target
	for(CFilterTargetList::iterator iter = mTargets->begin(); iter != mTargets->end(); iter++)
		(*iter)->RenameAccount(old_acct, new_acct);
}

// Delete account
void CTargetItem::DeleteAccount(const cdstring& old_acct)
{
	if (!mTargets)
		return;

	// Change references to this target
	for(CFilterTargetList::iterator iter = mTargets->begin(); iter != mTargets->end(); )
	{
		if ((*iter)->DeleteAccount(old_acct))
		{
			iter = mTargets->erase(iter);
			continue;
		}
		
		iter++;
	}
}

void CTargetItem::RenameFavourite(unsigned long index, const cdstring& new_name)
{
	if (!mTargets)
		return;

	// Change references to this cabinet
	for(CFilterTargetList::iterator iter = mTargets->begin(); iter != mTargets->end(); iter++)
		(*iter)->RenameFavourite(index, new_name);
}

void CTargetItem::RemoveFavourite(unsigned long index)
{
	if (!mTargets)
		return;

	// Remove references to this cabinet
	for(CFilterTargetList::iterator iter = mTargets->begin(); iter != mTargets->end(); )
	{
		if ((*iter)->RemoveFavourite(index))
		{
			iter = mTargets->erase(iter);
			continue;
		}
		
		iter++;
	}
}

const char* cScheduleDescriptors[] =
	{"None",
	 "Incoming",
	 "Open Mailbox",
	 "Close Mailbox",
	 "All",
	 NULL};

// Get text expansion for prefs
cdstring CTargetItem::GetInfo(void) const
{
	cdstring all = CFilterScript::GetInfo();
	all += cSpace;

	cdstring temp = cScheduleDescriptors[mSchedule];
	temp.quote();
	all += temp;
	all += cSpace;
	
	if (mTargets)
		all += mTargets->GetInfo();
	else
		all += "";

	return all;
}

// Convert text to items
bool CTargetItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	CFilterScript::SetInfo(txt, vers_prefs);

	char* p = txt.get();
	mSchedule = static_cast<ESchedule>(::strindexfind(p, cScheduleDescriptors, eNever));

	CFilterTargetList* targets = new CFilterTargetList;
	targets->SetInfo(txt, vers_prefs);
	SetTargets(targets);
	
	return true;
}
