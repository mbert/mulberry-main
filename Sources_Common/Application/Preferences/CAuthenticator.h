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


// CAuthenticator.h

// Class that holds authentication mechanism

#ifndef __CAUTHENTICATOR__MULBERRY__
#define __CAUTHENTICATOR__MULBERRY__

#include "CFutureItems.h"
#include "cdstring.h"

class CAuthPlugin;

class CAuthenticator
{
public:
	enum EAuthenticators
	{
		eNone = 0,
		ePlainText,
		eSSL,
		ePlugin
	};

			CAuthenticator(bool letter = false);
			CAuthenticator(const CAuthenticator& copy);
	virtual	~CAuthenticator();

			CAuthenticator& operator=(const CAuthenticator& copy);				// Assignment with same type

	int operator==(const CAuthenticator& comp) const;							// Compare with same type

	virtual EAuthenticators GetAuthenticatorType(void) const
		{ return mType; }
	virtual void SetAuthenticatorType(EAuthenticators type);
	virtual void ResetAuthenticatorType(EAuthenticators type);

	virtual cdstring GetSASLID() const;
	virtual const cdstring& GetDescriptor() const
		{ return mDescriptor; }
	virtual void SetDescriptor(const cdstring& desc);

	virtual CAuthenticator* GetAuthenticator(void) const
		{ return mAuthenticator; }
	virtual CAuthPlugin* GetPlugin() const;

	virtual bool RequiresUserPswd(void) const
		{ return mAuthenticator->RequiresUserPswd(); }
	virtual bool RequiresKerberos(void) const
		{ return mAuthenticator->RequiresKerberos(); }

	virtual cdstring GetUniqueness(void) const									// Get string that defines 'uniqueness' of authenticator
		{ return mAuthenticator->GetUniqueness(); }

	virtual const cdstring& GetActualUID() const
		{ return cdstring::null_str; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	EAuthenticators	mType;
	cdstring		mDescriptor;
	CAuthenticator* mAuthenticator;
};

class CAuthenticatorNone : public CAuthenticator
{
public:
			CAuthenticatorNone()
				: CAuthenticator(true) {}
			CAuthenticatorNone(const CAuthenticatorNone& copy)
				: CAuthenticator(copy), mFuture(copy.mFuture) { }
	virtual	~CAuthenticatorNone() {}

	int operator==(const CAuthenticatorNone& comp) const				// Compare with same type
		{ return true; }

	virtual bool RequiresUserPswd(void) const
		{ return false; }
	virtual bool RequiresKerberos(void) const
		{ return false; }

	virtual cdstring GetUniqueness(void) const									// Get string that defines 'uniqueness' of authenticator
		{ return cdstring::null_str; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	CFutureItems	mFuture;
};

class CAuthenticatorUserPswd : public CAuthenticator
{
public:
			CAuthenticatorUserPswd()
				: CAuthenticator(true)
				{ mSaveUID = false; mSavePswd = false; }
			CAuthenticatorUserPswd(const CAuthenticatorUserPswd& copy)
				: CAuthenticator(copy), mFuture(copy.mFuture)
				{ mUID = copy.mUID; mPswd = copy.mPswd; mSaveUID = copy.mSaveUID; mSavePswd = copy.mSavePswd; }
	virtual	~CAuthenticatorUserPswd() {}

	int operator==(const CAuthenticatorUserPswd& comp) const				// Compare with same type
		{ return (mUID == comp.mUID) &&
					(mSaveUID == comp.mSaveUID) && (mSavePswd == comp.mSavePswd); }

	virtual bool RequiresUserPswd(void) const
		{ return true; }
	virtual bool RequiresKerberos(void) const
		{ return false; }

	virtual const cdstring& GetUID(void) const
		{ return mUID; }
	virtual void SetUID(const cdstring& uid)
		{ if (mUID != uid) {mUID = uid; mPswd = cdstring::null_str; }}		// Reset password if UID changes

	virtual const cdstring& GetPswd(void) const
		{ return mPswd; }
	virtual void SetPswd(const cdstring& pswd)
		{ mPswd = pswd; }
	
	virtual bool GetSaveUID(void) const
		{ return mSaveUID; }
	virtual void SetSaveUID(bool save_uid)
		{ mSaveUID = save_uid; }

	virtual bool GetSavePswd(void) const
		{ return mSavePswd; }
	virtual void SetSavePswd(bool save_pswd)
		{ mSavePswd = save_pswd; }

	virtual cdstring GetUniqueness(void) const									// Get string that defines 'uniqueness' of authenticator
		{ return mUID; }

	virtual const cdstring& GetActualUID() const
		{ return mUID; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	cdstring 		mUID;
	cdstring 		mPswd;
	bool 			mSaveUID;
	bool 			mSavePswd;
	CFutureItems	mFuture;			// Future preference items
};

class CAuthenticatorKerberos : public CAuthenticator
{
public:
			CAuthenticatorKerberos()
				: CAuthenticator(true) { mDefaultPrincipal = true; mDefaultUID = true; }
			CAuthenticatorKerberos(const CAuthenticatorKerberos& copy)
				: CAuthenticator(copy), mFuture(copy.mFuture)
				{ mDefaultPrincipal = copy.mDefaultPrincipal; mServerPrincipal = copy.mServerPrincipal;
					mDefaultUID = copy.mDefaultUID; mUID = copy.mUID; }
	virtual	~CAuthenticatorKerberos() {}

	int operator==(const CAuthenticatorKerberos& comp) const						// Compare with same type
		{ return (mDefaultPrincipal == comp.mDefaultPrincipal) && (mServerPrincipal == comp.mServerPrincipal) &&
					(mDefaultUID == comp.mDefaultUID) && (mUID == comp.mUID); }

	virtual bool RequiresUserPswd(void) const
		{ return false; }
	virtual bool RequiresKerberos(void) const
		{ return true; }

	virtual bool GetDefaultPrincipal(void) const
		{ return mDefaultPrincipal; }
	virtual void SetUsePrincipal(bool use_default)
		{ mDefaultPrincipal = use_default; }

	virtual const cdstring& GetServerPrincipal(void) const
		{ return mServerPrincipal; }
	virtual void SetServerPrincipal(const cdstring& principal)
		{ mServerPrincipal = principal; }

	virtual bool GetDefaultUID(void) const
		{ return mDefaultUID; }
	virtual void SetUseUID(bool use_default)
		{ mDefaultUID = use_default; }

	virtual const cdstring& GetUID(void) const
		{ return mUID; }
	virtual void SetUID(const cdstring& uid)
		{ mUID = uid; }

	virtual const cdstring& GetRecoveredUID() const
		{ return mRecoveredUID; }
	virtual void SetRecoveredUID(const cdstring& uid)
		{ mRecoveredUID = uid; }

	virtual cdstring GetUniqueness(void) const									// Get string that defines 'uniqueness' of authenticator
		{ return mServerPrincipal + (mDefaultPrincipal ? "1" : "0") + mUID + (mDefaultUID ? "1" : "0"); }

	virtual const cdstring& GetActualUID() const
		{ return mRecoveredUID; }

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	bool			mDefaultPrincipal;		// Use default principal?
	cdstring		mServerPrincipal;		// Kerberos Principal
	bool			mDefaultUID;			// Use default user id?
	cdstring		mUID;					// Server user ID
	cdstring		mRecoveredUID;			// UID returned by plugin
	CFutureItems	mFuture;				// Future preference items
};

#endif
