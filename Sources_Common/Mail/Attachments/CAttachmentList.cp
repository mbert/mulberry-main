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


//	CAttachmentList.cp

#include "CAttachmentList.h"

#include "CAttachment.h"
#include <algorithm>

// Default constructor
CAttachmentList::CAttachmentList()
{
}

// Copy constructor
CAttachmentList::CAttachmentList(const CAttachmentList& copy)
{

	// For all entries in list to be copied
	for(CAttachmentList::const_iterator iter = copy.begin(); iter != copy.end(); iter++)
	{
		// Copy and add
		CAttachment* newAttach = CAttachment::CopyAttachment(**iter);
		push_back(newAttach);
	}

}

// Default destructor
CAttachmentList::~CAttachmentList()
{
}

// Recursively search cache for mbox with requested name
unsigned long CAttachmentList::FetchIndexOf(const CAttachment* attach) const
{
	CAttachmentList::const_iterator found = find(begin(), end(), (CAttachment*) attach);

	if (found != end())
	{
		return (found - begin()) + 1;
	}
	else
		return 0;
}

// Insert attachment at specific position
void CAttachmentList::InsertAttachmentAt(unsigned long index, CAttachment* attach)
{
	// Find it
	insert(begin() + index, attach);
}

// Delete attachment at specific position
void CAttachmentList::RemoveAttachmentAt(unsigned long index)
{
	// Remove
	erase(begin() + index);
}

// Delete the attachment from the list
void CAttachmentList::RemoveAttachment(CAttachment* attach, bool delete_it)
{
	CAttachmentList::iterator found = find(begin(), end(), attach);

	if (found != end())
	{
		// Delete before erase
		if (not delete_it)
			*found = NULL;
		erase(found);
	}
}

// Check for duplicate attachment
bool CAttachmentList::DuplicateItem(const CAttachment* test) const
{
	// For all entries
	for(CAttachmentList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Compare items
		if (*iter == test)
			return true;

		// Look in hierarchy
		else if ((*iter)->GetParts() && (*iter)->GetParts()->DuplicateItem(test))
			return true;
	}

	return false;
}
