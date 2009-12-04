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


// Header for CMessageRef class

#ifndef __CMESSAGEREF__MULBERRY__
#define __CMESSAGEREF__MULBERRY__

// Constants
class CMboxRef;
class CMessage;

class CMessageRef
{
public:
	explicit CMessageRef(const CMessage* msg);			// Constructed from actual message (may be NULL)
	CMessageRef(const CMessageRef &copy);				// Copy constructor
	virtual ~CMessageRef();

	CMessage* ResolveMessage() const;					// Get message from open mailbox

private:
	CMboxRef*		mMboxRef;							// Mailbox ref for owning mailbox
	unsigned long	mUIDValidity;						// UIDValidity of owning mailbox at time of creation
	unsigned long	mUID;								// UID of actual message

	void	InitMessageRef();
};

#endif
