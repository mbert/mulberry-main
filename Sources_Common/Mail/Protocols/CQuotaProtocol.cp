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


// CQuotaProtocol

#include "CQuotaProtocol.h"

#include "CQuotaClient.h"

// CQuotaProtocol: Handles quotas for all resources

// Constructor
CQuotaProtocol::CQuotaProtocol()
{
	mClient = nil;
	mHasQuota = false;
	mDisabled = false;
}

// Add unique quotaroot to list (copies original)
void CQuotaProtocol::AddQuotaRoot(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Try to find it in existing list
	CQuotaRootList::iterator found = ::find(mRoots.begin(), mRoots.end(), *root);
	
	// If found replace it with copy
	if (found != mRoots.end())
		*found = *root;
	else
		// Just add copy to list
		mRoots.push_back(*root);
}

// Add unique quotaroot to list (copies original)
CQuotaRoot* CQuotaProtocol::FindQuotaRoot(const char* txt)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return nil;

	// Create dummy for lookup
	CQuotaRoot dummy(txt);

	// Try to find it in existing list
	CQuotaRootList::iterator found = ::find(mRoots.begin(), mRoots.end(), dummy);
	
	// If found return it
	if (found != mRoots.end())
		return &(*found);
	else
		return nil;
}

// Set new value on server
void CQuotaProtocol::SetQuota(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Just send command to server
	// Server will take care of adding/replacing item in mRoots
	mClient->_SetQuota(root);
}

// Get value from server
void CQuotaProtocol::GetQuota(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Just send command to server
	// Server will take care of replacing item in mRoots
	mClient->_GetQuota(root);
}

// Get quota roots for a mailbox
void CQuotaProtocol::GetQuotaRoot(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Just send command to server
	// Server will take care of replacing item in mRoots
	mClient->_GetQuotaRoot(mbox);
}