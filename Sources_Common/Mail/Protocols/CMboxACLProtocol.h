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


// CMboxACLProtocol

#ifndef __CMBOXACLPROTOCOL__MULBERRY__
#define __CMBOXACLPROTOCOL__MULBERRY__

// CMboxACLProtocol: Handles quotas for all resources

class CACL;
class CMbox;
class CMboxACLClient;

class CMboxACLProtocol
{
public:
	CMboxACLProtocol();
	virtual ~CMboxACLProtocol() {}
	
	// Status of ACL support
	virtual void SetHasACL(bool has_acl)
		{ mHasACL = has_acl; }
	virtual bool GetHasACL()
		{ return mHasACL; }
	virtual void SetDisabled(bool disabled)
		{ mDisabled = disabled; }
	virtual bool GetDisabled()
		{ return mDisabled; }

	virtual bool UsingACLs()							// Indicates that client can use ACLs
		{ return mHasACL && !mDisabled; }

	virtual void SetACL(CMbox* mbox, CACL* acl);			// Set acl on server
	virtual void DeleteACL(CMbox* mbox, CACL* acl);			// Delete acl on server
	virtual void GetACL(CMbox* mbox);						// Get all acls for mailbox from server
	virtual void ListRights(CMbox* mbox, CACL* acl);		// Get allowed rights for user
	virtual void MyRights(CMbox* mbox);						// Get current user's rights to mailbox

protected:
	CMboxACLClient*	mClient;								// Its client
	bool			mHasACL;								// Does server support ACLs?
	bool			mDisabled;								// ACLs disabled by client
};

#endif
