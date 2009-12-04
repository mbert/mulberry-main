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

#include "CFilterProtocol.h"

#include "CConnectionManager.h"
#include "CSIEVEClient.h"

// CFilterProtocol: Handles server-based filters

// Constructor
CFilterProtocol::CFilterProtocol(CINETAccount* account)
	: CINETProtocol(account)
{
	mClient = NULL;

	CreateClient();
}

// Default destructor
CFilterProtocol::~CFilterProtocol()
{
	// Delete client
	RemoveClient();

}

void CFilterProtocol::CreateClient()
{

	RemoveClient();

	mClient = new CSIEVEClient(this);

	CINETProtocol::mClient = mClient;
}

void CFilterProtocol::RemoveClient()
{
	delete mClient;
	mClient = NULL;
	CINETProtocol::mClient = NULL;
}

#pragma mark ____________________________Commands

// Check for server extension
bool CFilterProtocol::HasExtension(EExtension ext) const
{
	return mClient->_HasExtension(ext);
}

// Check script size write to server
void CFilterProtocol::HaveSpace(const cdstring& name, unsigned long size)
{
	mClient->_HaveSpace(name, size);
}
					
// Store script on server
void CFilterProtocol::PutScript(const cdstring& name, const cdstring& script)
{
	mClient->_PutScript(name, script);
}

// List available scripts
void CFilterProtocol::ListScripts(cdstrvect& scripts, cdstring& active)
{
	mClient->_ListScripts(scripts, active);
}

// Make server script the active one
void CFilterProtocol::SetActive(const cdstring& script)
{
	mClient->_SetActive(script);
}

// Get named script
void CFilterProtocol::GetScript(const cdstring& name, cdstring& script)
{
	mClient->_GetScript(name, script);
}

// Delete names script
void CFilterProtocol::DeleteScript(const cdstring& script)
{
	mClient->_DeleteScript(script);
}

#pragma mark ____________________________Disconnected

const cdstring& CFilterProtocol::GetOfflineDefaultDirectory()
{
	// This never gets used as we don't support disconnected sync with filters
	return cdstring::null_str;
}
