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


// COptionsProtocol

#ifndef __COPTIONSPROTOCOL__MULBERRY__
#define __COPTIONSPROTOCOL__MULBERRY__

// COptionsProtocol: Handles quotas for all resources

#include "CINETProtocol.h"

#include "COptionsAccount.h"

// Classes
class COptionsClient;

class COptionsProtocol : public CINETProtocol
{
public:
	// Flags
	enum EOptionsFlags
	{
		eCanPartialReadWrite	= 1L << 16,
	};

		COptionsProtocol(COptionsAccount* account);
	virtual ~COptionsProtocol();

	virtual void	CreateClient();
	virtual void	CopyClient(const CINETProtocol& copy) {}
	virtual void	RemoveClient();

	virtual const COptionsAccount* GetOptionsAccount() const
		{ return static_cast<const COptionsAccount*>(GetAccount()); }
	virtual COptionsAccount* GetOptionsAccount()
		{ return static_cast<COptionsAccount*>(GetAccount()); }

			void	SetMap(cdstrmap* map)
		{ mMap = map; }
		cdstrmap*	GetMap() const
		{ return mMap; }

	virtual void	SetValue(const char* value)
		{ *mValue = value; }

			bool	DoesPartialPrefs() const;

	// Offline
	virtual const cdstring& GetOfflineDefaultDirectory();
	virtual void	GoOffline();

	// Handle options
	
	// Map based
			void	GetAllAttributes(const cdstring& entry);		// Get all options from client
			void	SetAllAttributes(const cdstring& entry);		// Write current map to client
			void	DeleteEntry(const cdstring& entry);				// Delete entry

	// Single
			void	GetAttribute(const cdstring& entry, const cdstring& attribute, cdstring& value);		// Get single option
			void	SetAttribute(const cdstring& entry, const cdstring& attribute, const cdstring& value);	// Set single option

protected:
	COptionsClient*	mClient;								// Its client
	cdstrmap*		mMap;									// Map read
	cdstring*		mValue;									// Single value read
};

#endif
