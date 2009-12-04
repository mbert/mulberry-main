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


// Header for Local Prefs client class

#ifndef __CLOCALPREFSCLIENT__MULBERRY__
#define __CLOCALPREFSCLIENT__MULBERRY__

#include "COptionsClient.h"

#include "cdfstream.h"

// consts

class CLocalPrefsClient: public COptionsClient
{

	// I N S T A N C E  V A R I A B L E S

private:
	cdstring		mCWD;							// Working directory for entire hierarchy
	cdfstream		mPrefs;

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CLocalPrefsClient(COptionsProtocol* options_owner);
			CLocalPrefsClient(const CLocalPrefsClient& copy,
						COptionsProtocol* options_owner);
	virtual	~CLocalPrefsClient();

private:
			void	InitPrefsClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

protected:

	// S T A R T  &  S T O P
	virtual void	Open();									// Start TCP (specify server)
	virtual void	Reset();								// Reset account
public:
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort

	// L O G I N  &  L O G O U T
	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server

	// P R O T O C O L
	virtual void	_Tickle(bool force_tickle);			// Do tickle
	virtual void	_PreProcess();						// About to start processing input
	virtual void	_PostProcess();						// Finished processing input

	// O P T I O N S
	virtual void	_FindAllAttributes(const cdstring& entry);		// Find all attributes in entry
	virtual void	_SetAllAttributes(const cdstring& entry);		// Set all attributes in entry
	virtual void	_DeleteEntry(const cdstring& entry);			// Delete the entire entry
	virtual void	_GetAttribute(const cdstring& entry,			// Get attribute
									const cdstring& attribute);
	virtual void	_SetAttribute(const cdstring& entry,			// Set attribute
									const cdstring& attribute,
									const cdstring& value);
	virtual void	_DeleteAttribute(const cdstring& entry,			// Delete attribute
									const cdstring& attribute);


protected:
	// H A N D L E  E R R O R

	// L O C A L  O P S
	
			void	GetFileName(cdstring& name);
};

#endif
