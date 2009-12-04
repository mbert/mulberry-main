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


// Header for SIEVE client class

#ifndef __CSIEVECLIENT__MULBERRY__
#define __CSIEVECLIENT__MULBERRY__

#include "CFilterClient.h"

// Others
class CSIEVEClient: public CFilterClient
{
	enum ECommandState
	{
		eNone = 0,
		eCAPABILITY,
		eHASSPACE,
		ePUTSCRIPT,
		eLISTSCRIPTS,
		eSETACTIVE,
		eGETSCRIPT,
		eDELETESCRIPT
	};

	class StCommandState
	{
	public:
		StCommandState(ECommandState state, CSIEVEClient* client)
			{ mClient = client; mClient->mCommandState = state; }
		~StCommandState()
			{ mClient->mCommandState = eNone; }
	private:
		CSIEVEClient* mClient;
	};
	
	friend class StCommandState;

	// I N S T A N C E  V A R I A B L E S

private:
	ECommandState					mCommandState;
	cdstrmap						mCapabilities;
	CFilterProtocol::EExtension		mExtensions;
	cdstrvect*						mStringListResult;
	cdstring*						mStringResult;

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CSIEVEClient(CFilterProtocol* owner);
			CSIEVEClient(const CSIEVEClient& copy,
						CFilterProtocol* owner);
	virtual	~CSIEVEClient();

private:
			void	InitSIEVEClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual int		ProcessString(cdstring& str);			// Process for output (maybe quote or literalise)

protected:
	virtual tcp_port GetDefaultPort();						// Get default port;

	// P R O T O C O L
	virtual bool	_ProcessGreeting();						// Process greeting response
	virtual void	_Capability(bool after_tls = false);	// Determine version
	virtual void	_InitCapability();						// Initialise capability flags to empty set
	virtual void	_ProcessCapability();					// Handle capability response
	virtual void	_NoCapability();						// Handle failed capability response

	virtual void	_PreProcess();							// About to start processing input
	virtual void	_PostProcess();							// Finished processing input

	virtual void	_ParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// COMMANDS
	virtual bool	_HasExtension(CFilterProtocol::EExtension ext) const;		// Check for server extension
	virtual void	_HaveSpace(const cdstring& name,							// Check script size write to server
								unsigned long size);
	virtual void	_PutScript(const cdstring& name,							// Store script on server
								const cdstring& script);
	virtual void	_ListScripts(cdstrvect& scripts,							// List available scripts and the active one
									cdstring& active);
	virtual void	_SetActive(const cdstring& script);							// Make server script the active one
	virtual void	_GetScript(const cdstring& name,							// Get named script
									cdstring& script);
	virtual void	_DeleteScript(const cdstring& script);						// Delete names script

protected:
	// H A N D L E  E R R O R
	virtual void	INETRecoverReconnect();					// Force reconnect
	virtual void	INETRecoverDisconnect();				// Force disconnect

	// H A N D L E  R E S P O N S E
	virtual void	INETParseResponse(char** txt,
							CINETClientResponse* response);		// Parse general response
	virtual void 	INETParseTagged(char** txt,
							CINETClientResponse* response);		// Parse tagged response sent by server

	void 	SIEVEParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
	void 	SIEVEParseResponseCode(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// P A R S I N G  I N F O


	// P A R S I N G  SIEVE  I N F O
			void	SIEVEParseCAPABILITYResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	SIEVEParseLISTSCRIPTSResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	SIEVEParseGETSCRIPTResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	SIEVEParseExtensionItem(char** txt);		// Parse SIEVE extension items
};

#endif
