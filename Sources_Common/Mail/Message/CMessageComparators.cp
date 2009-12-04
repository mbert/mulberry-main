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


// Source for CMessageComparator class

#include "CMessageComparators.h"

#include "CAddressList.h"
#include "CAddress.h"
#include "CMessage.h"
#include "CMessageThread.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include <stdlib.h>
#include <string.h>

#include <algorithm>

#pragma mark ____________________________Specializations

#if 0
typedef bool (*voidCompare)(const void*, const void*);
void sort (CMessageList::iterator first, CMessageList::iterator last, MessageCompare comp);
void sort (CMessageList::iterator first, CMessageList::iterator last, MessageCompare comp)
{
	sort(reinterpret_cast<vector<void*>::iterator>(first),
			reinterpret_cast<vector<void*>::iterator>(last),
			reinterpret_cast<voidCompare>(comp));
}

CMessageList::iterator lower_bound (CMessageList::iterator first, CMessageList::iterator last, const CMessage*& value, MessageCompare comp);
CMessageList::iterator lower_bound (CMessageList::iterator first, CMessageList::iterator last, const CMessage*& value, MessageCompare comp)
{
	return
		reinterpret_cast<CMessageList::iterator>(lower_bound(
				reinterpret_cast<vector<void*>::iterator>(first),
				reinterpret_cast<vector<void*>::iterator>(last),
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
				reinterpret_cast<const void*&>(&value),
#else
				static_cast<const void*&>(&value),
#endif
				reinterpret_cast<voidCompare>(comp)));
}

typedef bool (*voidPartition)(void*);
CMessageList::iterator stable_partition (CMessageList::iterator first, CMessageList::iterator last, MessagePartition pred);
CMessageList::iterator stable_partition (CMessageList::iterator first, CMessageList::iterator last, MessagePartition pred)
{
	return
		reinterpret_cast<CMessageList::iterator>(stable_partition(
			reinterpret_cast<vector<void*>::iterator>(first),
			reinterpret_cast<vector<void*>::iterator>(last),
			reinterpret_cast<voidPartition>(pred)));
}
#endif

#pragma mark ____________________________CMessageComparator

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E O R D E R C O M P A R A T O R
// __________________________________________________________________________________________________

long CMessageComparator::sOrder = 1;
long CMessageComparator::sNumDiff = 0;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageComparator::CMessageComparator(bool ascending)
{
	mOrder = (ascending) ? 1 : -1;

} // CMessageComparator::CMessageComparator

// Default destructor
CMessageComparator::~CMessageComparator()
{
} // CMessageComparator::~CMessageComparator

void CMessageComparator::SortMessageList(CMessageList* list)
{
	sOrder = mOrder;

	std::sort(list->begin(), list->end(), GetComparator());
}

CMessageList::iterator CMessageComparator::LowerBound(CMessageList* list, const CMessage* aMsg)
{
	sOrder = mOrder;

	return std::lower_bound(list->begin(), list->end(), const_cast<CMessage*>(aMsg), GetComparator());
}

bool CMessageComparator::TestCachedState(CMessage* inMsg1, CMessage* inMsg2, bool& result)
{
	sNumDiff = inMsg1->GetMessageNumber() - inMsg2->GetMessageNumber();

	bool cached1 = inMsg1->IsFullyCached();
	bool cached2 = inMsg2->IsFullyCached();

	if (!cached1 && !cached2)
	{
		// Both uncached => separate by number
		 result = (sOrder * sNumDiff < 0);
		 return true;
	}
	else if (!cached1 || !cached2)
	{
		// One is uncached, always place after cached
		result  = !cached2;
		return true;
	}
	else
		return false;
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C N A M E D A D D R E S S C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNamedAddressComparator::CNamedAddressComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CNamedAddressComparator::CNamedAddressComparator

// Default destructor
CNamedAddressComparator::~CNamedAddressComparator()
{
} // CNamedAddressComparator::~CNamedAddressComparator

bool CNamedAddressComparator::CompareNames(CMessage* inMsg1, CMessage* inMsg2, char* name1, char* name2)
{
	// Handle zero length case
	size_t len1 = ::strlen(name1);
	size_t len2 = ::strlen(name2);
	if (!len1 && !len2)
		return (sOrder * sNumDiff < 0);
	else if (!len1) return (sOrder < 0);
	else if (!len2) return (-sOrder < 0);

	// Get pointer to last name for compare
	char* last1 = ::strrchr(name1, ' ');
	char* last2 = ::strrchr(name2, ' ');

	// Make sure somethings found
	if (!last1)
		last1 = name1;
	else
		last1++;
	if (!last2)
		last2 = name2;
	else
		last2++;

	// Do compare
	long result = ::strcmpnocase(last1, last2);

	// Return if not equal or no more to compare
	if (result || ((last1 == name1) || (last2 == name2)))
	{
		// Make sure messages are unique
		if (!result)
			result = sNumDiff;
		return (sOrder * result < 0);
	}

	// Tie off
	*(--last1) = '\0';
	*(--last2) = '\0';

	// Do remaining compare
	result = ::strcmpnocase(name1, name2);

	// Untie
	*last1 = ' ';
	*last2 = ' ';

	// Make sure messages are unique
	if (!result)
		result = sNumDiff;

	// Return result
	return (sOrder * result < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E T O C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageToComparator::CMessageToComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageToComparator::CMessageToComparator

// Default destructor
CMessageToComparator::~CMessageToComparator()
{
} // CMessageToComparator::~CMessageToComparator

// Get required names to test
bool CMessageToComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetTo()->size())
		name1 = theEnv1->GetTo()->front()->GetNamedAddress();

	if (theEnv2 && theEnv2->GetTo()->size())
		name2 = theEnv2->GetTo()->front()->GetNamedAddress();

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E F R O M C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageFromComparator::CMessageFromComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageFromComparator::CMessageFromComparator

// Default destructor
CMessageFromComparator::~CMessageFromComparator()
{
} // CMessageFromComparator::~CMessageFromComparator

// Get required names to test
bool CMessageFromComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetFrom()->size())
		name1 = theEnv1->GetFrom()->front()->GetNamedAddress();

	if (theEnv2 && theEnv2->GetFrom()->size())
		name2 = theEnv2->GetFrom()->front()->GetNamedAddress();

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E S E N D E R C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSenderComparator::CMessageSenderComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageSenderComparator::CMessageSenderComparator

// Default destructor
CMessageSenderComparator::~CMessageSenderComparator()
{
} // CMessageSenderComparator::~CMessageSenderComparator

// Get required names to test
bool CMessageSenderComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetSender()->size())
		name1 = theEnv1->GetSender()->front()->GetNamedAddress();

	if (theEnv2 && theEnv2->GetSender()->size())
		name2 = theEnv2->GetSender()->front()->GetNamedAddress();

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E R E P L Y T O C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageReplyToComparator::CMessageReplyToComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageReplyToComparator::CMessageReplyToComparator

// Default destructor
CMessageReplyToComparator::~CMessageReplyToComparator()
{
} // CMessageReplyToComparator::~CMessageReplyToComparator

// Get required names to test
bool CMessageReplyToComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetReplyTo()->size())
		name1 = theEnv1->GetReplyTo()->front()->GetNamedAddress();

	if (theEnv2 && theEnv2->GetReplyTo()->size())
		name2 = theEnv2->GetReplyTo()->front()->GetNamedAddress();

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E C C C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageCcComparator::CMessageCcComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageCcComparator::CMessageCcComparator

// Default destructor
CMessageCcComparator::~CMessageCcComparator()
{
} // CMessageCcComparator::CMessageCcComparator

// Get required names to test
bool CMessageCcComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetCC()->size())
		name1 = theEnv1->GetCC()->front()->GetNamedAddress();

	if (theEnv2 && theEnv2->GetCC()->size())
		name2 = theEnv2->GetCC()->front()->GetNamedAddress();

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E S M A R T C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSmartComparator::CMessageSmartComparator(bool ascending)
	: CNamedAddressComparator(ascending)
{
} // CMessageSmartComparator::CMessageSmartComparator

// Default destructor
CMessageSmartComparator::~CMessageSmartComparator()
{
} // CMessageSmartComparator::~CMessageSmartComparator

// Get required names to test
bool CMessageSmartComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	cdstring name1;
	cdstring name2;

	if (theEnv1 && theEnv1->GetFrom()->size())
	{
		// Compare address with current user
		if (inMsg1->IsSmartFrom())
		{
			// Check for to
			if (theEnv1->GetTo()->size())
				name1 = theEnv1->GetTo()->front()->GetNamedAddress();

		}
		else
		{
			// Check for from
			if (theEnv1->GetFrom()->size())
				name1 = theEnv1->GetFrom()->front()->GetNamedAddress();
		}
	}

	if (theEnv2 && theEnv2->GetFrom()->size())
	{
		// Compare address with current user
		if (inMsg2->IsSmartFrom())
		{
			// Check for to
			if (theEnv2->GetTo()->size())
				name2 = theEnv2->GetTo()->front()->GetNamedAddress();
		}
		else
		{
			// Check for from
			if (theEnv2->GetFrom()->size())
				name2 = theEnv2->GetFrom()->front()->GetNamedAddress();
		}
	}

	return CompareNames(inMsg1, inMsg2, name1, name2);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E S U B J E C T C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSubjectComparator::CMessageSubjectComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageSubjectComparator::CMessageSubjectComparator

// Default destructor
CMessageSubjectComparator::~CMessageSubjectComparator()
{
} // CMessageSubjectComparator::~CMessageSubjectComparator

bool CMessageSubjectComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	// Get subjects
	bool re_subj1;
	bool re_subj2;
	const char* subj1 = theEnv1 ? theEnv1->GetThread(re_subj1) : NULL;
	const char* subj2 = theEnv2 ? theEnv2->GetThread(re_subj2) : NULL;

	// Handle case of empty subjects
	if ((!subj1 || !*subj1) && (!subj2 || !*subj2))
		return (sOrder * sNumDiff < 0);
	else if (!subj1 || !*subj1)
		return (sOrder < 0);
	else if (!subj2 || !*subj2)
		return (-sOrder < 0);

	// Now compare text
	long test = ::strcmpnocase(subj1, subj2);

	// Make sure messages are unique
	if (test==0)
	{
		if (re_subj1 ^ re_subj2)
			test = re_subj1 ? 1 : -1;
		else
			test = sNumDiff;
	}

	return (sOrder * test < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E T H R E A D C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

const char* CMessageThreadComparator::sPartitionThread = NULL;
unsigned long CMessageThreadComparator::sPartitionThreadHash = 0;

// Default constructor
CMessageThreadComparator::CMessageThreadComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageSubjectComparator::CMessageSubjectComparator

// Default destructor
CMessageThreadComparator::~CMessageThreadComparator()
{
} // CMessageSubjectComparator::~CMessageSubjectComparator

void CMessageThreadComparator::SortMessageList(CMessageList* list)
{
	// Always do in ascending order
	sOrder = 1;

	// Sort by date first (always do in ascending order)
	std::sort(list->begin(), list->end(), CMessageDateComparator::GetDateComparator());

	// Only if finite size
	if (list->size())
	{
		// Now look for threads the slow way
		CMessageList::iterator iter = list->begin();

		while(iter < list->end() - 1)
		{
			// Must check cached state
			if (!(*iter)->IsFullyCached())
				// This must be the first of the uncached messages - no need to partition any further
				break;

			// Set thread to compare with
			bool dummy;
			sPartitionThread = (*iter)->GetEnvelope()->GetThread(dummy);
			sPartitionThreadHash = (*iter)->GetEnvelope()->GetThreadHash();
			
			// Make this message top-level thread
			(*iter)->SetThreadInfo(0, NULL, NULL, NULL, NULL);

			// Use partition
			iter++;
			iter = std::stable_partition(iter, list->end(), &PartitionFN);
		}
	}
	
	// Do reversal
	if (mOrder < 0)
	{
		// Create temp list
		CMessageList temp(*list);
		temp.SetOwnership(false);
		list->clear();

		CMessageList::iterator start = temp.begin();
		while(start != temp.end()) 
		{
			// Find end of this thread
			CMessageList::iterator stop = start + 1;
			while((stop != temp.end()) && (*stop)->GetThreadDepth())
				stop++;
			
			// Copy entire thread to start of results
			list->insert(list->begin(), start, stop);
			start = stop;
		}
	}
}

// Algorithm - different subjects are equal (i.e. date order preserved),
// same threads
bool CMessageThreadComparator::PartitionFN(CMessage* inMsg)
{
	// Do not bother with uncached
	if (!inMsg->IsFullyCached())
		return false;

	// Test hash's first
	bool result = (sPartitionThreadHash == inMsg->GetEnvelope()->GetThreadHash());
	if (!result)
		return false;

	// Now test subjects just in case different subjects mapped to the same hash
	bool re_thread;
	const char* thread = inMsg->GetEnvelope()->GetThread(re_thread);

	result = !::strcmpnocase(thread, sPartitionThread);

	// Make it a child thread one-level down if matched
	if (result)
		inMsg->SetThreadInfo(1, NULL, NULL, NULL, NULL);

	return result;
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E T H R E A D I D C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageMessageIDComparator::CMessageMessageIDComparator(bool ascending)
	: CMessageComparator(ascending)
{
}

// Default destructor
CMessageMessageIDComparator::~CMessageMessageIDComparator()
{
}

void CMessageMessageIDComparator::SortMessageList(CMessageList* list)
{
	sOrder = mOrder;
	CMessageThread::ThreadMessages(list);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E D A T E C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageDateComparator::CMessageDateComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageDateComparator::CMessageDateComparator

// Default destructor
CMessageDateComparator::~CMessageDateComparator()
{
} // CMessageDateComparator::~CMessageDateComparator

bool CMessageDateComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	CEnvelope* theEnv1 = inMsg1->GetEnvelope();
	CEnvelope* theEnv2 = inMsg2->GetEnvelope();

	long test = 0;
	if (!theEnv1 && !theEnv2)
		test = 0;
	else if (!theEnv1)
		test = -1;
	else if (!theEnv2)
		test = 1;
	else
		test = theEnv1->GetUTCDate() - theEnv2->GetUTCDate();

	// Make sure messages are unique
	if (test==0)
		test = sNumDiff;

	return (sOrder * test < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E I N T E R N A L D A T E C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageInternalDateComparator::CMessageInternalDateComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageInternalDateComparator::CMessageInternalDateComparator

// Default destructor
CMessageInternalDateComparator::~CMessageInternalDateComparator()
{
} // CMessageInternalDateComparator::~CMessageInternalDateComparator

bool CMessageInternalDateComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	long test = inMsg1->GetUTCInternalDate() - inMsg2->GetUTCInternalDate();

	// Make sure messages are unique
	if (test==0)
		test = sNumDiff;

	return (sOrder * test < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E S I Z E C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSizeComparator::CMessageSizeComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageSizeComparator::CMessageSizeComparator

// Default destructor
CMessageSizeComparator::~CMessageSizeComparator()
{
} // CMessageSizeComparator::~CMessageSizeComparator

bool CMessageSizeComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	// Difference between size is comparison
	long test = inMsg1->GetSize() - inMsg2->GetSize();

	// Make sure messages are unique
	if (test==0)
		test = inMsg1->GetMessageNumber() - inMsg2->GetMessageNumber();

	return (sOrder * test < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E F L A G S C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageFlagsComparator::CMessageFlagsComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageFlagsComparator::CMessageFlagsComparator

// Default destructor
CMessageFlagsComparator::~CMessageFlagsComparator()
{
} // CMessageFlagsComparator::~CMessageFlagsComparator

bool CMessageFlagsComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	long test = 0;

	// Check deleted first - deleted are last
	if (inMsg1->IsDeleted() ^ inMsg2->IsDeleted())
		test = (inMsg1->IsDeleted() ? 1 : -1);

	// Check flagged next - important messages
	else if (inMsg1->IsFlagged() ^ inMsg2->IsFlagged())
		test = (inMsg1->IsFlagged() ? -1 : 1);

	// Check new next
	else if (inMsg1->IsUnseen() ^ inMsg2->IsUnseen())
		test = (inMsg1->IsUnseen() ? -1 : 1);

	// Check answered next
	else if (inMsg1->IsAnswered() ^ inMsg2->IsAnswered())
		test = (inMsg1->IsAnswered() ? -1 : 1);

	else
		test = inMsg1->GetMessageNumber() - inMsg2->GetMessageNumber();

	return (sOrder * test < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E N U M B E R C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageNumberComparator::CMessageNumberComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageNumberComparator::CMessageNumberComparator

// Default destructor
CMessageNumberComparator::~CMessageNumberComparator()
{
} // CMessageNumberComparator::~CMessageNumberComparator

bool CMessageNumberComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	long ndiff = inMsg1->GetMessageNumber() - inMsg2->GetMessageNumber();
	return (sOrder * ndiff < 0);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E A T T A C H M E N T C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageAttachmentComparator::CMessageAttachmentComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageAttachmentComparator::CMessageAttachmentComparator

// Default destructor
CMessageAttachmentComparator::~CMessageAttachmentComparator()
{
} // CMessageAttachmentComparator::~CMessageAttachmentComparator

bool CMessageAttachmentComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	// Order messages by parts and part type:
	// text only
	// styled text only
	// encrypted
	// attachments

	long key1 = 0;
	long key2 = 0;

	if (inMsg1->GetBody() && (inMsg1->GetBody()->CountParts() > 1))
	{
		bool is_styled = inMsg1->GetBody()->HasStyledText();
		bool is_attach = inMsg1->GetBody()->HasNonText();
		bool is_signed = inMsg1->GetBody()->IsVerifiable();
		bool is_encrypted = inMsg1->GetBody()->IsDecryptable();

		if (is_attach)
			key1 = 4;
		else if (is_encrypted)
			key1 = 3;
		else if (is_styled)
			key1 = 2;
		else
			key1 = 1;
	}

	if (inMsg2->GetBody() && (inMsg2->GetBody()->CountParts() > 1))
	{
		bool is_styled = inMsg2->GetBody()->HasStyledText();
		bool is_attach = inMsg2->GetBody()->HasNonText();
		bool is_signed = inMsg2->GetBody()->IsVerifiable();
		bool is_encrypted = inMsg2->GetBody()->IsDecryptable();

		if (is_attach)
			key2 = 4;
		else if (is_encrypted)
			key2 = 3;
		else if (is_styled)
			key2 = 2;
		else
			key2 = 1;
	}

	long pdiff = key1 - key2;
	return (pdiff ? (sOrder * pdiff < 0) : (sOrder * sNumDiff < 0));
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E P A R T S C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessagePartsComparator::CMessagePartsComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessagePartsComparator::CMessagePartsComparator

// Default destructor
CMessagePartsComparator::~CMessagePartsComparator()
{
} // CMessagePartsComparator::~CMessagePartsComparator

bool CMessagePartsComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	long pdiff = (inMsg1->GetBody() ? inMsg1->GetBody()->CountParts() : 0) -
					(inMsg2->GetBody() ? inMsg2->GetBody()->CountParts() : 0);
	return (pdiff ? (sOrder * pdiff < 0) : (sOrder * sNumDiff < 0));
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E M A T C H C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageMatchComparator::CMessageMatchComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageMatchComparator::CMessageMatchComparator

// Default destructor
CMessageMatchComparator::~CMessageMatchComparator()
{
} // CMessageMatchComparator::~CMessageMatchComparator

bool CMessageMatchComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	long pdiff = inMsg1->IsSearch() - inMsg2->IsSearch();
	return (pdiff ? (sOrder * pdiff > 0) : (sOrder * sNumDiff < 0));
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E D I S C O N N E C T E D C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageDisconnectedComparator::CMessageDisconnectedComparator(bool ascending)
	: CMessageComparator(ascending)
{
} // CMessageDisconnectedComparator::CMessageDisconnectedComparator

// Default destructor
CMessageDisconnectedComparator::~CMessageDisconnectedComparator()
{
} // CMessageDisconnectedComparator::~CMessageDisconnectedComparator

bool CMessageDisconnectedComparator::CompareFN(CMessage* inMsg1, CMessage* inMsg2)
{
	// Test cached state first
	bool result = false;
	if (TestCachedState(inMsg1, inMsg2, result))
		return result;

	long pdiff = inMsg1->IsFullLocal() - inMsg2->IsFullLocal();
	if (!pdiff)
		pdiff = inMsg1->IsPartialLocal() - inMsg2->IsPartialLocal();
	return (pdiff ? (sOrder * pdiff > 0) : (sOrder * sNumDiff < 0));
}
