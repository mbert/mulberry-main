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


// Source for CFilterTarget class

#include "CFilterTarget.h"

#include "char_stream.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CStringUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

void CFilterTarget::_copy(const CFilterTarget& copy)
{
	mTarget = copy.mTarget;

	// Copy data based on type
	switch(mTarget)
	{
	case eNone:
	case eAny:
	default:
		mData = NULL;
		break;
	case eMailbox:
	case eCabinet:
	case eAccount:
		mData = new CDataItem<cdstring>(*static_cast<CDataItem<cdstring>*>(copy.mData));
		break;
	}
}

void CFilterTarget::_tidy()
{
	// Delete data based on type
	delete mData;
	mData = NULL;
}

bool CFilterTarget::Match(const CMbox* mbox) const
{
	switch(mTarget)
	{
	case eNone:
	default:
		return false;
	case eAny:
		return true;

	case eMailbox:
		return mbox->GetAccountName() == GetStringData()->GetData();

	case eCabinet:
	{
		// Get index of favourite item
		CMailAccountManager::EFavourite fav = CMailAccountManager::sMailAccountManager->GetFavouriteFromID(GetStringData()->GetData());

		// Test to see if mailbox is in cabinet
		return CMailAccountManager::sMailAccountManager->IsFavouriteItem(fav, mbox);
	}

	case eAccount:
		return mbox->GetProtocol()->GetAccountName() == GetStringData()->GetData();
	}
}

// Rename account
bool CFilterTarget::RenameAccount(const cdstring& old_acct, const cdstring& new_acct)
{
	if (mTarget == eMailbox)
	{
		cdstring old_copy = GetStringData()->GetData();
		if (!::strncmp(old_copy, old_acct, old_acct.length()) &&
			(old_copy.c_str()[old_acct.length()] == cMailAccountSeparator))
		{
			cdstring new_copy = new_acct;
			new_copy += &old_copy.c_str()[old_acct.length()];
			GetStringData()->GetData() = new_copy;
			return true;
		}
	}
	
	return false;
}

// Delete account
bool CFilterTarget::DeleteAccount(const cdstring& old_acct)
{
	if (mTarget == eMailbox)
	{
		cdstring old_copy = GetStringData()->GetData();
		if (!::strncmp(old_copy, old_acct, old_acct.length()) &&
			(old_copy.c_str()[old_acct.length()] == cMailAccountSeparator))
		{
			return true;
		}
	}
	
	return false;
}

bool CFilterTarget::RenameFavourite(unsigned long index, const cdstring& new_name)
{
	// Change references to this target
	if (mTarget == eCabinet)
	{
		if (CMailAccountManager::sMailAccountManager->GetFavouriteID(static_cast<CMailAccountManager::EFavourite>(index)) == GetStringData()->GetData())
		{
			cdstring new_id = cdstring("@") + new_name;
			GetStringData()->GetData() = new_id;
			return true;
		}
	}
	
	return false;
}

bool CFilterTarget::RemoveFavourite(unsigned long index)
{
	// Change references to this target
	if (mTarget == eCabinet)
	{
		if (CMailAccountManager::sMailAccountManager->GetFavouriteID(static_cast<CMailAccountManager::EFavourite>(index)) == GetStringData()->GetData())
			return true;
	}

	return false;
}

const char* cTargetDescriptors[] =
	{"None",
	 "Mailbox",
	 "Cabinet",
	 "Account",
	 "All",
	 NULL};

// Get text expansion for prefs
cdstring CFilterTarget::GetInfo(void) const
{
	cdstring info;
	cdstring temp = cTargetDescriptors[mTarget];
	temp.quote();
	info += temp;
	
	if (mData)
	{
		info += cSpace;
		info += mData->GetInfo();
	}
	
	return info;
}

// Convert text to items
bool CFilterTarget::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	char* p = txt.get();
	mTarget = static_cast<ETarget>(::strindexfind(p, cTargetDescriptors, eNone));

	switch(mTarget)
	{
	case eNone:
	case eAny:
	default:
		mData = NULL;
		break;
	case eMailbox:
	case eCabinet:
	case eAccount:
		mData = new CDataItem<cdstring>;
		break;
	}

	if (mData)
		mData->SetInfo(txt, vers_prefs);
	
	return true;
}
