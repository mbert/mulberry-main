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

// CRegistration.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 20-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements the app & plug-in registration scheme.
//
// History:
// 20-Nov-1997: Created initial header and implementation.
//

#ifndef __CREGISTRATION__MULBERRY__
#define __CREGISTRATION__MULBERRY__

#include "cdstring.h"

#include <time.h>

class CPlugin;

class CRegistration
{
public:
	static CRegistration sRegistration;

	CRegistration();
	~CRegistration() {}

	// Set parameters
	void SetLicensee(const cdstring& licensee);
	void SetOrganisation(const cdstring& organisation);
	void SetSerialNumber(const cdstring& serialNumber);
	void SetAppUserCode(const cdstring& user_code)
		{ mAppUserCode = user_code; }
	void SetAppDemoTimeout(time_t timeout)
		{ mAppTimeout = timeout; }

	// Getters
	const cdstring& GetLicensee() const
		{ return mLicensee; }
	const cdstring& GetOrganisation() const
		{ return mOrganisation; }
	const cdstring& GetSerialNumber() const
		{ return mSerialNumber; }
	time_t GetAppDemoTimeout() const
		{ return mAppTimeout; }

	// Get registration resource info
	void LoadAppInfo();
	void ClearAppInfo();
	bool WriteAppInfo();
	bool WriteAppDemoInfo();

	bool ValidInfoCurrent(const cdstring& serial, const cdstring& regkey) const;
	bool ValidInfoOld(const cdstring& serial, const cdstring& regkey) const;
	bool ValidSerial(const cdstring& serial) const;			// Validate registration
	bool LegalSerial(const cdstring& serial) const;			// Check for legal serial number

	void ClearPluginInfo() const;
	bool WritePluginInfo(bool demo);

	bool CheckAppRegistration(bool current) const;
	bool CheckCryptoRegistration() const;

	bool AppDemoExpired() const;
	bool CryptoDemoExpired(void) const;
	bool CryptoDemoValid(void) const;

	// Do registration dialog
	bool DoAppRegister(bool& demo);
	bool DoPluginRegister(const CPlugin& plugin, bool& demo, bool& delete_it, bool allow_demo, bool allow_delete);

private:
	cdstring mLicensee;
	cdstring mOrganisation;
	cdstring mSerialNumber;
	time_t	 mAppTimeout;
	time_t	 mCryptoTimeout;
	cdstring mAppUserCode;
	cdstring mCryptoUserCode;
	cdstring mAppRegistrationKey;					// Key for v3 application registration
	cdstring mCryptoRegistrationKey;				// Key for v3 crypto plugin registration

	unsigned long GetFullCRC(unsigned long key) const;
	unsigned long CRCCalculate(const cdstring& str, unsigned long key) const;	// Calculate CRC code from text
	
	cdstring GetFullCode(const cdstring& key) const;
	cdstring DigestCode(const cdstring& str, const cdstring& key) const;
	
	const char* GetIllegalCopyStr() const;
	
	unsigned long GetCRCKeyV1() const;
	unsigned long GetCRCKeyV2() const;
	const char* GetKeyV3() const;
	unsigned long GetCryptoKeyV3() const;
	const char* GetKeyV4() const;

	bool ValidInfoV3(const cdstring& serial, const cdstring& regkey) const;
	bool ValidInfoV1_2(const cdstring& serial, const cdstring& regkey) const;

	bool	ReadAppResource();
	void	ReadAppRegistration();
	void	WriteAppRegistration() const;
	void	ClearAppRegistration() const;

	void	WritePluginRegistration() const;

	void	ReadRegistrationMap(cdstrmap& kvmap) const;
	void	WriteRegistrationMap(const cdstrmap& kvmap) const;
};

#endif
