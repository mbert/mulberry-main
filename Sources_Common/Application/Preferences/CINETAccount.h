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


// CINETAccount.h

// Header file for class/structs that define mail accounts

#ifndef __CINETACCOUNT__MULBERRY__
#define __CINETACCOUNT__MULBERRY__

#include "CAuthenticator.h"
#include "CDisplayItem.h"
#include "CFutureItems.h"

#include "cdstring.h"

#include "ptrvector.h"

// Classes

class CINETAccount
{
public:
	enum EINETServerType
	{
		eUnknown = 0,
		eIMAP,
		ePOP3,
		eLocal,
		eSMTP,
		eNNTP,
		eWebDAVPrefs,
		eIMSP,
		eACAP,
		eLDAP,
		eWHOISPP,
		eFinger,
		eLocalAdbk,
		eManageSIEVE,
		eOSAdbk,
		eLocalCalendar,
		eHTTPCalendar,
		eWebDAVCalendar,
		eCalDAVCalendar,
		eCardDAVAdbk,
		eMax = eCardDAVAdbk
	};

	enum ETLSType
	{
		eNoTLS = 0,
		eSSL,
		eSSLv3,
		eTLS,
		eTLSBroken,
		eTLSTypeMax = eTLSBroken
	};

			CINETAccount();
			CINETAccount(const CINETAccount& copy)
				{ _copy(copy); }
	virtual	~CINETAccount() { mProtocol = NULL; }

	CINETAccount& operator=(const CINETAccount& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CINETAccount& comp) const;				// Compare with same type

	// Initialise
	virtual void NewAccount();									// New account being created

	// Getters/setters
	virtual void* GetProtocol() const
		{ return mProtocol; }
	virtual void SetProtocol(void* proto)
		{ mProtocol = proto; }

	virtual const cdstring& GetUniqueness() const
		{ return mServerIP; }

	virtual const cdstring& GetName() const
		{ return mName; }
	virtual void SetName(const cdstring& name)
		{ mName = name; }

	virtual EINETServerType GetServerType() const
		{ return mServerType; }
	virtual const char* GetServerTypeString() const;
	virtual void SetServerType(EINETServerType type);

	const cdstring&	GetServerIP() const
		{ return mServerIP; }
	void SetServerIP(const cdstring& server_ip)
		{ mServerIP = server_ip; }

	ETLSType GetTLSType() const
		{ return mTLSType; }
	void SetTLSType(ETLSType type)
		{ mTLSType = type; }
	bool SupportsTLSType(ETLSType type) const;
	bool IsSecure() const
	{
		// Check for TLS
		return mTLSType != eNoTLS;
	}

	bool GetUseTLSClientCert() const
		{ return mUseTLSClientCert; }
	void SetUseTLSClientCert(bool use)
		{ mUseTLSClientCert = use; }

	const cdstring& GetTLSClientCert() const
		{ return mTLSClientCert; }
	void SetTLSClientCert(const cdstring cert)
		{ mTLSClientCert = cert; }

	virtual const CAuthenticator& GetAuthenticator() const
		{ return mAuthenticator; }
	virtual CAuthenticator& GetAuthenticator()
		{ return mAuthenticator; }
	virtual CAuthenticator::EAuthenticators GetAuthenticatorType() const
		{ return mAuthenticator.GetAuthenticatorType(); }

	virtual CAuthenticatorUserPswd* GetAuthenticatorUserPswd()
		{ return (CAuthenticatorUserPswd*) mAuthenticator.GetAuthenticator(); }
	virtual const CAuthenticatorUserPswd* GetAuthenticatorUserPswd() const
		{ return (CAuthenticatorUserPswd*) mAuthenticator.GetAuthenticator(); }

	virtual CAuthenticatorKerberos* GetAuthenticatorKerberos()
		{ return (CAuthenticatorKerberos*) mAuthenticator.GetAuthenticator(); }
	virtual const CAuthenticatorKerberos* GetAuthenticatorKerberos() const
		{ return (CAuthenticatorKerberos*) mAuthenticator.GetAuthenticator(); }

	virtual cdstring GetInfo() const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

	bool	GetLogonAtStart() const
		{ return mLogonAtStart; }
	void	SetLoginAtStart(bool logon_at_start)
		{ mLogonAtStart = logon_at_start; }

	CDisplayItem& GetCWD()
		{ return mCWD; }
	const CDisplayItem& GetCWD() const
		{ return mCWD; }

protected:
	cdstring				mName;
	EINETServerType			mServerType;
	cdstring				mServerIP;
	ETLSType				mTLSType;
	bool					mUseTLSClientCert;
	cdstring				mTLSClientCert;
	CAuthenticator			mAuthenticator;
	bool					mLogonAtStart;
	CDisplayItem			mCWD;
	CFutureItems			mFuture;
	void*					mProtocol;

private:
	void _copy(const CINETAccount& copy);
};

typedef ptrvector<CINETAccount> CINETAccountList;

#endif
