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


// CQuotaClient

#ifndef __CQUOTACLIENT__MULBERRY__
#define __CQUOTACLIENT__MULBERRY__

// CQuotaClient: Handles quotas for all resources

class CMbox;
class CQuotaProtocol;
class CQuotaRoot;

class CQuotaClient
{
public:
		CQuotaClient(CQuotaProtocol* owner)
			{ mOwner = owner; 
			  mQuotaMbox = nil; }
		CQuotaClient(const CQuotaClient& copy, CQuotaProtocol* owner)
			{ mOwner = owner; 
			  mQuotaMbox = nil; }
	virtual ~CQuotaClient() {}
	
	// These must be implemented by specific client
	virtual void _SetQuota(CQuotaRoot* root) = 0;			// Set quota root values on server
	virtual void _GetQuota(CQuotaRoot* root) = 0;			// Get quota root values from server
	virtual void _GetQuotaRoot(CMbox* mbox) = 0;			// Get quota roots for a mailbox

protected:
	CQuotaProtocol*	mOwner;
	CMbox*			mQuotaMbox;								// Mailbox to get quotas for

};

#endif
