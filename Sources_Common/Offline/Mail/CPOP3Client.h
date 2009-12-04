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


// Header for POP3 client class

#ifndef __CPOP3CLIENT__MULBERRY__
#define __CPOP3CLIENT__MULBERRY__

#include "CLocalClient.h"

// Classes

class CPOP3Client: public CLocalClient
{
	enum ECommandState
	{
		eNone = 0,
		eSTAT,
		eLIST,
		eRETR,
		eDELE,
		eNOOP,
		eRSET,
		eQUIT,
		eTOP,
		eUIDL,
		eUSER,
		ePASS,
		eAPOP,
		eCAPA,
		eAUTH,
		eSTLS
	};

	class StCommandState
	{
	public:
		StCommandState(ECommandState state, CPOP3Client* client)
			{ mClient = client; mClient->mCommandState = state; }
		~StCommandState()
			{ mClient->mPOP3Response.clear(); mClient->mCommandState = eNone; }
	private:
		CPOP3Client* mClient;
	};
	
	friend class StCommandState;

	// I N S T A N C E  V A R I A B L E S

private:
	cdstrvect		mPOP3Response;
	ECommandState	mCommandState;
	bool			mDoesAPOP;
	cdstring		mAPOPKey;
	bool			mDoesUIDL;
	bool			mDoesCAPA;

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CPOP3Client(CMboxProtocol* owner);
			CPOP3Client(const CPOP3Client& copy, CMboxProtocol* owner);
	virtual	~CPOP3Client();

private:
			void	InitPOP3Client();

public:
	virtual CINETClient*	CloneConnection();		// Create duplicate, empty connection

	// S T A R T  &  S T O P
protected:
	virtual void	CheckCWD();								// Check CWD
public:
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort

protected:
	virtual tcp_port GetDefaultPort();						// Get default port;
	virtual const char*	GetAuthCommand() const;				// Get name of command for TLS
	virtual const char*	GetStartTLSCommand() const;				// Get name of command for TLS

	virtual void	DoStartTLS();							// Start TLS
	virtual void	DoAnonymousAuthentication();			// Do authentication
	virtual void	DoPlainAuthentication();				// Do authentication

public:
	virtual void	Logoff();								// Logoff from server

	// P R O T O C O L
	virtual void	_Tickle(bool force_tickle);				// Do tickle
protected:
	virtual bool	_ProcessGreeting();						// Process greeting response
	virtual void	_Capability(bool after_tls = false);	// Determine version
	virtual void	_InitCapability();						// Initialise capability flags to empty set
	virtual void	_ProcessCapability();					// Handle capability response
	virtual void	_NoCapability();						// Handle failed capability response

	// H A N D L E  R E S P O N S E
	virtual void 	INETParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	virtual void	_ParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// M B O X
	virtual void	_OpenMbox(CMbox* mbox);				// Do open mailbox
	virtual void	_CloseMbox(CMbox* mbox);			// Do close mailbox
	virtual void	_CheckMbox(CMbox* mbox,				// Do check
								bool fast = false);

	// M E S S A G E S

	// A C L S

	// Q U O T A S

protected:
	CMbox* mPOP3INBOX;

	// H A N D L E  E R R O R

	// L O C A L  O P S
			void	DoPOP3(CMbox* mbox);
			void	DoMD5(unsigned char [16], const cdstring& value) const;
			void	DoMD5Hex(cdstring& digest, const cdstring& value) const;
			void	NewUIDLs(const ulvector& uidls, ulvector& msgs, ulvector& deletes);
			void	PopMessages(CMbox* mbox, const ulvector& msgs, const ulvector& uidls, const ulvector& sizes);

			void	GetRecorderName(const CMbox* mbox, cdstring& record_name) const;

	// C O M M A N D S
			void	DoSTAT(unsigned long& msgs, unsigned long& size);
			void	DoLIST(ulvector& sizes);
			void	DoRETR(unsigned long msg);
			void	DoDELE(unsigned long msg);
			void	DoTOP(unsigned long msg);
			void	DoUIDL(ulvector& uidls);
			void	DoUSER(const cdstring& user);
			void	DoPASS(const cdstring& pass);
			void	DoAPOP(const cdstring& user, const cdstring& pass);

	// P A R S I N G  P O P 3  I N F O
			void	GetResponseLines(CINETClientResponse* response);		// Parse text sent by server
			void	GetResponseStream();						// Parse text sent by server

			void	POP3ParseSTATResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	POP3ParseLISTResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	POP3ParseRETRResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	POP3ParseTOPResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	POP3ParseUIDLResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
			void	POP3ParseCAPAResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
};

#endif
