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


// Source for CMessageList class

#include "CMessageList.h"

#include "CMessage.h"
#include "CMessageComparators.h"
#include "CMessageThread.h"
#include "CPreferences.h"

#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E L I S T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageList::CMessageList(bool sorted)
	: mSorted(sorted)
{
	// Set default state
	mComparator = NULL;
	mSortBy = cSortMessageNumber;
	mShowBy = cShowMessageAscending;
	mSortDirty = true;
	mExternal = false;

	// Owns messages
	mOwnsMsgs = true;

	// Set appropriate comparator
	if (IsSorted())
		SetComparator(new CMessageNumberComparator());
}

// Default destructor
CMessageList::~CMessageList()
{
	// Delete all messages if owned
	if (mOwnsMsgs)
		DeleteAll();
	
	// Otherwise delete fakes only when sorted by thread
	else if (mSortBy == cSortMessageThread)
		DeleteFakes();

	delete mComparator;
	mComparator = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set sorting by
void CMessageList::SetSortBy(ESortMessageBy sort_by, EShowMessageBy show_by, bool external)
{
	// Only do if changing
	if (!mSortDirty &&
		(sort_by == mSortBy) &&
		(show_by == mShowBy) &&
		(external == mExternal))
		return;

	// Look for change from thread sorting to something else and clear out unwanted thread info
	bool comparator_change = (sort_by != mSortBy);
	if (comparator_change && (mSortBy == cSortMessageThread))
		ClearThreadInfo();

	// Change flag
	mSortBy = sort_by;
	mExternal = external;

	// Set flag
	mShowBy = show_by;

	if (comparator_change)
	{
		// Set appropriate comparator
		switch(sort_by)
		{
		case cSortMessageTo:
			SetComparator(new CMessageToComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageFrom:
			SetComparator(new CMessageFromComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageSender:
			SetComparator(new CMessageSenderComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageReplyTo:
			SetComparator(new CMessageReplyToComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageCc:
			SetComparator(new CMessageCcComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageSubject:
			SetComparator(new CMessageSubjectComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageThread:
			//SetComparator(new CMessageThreadComparator(mShowBy==cShowMessageAscending));
			SetComparator(new CMessageMessageIDComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageDateSent:
			SetComparator(new CMessageDateComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageDateReceived:
			SetComparator(new CMessageInternalDateComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageSize:
			SetComparator(new CMessageSizeComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageFlags:
			SetComparator(new CMessageFlagsComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageNumber:
			SetComparator(new CMessageNumberComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageSmart:
			SetComparator(new CMessageSmartComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageAttachment:
			SetComparator(new CMessageAttachmentComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageParts:
			SetComparator(new CMessagePartsComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageMatching:
			SetComparator(new CMessageMatchComparator(mShowBy==cShowMessageAscending));
			break;

		case cSortMessageDisconnected:
			SetComparator(new CMessageDisconnectedComparator(mShowBy==cShowMessageAscending));
			break;

		default:
			SetComparator(new CMessageNumberComparator(mShowBy==cShowMessageAscending));
			break;
		}
	}
	else
	{
		// Set current comparator sort order
		mComparator->SetAscending(mShowBy==cShowMessageAscending);

		// Force resort
		Sort();
	}
}

// Set a new comparison method
void  CMessageList::SetComparator(CMessageComparator* comp)
{
	delete mComparator;
	mComparator = comp;
	if (comp)
		Sort();
}

// Do sort now
void  CMessageList::Sort()
{
	// Ignore if externally maintained sort
	if (!mExternal)
	{
		// Always clear out any fake messages in the message list
		// These may exist after a thread operation that inserts holes
		RemoveFakes();
	
		mComparator->SortMessageList(this);
	}
	
	// Make as clean sort
	mSortDirty = false;
}

// Search cache for message
unsigned long CMessageList::FetchIndexOf(const CMessage* msg) const
{
	CMessageList::const_iterator found = find(begin(), end(), msg);

	if (found != end())
		return (found - begin()) + 1;
	else
		return 0;
}

// Add message in sorted order
void CMessageList::push_back_sorted(const CMessage*& aMsg)
{
	if (mExternal)
		push_back(const_cast<CMessage*>(aMsg));
	else
	{
		CMessageList::iterator found = mComparator->LowerBound(this, aMsg);

		insert(found, const_cast<CMessage*>(aMsg));
	}
}

// Add entire message thread containing chosen message
void CMessageList::AddThread(const CMessage* msg)
{
	// Get top-level parent
	const CMessage* parent = msg;
	while(parent->GetThreadParent() != NULL)
		parent = parent->GetThreadParent();
	
	// Add the parent and its children
	AddThreadParent(parent);
}

// Add entire message thread from parent down
void CMessageList::AddThreadParent(const CMessage* msg)
{
	// Add this one
	CMessageList::const_iterator found = find(begin(), end(), msg);
	if (found == end())
		push_back(const_cast<CMessage*>(msg));
	
	// Add children
	const CMessage* child = msg->GetThreadChild();
	while(child != NULL)
	{
		// Add the child (and all its children)
		AddThreadParent(child);
		
		// Get next child
		child = child->GetThreadNext();
	}
}

// Count number of messages with flag set
unsigned long CMessageList::CountFlags(NMessage::EFlags set, NMessage::EFlags not_set) const
{
	unsigned long i = 0;

	// Find all in list
	for(CMessageList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// If matches increment counter
		if (*iter &&
			((set == NMessage::eNone) || (*iter)->HasFlag(set)) &&
			((not_set == NMessage::eNone) || !(*iter)->HasFlag(not_set)))
			i++;
	}

	return i;
}

// Check for at least one flag set/unset
bool CMessageList::AnyFlags(NMessage::EFlags set, NMessage::EFlags not_set) const
{
	// Find all in list
	// NB Do this in reverse as we are going to assume a higher likely hood of deleted
	// messages at the end of a large mailbox than at the beginning - i.e. short circuit
	// should occur sooner with reverse iterator
	for(CMessageList::const_reverse_iterator riter = rbegin(); riter != rend(); riter++)
	{
		// If matches short circuit and return
		if (*riter &&
			((set == NMessage::eNone) || (*riter)->HasFlag(set)) &&
			((not_set == NMessage::eNone) || !(*riter)->HasFlag(not_set)))
			return true;
	}

	return false;
}

// Remove the message from the list
void CMessageList::RemoveMessage(CMessage* msg)
{
	CMessageList::iterator found = find(begin(), end(), msg);

	if (found != end())
		erase(found);
}

// Replace the message with another
void CMessageList::ReplaceMessage(CMessage* msg, CMessage* replace)
{
	CMessageList::iterator found = find(begin(), end(), msg);

	if (found != end())
		*found = replace;
}

// Delete all messages from the list
void CMessageList::DeleteAll()
{
	// Delete all in list
	for(CMessageList::iterator iter = begin(); iter != end(); iter++)
		delete *iter;

	// Empty list
	clear();

}

// Delete all fake messages from the list
void CMessageList::DeleteFakes()
{
	// Delete all fakes in list
	for(CMessageList::iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter)->IsFake())
			delete *iter;
		else
			(*iter)->ClearThreadInfo();
	}

	// Empty list
	clear();
}


// Remove all fake messages from the list
void CMessageList::RemoveFakes()
{
	// Always clear out any fake messages in the message list
	for(CMessageList::iterator iter = begin(); iter != end(); )
	{
		if ((*iter)->IsFake())
		{
			delete *iter;
			iter = erase(iter);
			continue;
		}
		else
			(*iter)->ClearThreadInfo();
		
		iter++;
	}
}

// Remove thread info when changing from thread sort
void CMessageList::ClearThreadInfo()
{
	// Always clear out thread info
	for(CMessageList::iterator iter = begin(); iter != end(); iter++)
		(*iter)->ClearThreadInfo();
}

// Renumber all messages from the list
void CMessageList::Renumber()
{
	// Renumber all in list
	unsigned long count = 1;
	for(CMessageList::iterator iter = begin(); iter != end(); iter++)
	{
		if (*iter)
			(*iter)->SetMessageNumber(count);
		count++;
	}

}

// Get the message before the one specified
CMessage* CMessageList::GetPrevMessage(const CMessage* aMsg) const
{
	if ((mShowBy == cShowMessageAscending) ^ CPreferences::sPrefs->mNextIsNewest.GetValue())
		return GetSortedNextMessage(aMsg);
	else
		return GetSortedPrevMessage(aMsg);

}

// Get the message after the one specified
CMessage* CMessageList::GetNextMessage(const CMessage* aMsg) const
{
	if ((mShowBy == cShowMessageAscending) ^ CPreferences::sPrefs->mNextIsNewest.GetValue())
		return GetSortedPrevMessage(aMsg);
	else
		return GetSortedNextMessage(aMsg);

}

// Get the message before the one specified
unsigned long CMessageList::GetPrevMessageIndex(unsigned long index) const
{
	if ((mShowBy == cShowMessageAscending) ^ CPreferences::sPrefs->mNextIsNewest.GetValue())
		return GetSortedNextMessageIndex(index);
	else
		return GetSortedPrevMessageIndex(index);

}

// Get the message after the one specified
unsigned long CMessageList::GetNextMessageIndex(unsigned long index) const
{
	if ((mShowBy == cShowMessageAscending) ^ CPreferences::sPrefs->mNextIsNewest.GetValue())
		return GetSortedPrevMessageIndex(index);
	else
		return GetSortedNextMessageIndex(index);

}

// Get the message before the one specified
CMessage* CMessageList::GetSortedPrevMessage(const CMessage* aMsg) const
{
	// Get index of specified message or one beyond last if null
	unsigned long index = aMsg ? FetchIndexOf(aMsg) : size() + 1;

	// If its not the first return the previous message - skipping fakes
	while(index > 1)
	{
		index--;
		CMessage* prev = at(index - 1);
		if (!prev->IsFake())
			return prev;
	}

	// First message so no previous
	return NULL;

}

// Get the message after the one specified
CMessage* CMessageList::GetSortedNextMessage(const CMessage* aMsg) const
{
	// Get index of specified message
	unsigned long index = FetchIndexOf(aMsg);

	// If its not the last return the next message - skipping fakes
	while(index < size())
	{
		index++;
		CMessage* next = at(index - 1);
		if (!next->IsFake())
			return next;
	}

	// Last message so no next
	return NULL;

}

// Get the message before the one specified
unsigned long CMessageList::GetSortedPrevMessageIndex(unsigned long index) const
{
	// Adjust index one beyond last if 0
	if (index == 0)
		index = size() + 1;
	
	// If its not the first return the previous message - skipping fakes
	while(index > 1)
	{
		index--;
		CMessage* prev = at(index - 1);
		if (!prev->IsFake())
			return index;
	}

	// First message so no previous
	return 0;

}

// Get the message after the one specified
unsigned long CMessageList::GetSortedNextMessageIndex(unsigned long index) const
{
	// If its not the last return the next message - skipping fakes
	while(index < size())
	{
		index++;
		CMessage* next = at(index - 1);
		if (!next->IsFake())
			return index;
	}

	// Last message so no next
	return 0;

}
