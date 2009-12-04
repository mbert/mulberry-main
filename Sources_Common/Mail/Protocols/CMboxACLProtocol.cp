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

#include "CMboxACLProtocol.h"

#include "CMboxACLClient.h"

// CMboxACLProtocol: Handles quotas for all resources

// Constructor
CMboxACLProtocol::CMboxACLProtocol()
{
	mClient = nil;
	mHasACL = false;
	mDisabled = false;
}

// Set new value on server
void CMboxACLProtocol::SetACL(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Mbox will take care of adding/replacing in list if successful
	mClient->_SetACL(mbox, acl);
}

// Delete value on server
void CMboxACLProtocol::DeleteACL(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Mbox will take care of adding/replacing in list if successful
	mClient->_DeleteACL(mbox, acl);
}

// Get values on server
void CMboxACLProtocol::GetACL(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_GetACL(mbox);
}

// List rights on server
void CMboxACLProtocol::ListRights(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_ListRights(mbox, acl);
}

// List user's rights on server
void CMboxACLProtocol::MyRights(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_MyRights(mbox);
}

