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


// Header for CMessageList class

#ifndef __CMESSAGELIST__MULBERRY__
#define __CMESSAGELIST__MULBERRY__

#include <vector>
#include "CMessageFwd.h"
#include "CMessageListFwd.h"

// Classes

class CMessage;
class CMessageComparator;

class CMessageList : public std::vector<CMessage*>
{
public:
	CMessageList(bool sorted = false);
	~CMessageList();

	// O T H E R  M E T H O D S

	void	SetSortBy(ESortMessageBy sort_by,					// Set sorting by
						EShowMessageBy show_by,
						bool external = false);
	void	SetComparator(CMessageComparator* comp);

	void	SetOwnership(bool owns)								// Determine if list should delete messages
				{ mOwnsMsgs = owns; }

	bool	IsSorted() const								// Is list supposed to be sorted
				{ return mSorted; }
	void	SortDirty()										// List changed since last sort
				{ mSortDirty = true; }
	void	Sort();											// Do sort now
	unsigned long	FetchIndexOf(const CMessage* msg) const;	// Recursively search cache for message
	void	push_back_sorted(const CMessage*& aMsg);			// Add message in sorted order

	unsigned long 	CountFlags(NMessage::EFlags set,			// Get number of messages with matching flags
								NMessage::EFlags not_set = NMessage::eNone) const;
	bool 	AnyFlags(NMessage::EFlags set,						// Check for at least one flag set/unset
								NMessage::EFlags not_set = NMessage::eNone) const;

	void	RemoveMessage(CMessage* msg);					// Remove the message from the list
	void	ReplaceMessage(CMessage* msg,					// Replace the message with another
							CMessage* replace);
	void  	DeleteAll();									// Delete all messages from list
	void  	DeleteFakes();									// Delete all fake messages from list
	void  	RemoveFakes();									// Remove all fake messages from list
	void  	ClearThreadInfo();								// Remove thread info when changing from thread sort

	void  	Renumber();										// Renumber all messages in list

	CMessage* GetPrevMessage(const CMessage* aMsg) const;		// Get the message before the one specified in ascending order
	CMessage* GetNextMessage(const CMessage* aMsg) const;		// Get the message after the one specified in ascending order

	unsigned long GetPrevMessageIndex(unsigned long index) const;		// Get the message index before the one specified in ascending order
	unsigned long GetNextMessageIndex(unsigned long index) const;		// Get the message index after the one specified in ascending order


	void AddThread(const CMessage* msg);					// Add entire message thread containing chosen message	
	void AddThreadParent(const CMessage* msg);				// Add entire message thread from parent down

protected:
	ESortMessageBy						mSortBy;
	EShowMessageBy						mShowBy;
	bool								mSorted;
	bool								mSortDirty;
	bool								mExternal;
	CMessageComparator*					mComparator;
	bool								mOwnsMsgs;

	CMessage* GetSortedPrevMessage(const CMessage* aMsg) const;	// Get the message before the one specified in sorted order
	CMessage* GetSortedNextMessage(const CMessage* aMsg) const;	// Get the message after the one specified in sorted order

	unsigned long GetSortedPrevMessageIndex(unsigned long index) const;	// Get the message index before the one specified in sorted order
	unsigned long GetSortedNextMessageIndex(unsigned long index) const;	// Get the message index after the one specified in sorted order

};

#endif
