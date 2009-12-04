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


// Header for INET client class

// This is an abstract base class that can be used with IMAP, IMSP & ACAP protocols

#ifndef __CINETCLIENT__MULBERRY__
#define __CINETCLIENT__MULBERRY__

#include "CLog.h"
#include "CNetworkException.h"
#include "CTCPStream.h"
#include "CStreamTypeFwd.h"

#include "cdstring.h"

#include <time.h>

#include <vector>

extern const char* cSpace;

// Others

#define TAG_TEST(x)		(((x)==cTagOK) || \
						 ((x)==cTagNO) || \
						 ((x)==cTagBAD))
#define STAR_TEST(x)	(((x)==cStarBYE) || \
						 ((x)==cStarOK) || \
						 ((x)==cStarNO) || \
						 ((x)==cStarBAD))	
#define PLUS_TEST(x)	((x)==cPlusLabel)

// Classes
class CINETProtocol;
class CTCPException;

class CINETClient
{
public:
	enum EINETResponseCode
	{
		cTagOK,
		cTagNO,
		cTagBAD,
		cStarOK,
		cStarNO,
		cStarBAD,
		cStarPREAUTH,
		cStarBYE,
		cStarCAPABILITY,
		cPlusLabel,
		cNoResponse,
		cResponseError,
		cINETResponseCode_Last
	};

protected:
	class CINETClientResponse
	{
	public:
		int			code;								// Code
		cdstring	tag_msg;							// Tagged message (stripped of tag and code)
		cdstrvect	untag_msgs;							// Untagged message list (stripped of * only)

		CINETClientResponse();
		CINETClientResponse(const CINETClientResponse& copy);				// Copy constructor

		CINETClientResponse& operator=(const CINETClientResponse& copy);	// Assignment with same type
		~CINETClientResponse() {};

		bool	FindTagged(const char* tagged) const;			// Find some text in a tagged response
		const cdstring& GetTagged() const						// Get tagged response
			{ return tag_msg; }
		
		void	AddUntagged(const char* untagged);				// Added untagged response to list
		bool	CheckUntagged(const char* untagged,				// Check for some text in an untagged response
								bool token = false) const;
		

		const cdstring& GetUntagged(const char* key) const;		// Get untagged response matching key
		cdstring PopUntagged(const char* key);					// Get untagged response matching key and remove from list
		cdstring PopUntagged();									// get last untagged response added

		void	Clear();										// Clear messages
	};

public:
	class CINETException : public CNetworkException
	{
	public:
		enum { class_ID = 'inet' };

		enum EINETException
		{
			err_NoResponse = '*NO ',
			err_BadResponse = '*BAD',
			err_BadParse = '*PAR',
			err_INETUnknown = '*UKN'
		};

		// Only allow construction from the error codes we know about
		CINETException(EINETException err_code) : CNetworkException(err_code)
			{ _class = class_ID; }
	};

protected:
	class StINETClientAction
	{
	public:
		StINETClientAction(CINETClient* client, const char* status_id, const char* err_id, const char* nobad_id, const cdstring& err_context = cdstring::null_str)
			{ mClient = client; client->INETStartAction(status_id, err_id, nobad_id, err_context); }
		~StINETClientAction()
			{ mClient->INETStopAction(); }
	private:
		CINETClient* mClient;
	};

	class StINETClientPause
	{
	public:
		StINETClientPause(CINETClient* client)
			{ mClient = client; mWasBusy = client->INETPauseAction(true); }
		~StINETClientPause()
			{ if (mWasBusy) mClient->INETPauseAction(false); }
	private:
		CINETClient* mClient;
		bool mWasBusy;
	};

	class StINETClientSilent
	{
	public:
		StINETClientSilent(CINETClient* client)
			{ mClient = client; mWasSilent = mClient->mSilentCommand; mClient->mSilentCommand = true; }
		~StINETClientSilent()
			{ mClient->mSilentCommand = mWasSilent; }
	private:
		CINETClient* mClient;
		bool mWasSilent;
	};

	friend class StINETClientAction;
	friend class StINETClientPause;
	friend class StINETClientSilent;

	// I N S T A N C E  V A R I A B L E S
	
	CINETProtocol*		mOwner;							// Protocol that owns this
	CINETAccount*		mPrivateAccount;				// Private account
	CTCPStream*			mStream;						// Comms stream
	cdstring			mServerAddr;					// Text of server address
	cdstring			mGreeting;						// Greeting string
	cdstring			mCapability;					// Capability string
	bool				mAsyncLiteral;					// Non-synchronising literals supported
	bool				mLoginAllowed;					// Has 'LOGIN' command
	bool				mAuthLoginAllowed;				// Has 'AUTH=LOGIN' command
	bool				mAuthPlainAllowed;				// Has 'AUTH=PLAIN' command
	bool				mAuthAnonAllowed;				// Has 'AUTH=ANONYMOUS' command
	bool				mAuthInitialClientData;			// Allows initial client data with AUTH
	bool				mAuthBase64;					// Do Base64 encoding of AUTH data
	bool				mSTARTTLSAllowed;				// Has STARTTLS capability
	char*				mLineData;						// Receive line buffer
	char*				mLongLine;						// Receive long line buffer
	char				mTag[16];						// Tag
	unsigned long		mTagCount;						// Tag counter
	bool				mUseTag;						// Use tags
	bool				mDoesTaggedResponses;			// Protocol does tagged responses
	CINETClientResponse	mLastResponse;					// Last response code received
	LStream*			mRcvStream;						// Stream to receive data
	costream*			mRcvOStream;					// Stream to copy data
	bool				mSilentCommand;					// Do not display status
	bool				mSilentError;					// Do not explicitly handle error
	bool				mAllowLog;						// Allow logging
	CLog				mLog;							// Logging control
	CLog::ELogType		mLogType;						// Type of log
	unsigned long		mItemCtr;						// Count different types of items being read in
	unsigned long		mItemTotal;						// Total number of items to be read in
	time_t				mStatusUpdateTime;				// Time since last status update
	unsigned long		mStatusQueue;					// Pending status items
	const char*			mStatusID;						// Status string
	const char*			mErrorID;						// Error string for general errors
	const char*			mNoBadID;						// Error string for NO's and BAD's
	cdstring			mErrContext;					// Object specific error context

	enum
	{
		eQueueNoFlags = 0,
		eQueueProcess = 1 << 0,
		eQueueLiteral = 1 << 1,
		eQueueManualLiteral = 1 << 2,
		eQueueBuffer = 1 << 3
	};
		
	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CINETClient();
			CINETClient(CINETProtocol* owner);
			CINETClient(const CINETClient& copy, CINETProtocol* owner);
	virtual	~CINETClient();

	virtual CINETClient*	CloneConnection();				// Create duplicate, empty connection

	CINETAccount*	GetPrivateAccount() const
	{
		return mPrivateAccount;
	}
	void SetPrivateAccount(CINETAccount* acct);
	CINETAccount*	GetAccount() const;

	const cdstring& GetCertText() const;

private:
			void	InitINETClient();

	// O T H E R  M E T H O D S
protected:
	virtual void	SendString(const char* str, int flags);					// Add string for output processing
	virtual int		ProcessString(cdstring& str);				// Process for output (maybe quote or literalise)
	virtual int		GetManualLiteralLength();				// Get length of manually processed literal
	virtual void	SendManualLiteral();					// Send manual literal

	virtual void	InitItemCtr(unsigned long total = 0);		// Initialise item counter
	virtual void	BumpItemCtr(const char* rsrcid);			// Bump the item counter
	virtual void	DisplayItemCtr(const char* rsrcid) const;	// Display the item counter

public:
	// S T A R T  &  S T O P
	virtual void	Open();									// Start TCP (specify server)
	virtual void	Reset();								// Reset acount info
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort
	virtual void	Forceoff();								// Forced close

protected:
	virtual void	LookupServer();							// Lookup server on net

public:
	// L O G I N  &  L O G O U T
	virtual void	Logon();								// Logon to server

protected:
	virtual tcp_port GetDefaultPort();							// Get default port;
	virtual const char*	GetAuthCommand() const;					// Get name of command for SASL
	virtual const char*	GetStartTLSCommand() const;				// Get name of command for TLS
	virtual bool	DoTLSClientCertificate();					// Setup TLS certificate
	virtual void	DoStartTLS();								// Start TLS
	virtual bool	DoAuthentication();							// Do authentication
	virtual void	DoAnonymousAuthentication();				// Do authentication
	virtual void	DoPlainAuthentication();					// Do authentication
	virtual void	DoExternalAuthentication();					// Do authentication
	virtual void	DoPluginAuthentication();					// Do authentication

public:
	virtual void	Logoff();									// Logoff from server

	virtual const cdstring& GetGreeting()
		{ return mGreeting; }

	virtual const cdstring& GetCapability()
		{ return mCapability; }

	// P R O T O C O L
	virtual void	_Tickle(bool force_tickle);				// Do tickle
protected:
	virtual bool	_ProcessGreeting();						// Process greeting response
	virtual void	_Capability(bool after_tls = false);	// Determine version
	virtual void	_InitCapability() {}					// Initialise capability flags to empty set
	virtual void	_ProcessCapability() {}					// Handle capability response
	virtual void	_NoCapability() {}						// Handle failed capability response
	virtual void	_Authenticate();						// Authenticate with server

	virtual void	_PreProcess() {}						// About to start processing input
	virtual void	_PostProcess() {}						// Finished processing input

	virtual void	_ParseResponse(char** txt,
							CINETClientResponse* response) {}	// Parse text sent by server

	// S T A T U S
	virtual void	INETStartAction(const char* status_id,			// Starting an INET call
									const char* err_id,
									const char* nobad_id,
									const cdstring& err_context);
	virtual bool	INETPauseAction(bool pause);					// Pause UI busy
	virtual void	INETStopAction();								// Stopping an INET call

	// S E N D  D A T A
	virtual void	INETNextTag();									// Update tag
	virtual void	INETStartSend(const char* status_id, const char* err_id, const char* nobad_id,
									const cdstring& err_context = cdstring::null_str, bool handle_throw = true);
	virtual void	INETSendString(const char* str, int flags = eQueueNoFlags, bool handle_throw = true);
	virtual void	INETFinishSend(bool handle_throw = true);

	// R E C E I V E  D A T A
	virtual char*	INETGetLine();									// Get a line of data - possibly long
	virtual void	INETProcess();									// Process IMAP request

	// H A N D L E  R E S P O N S E
	virtual bool	INETCompareResponse(EINETResponseCode compare);		// Compare responses
	virtual bool	INETCheckLastResponse(EINETResponseCode compare);	// Check last response

	virtual void 	INETParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server
	virtual void	INETParseTagged(char** txt,
							CINETClientResponse* response);		// Parse tagged response

	// H A N D L E  E R R O R S
	virtual void	INETHandleError(std::exception& ex,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition

	virtual void	INETDisplayError(std::exception& ex,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition
	virtual void	INETDisplayError(CINETException& ex,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition
	virtual void	INETDisplayError(CTCPException& ex,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition
	virtual void	INETDisplayError(ExceptionCode err,
							const char* err_id,
							const char* nobad_id);					// Handle an error condition

	virtual void	INETTryRecoverError(CNetworkException& ex,	// Try to recover from the error
							const char* err_id,
							const char* nobad_id);
	virtual void	INETRecoverReconnect();						// Force reconnect
	virtual void	INETRecoverDisconnect();					// Force disconnect
	virtual void	INETGetErrorContext(cdstring& error) const;		// Get error context string
	virtual const char*	INETGetErrorDescriptor() const;				// Descriptor for object error context

	// P A R S I N G  S T R I N G S
	virtual void	INETParseMultiString(char** txt,					// Parse INET multi-valued string reply
											cdstrvect& strs,
											bool nullify = false);
	virtual char*	INETParseString(char** txt, bool nullify = false);	// Parse INET string reply
	virtual void	INETParseStringStream(char** txt);					// Parse INET string reply to a stream

	virtual const char* INETGetAtomSpecials() const;					// Get ATOM special characters for parsing
};

#endif
