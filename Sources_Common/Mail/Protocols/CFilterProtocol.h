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


// CFilterProtocol

#ifndef __CFILTERPROTOCOL__MULBERRY__
#define __CFILTERPROTOCOL__MULBERRY__

// CFilterProtocol: Handles quotas for all resources

#include "CINETProtocol.h"

// Classes
class CFilterClient;

class CFilterProtocol : public CINETProtocol
{
public:
	enum EExtension
	{
		eNone	 	= 0,
		eReject		= 1L << 0,
		eFileInto	= 1L << 1,
		eVacation	= 1L << 2,
		eRelational = 1L << 3,
        eIMAP4Flags = 1L << 4
	};

			CFilterProtocol(CINETAccount* account);
	virtual ~CFilterProtocol();

	virtual void	CreateClient();
	virtual void	CopyClient(const CINETProtocol& copy) {}
	virtual void	RemoveClient();

	// Offline
	virtual const cdstring& GetOfflineDefaultDirectory();

	// Commands
			bool	HasExtension(EExtension ext) const;							// Check for server extension
			void	HaveSpace(const cdstring& name, unsigned long size);		// Check script size write to server
								
			void	PutScript(const cdstring& name, const cdstring& script);	// Store script on server
								
			void	ListScripts(cdstrvect& scripts, cdstring& active);			// List available scripts
									
			void	SetActive(const cdstring& script);							// Make server script the active one
			void	GetScript(const cdstring& name, cdstring& script);			// Get named script
									
			void	DeleteScript(const cdstring& script);					// Delete names script

protected:
	CFilterClient*	mClient;								// Its client
};

#endif
