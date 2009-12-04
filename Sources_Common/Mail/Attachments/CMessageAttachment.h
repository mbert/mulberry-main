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


//	CMessageAttachment.h

#ifndef __CMESSAGEATTACHMENT__MULBERRY__
#define __CMESSAGEATTACHMENT__MULBERRY__

#include "CAttachment.h"

// Classes

class	CMessageAttachment : public CAttachment {

public:
						CMessageAttachment(CMessage* msg, CMessage* submsg);
						CMessageAttachment(const CMessageAttachment& copy);
	virtual				~CMessageAttachment();

	virtual void		ProcessSend();						// Process files for sending
	virtual void		ProcessContent();

	virtual bool		TestOwnership(const CMbox* owner) const;		// Check whether any part owned by mbox
	virtual bool		ContainsServer(const CMboxProtocol* proto) const;					// Server contained
	virtual bool		ContainsMailbox(const CMbox* mbox) const;							// Mailbox contained
	virtual bool		ContainsMessage(const CMessage* msg) const;							// Message contained

	virtual CMessage*	GetMessage() const
							{ return mOriginal; }

	virtual bool		CanDisplay() const
							{ return IsText(); }
	virtual bool		CanEdit() const
							{ return false; }
	virtual bool		CanChange() const
							{ return false; }

	virtual void		WriteToStream(costream& stream,
										unsigned long& level,
										bool dummy_files,
										CProgress* progress,
										CMessage* owner,
										unsigned long count = 0,
										unsigned long start = 1) const;

private:
	CMessage*		mOriginal;
	CMessage*		mSubMessage;

			void		InitMessageAttachment();

};

#endif
