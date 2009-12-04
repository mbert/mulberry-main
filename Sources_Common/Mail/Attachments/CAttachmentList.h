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


//	CAttachmentList.h

#ifndef __CATTACHMENTLIST__MULBERRY__
#define __CATTACHMENTLIST__MULBERRY__

#include "ptrvector.h"

// Classes

class CAttachment;

class CAttachmentList : public ptrvector<CAttachment> {

public:
						CAttachmentList();
						CAttachmentList(const CAttachmentList& copy);
						~CAttachmentList();

	unsigned long		FetchIndexOf(const CAttachment* attach) const;						// Get position of mbox in the list

	void				InsertAttachmentAt(unsigned long index, CAttachment* attach);		// Insert attachment at specific position
	void				RemoveAttachmentAt(unsigned long index);							// Remove attachment at specific position

	void				RemoveAttachment(CAttachment* attach, bool delete_it = true);		// Remove (and possibly delete) the attachment from the list

	bool				DuplicateItem(const CAttachment* test) const;						// Look for duplicate in full hierarchy
};

#endif
