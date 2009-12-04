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


// CMboxACLClient

#ifndef __CMBOXACLCLIENT__MULBERRY__
#define __CMBOXACLCLIENT__MULBERRY__

// CMboxACLClient: Handles quotas for all resources

// Classes

class CACL;
class CMbox;
class CMboxACLProtocol;

class CMboxACLClient
{
public:
		CMboxACLClient(CMboxACLProtocol* owner)
			{ mOwner = owner; 
				mACLMbox = nil;
				mACL = nil; }
		CMboxACLClient(const CMboxACLClient& copy, CMboxACLProtocol* owner)
			{ mOwner = owner; 
				mACLMbox = nil;
				mACL = nil; }
	virtual ~CMboxACLClient() {}
	
	// These must be implemented by specific client
	virtual void _SetACL(CMbox* mbox, CACL* acl) = 0;			// Set acl on server
	virtual void _DeleteACL(CMbox* mbox, CACL* acl) = 0;		// Delete acl on server
	virtual void _GetACL(CMbox* mbox) = 0;						// Get all acls for mailbox from server
	virtual void _ListRights(CMbox* mbox, CACL* acl) = 0;		// Get allowed rights for user
	virtual void _MyRights(CMbox* mbox) = 0;					// Get current user's rights to mailbox

protected:
	CMboxACLProtocol*	mOwner;
	CMbox*				mACLMbox;								// Mailbox to get ACLs for
	CACL*				mACL;									// ACL being processed (LISTRIGHTS only)

};

#endif
