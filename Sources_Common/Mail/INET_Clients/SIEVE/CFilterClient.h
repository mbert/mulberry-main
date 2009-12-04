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


// CFilterClient

// Abstract base class that encapsulates the functionality of a remote options store.
// This is used by the high level CFilterProtocol object to control the options store.
// Derived classes will implement the backend options client.
// e.g. CSIEVEClient is derived from this class.

#ifndef __CFILTERCLIENT__MULBERRY__
#define __CFILTERCLIENT__MULBERRY__

#include "CINETClient.h"

#include "CFilterProtocol.h"

#include "cdstring.h"

// Classes

class CFilterClient : public CINETClient
{
public:
		CFilterClient(CFilterProtocol* owner)
			: CINETClient(owner) {}
		CFilterClient(const CFilterClient& copy, CFilterProtocol* owner)
			: CINETClient(copy, owner) {}
	virtual ~CFilterClient() {}

	virtual bool	_HasExtension(CFilterProtocol::EExtension ext) const = 0;	// Check for server extension
	virtual void	_HaveSpace(const cdstring& name,							// Check script size write to server
								unsigned long size) = 0;
	virtual void	_PutScript(const cdstring& name,							// Store script on server
								const cdstring& script) = 0;
	virtual void	_ListScripts(cdstrvect& scripts,							// List available scripts
									cdstring& active) = 0;
	virtual void	_SetActive(const cdstring& script) = 0;						// Make server script the active one
	virtual void	_GetScript(const cdstring& name,							// Get named script
									cdstring& script) = 0;
	virtual void	_DeleteScript(const cdstring& script) = 0;					// Delete names script
};

#endif
