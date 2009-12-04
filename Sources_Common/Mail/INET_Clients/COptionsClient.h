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


// COptionsClient

// Abstract base class that encapsulates the functionality of a remote options store.
// This is used by the high level COPtionsProtocol object to control the options store.
// Derived classes will implement the backend options client.
// e.g. CIMSPClient is derived from this class. Another class could be CACAPClient.

#ifndef __COPTIONSCLIENT__MULBERRY__
#define __COPTIONSCLIENT__MULBERRY__

#include "CINETClient.h"

#include "COptionsProtocol.h"

#include "cdstring.h"

// Classes

class COptionsClient : virtual public CINETClient
{
public:
		COptionsClient(COptionsProtocol* owner)
			: CINETClient(owner)
			{ InitOptionsClient(owner); }
		COptionsClient(const COptionsClient& copy, COptionsProtocol* owner)
			: CINETClient(copy, owner)
			{ InitOptionsClient(owner); }
	virtual ~COptionsClient() { mSingleKey = NULL; }

	virtual void	SetVendor(const cdstring& vendor)
		{ mVendor = vendor; }
	virtual void	SetProduct(const cdstring& product)
		{ mProduct = product; }

	virtual void	_FindAllAttributes(const cdstring& entry) = 0;		// Find all attributes in entry
	virtual void	_SetAllAttributes(const cdstring& entry) = 0;		// Set all attributes in entry
	virtual void	_DeleteEntry(const cdstring& entry) = 0;			// Delete the entire entry
	virtual void	_GetAttribute(const cdstring& entry,				// Get attribute
									const cdstring& attribute) = 0;
	virtual void	_SetAttribute(const cdstring& entry,				// Set attribute
									const cdstring& attribute,
									const cdstring& value) = 0;
	virtual void	_DeleteAttribute(const cdstring& entry,				// Delete attribute
									const cdstring& attribute) = 0;

protected:
	cdstring			mVendor;									// Vendor namespace
	cdstring			mProduct;									// Product namespace
	bool				mOptions;									// Running as options server
	unsigned long		mPreKeySize;								// Size of text added to front of key
	bool				mSingle;									// Looking up single option
	const cdstring*		mSingleKey;									// Key to match

	COptionsProtocol*	GetOptionsOwner() const							// Return type-cast owner
		{ return static_cast<COptionsProtocol*>(mOwner); }

private:
	void InitOptionsClient(COptionsProtocol* owner)
		{ mOptions = false; mPreKeySize = 0; mSingle = false; mSingleKey = NULL; }
};

#endif
