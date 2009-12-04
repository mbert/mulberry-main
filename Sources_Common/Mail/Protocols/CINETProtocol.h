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


// Header for CMailProtocol class

#ifndef __CINETPROTOCOL__MULBERRY__
#define __CINETPROTOCOL__MULBERRY__

#include "CBroadcaster.h"

#include "CINETAccount.h"

#include "cdmutex.h"
#include "cdstring.h"
#include "SBitFlags.h"

#include <time.h>

// Types

// Classes
class CINETClient;

class CINETProtocol : public CBroadcaster
{
public:
	// State Flags
	enum EFlags
	{
		eNone = 0,
		
		// Error handling state
		eErrorProcess			= 1L << 0,
		eNoErrorAlert			= 1L << 1,
		eNoRecovery				= 1L << 2,
		
		// Offline/disconnected state
		eIsOffline				= 1L << 3,
		eCanDisconnect			= 1L << 4,
		eForceDisconnect		= 1L << 5,
		eDisconnected			= 1L << 6
	};

	// State
	enum EINETState
	{
		eINETNotOpen,
		eINETOpen,
		eINETLoggedOn,
		eINETLoggedOff
	};

	// Messages for broadcast
	enum
	{
		eBroadcast_Logon = 'iplo',
		eBroadcast_Logoff = 'iplf'
	};

					CINETProtocol(CINETAccount* account);
					CINETProtocol(const CINETProtocol& copy);
	virtual 		~CINETProtocol();

	cdmutex& _get_mutex()
		{ return _mutex; }
	const cdmutex&	_get_mutex() const
		{ return _mutex; }

	virtual void	CreateClient() = 0;
	virtual void	CopyClient(const CINETProtocol& copy) = 0;
	virtual void	RemoveClient() = 0;

	// Flags
	void	SetFlags(EFlags new_flags, bool add = true)		// Set flags
		{ mFlags.Set(new_flags, add); }
	EFlags	GetFlags() const								// Get flags
		{ return (EFlags) mFlags.Get(); }

	// Account
	virtual const CINETAccount* GetAccount() const
		{ return mAccount; }
	virtual CINETAccount* GetAccount()
		{ return mAccount; }
	virtual const cdstring& GetAccountName() const
		{ return mAccountName; }
	virtual CINETAccount::EINETServerType GetAccountType() const
		{ return mAccountType; }
	virtual const cdstring& GetAccountUniqueness() const
		{ return mAccountUniqueness; }
	virtual void SetAccount(CINETAccount* account);
	virtual void RemoveAccount();
	virtual void DirtyAccount();

	virtual const cdstring& GetAuthenticatorUniqueness() const
		{ return mAuthenticatorUniqueness; }
	virtual void SetAuthenticatorUniqueness(const cdstring& uniqueness)
		{ mAuthenticatorUniqueness = uniqueness; }

			bool IsOfflineAllowed() const;
			bool IsOpenAllowed() const;

	// Check state

	virtual void	SetState(EINETState state)
		{ mMPState = state; }
	virtual bool	IsNotOpen() const
		{ return mMPState == eINETNotOpen; }
	virtual bool	IsOpen() const
		{ return mMPState == eINETOpen;}
	virtual bool	IsLoggedOn() const
		{ return mMPState == eINETLoggedOn; }
	virtual bool	IsLoggedOff() const
		{ return mMPState == eINETLoggedOff; }

	virtual bool	IsSecure() const;

	virtual void	SetErrorProcess(bool errorProcess)
		{ mFlags.Set(eErrorProcess, errorProcess); }
	virtual bool	IsErrorProcess() const
		{ return mFlags.IsSet(eErrorProcess); }

	virtual void	SetNoErrorAlert(bool no_error)
		{ mFlags.Set(eNoErrorAlert, no_error); }
	virtual bool	GetNoErrorAlert() const
		{ return mFlags.IsSet(eNoErrorAlert); }

	virtual void	SetNoRecovery(bool no_recovery)
		{ mFlags.Set(eNoRecovery, no_recovery); }
	virtual bool	GetNoRecovery() const
		{ return mFlags.IsSet(eNoRecovery); }

	static cdstring GetCachedPswd(const cdstring& uid);
	static void SetCachedPswd(const cdstring& uid, const cdstring& pswd);

	virtual void	SetDescriptor(const char* desc)
		{ mDescriptor = desc; }
	virtual const cdstring&	GetDescriptor() const
		{ return mDescriptor;}
	virtual cdstring GetURL(bool full = false) const;

	virtual void	SetType(const char* type)
		{ mType = type; }
	virtual const cdstring&	GetType() const
		{ return mType;}

	const cdstring& GetGreeting() const;
	const cdstring& GetCapability() const;
	const cdstring& GetCertText() const;

	// L O G I N  &  L O G O U T
	virtual void	Open();								// Open connection to protocol server
	virtual void	Close();							// Close connection to protocol server
	virtual void	Logon();							// Logon to protocol server
	virtual void	Logoff();							// Logoff from protocol server
	virtual void	Forceon();							// Forced reconnect
	virtual void	Forceoff();							// Forced close
	static  void	ClearUserPswdCache()				// Clear user/pswd cache
		{ sUserPswdCache.clear(); }
	virtual void	Abort(bool silent = true,			// Porgram initiated abort
							bool recover = false);

	virtual	void	SpendTime(bool force_tickle);		// Called during idle

	// O F F L I N E
	virtual bool	IsOffline() const
		{ return mFlags.IsSet(eIsOffline); }
	virtual bool	CanDisconnect() const
		{ return mFlags.IsSet(eCanDisconnect); }
	virtual bool	IsForceDisconnect() const
		{ return mFlags.IsSet(eForceDisconnect); }
	virtual bool	IsDisconnected() const
		{ return mFlags.IsSet(eDisconnected); }
	virtual const cdstring& GetOfflineCWD() const
		{ return mOfflineCWD; }
	virtual void	InitOffline();
	virtual void	InitOfflineCWD(bool check = true);
	virtual void	DefaultOfflineCWD(cdstring& cwd, bool check = true);
	virtual void	InitDisconnect();
	virtual const cdstring& GetOfflineDefaultDirectory() = 0;
	virtual void	RenameOffline();
	virtual void	RemoveOffline();
	virtual void	GoOffline();
	virtual void	GoOnline();
	virtual void	ForceDisconnect(bool force);
	virtual void	SetSynchronising();					// Set into synchronising mode

	// Caching
	CINETProtocol*	NewConnection();						// Get a new connection cloned from this one
	void			EndConnection(CINETProtocol* proto,		// Connection is freed
									bool close_it = false);
	void			ConnectionIdle();						// Idle time processing
	void			CloseConnection(CINETProtocol* proto);	// Close out a connection
	void			CleanConnections();						// Remove all free connections
	virtual CINETProtocol*	CloneConnection();				// Create duplicate, empty connection

protected:
	struct SConnectionCache
	{
		CINETProtocol*	mConnection;					// The connection
		bool			mInUse;							// Whether currently being used
		time_t			mFreeTime;						// Time that it was last freed
	};
	typedef std::vector<SConnectionCache> CConnections;

	cdmutex				_mutex;							// Thread blocking
	SBitFlags			mFlags;							// State flags
	CINETAccount*		mAccount;						// Account info
	cdstring			mAccountName;					// Account name
	CINETAccount::EINETServerType	mAccountType;		// Account type
	cdstring			mAccountUniqueness;				// Account uniqueness
	cdstring			mAuthenticatorUniqueness;		// Current authenticator uniqueness
	EINETState			mMPState;						// Current state of protocol
	cdstring			mDescriptor;					// Protocol description
	cdstring			mType;							// Type of server as string
	CINETClient*		mClient;						// The client
	static cdstrmap		sUserPswdCache;					// Cache of user id & password

	// Connection caching bits
	cdmutex				_cache_mutex;					// Mutex for connection cache
	CConnections		mCachedConnections;				// Connections in cache

	// Offline bits
	cdstring			mOfflineCWD;						// Directory for offline
};

// Stack based class to do nested protocol logins/logouts
class StProtocolLogin
{
public:
	StProtocolLogin(CINETProtocol* proto);
	~StProtocolLogin();
private:
	CINETProtocol* mProtocol;
	bool mWasLoggedIn;
};

#endif
