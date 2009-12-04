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


//	CAliasAttachment.h

#ifndef __CALIASATTACHMENT__MULBERRY__
#define __CALIASATTACHMENT__MULBERRY__

#include "CAttachment.h"

// Classes

class	CAliasAttachment : public CAttachment
{
public:
						CAliasAttachment(CMessage* owner, const CAttachment* original);
						CAliasAttachment(const CAliasAttachment& copy);
	virtual				~CAliasAttachment();

	virtual void		ProcessSend();									// Process attachments for sending
	virtual void		ProcessContent();								// Process attachments for sending

	virtual bool		TestOwnership(const CMbox* owner) const;		// Check whether any part owned by mbox
	virtual bool		ContainsServer(const CMboxProtocol* proto) const;					// Server contained
	virtual bool		ContainsMailbox(const CMbox* mbox) const;							// Mailbox contained
	virtual bool		ContainsMessage(const CMessage* msg) const;							// Message contained

	virtual void		WriteDataToStream(costream& stream,
											bool dummy_files,
											CProgress* progress,
											CMessage* owner = NULL,
											unsigned long count = 0,
											unsigned long start = 1) const;

private:
	const CAttachment*	mOriginal;
		  CMessage*		mOwner;

			void		InitAliasAttachment();

};

#endif
