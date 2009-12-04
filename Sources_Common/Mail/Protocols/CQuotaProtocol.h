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

#ifndef __CQUOTAPROTOCOL__MULBERRY__
#define __CQUOTAPROTOCOL__MULBERRY__

#include "CQuotas.h"

// CQuotaProtocol: Handles quotas for all resources

class CMbox;
class CQuotaClient;

class CQuotaProtocol
{
public:
		CQuotaProtocol();
	virtual ~CQuotaProtocol() {}
	
	// Status of Quota support
	virtual void SetHasQuota(bool has_quota)
		{ mHasQuota = has_quota; }
	virtual bool GetHasQuota()
		{ return mHasQuota; }
	virtual void SetDisabled(bool disabled)
		{ mDisabled = disabled; }
	virtual bool GetDisabled()
		{ return mDisabled; }

	virtual bool UsingQuotas()							// Indicates that client can use quotas
		{ return mHasQuota && !mDisabled; }

	virtual void AddQuotaRoot(CQuotaRoot* root);			// Add unique quotaroot
	virtual CQuotaRoot* FindQuotaRoot(const char* txt);		// Find quotaroot from title

	virtual void SetQuota(CQuotaRoot* root);				// Set new value on server
	virtual void GetQuota(CQuotaRoot* root);				// Get value from server
	virtual void GetQuotaRoot(CMbox* mbox);					// Get quota roots for a mailbox

protected:
	CQuotaClient*	mClient;								// Its client
	CQuotaRootList	mRoots;									// List of quota roots known about on this server
	bool			mHasQuota;								// Does server support quotas?
	bool			mDisabled;								// Quotas disabled by client
};

#endif
