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


// Source for CMessageRef class

#include "CMessageRef.h"

#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMessage.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructed from actual message only
CMessageRef::CMessageRef(const CMessage* msg)
{
	// Common init
	InitMessageRef();

	// msg is allowed to be NULL!
	// Also's msg's mailbox may be NULL if it is a thread fake
	if (msg && msg->GetMbox())
	{
		// Make a mailbox ref from it owner mailbox
		mMboxRef = new CMboxRef(msg->GetMbox());

		// Save the UIDValidity of the owner mailbox
		mUIDValidity = msg->GetMbox()->GetUIDValidity();
		
		// Save the UID of the actual message
		// NB message must be cached at this point so UID is valid
		mUID = msg->GetUID();
	}
}

// Copy constructor
CMessageRef::CMessageRef(const CMessageRef &copy)
{
	// Common init
	InitMessageRef();

	// Copy items
	mMboxRef = copy.mMboxRef ? new CMboxRef(*copy.mMboxRef) : NULL;
	mUIDValidity = copy.mUIDValidity;
	mUID = copy.mUID;
}

// Default destructor
CMessageRef::~CMessageRef()
{
	delete mMboxRef;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Common init
void CMessageRef::InitMessageRef()
{
	mMboxRef = NULL;;
	mUIDValidity = 0;
	mUID = 0;
}

// Get message from open mailbox
CMessage* CMessageRef::ResolveMessage() const
{
	// Never resolve a message with UID 0 - that means it is not cached and we can never
	// find which message it actually is
	if (!mUID)
		return NULL;

	// If original message was NULL the mbox ref will be NULL
	if (!mMboxRef)
		return NULL;

	// First try and resolve the mailbox to one that exists
	// Use the force flag to forcibly list it if it is not present
	CMbox* mbox = mMboxRef->ResolveMbox(true);
	
	// If no mailbox return
	if (!mbox)
		return NULL;
	
	// Try to get the open mailbox for this one
	CMbox* open_mbox = mbox->IsOpen() ? mbox : mbox->GetProtocol()->FindOpenMbox(mbox->GetName());
	
	// If no open mailbox we have to return NULL - we don't attempt to open it ourselves
	// One day we might do this to allow access to messages in closed mailboxes
	// This will allow (for example) mailboxes containing replies to be closed
	// before the reply is sent or to implement a virtual mailbox of messages
	// from other mailboxes for combined search results etc
	if (!open_mbox)
		return NULL;
	
	// Now compare uid validities - must fail if they do not match
	if (mUIDValidity != open_mbox->GetUIDValidity())
		return NULL;
	
	// Now fetch the message with this UID
	return open_mbox->GetMessageUID(mUID);
}
